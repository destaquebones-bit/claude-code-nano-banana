#pragma once
#include "DspCommon.h"
#include <vector>

// Monophonic pitch tracker for basslines, using the YIN difference function.
//
// Tech house basslines are monophonic and rolling, which is exactly the case
// YIN handles well -- and it's what makes note-aware processing possible at
// all. Everything downstream (the harmonic grid, per-note levelling, the
// exciter) depends on this being right, so it errs toward reporting low
// confidence instead of a confident wrong answer.
//
// It runs on a heavily decimated copy of the signal: a bass fundamental lives
// between roughly 35 and 250 Hz, so ~4 kHz is plenty of sample rate, and
// decimating first makes the O(window x lags) search cheap enough to run in a
// plugin. The signal is low-passed at 400 Hz before decimation both to
// anti-alias and because harmonics above that only confuse the search.
class PitchTracker
{
public:
    static constexpr float minHz = 35.0f;
    static constexpr float maxHz = 250.0f;
    static constexpr int targetDecimatedRate = 4000;

    void prepare (double hostSampleRate)
    {
        sampleRate = hostSampleRate;
        decimationFactor = juce::jmax (1, (int) std::floor (hostSampleRate / targetDecimatedRate));
        decimatedRate = hostSampleRate / decimationFactor;

        juce::dsp::ProcessSpec preSpec { hostSampleRate, 512, 1 };
        for (auto* f : { &antiAlias1, &antiAlias2 })
        {
            f->prepare (preSpec);
            f->setCoefficients (juce::dsp::IIR::Coefficients<float>::makeLowPass (hostSampleRate, 400.0, 0.7071));
        }

        maxLag = (int) std::ceil (decimatedRate / minHz);
        minLag = juce::jmax (2, (int) std::floor (decimatedRate / maxHz));
        windowSize = juce::nextPowerOfTwo (2 * maxLag);
        hopSize = juce::jmax (16, windowSize / 8);

        buffer.assign ((size_t) windowSize, 0.0f);
        // Sized here rather than inside analyse(): analyse() runs on the audio
        // thread, and the first resize there would allocate.
        linear.assign ((size_t) windowSize, 0.0f);
        diff.assign ((size_t) maxLag + 1, 0.0f);
        cumulative.assign ((size_t) maxLag + 1, 0.0f);

        writePos = 0;
        samplesSinceAnalysis = 0;
        decimationCounter = 0;
        fundamentalHz = 0.0f;
        confidence = 0.0f;
        reset();
    }

    void reset()
    {
        std::fill (buffer.begin(), buffer.end(), 0.0f);
        antiAlias1.reset();
        antiAlias2.reset();
        fundamentalHz = 0.0f;
        confidence = 0.0f;
    }

    // The audio delay (in host samples) that makes a pitch estimate line up
    // with the audio it describes. The estimate covers the analysis window, so
    // its "centre of mass" is half a window back, plus up to one hop of age.
    int getAnalysisLatencySamples() const
    {
        return (windowSize / 2 + hopSize) * decimationFactor;
    }

    // Feed every input sample (mono sum). Returns true on blocks where a new
    // estimate was produced.
    bool processSample (float x)
    {
        const float filtered = antiAlias2.processSample (0, antiAlias1.processSample (0, x));

        if (++decimationCounter < decimationFactor)
            return false;
        decimationCounter = 0;

        buffer[(size_t) writePos] = filtered;
        writePos = (writePos + 1) % windowSize;

        if (++samplesSinceAnalysis < hopSize)
            return false;
        samplesSinceAnalysis = 0;

        analyse();
        return true;
    }

