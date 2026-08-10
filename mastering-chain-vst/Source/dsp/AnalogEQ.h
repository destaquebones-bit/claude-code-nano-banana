#pragma once
#include "DspCommon.h"

// 4-band mastering EQ modeled after passive/tube program EQs (Pultec EQP-1A
// style low end + 2 sweepable mid bells + an air shelf), plus an optional
// "iron" stage that adds a touch of transformer-style saturation so boosts
// don't sound sterile. This is the tonal-shaping stage of the chain.
//
// Pultec trick: boosting AND cutting near the same low frequency at once
// doesn't cancel out (different filter shapes/Qs), it produces a lift with
// a tightened, non-boomy low end -- a classic mastering move that's hard to
// get from a plain parametric EQ.
class AnalogEQ
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (auto* f : { &lowBoost, &lowCut, &lowMid, &highMid, &airShelf })
            f->prepare (spec);
        reset();
    }

    void reset()
    {
        for (auto* f : { &lowBoost, &lowCut, &lowMid, &highMid, &airShelf })
            f->reset();
    }

    struct Params
    {
        float lowFreq = 60.0f, lowBoostDb = 0.0f, lowCutDb = 0.0f;
        float lowMidFreq = 400.0f, lowMidGainDb = 0.0f, lowMidQ = 0.7f;
        float highMidFreq = 2500.0f, highMidGainDb = 0.0f, highMidQ = 0.7f;
        float airFreq = 12000.0f, airGainDb = 0.0f;
        float ironAmount = 0.0f; // 0-100 %
    };

    // Recomputes filter coefficients. Cheap enough to call once per block,
    // not per sample -- do NOT call per-sample, IIR coefficient design
    // involves several trig calls.
    void updateParameters (const Params& p)
    {
        if (sampleRate <= 0.0)
            return;

        using Coeffs = juce::dsp::IIR::Coefficients<float>;

        lowBoost.setCoefficients (Coeffs::makeLowShelf (sampleRate, p.lowFreq, 0.7f,
            juce::Decibels::decibelsToGain (p.lowBoostDb)));

        // Cut frequency sits above the boost frequency, narrower Q --
        // the classic Pultec "boost low, cut a bit higher" trick.
        lowCut.setCoefficients (Coeffs::makePeakFilter (sampleRate, p.lowFreq * 1.8f, 1.2f,
            juce::Decibels::decibelsToGain (-p.lowCutDb)));

        lowMid.setCoefficients (Coeffs::makePeakFilter (sampleRate, p.lowMidFreq, p.lowMidQ,
            juce::Decibels::decibelsToGain (p.lowMidGainDb)));

        highMid.setCoefficients (Coeffs::makePeakFilter (sampleRate, p.highMidFreq, p.highMidQ,
            juce::Decibels::decibelsToGain (p.highMidGainDb)));

        airShelf.setCoefficients (Coeffs::makeHighShelf (sampleRate, p.airFreq, 0.7f,
            juce::Decibels::decibelsToGain (p.airGainDb)));

        ironAmount = juce::jlimit (0.0f, 1.0f, p.ironAmount / 100.0f);
    }

    float processSample (int channel, float x)
    {
        float y = x;
        y = lowBoost.processSample (channel, y);
        y = lowCut.processSample (channel, y);
        y = lowMid.processSample (channel, y);
        y = highMid.processSample (channel, y);
        y = airShelf.processSample (channel, y);

        if (ironAmount > 0.0f)
        {
            // Gentle transformer-style coloration: barely-there tanh
            // saturation, mixed in proportionally to ironAmount so it
            // never turns into an obvious distortion effect.
            const float driven = DspCommon::asymmetricTanh (y * 1.6f, 0.08f) / 1.6f;
            y = y + ironAmount * (driven - y);
        }
        return y;
    }

private:
    double sampleRate = 44100.0;
    float ironAmount = 0.0f;
    DspCommon::StereoIIR lowBoost, lowCut, lowMid, highMid, airShelf;
};
