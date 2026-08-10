#pragma once
#include "DspCommon.h"

// Tape-style saturation stage: asymmetric tanh waveshaping (odd + a little
// even harmonic content, like real tape/tube asymmetry) followed by a
// gentle high-frequency rolloff that mimics tape head/bias losses, plus a
// small low-end lift for the "fullness" tape adds. Meant to run oversampled
// by the caller (see PluginProcessor) since waveshaping generates harmonics
// that can alias if processed at the plugin's native sample rate.
class TapeSaturator
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        headFilter.prepare (spec);
        lowLift.prepare (spec);
        reset();
    }

    void reset()
    {
        headFilter.reset();
        lowLift.reset();
    }

    struct Params
    {
        float driveDb = 6.0f;
        float warmthPercent = 35.0f;
        float mixPercent = 100.0f;
    };

    void updateParameters (const Params& p)
    {
        if (sampleRate <= 0.0)
            return;

        driveGain = juce::Decibels::decibelsToGain (p.driveDb);
        warmth = juce::jlimit (0.0f, 1.0f, p.warmthPercent / 100.0f);
        mix = juce::jlimit (0.0f, 1.0f, p.mixPercent / 100.0f);

        // More warmth = darker tape head response (lower cutoff) and a
        // touch more low-end lift, echoing how real tape saturation both
        // rolls off top end and thickens the bottom as drive increases.
        const float headCutoffHz = juce::jmap (warmth, 0.0f, 1.0f, 16000.0f, 6000.0f);
        headFilter.setCoefficients (juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            sampleRate, headCutoffHz, 0.7f, juce::Decibels::decibelsToGain (juce::jmap (warmth, 0.0f, 1.0f, 0.0f, -3.0f))));

        const float liftDb = juce::jmap (warmth, 0.0f, 1.0f, 0.0f, 2.0f);
        lowLift.setCoefficients (juce::dsp::IIR::Coefficients<float>::makeLowShelf (
            sampleRate, 90.0f, 0.7f, juce::Decibels::decibelsToGain (liftDb)));

        // Compensate for the level lost to tanh compression so unity drive
        // doesn't quietly drop output level.
        outputComp = driveGain > 0.0f ? 1.0f / std::tanh (juce::jmin (driveGain, 8.0f)) : 1.0f;
    }

    // channel index only used to keep the two per-channel filters separate;
    // the waveshaper itself is stateless/sample-independent.
    float processSample (int channel, float x)
    {
        const float driven = DspCommon::asymmetricTanh (x * driveGain, 0.15f * warmth) * outputComp;
        float shaped = headFilter.processSample (channel, driven);
        shaped = lowLift.processSample (channel, shaped);
        return x + mix * (shaped - x);
    }

private:
    double sampleRate = 44100.0;
    float driveGain = 1.0f, warmth = 0.0f, mix = 1.0f, outputComp = 1.0f;
    DspCommon::StereoIIR headFilter, lowLift;
};
