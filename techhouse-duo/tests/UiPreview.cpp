// Renders the visualisers offscreen with data produced by actually running the
// DSP, and writes a PNG.
//
// This exists because the standalone app mutes its audio input, so the only way
// to see the spectrum, duck meter and note map with real content -- rather than
// their empty states -- is to drive them from the same modules the plugin uses
// and render them headlessly. It doubles as a check that the paint routines
// survive real value ranges (negative dB, empty bands, out-of-scale deviations).
//
//   cmake --build build-tests --target UiPreview
//   ./build-tests/UiPreview_artefacts/Release/UiPreview out.png

#include "../Source/ui/Visualisers.h"
#include "../Source/ui/AnalogLookAndFeel.h"
#include "../Source/dsp/SpectralTamer.h"
#include "../Source/dsp/KickDucker.h"
#include "../Source/dsp/NoteLevelCompensator.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <cmath>
#include <vector>

namespace
{
    constexpr double sr = 48000.0;

    std::vector<float> makeBassNote (float hz, float seconds, float extraHarmonic, int extraIndex)
    {
        const int n = (int) (seconds * sr);
        std::vector<float> out ((size_t) n, 0.0f);
        for (int i = 0; i < n; ++i)
        {
            const double t = i / sr;
            double v = 0.0;
            for (int k = 1; k <= 8; ++k)
            {
                double amp = 1.0 / k;
                if (k == extraIndex)
                    amp *= (1.0 + extraHarmonic);
                v += amp * std::sin (2.0 * juce::MathConstants<double>::pi * hz * k * t);
            }
            out[(size_t) i] = (float) (0.25 * v);
        }
        return out;
    }

    // A short, decaying kick: enough for the ducker's band detectors to settle
    // on a realistic spectral shape.
    std::vector<float> makeKick (float seconds)
    {
        const int n = (int) (seconds * sr);
        std::vector<float> out ((size_t) n, 0.0f);
        for (int i = 0; i < n; ++i)
        {
            const double t = i / sr;
            const double phase = 2.0 * juce::MathConstants<double>::pi
                                  * (55.0 * t + 40.0 * (1.0 - std::exp (-t * 25.0)) / 25.0);
            const double env = std::exp (-t * 14.0);
            out[(size_t) i] = (float) (0.9 * env * std::sin (phase));
        }
        return out;
    }

    void fillSpectrum (SpectrumView& view)
    {
        juce::dsp::ProcessSpec spec { sr, 512, 2 };
        SpectralTamer tamer;
        tamer.prepare (spec);

        SpectralTamer::Params p;
        p.depth = 0.8f;
        p.toleranceDb = 3.0f;
        p.maxCutDb = 14.0f;
        tamer.setParams (p);

        const float f0 = 55.0f;
        float grid[8];
        for (int k = 0; k < 8; ++k)
            grid[k] = f0 * (float) (k + 1);
        tamer.setBandCentres (grid, 8);

        auto signal = makeBassNote (f0, 2.0f, 6.0f, 4);
        for (int start = 0; start < (int) signal.size(); start += 512)
        {
            const int count = juce::jmin (512, (int) signal.size() - start);
            for (int i = 0; i < count; ++i)
            {
                tamer.processDetector (signal[(size_t) (start + i)]);
                tamer.processSample (0, signal[(size_t) (start + i)]);
            }
            tamer.updateReductions (1.0f, count);
        }

        view.numBands = tamer.getNumBands();
        view.noteAware = true;
        view.fundamentalHz = f0;
        view.noteName = DspCommon::midiNoteName (DspCommon::freqToMidiNote (f0));
        for (int b = 0; b < view.numBands; ++b)
        {
            view.bandFreq[b] = tamer.getBandCentre (b);
            view.bandLevelDb[b] = tamer.getBandLevelDb (b);
            view.bandCutDb[b] = tamer.getBandReductionDb (b);
        }

        std::printf ("spectrum: %d bands, worst = harmonic %d, cut %.1f dB\n",
                      view.numBands, tamer.getWorstBandIndex() + 1, tamer.getMaxReductionDb());
    }

    void fillDuckMeter (DuckMeter& meter)
    {
        juce::dsp::ProcessSpec spec { sr, 512, 2 };
        KickDucker ducker;
        ducker.prepare (spec);

        KickDucker::Params p;
        p.amount = 0.7f;
        p.thresholdDb = -40.0f;
        p.maxDuckDb = 12.0f;
        ducker.setParams (p);

        auto kick = makeKick (0.12f);
        for (int start = 0; start < (int) kick.size(); start += 128)
        {
            const int count = juce::jmin (128, (int) kick.size() - start);
            for (int i = 0; i < count; ++i)
                ducker.processSidechainSample (kick[(size_t) (start + i)]);
            ducker.updateDucking (count);
        }

        for (int b = 0; b < Link::numBands; ++b)
        {
            meter.duckDb[b] = ducker.getBandDuckDb (b);
            meter.kickBandDb[b] = ducker.getBandSidechainDb (b);
        }

        std::printf ("duck meter: max duck %.1f dB\n", ducker.getMaxDuckDb());
    }

