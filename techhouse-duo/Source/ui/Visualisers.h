#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AnalogLookAndFeel.h"
#include "../dsp/LinkBus.h"
#include "../dsp/DspCommon.h"

// The three views that make the note-aware processing visible. All of the data
// they draw is already computed by the DSP to do its job -- before these
// existed the plugin knew which harmonic it was cutting and which notes ran
// hot, and reported both as a single number.

// Log-frequency display of the tracked note's harmonic grid, each band's level,
// and the cut currently applied to it. This is the one view that shows what
// makes the plugin different: the bars are not fixed frequencies, they move
// with the bassline.
struct SpectrumView : public juce::Component
{
    static constexpr int maxBands = 10;
    static constexpr float minHz = 25.0f;
    static constexpr float maxHz = 1200.0f;

    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        juce::ColourGradient bg (Palette::panelDeep, 0.0f, bounds.getY(),
                                  Palette::panelDeep.darker (0.5f), 0.0f, bounds.getBottom(), false);
        g.setGradientFill (bg);
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (Palette::sectionEdge);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 6.0f, 1.0f);

        auto plot = bounds.reduced (10.0f, 8.0f);
        plot.removeFromBottom (14.0f); // frequency ruler
        const float bottom = plot.getBottom();

        drawFrequencyRuler (g, plot, bounds);

        // Reference line at the mud region the genre notes call out: bass
        // harmonics stacking between roughly 200 and 400 Hz is where "abafado"
        // actually lives, so it is worth marking permanently.
        {
            const float x1 = xForHz (200.0f, plot);
            const float x2 = xForHz (400.0f, plot);
            g.setColour (Palette::cut.withAlpha (0.06f));
            g.fillRect (x1, plot.getY(), x2 - x1, plot.getHeight());
        }

        if (numBands <= 0)
        {
            g.setColour (Palette::textDim);
            g.setFont (12.0f);
            g.drawFittedText ("no signal", plot.toNearestInt(), juce::Justification::centred, 1);
            return;
        }

        const float barWidth = juce::jmax (6.0f, plot.getWidth() / 40.0f);

        for (int b = 0; b < numBands; ++b)
        {
            const float hz = bandFreq[b];
            if (hz <= 0.0f)
                continue;

            const float x = xForHz (hz, plot);
            const float level = juce::jlimit (0.0f, 1.0f, (bandLevelDb[b] + 72.0f) / 72.0f);
            const float barTop = bottom - level * plot.getHeight();

            auto bar = juce::Rectangle<float> (x - barWidth * 0.5f, barTop, barWidth, bottom - barTop);

            // Level bar, brighter for the fundamental so the eye finds the note.
            const auto barColour = (b == 0 && noteAware) ? Palette::amber : Palette::amber.withAlpha (0.55f);
            g.setColour (barColour.withAlpha (noteAware ? barColour.getFloatAlpha() : 0.3f));
            g.fillRoundedRectangle (bar, 2.0f);

            // The cut, drawn as the slice removed from the top of the bar --
            // so a cut is visibly taken *out of* the energy that was there.
            if (bandCutDb[b] > 0.05f)
            {
                const float cutFraction = juce::jlimit (0.0f, 1.0f, bandCutDb[b] / 18.0f);
                const float cutHeight = juce::jmin (bar.getHeight(), cutFraction * plot.getHeight());
                g.setColour (Palette::cut.withAlpha (0.85f));
                g.fillRoundedRectangle (bar.withHeight (cutHeight), 2.0f);

                g.setColour (Palette::cut);
                g.setFont (juce::Font (9.0f, juce::Font::bold));
                g.drawFittedText (juce::String (bandCutDb[b], 1),
                                   (int) (x - 16.0f), (int) (barTop - 12.0f), 32, 11,
                                   juce::Justification::centred, 1);
            }

            // Harmonic index, so "the 4th harmonic is resonant" is readable
            // straight off the picture.
            g.setColour (noteAware ? Palette::textDim : Palette::textDim.withAlpha (0.4f));
            g.setFont (9.0f);
            g.drawFittedText (noteAware ? juce::String (b + 1) : juce::String(),
                               (int) (x - 10.0f), (int) (bottom + 1.0f), 20, 11,
                               juce::Justification::centred, 1);
        }

        // Note readout, top-left.
        g.setFont (juce::Font (13.0f, juce::Font::bold));
        if (noteAware)
        {
            g.setColour (Palette::amber);
            g.drawFittedText (noteName + juce::String::formatted ("  %.1f Hz", fundamentalHz),
                               (int) plot.getX(), (int) plot.getY(), 180, 15, juce::Justification::topLeft, 1);
        }
        else
        {
            g.setColour (Palette::textDim);
            g.drawFittedText ("tracking...", (int) plot.getX(), (int) plot.getY(), 180, 15,
                               juce::Justification::topLeft, 1);
        }

        g.setFont (9.0f);
        g.setColour (Palette::textDim);
        g.drawFittedText (noteAware ? "harmonic grid follows the note" : "fixed grid (low confidence)",
                           (int) (plot.getRight() - 200.0f), (int) plot.getY(), 200, 13,
                           juce::Justification::topRight, 1);
    }

    static float xForHz (float hz, juce::Rectangle<float> plot)
    {
        const float t = std::log (juce::jlimit (minHz, maxHz, hz) / minHz) / std::log (maxHz / minHz);
        return plot.getX() + t * plot.getWidth();
    }

    void drawFrequencyRuler (juce::Graphics& g, juce::Rectangle<float> plot, juce::Rectangle<float>) const
    {
        const float marks[] = { 30.0f, 50.0f, 100.0f, 200.0f, 400.0f, 800.0f };
        g.setFont (9.0f);
        for (float hz : marks)
        {
            const float x = xForHz (hz, plot);
            g.setColour (Palette::grid);
            g.drawVerticalLine ((int) x, plot.getY(), plot.getBottom());
            g.setColour (Palette::textDim.withAlpha (0.6f));
            g.drawFittedText (hz >= 1000.0f ? juce::String (hz / 1000.0f, 1) + "k" : juce::String ((int) hz),
                               (int) (x - 16.0f), (int) (plot.getBottom() + 12.0f), 32, 11,
                               juce::Justification::centred, 1);
        }
    }

    int numBands = 0;
    bool noteAware = false;
    float fundamentalHz = 0.0f;
    juce::String noteName = "--";
    float bandFreq[maxBands] {};
    float bandLevelDb[maxBands] {};
    float bandCutDb[maxBands] {};
};

