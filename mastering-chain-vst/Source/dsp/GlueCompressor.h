#pragma once
#include "DspCommon.h"

// Stereo-linked, VCA-style bus compressor -- the "glue" stage that makes a
// mix (or a stack of stems bounced to stereo) feel like one cohesive
// signal, the way an SSL/API-style bus compressor does across a whole mix
// bus. Detection is linked across L/R (never independent) so the stereo
// image doesn't shift under gain reduction.
class GlueCompressor
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        envelope.prepare (spec.sampleRate);
        envelope.reset();
        // ~5ms RMS-style detector smoothing, typical of program-material bus compressors.
        rmsCoeff = std::exp (-1.0f / (float) (0.005 * spec.sampleRate));
        rmsSquared = 0.0f;
        gainReductionDb = 0.0f;
    }

    struct Params
    {
        float thresholdDb = -18.0f;
        float ratio = 2.0f;
        float attackMs = 10.0f;
        float releaseMs = 100.0f;
        bool autoRelease = true;
        float kneeDb = 6.0f;
        float makeupDb = 0.0f;
        bool autoMakeup = true;
        float mixPercent = 100.0f;
    };

    void updateParameters (const Params& p)
    {
        params = p;
        const float release = p.autoRelease ? autoReleaseMs() : p.releaseMs;
        envelope.setTimes (p.attackMs, release);

        // Rough auto-makeup: half of the gain that would be removed from a
        // signal sitting ~10dB above threshold -- a common "get you close"
        // heuristic, not a loudness-matched calculation.
        autoMakeupDb = p.autoMakeup
            ? DspCommon::softKneeGainReductionDb (10.0f, p.ratio, p.kneeDb) * 0.5f
            : 0.0f;
    }

    // Called once per sample block/sample with the linked (max of L/R abs)
    // detector input; returns the linear gain to multiply onto both channels.
    float computeGain (float linkedAbsSample)
    {
        rmsSquared = rmsCoeff * rmsSquared + (1.0f - rmsCoeff) * linkedAbsSample * linkedAbsSample;
        const float rms = std::sqrt (juce::jmax (rmsSquared, 1.0e-12f));
        const float levelDb = juce::Decibels::gainToDecibels (rms, -120.0f);
        const float overDb = levelDb - params.thresholdDb;

        const float targetReductionDb = DspCommon::softKneeGainReductionDb (overDb, params.ratio, params.kneeDb);
        gainReductionDb = envelope.process (targetReductionDb);

        const float makeupDb = params.autoMakeup ? autoMakeupDb : params.makeupDb;
        const float gain = juce::Decibels::decibelsToGain (-gainReductionDb + makeupDb);

        const float wet = juce::jlimit (0.0f, 1.0f, params.mixPercent / 100.0f);
        return (1.0f - wet) + wet * gain; // parallel-blend the gain itself
    }

    float getCurrentGainReductionDb() const { return gainReductionDb; }

private:
    float autoReleaseMs() const
    {
        // Program-dependent release: faster when gain reduction is heavy
        // (so it doesn't smother transients), slower when it's light.
        return juce::jmap (juce::jlimit (0.0f, 20.0f, gainReductionDb), 0.0f, 20.0f, 250.0f, 60.0f);
    }

    double sampleRate = 44100.0;
    Params params;
    DspCommon::OnePoleEnvelope envelope;
    float rmsCoeff = 0.0f;
    float rmsSquared = 0.0f;
    float gainReductionDb = 0.0f;
    float autoMakeupDb = 0.0f;
};
