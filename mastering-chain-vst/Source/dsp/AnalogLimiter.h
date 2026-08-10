#pragma once
#include "DspCommon.h"
#include <deque>

// Stereo-linked lookahead brickwall limiter -- the final safety stage that
// guarantees the ceiling is never crossed (the whole point of putting a
// mastering chain in one plugin: everything upstream can be pushed harder
// because this stage always catches it). Meant to run inside the same
// oversampled block as TapeSaturator (see PluginProcessor) so peak
// detection sees inter-sample ("true") peaks, not just sample-and-hold ones.
class AnalogLimiter
{
public:
    static constexpr float lookaheadMs = 5.0f;

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        lookaheadSamples = juce::jmax (1, (int) std::ceil (0.001 * lookaheadMs * sampleRate));

        for (auto& buf : delayBuffer)
        {
            buf.assign ((size_t) lookaheadSamples + 1, 0.0f);
        }
        writePos = 0;
        sampleCounter = 0;
        gainDbWindow.clear();
        envelope.prepare (sampleRate);
        envelope.reset (0.0f);
    }

    void reset()
    {
        for (auto& buf : delayBuffer)
            std::fill (buf.begin(), buf.end(), 0.0f);
        writePos = 0;
        sampleCounter = 0;
        gainDbWindow.clear();
        envelope.reset (0.0f);
    }

    int getLatencySamples() const { return lookaheadSamples; }

    struct Params
    {
        float ceilingDb = -1.0f;
        float driveDb = 0.0f;
        float releaseMs = 60.0f;
    };

    void updateParameters (const Params& p)
    {
        ceilingLinear = juce::Decibels::decibelsToGain (p.ceilingDb);
        ceilingDb = p.ceilingDb;
        driveGain = juce::Decibels::decibelsToGain (p.driveDb);
        // Attack is effectively "free" (the lookahead window already sees
        // the peak coming); only release needs a musical time constant.
        envelope.setTimes (0.5f, p.releaseMs);
    }

    // Processes one linked stereo sample in place. `numChannels` must be 1 or 2;
    // for mono, pass the same value via both refs.
    void processStereoSample (float& left, float& right)
    {
        const float drivenL = left * driveGain;
        const float drivenR = right * driveGain;

        // Push driven (pre-limit) samples into the delay line -- this is
        // the signal that will actually be output, just time-shifted.
        delayBuffer[0][(size_t) writePos] = drivenL;
        delayBuffer[1][(size_t) writePos] = drivenR;

        // Linked stereo peak: the louder of the two channels dictates the
        // gain reduction applied to both, so the stereo image never shifts.
        const float peak = juce::jmax (std::abs (drivenL), std::abs (drivenR), 1.0e-8f);
        const float peakDb = juce::Decibels::gainToDecibels (peak);
        const float neededGainDb = juce::jmin (0.0f, ceilingDb - peakDb);

        // Monotonic-deque sliding-window minimum: gainDbWindow always holds
        // an increasing sequence of values, so the front is always the
        // window's minimum in O(1) amortized per sample.
        while (! gainDbWindow.empty() && gainDbWindow.back().second >= neededGainDb)
            gainDbWindow.pop_back();
        gainDbWindow.emplace_back (sampleCounter, neededGainDb);
        while (gainDbWindow.front().first <= sampleCounter - lookaheadSamples)
            gainDbWindow.pop_front();

        const float windowMinDb = gainDbWindow.front().second;
        const float smoothedDb = envelope.process (windowMinDb);
        const float gain = juce::Decibels::decibelsToGain (smoothedDb);

        const int readPos = (writePos + 1) % (lookaheadSamples + 1); // oldest sample = next write slot
        float outL = delayBuffer[0][(size_t) readPos] * gain;
        float outR = delayBuffer[1][(size_t) readPos] * gain;

        // Hard safety clamp: the lookahead+release smoothing should already
        // keep us under ceiling, but a true brickwall promise shouldn't
        // depend on smoothing alone.
        outL = juce::jlimit (-ceilingLinear, ceilingLinear, outL);
        outR = juce::jlimit (-ceilingLinear, ceilingLinear, outR);

        left = outL;
        right = outR;

        writePos = (writePos + 1) % (lookaheadSamples + 1);
        ++sampleCounter;
    }

    float getCurrentGainReductionDb() const
    {
        return gainDbWindow.empty() ? 0.0f : -envelope.getValue();
    }

private:
    double sampleRate = 44100.0;
    int lookaheadSamples = 1;
    std::array<std::vector<float>, 2> delayBuffer;
    int writePos = 0;
    int64_t sampleCounter = 0;

    float ceilingDb = -1.0f;
    float ceilingLinear = 0.891f;
    float driveGain = 1.0f;

    std::deque<std::pair<int64_t, float>> gainDbWindow;
    DspCommon::OnePoleEnvelope envelope;
};