// Per-band kick ducking. Full-band sidechain gives you one number; this shows
// that only the bands the kick occupies are being pulled down, which is the
// whole argument for doing it spectrally.
struct DuckMeter : public juce::Component
{
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (Palette::panelDeep);
        g.fillRoundedRectangle (bounds, 5.0f);
        g.setColour (Palette::sectionEdge);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 5.0f, 1.0f);

        auto area = bounds.reduced (8.0f, 6.0f);
        auto labelRow = area.removeFromBottom (12.0f);

        const float slot = area.getWidth() / (float) Link::numBands;

        for (int b = 0; b < Link::numBands; ++b)
        {
            auto cell = juce::Rectangle<float> (area.getX() + b * slot, area.getY(), slot, area.getHeight())
                            .reduced (slot * 0.18f, 0.0f);

            g.setColour (Palette::grid);
            g.fillRoundedRectangle (cell, 2.0f);

            // Kick energy in this band, dim, as the reason for the duck.
            const float kickLevel = juce::jlimit (0.0f, 1.0f, (kickBandDb[b] + 70.0f) / 70.0f);
            if (kickLevel > 0.01f)
            {
                g.setColour (Palette::textDim.withAlpha (0.25f));
                g.fillRoundedRectangle (cell.withTop (cell.getBottom() - kickLevel * cell.getHeight()), 2.0f);
            }

            // The duck itself, drawn downward from the top: the gap is the
            // space being made for the kick.
            const float duck = juce::jlimit (0.0f, 1.0f, duckDb[b] / 12.0f);
            if (duck > 0.005f)
            {
                g.setColour (Palette::cut.withAlpha (0.9f));
                g.fillRoundedRectangle (cell.withHeight (duck * cell.getHeight()), 2.0f);
            }

            g.setColour (Palette::textDim.withAlpha (0.7f));
            g.setFont (8.0f);
            g.drawFittedText (juce::String ((int) Link::bandCentres[(size_t) b]),
                               (int) (area.getX() + b * slot), (int) labelRow.getY(),
                               (int) slot, 11, juce::Justification::centred, 1);
        }
    }

    float duckDb[Link::numBands] {};
    float kickBandDb[Link::numBands] {};
};

