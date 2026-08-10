#pragma once
#include "DspCommon.h"

// Mid/Side stereo widener with a bass-mono safety net: below the crossover
// frequency the side signal is high-passed (i.e. forced toward mono), which
// is the standard mastering-chain trick to keep low end phase-coherent and
// club/vinyl/mono-compatible while still widening the mix above it. Only
// meaningful for exactly 2 channels; anything else passes through untouched.
class StereoWidener
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        numChannels = (int) spec.numChannels;
        sideHighPass.prepare (spec);
        reset();
    }

    void reset() { sideHighPass.reset(); }

    struct Params
    {
        float widthPercent = 100.0f;
        float bassMonoHz = 120.0f;
    };

    void updateParameters (const Params& p)
    {
        if (sampleRate <= 0.0)
            return;
        width = juce::jlimit (0.0f, 2.0f, p.widthPercent / 100.0f);
        sideHighPass.setCoefficients (juce::dsp::IIR::Coefficients<float>::makeHighPass (
            sampleRate, juce::jlimit (10.0f, 500.0f, p.bassMonoHz), 0.707f));
    }

    bool isUsable() const { return numChannels == 2; }

    // Must be called once per sample with both channel values; writes back in place.
    void processSample (float& left, float& right)
    {
        if (numChannels != 2)
            return;

        const float mid = 0.5f * (left + right);
        float side = 0.5f * (left - right);

        // Only the content above the crossover gets widened; below it the
        // side signal is filtered toward zero, collapsing that band to mono.
        side = sideHighPass.processSample (0, side);
        side *= width;

        left = mid + side;
        right = mid - side;
    }

private:
    double sampleRate = 44100.0;
    int numChannels = 2;
    float width = 1.0f;
    DspCommon::StereoIIR sideHighPass; // only channel 0 of this is used (mono side signal)
};
