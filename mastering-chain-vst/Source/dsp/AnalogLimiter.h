#pragma once
#include "DspCommon.h"
#include <vector>

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
        gainDbWindow.prepare (lookaheadSamples + 2);
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
            gainDbWindow.popBack();
        gainDbWindow.pushBack (sampleCounter, neededGainDb);
        while (! gainDbWindow.empty() && gainDbWindow.front().first <= sampleCounter - lookaheadSamples)
            gainDbWindow.popFront();

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

    // Fixed-capacity circular deque, sized once in prepare(). A std::deque
    // here allocated and freed chunks while running -- on the audio thread,
    // driven by signal content, which is the worst possible time for it. The
    // window can never hold more than one entry per lookahead sample, so the
    // capacity is known up front and the ring never needs to grow.
    struct MonotonicWindow
    {
        std::vector<std::pair<int64_t, float>> data;
        int head = 0, count = 0;

        void prepare (int capacity)
        {
            data.assign ((size_t) juce::jmax (1, capacity), { 0, 0.0f });
            clear();
        }

        void clear() { head = 0; count = 0; }
        bool empty() const { return count == 0; }
        int capacity() const { return (int) data.size(); }

        const std::pair<int64_t, float>& front() const { return data[(size_t) head]; }
        const std::pair<int64_t, float>& back() const
        {
            return data[(size_t) ((head + count - 1) % capacity())];
        }

        void popFront() { head = (head + 1) % capacity(); --count; }
        void popBack() { --count; }

        void pushBack (int64_t position, float value)
        {
            if (count == capacity())
                popFront(); // cannot happen given the sizing, but never overrun
            data[(size_t) ((head + count) % capacity())] = { position, value };
            ++count;
        }
    };

    MonotonicWindow gainDbWindow;
    DspCommon::OnePoleEnvelope envelope;
};