// The note map: how loud each note of the bassline runs relative to the
// average of all of them. A tall spread here *is* the one-note-bass problem,
// stated as a picture instead of a single dB figure.
struct NoteMap : public juce::Component
{
    void paint (juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour (Palette::panelDeep);
        g.fillRoundedRectangle (bounds, 5.0f);
        g.setColour (Palette::sectionEdge);
        g.drawRoundedRectangle (bounds.reduced (0.5f), 5.0f, 1.0f);

        auto area = bounds.reduced (8.0f, 6.0f);
        auto labelRow = area.removeFromBottom (12.0f);
        const float midY = area.getCentreY();

        g.setColour (Palette::grid);
        g.drawHorizontalLine ((int) midY, area.getX(), area.getRight());

        if (noteCount == 0)
        {
            g.setColour (Palette::textDim);
            g.setFont (11.0f);
            g.drawFittedText ("learning - play a few bars of the bassline",
                               area.toNearestInt(), juce::Justification::centred, 1);
            return;
        }

        const float slot = area.getWidth() / (float) juce::jmax (1, noteCount);
        constexpr float fullScaleDb = 8.0f;

        for (int i = 0; i < noteCount; ++i)
        {
            const float deviation = juce::jlimit (-fullScaleDb, fullScaleDb, notes[i].deviationDb);
            const float halfHeight = area.getHeight() * 0.5f;
            const float barHeight = std::abs (deviation) / fullScaleDb * halfHeight;

            auto cell = juce::Rectangle<float> (area.getX() + i * slot, 0.0f, slot, 0.0f)
                            .reduced (juce::jmin (4.0f, slot * 0.2f), 0.0f);

            auto bar = deviation >= 0.0f
                ? juce::Rectangle<float> (cell.getX(), midY - barHeight, cell.getWidth(), barHeight)
                : juce::Rectangle<float> (cell.getX(), midY, cell.getWidth(), barHeight);

            const bool isCurrent = notes[i].midiNote == currentNote;
            const bool offender = std::abs (deviation) > 3.0f;

            g.setColour (offender ? Palette::cut.withAlpha (isCurrent ? 1.0f : 0.7f)
                                   : Palette::good.withAlpha (isCurrent ? 1.0f : 0.55f));
            g.fillRoundedRectangle (bar, 2.0f);

            if (isCurrent)
            {
                g.setColour (Palette::amber);
                g.drawRoundedRectangle (cell.withY (area.getY()).withHeight (area.getHeight()).reduced (0.5f), 2.0f, 1.0f);
            }

            g.setColour (isCurrent ? Palette::amber : Palette::textDim.withAlpha (0.7f));
            g.setFont (8.0f);
            g.drawFittedText (notes[i].name, (int) (area.getX() + i * slot), (int) labelRow.getY(),
                               (int) slot, 11, juce::Justification::centred, 1);
        }
    }

    struct Entry
    {
        int midiNote = -1;
        float deviationDb = 0.0f;
        juce::String name;
    };

    static constexpr int maxEntries = 24;
    Entry notes[maxEntries];
    int noteCount = 0;
    int currentNote = -1;
};