    void fillNoteMap (NoteMap& map)
    {
        NoteLevelCompensator comp;
        comp.prepare (sr);
        NoteLevelCompensator::Params p;
        p.amount = 1.0f;
        comp.setParams (p);

        // A bassline where one note sits clearly hot -- the case the note map
        // is meant to make obvious at a glance.
        struct Step { float hz; float amp; };
        const Step steps[] = {
            { 41.2f, 0.13f }, { 49.0f, 0.12f }, { 55.0f, 0.22f },
            { 61.7f, 0.11f }, { 65.4f, 0.14f }, { 73.4f, 0.10f },
        };

        for (int repeat = 0; repeat < 8; ++repeat)
            for (const auto& step : steps)
                for (int i = 0; i < (int) (0.3 * sr); ++i)
                    comp.processSample (step.amp * (float) std::sin (2.0 * juce::MathConstants<double>::pi * step.hz * i / sr),
                                         step.hz, 0.9f);

        const float average = comp.getAverageLevelDb();
        int count = 0;
        for (int n = 0; n < 128 && count < NoteMap::maxEntries; ++n)
        {
            if (comp.getNoteObservations (n) < 3)
                continue;
            map.notes[count].midiNote = n;
            map.notes[count].deviationDb = comp.getNoteLevelDb (n) - average;
            map.notes[count].name = DspCommon::midiNoteName (n);
            ++count;
        }
        map.noteCount = count;
        map.currentNote = DspCommon::freqToMidiNote (55.0f);

        std::printf ("note map: %d notes, spread %.1f dB\n", count, comp.getNoteSpreadDb());
    }

    void renderInto (juce::Graphics& g, juce::Component& component, juce::Rectangle<int> area)
    {
        component.setBounds (area);
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (area);
        g.setOrigin (area.getPosition());
        component.paintEntireComponent (g, false);
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    const juce::String outPath = argc > 1 ? juce::String (argv[1]) : juce::String ("ui-preview.png");

    AnalogLookAndFeel lnf;
    juce::LookAndFeel::setDefaultLookAndFeel (&lnf);

    SpectrumView spectrum;
    DuckMeter duckMeter;
    NoteMap noteMap;

    fillSpectrum (spectrum);
    fillDuckMeter (duckMeter);
    fillNoteMap (noteMap);

    constexpr int width = 700;
    constexpr int pad = 12;
    constexpr int spectrumH = 170, duckH = 70, noteH = 84, captionH = 18;
    const int height = pad + captionH + spectrumH + pad + captionH + duckH + pad + captionH + noteH + pad;

    juce::Image image (juce::Image::ARGB, width, height, true);
    juce::Graphics g (image);

    juce::ColourGradient bg (Palette::panel, 0.0f, 0.0f, Palette::panelDeep, 0.0f, (float) height, false);
    g.setGradientFill (bg);
    g.fillAll();

    int y = pad;
    auto caption = [&] (const juce::String& text)
    {
        g.setColour (Palette::amber);
        g.setFont (juce::Font (11.0f, juce::Font::bold));
        g.drawFittedText (text, pad, y, width - 2 * pad, captionH, juce::Justification::centredLeft, 1);
        y += captionH;
    };

    caption ("HARMONIC SPECTRUM  -  grid follows the tracked note, cuts drawn in red");
    renderInto (g, spectrum, { pad, y, width - 2 * pad, spectrumH });
    y += spectrumH + pad;

    caption ("KICK DUCKING  -  per band, only where the kick actually sits");
    renderInto (g, duckMeter, { pad, y, width - 2 * pad, duckH });
    y += duckH + pad;

    caption ("NOTE MAP  -  level of each note relative to the bassline average");
    renderInto (g, noteMap, { pad, y, width - 2 * pad, noteH });

    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);

    juce::File outFile (juce::File::getCurrentWorkingDirectory().getChildFile (outPath));
    outFile.deleteFile();
    juce::FileOutputStream stream (outFile);
    if (! stream.openedOk())
    {
        std::printf ("could not open %s for writing\n", outFile.getFullPathName().toRawUTF8());
        return 1;
    }

    juce::PNGImageFormat png;
    if (! png.writeImageToStream (image, stream))
    {
        std::printf ("failed to encode PNG\n");
        return 1;
    }

    std::printf ("wrote %s (%dx%d)\n", outFile.getFullPathName().toRawUTF8(), width, height);
    return 0;
}
