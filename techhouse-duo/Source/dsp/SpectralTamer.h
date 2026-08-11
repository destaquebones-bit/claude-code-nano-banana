#pragma once
#include "DspCommon.h"

// Dynamic resonance tamer over an arbitrary, re-tunable band grid.
//
// The measurement idea: a natural bass/kick tone rolls off smoothly from band
// to band, which in dB is close to a straight line. So instead of comparing a
// band against a fixed threshold (what a plain dynamic EQ does), we compare it
// against the average of its two neighbours. A band that sits on the smooth
// rolloff scores zero excess no matter how loud the source is; a band that
// pokes out above its neighbours is a resonance, and only that gets cut. This
// is what makes it level-independent and program-independent.
//
// The grid is settable per block, which is what makes the same engine serve
// both modes:
//   - Bass mode passes k*f0 (k = 1..N) from the pitch tracker, so the cuts
//     track the note as the bassline moves -- a resonance is identified as
//     "the 4th harmonic of whatever is playing", not "250 Hz".
//   - Kick mode passes a fixed log-spaced grid, since a kick is inharmonic and
//     has no fundamental to track.
class SpectralTamer
{
public:
    static constexpr int maxBands = 10;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (int b = 0; b < maxBands; ++b)
        {
            measure[b].prepare (spec);
            cut[b].prepare (spec);
            envelope[b].prepare (spec.sampleRate);
            envelope[b].setTimes (3.0f, 60.0f);
            smoothing[b].prepare (spec.sampleRate);
            smoothing[b].setTimes (5.0f, 90.0f);
            levelDb[b] = -120.0f;
            reductionDb[b] = 0.0f;
            centres[b] = 0.0f;
        }
        numBands = 0;
    }

    void reset()
    {
        for (int b = 0; b < maxBands; ++b)
        {
            measure[b].reset();
            cut[b].reset();
            envelope[b].reset();
            smoothing[b].reset();
        }
    }

    struct Params
    {
        float depth = 0.6f;        // 0-1, how much of the detected excess to remove
        float toleranceDb = 3.0f;  // excess below this is considered musical, left alone
        float maxCutDb = 12.0f;
        float attackMs = 5.0f;
        float releaseMs = 90.0f;
        float protectFirstBand = 1.0f; // 1 = never cut band 0 (the fundamental)
    };

    void setParams (const Params& p)
    {
        params = p;
        for (int b = 0; b < maxBands; ++b)
            smoothing[b].setTimes (p.attackMs, p.releaseMs);
    }

    // Re-tunes the grid. Bands above Nyquist (or above `maxFreq`) are dropped,
    // so a high fundamental simply results in fewer active bands rather than
    // filters being designed at invalid frequencies.
    void setBandCentres (const float* freqs, int count, float maxFreq = 900.0f)
    {
        if (sampleRate <= 0.0)
            return;

        const float nyquistLimit = (float) (sampleRate * 0.45);
        int active = 0;

        for (int i = 0; i < juce::jmin (count, (int) maxBands); ++i)
        {
            const float f = freqs[i];
            if (f < 20.0f || f > juce::jmin (maxFreq, nyquistLimit))
                continue;

            // Q rises with band index: on a harmonic grid the bands get
            // proportionally closer together as k grows (spacing/centre = 1/k),
            // so the filters must narrow to keep isolating one harmonic.
            const float q = juce::jlimit (1.0f, 14.0f, 1.4f * (float) (active + 1));

            centres[active] = f;
            measure[active].setCoefficients (
                juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, f, q));
            currentQ[active] = q;
            ++active;
        }

        numBands = active;
    }

    // Call once per block after setBandCentres: turns the measured levels into
    // per-band cut amounts and re-designs the cut filters. `samplesElapsed` is
    // the block length, needed so the attack/release times stay correct
    // regardless of the host's buffer size.
    void updateReductions (float confidence, int samplesElapsed)
    {
        if (sampleRate <= 0.0 || numBands == 0)
            return;

        for (int b = 0; b < numBands; ++b)
        {
            const float neighbourRef = neighbourAverageDb (b);
            float excess = levelDb[b] - neighbourRef - params.toleranceDb;

            if (b == 0 && params.protectFirstBand > 0.5f)
                excess = 0.0f; // never thin out the fundamental itself

            const float target = juce::jlimit (0.0f, params.maxCutDb,
                                                juce::jmax (0.0f, excess) * params.depth);

            // Confidence scales the whole action: when the pitch tracker isn't
            // sure, the grid may be wrong, so the note-aware cuts fade out
            // rather than cutting at frequencies that mean nothing.
            reductionDb[b] = smoothing[b].processOverSamples (target, samplesElapsed)
                                * juce::jlimit (0.0f, 1.0f, confidence);

            cut[b].setCoefficients (juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                sampleRate, centres[b], juce::jmax (0.7f, currentQ[b] * 0.85f),
                DspCommon::dbToGain (-reductionDb[b])));
        }
    }

    // Per-sample: feed the detector (channel 0 only is enough -- bass content is
    // near-mono and it keeps both channels cut identically, which preserves the
    // stereo image) and apply the cuts.
    void processDetector (float monoSample)
    {
        for (int b = 0; b < numBands; ++b)
        {
            const float banded = measure[b].processSample (0, monoSample);
            const float env = envelope[b].process (std::abs (banded));
            levelDb[b] = DspCommon::gainToDb (env);
        }
    }

    float processSample (int channel, float x)
    {
        for (int b = 0; b < numBands; ++b)
            x = cut[b].processSample (channel, x);
        return x;
    }

    int getNumBands() const { return numBands; }
    float getBandCentre (int b) const { return centres[juce::jlimit (0, maxBands - 1, b)]; }
    float getBandReductionDb (int b) const { return reductionDb[juce::jlimit (0, maxBands - 1, b)]; }
    float getBandLevelDb (int b) const { return levelDb[juce::jlimit (0, maxBands - 1, b)]; }

    float getMaxReductionDb() const
    {
        float m = 0.0f;
        for (int b = 0; b < numBands; ++b)
            m = juce::jmax (m, reductionDb[b]);
        return m;
    }

    int getWorstBandIndex() const
    {
        int worst = -1;
        float m = 0.25f; // ignore trivial amounts so the UI doesn't flicker
        for (int b = 0; b < numBands; ++b)
        {
            if (reductionDb[b] > m)
            {
                m = reductionDb[b];
                worst = b;
            }
        }
        return worst;
    }

private:
    // Local trend estimate. Edge bands only have one neighbour, so they borrow
    // it plus a mild assumed rolloff rather than being compared against nothing.
    float neighbourAverageDb (int b) const
    {
        constexpr float assumedRolloffDb = 3.0f;
        if (numBands == 1)
            return levelDb[0];
        if (b == 0)
            return levelDb[1] + assumedRolloffDb;
        if (b == numBands - 1)
            return levelDb[b - 1] - assumedRolloffDb;
        return 0.5f * (levelDb[b - 1] + levelDb[b + 1]);
    }

    double sampleRate = 44100.0;
    int numBands = 0;
    Params params;

    std::array<DspCommon::StereoIIR, maxBands> measure, cut;
    std::array<DspCommon::OnePoleEnvelope, maxBands> envelope, smoothing;
    std::array<float, maxBands> centres {}, currentQ {}, levelDb {}, reductionDb {};
};
