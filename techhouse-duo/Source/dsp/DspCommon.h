#pragma once
#include <juce_dsp/juce_dsp.h>

// Shared building blocks for both modes of the plugin.
namespace DspCommon
{
    // One IIR::Filter per channel sharing a single coefficients object, so a
    // "band" can be re-tuned once per block and then run per-sample per channel.
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

    // One-pole envelope follower with independent attack/release times in ms.
    class OnePoleEnvelope
    {
    public:
        void prepare (double sr) { sampleRate = sr; reset(); }
        void reset (float startValue = 0.0f) { value = startValue; }

        void setTimes (float newAttackMs, float newReleaseMs)
        {
            attackMs = newAttackMs;
            releaseMs = newReleaseMs;
            attackCoeff  = timeToCoeff (attackMs, 1);
            releaseCoeff = timeToCoeff (releaseMs, 1);
        }

        // Per-sample use: call once for every audio sample.
        float process (float target)
        {
            const float coeff = target > value ? attackCoeff : releaseCoeff;
            value = target + coeff * (value - target);
            return value;
        }

        // Control-rate use: call once per block, passing how many samples have
        // elapsed since the last call. Without this, an envelope ticked once
        // per block would run its attack/release times slower by exactly the
        // block size -- a 5 ms attack silently becoming 2.5 s at 512 samples.
        // Block sizes also vary between hosts and even between callbacks, so
        // the coefficient has to be derived from the real elapsed time rather
        // than assumed.
        float processOverSamples (float target, int samplesElapsed)
        {
            const float timeMs = target > value ? attackMs : releaseMs;
            const float coeff = timeToCoeff (timeMs, juce::jmax (1, samplesElapsed));
            value = target + coeff * (value - target);
            return value;
        }

        float getValue() const { return value; }

    private:
        float timeToCoeff (float timeMs, int samplesElapsed) const
        {
            if (timeMs <= 0.0f || sampleRate <= 0.0)
                return 0.0f;
            return std::exp (-(float) samplesElapsed / (float) (0.001 * timeMs * sampleRate));
        }

        double sampleRate = 44100.0;
        float value = 0.0f, attackCoeff = 0.0f, releaseCoeff = 0.0f;
        float attackMs = 10.0f, releaseMs = 100.0f;
    };

    // Linkwitz-Riley 4th-order crossover (cascaded Butterworth pairs), used to
    // split the kick into sub / body / top without the phase smearing a naive
    // filter split would cause. Sums back to flat magnitude.
    struct LR4Crossover
    {
        void prepare (const juce::dsp::ProcessSpec& spec)
        {
            for (auto* f : { &lp1, &lp2, &hp1, &hp2 })
                f->prepare (spec);
            reset();
        }

        void reset()
        {
            for (auto* f : { &lp1, &lp2, &hp1, &hp2 })
                f->reset();
        }

        void setCrossover (double sampleRate, float freq)
        {
            auto lp = juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, freq, 0.7071f);
            auto hp = juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, freq, 0.7071f);
            lp1.setCoefficients (lp);
            lp2.setCoefficients (lp);
            hp1.setCoefficients (hp);
            hp2.setCoefficients (hp);
        }

        // Writes the low band to `low` and the high band to `high`.
        void split (int channel, float x, float& low, float& high)
        {
            low  = lp2.processSample (channel, lp1.processSample (channel, x));
            high = hp2.processSample (channel, hp1.processSample (channel, x));
        }

        StereoIIR lp1, lp2, hp1, hp2;
    };

    inline float dbToGain (float db) { return juce::Decibels::decibelsToGain (db, -120.0f); }
    inline float gainToDb (float g)  { return juce::Decibels::gainToDecibels (g, -120.0f); }

    // MIDI note number for a frequency, and the note name for display.
    inline int freqToMidiNote (float hz)
    {
        if (hz <= 0.0f)
            return -1;
        return (int) std::lround (69.0 + 12.0 * std::log2 ((double) hz / 440.0));
    }

    inline juce::String midiNoteName (int note)
    {
        if (note < 0 || note > 127)
            return "--";
        static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        return juce::String (names[note % 12]) + juce::String (note / 12 - 1);
    }
}
