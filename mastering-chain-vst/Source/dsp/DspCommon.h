#pragma once
#include <juce_dsp/juce_dsp.h>

// Small shared building blocks used by every module in dsp/.
namespace DspCommon
{
    // One IIR::Filter per channel sharing a single coefficients object,
    // so a filter "band" can be updated once per block and applied
    // sample-by-sample per channel.
    struct StereoIIR
    {
        std::array<juce::dsp::IIR::Filter<float>, 2> filters;

        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            for (auto& f : filters)
                f.prepare (spec);
        }

        void reset()
        {
            for (auto& f : filters)
                f.reset();
        }

        void setCoefficients (juce::dsp::IIR::Coefficients<float>::Ptr c)
        {
            for (auto& f : filters)
                f.coefficients = c;
        }

        float processSample (int channel, float x)
        {
            return filters[(size_t) juce::jlimit (0, 1, channel)].processSample (x);
        }
    };

    // One-pole envelope follower with independent attack/release time
    // constants, in milliseconds. Used by the compressor and limiter for
    // program-dependent gain smoothing.
    class OnePoleEnvelope
    {
    public:
        void prepare (double sr) { sampleRate = sr; }
        void reset (float startValue = 0.0f) { value = startValue; }

        void setTimes (float attackMs, float releaseMs)
        {
            attackCoeff  = timeToCoeff (attackMs);
            releaseCoeff = timeToCoeff (releaseMs);
        }

        float process (float target)
        {
            const float coeff = target > value ? attackCoeff : releaseCoeff;
            value = target + coeff * (value - target);
            return value;
        }

        float getValue() const { return value; }

    private:
        float timeToCoeff (float timeMs) const
        {
            if (timeMs <= 0.0f || sampleRate <= 0.0)
                return 0.0f;
            return std::exp (-1.0f / (float) (0.001 * timeMs * sampleRate));
        }

        double sampleRate = 44100.0;
        float value = 0.0f;
        float attackCoeff = 0.0f;
        float releaseCoeff = 0.0f;
    };

    inline float softKneeGainReductionDb (float overDb, float ratio, float kneeDb)
    {
        // Standard soft-knee compressor curve. overDb = level above threshold (can be negative).
        if (kneeDb <= 0.0f)
            return overDb > 0.0f ? overDb * (1.0f - 1.0f / ratio) : 0.0f;

        const float half = kneeDb * 0.5f;
        if (overDb <= -half)
            return 0.0f;
        if (overDb >= half)
            return overDb * (1.0f - 1.0f / ratio);

        // Quadratic interpolation through the knee region.
        const float x = overDb + half;
        return (x * x) / (2.0f * kneeDb) * (1.0f - 1.0f / ratio);
    }

    // Asymmetric tanh waveshaper: adds a small even-harmonic bias
    // (controlled by `asymmetry`) on top of the classic odd-harmonic
    // tanh saturation, which is what makes tube/tape stages sound
    // "warmer" than a purely symmetric clipper.
    inline float asymmetricTanh (float x, float asymmetry)
    {
        const float biased = x + asymmetry * x * x;
        return std::tanh (biased);
    }
}