    float getFundamentalHz() const { return fundamentalHz; }
    float getConfidence() const { return confidence; }

private:
    void analyse()
    {
        // Unwrap the circular buffer into linear order so lag indexing is simple.
        for (int i = 0; i < windowSize; ++i)
            linear[(size_t) i] = buffer[(size_t) ((writePos + i) % windowSize)];

        // Bail out on silence rather than reporting a pitch for noise.
        double energy = 0.0;
        for (float v : linear)
            energy += (double) v * v;
        if (energy < 1.0e-7)
        {
            confidence = 0.0f;
            fundamentalHz = 0.0f;
            return;
        }

        const int halfWindow = windowSize / 2;
        const int usableMaxLag = juce::jmin (maxLag, halfWindow - 1);

        // YIN step 1: squared difference function.
        for (int lag = minLag; lag <= usableMaxLag; ++lag)
        {
            double sum = 0.0;
            for (int i = 0; i < halfWindow; ++i)
            {
                const double d = (double) linear[(size_t) i] - (double) linear[(size_t) (i + lag)];
                sum += d * d;
            }
            diff[(size_t) lag] = (float) sum;
        }

        // YIN step 2: cumulative mean normalised difference. This is what
        // removes the "zero lag is always best" bias of raw autocorrelation and
        // is the main reason YIN beats plain autocorrelation on octave errors.
        cumulative[(size_t) minLag] = 1.0f;
        double runningSum = 0.0;
        for (int lag = minLag; lag <= usableMaxLag; ++lag)
        {
            runningSum += (double) diff[(size_t) lag];
            cumulative[(size_t) lag] = runningSum > 0.0
                ? (float) (diff[(size_t) lag] * (lag - minLag + 1) / runningSum)
                : 1.0f;
        }

        // YIN step 3: first local minimum below the absolute threshold, falling
        // back to the global minimum. Taking the *first* qualifying dip rather
        // than the deepest one is what avoids locking onto an octave down.
        constexpr float yinThreshold = 0.15f;
        int bestLag = -1;
        for (int lag = minLag + 1; lag < usableMaxLag; ++lag)
        {
            if (cumulative[(size_t) lag] < yinThreshold
                && cumulative[(size_t) lag] <= cumulative[(size_t) (lag + 1)])
            {
                bestLag = lag;
                break;
            }
        }

        if (bestLag < 0)
        {
            float best = 1.0f;
            for (int lag = minLag; lag <= usableMaxLag; ++lag)
            {
                if (cumulative[(size_t) lag] < best)
                {
                    best = cumulative[(size_t) lag];
                    bestLag = lag;
                }
            }
            // Nothing periodic enough to call a note.
            if (bestLag < 0 || best > 0.5f)
            {
                confidence = juce::jmax (0.0f, confidence - 0.25f);
                if (confidence <= 0.0f)
                    fundamentalHz = 0.0f;
                return;
            }
        }

        const float refinedLag = parabolicRefine (bestLag, usableMaxLag);
        const float hz = (float) (decimatedRate / refinedLag);

        if (hz < minHz || hz > maxHz)
        {
            confidence = juce::jmax (0.0f, confidence - 0.25f);
            return;
        }

        const float rawConfidence = juce::jlimit (0.0f, 1.0f, 1.0f - cumulative[(size_t) bestLag]);

        // Glide toward the new estimate instead of jumping: a one-frame outlier
        // shouldn't yank the entire harmonic grid. Large jumps (a genuine new
        // note) are allowed through quickly, small ones are smoothed.
        const bool bigJump = fundamentalHz <= 0.0f
                              || std::abs (std::log2 (hz / fundamentalHz)) > 0.25f;
        const float glide = bigJump ? 0.7f : 0.25f;
        fundamentalHz = fundamentalHz <= 0.0f ? hz : fundamentalHz + glide * (hz - fundamentalHz);
        confidence = confidence + 0.4f * (rawConfidence - confidence);
    }

    // Sub-sample refinement of the minimum: without this the pitch quantises to
    // whole lags, which at 4 kHz is several Hz of error at the top of the range
    // -- enough to put the upper harmonics of the grid noticeably off.
    float parabolicRefine (int lag, int usableMaxLag) const
    {
        if (lag <= minLag || lag >= usableMaxLag)
            return (float) lag;

        const float a = cumulative[(size_t) (lag - 1)];
        const float b = cumulative[(size_t) lag];
        const float c = cumulative[(size_t) (lag + 1)];
        const float denom = a - 2.0f * b + c;
        if (std::abs (denom) < 1.0e-9f)
            return (float) lag;
        return (float) lag + 0.5f * (a - c) / denom;
    }

    double sampleRate = 44100.0, decimatedRate = 4000.0;
    int decimationFactor = 11, decimationCounter = 0;
    int windowSize = 512, hopSize = 64, minLag = 16, maxLag = 114;
    int writePos = 0, samplesSinceAnalysis = 0;

    DspCommon::StereoIIR antiAlias1, antiAlias2;
    std::vector<float> buffer, linear, diff, cumulative;

    float fundamentalHz = 0.0f, confidence = 0.0f;
};
