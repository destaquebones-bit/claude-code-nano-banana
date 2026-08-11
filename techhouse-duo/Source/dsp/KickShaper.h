#pragma once
#include "DspCommon.h"

// Kick-mode processing: transient shaping plus low-end tail control.
//
// The single biggest cause of a muddy low end in this genre is not the bass at
// all -- it's the kick's low-frequency tail overlapping the bass note that
// follows it. The reference notes in this repo describe the genre's kick as
// "short, punchy, fundamental around 50-60Hz, little sustain", and a kick that
// rings longer than that is what fills the space the bassline needs.
//
// TailTightener therefore triggers a decay envelope on each detected hit and
// applies it *only to the low band*, so the click and body stay intact while the
// low-frequency energy is shortened to a chosen length. That is a different
// operation from compressing or gating the kick, both of which act on the whole
// signal and change the attack.
class KickShaper
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        lowSplit.prepare (spec);
        highSplit.prepare (spec);
        onsetFast.prepare (spec.sampleRate);
        onsetFast.setTimes (0.5f, 12.0f);
        onsetSlow.prepare (spec.sampleRate);
        onsetSlow.setTimes (25.0f, 180.0f);
        transientEnv.prepare (spec.sampleRate);
        transientEnv.setTimes (0.5f, 45.0f);
        reset();
    }

    void reset()
    {
        lowSplit.reset();
        highSplit.reset();
        onsetFast.reset();
        onsetSlow.reset();
        transientEnv.reset();
        tailEnvelope = 1.0f;
        gateOpen = false;
        transientStrength = 0.0f;
    }

    struct Params
    {
        float subFreq = 120.0f;      // low/high split for tail control
        float topFreq = 1200.0f;     // body/click split
        float subGainDb = 0.0f;
        float topGainDb = 0.0f;
        float tailAmount = 0.0f;     // 0-1, how far the low tail is pulled down
        float tailMs = 120.0f;       // how long the low end is allowed to ring
        float attackAmount = 0.0f;   // -1..1 transient attack cut/boost
    };

    void setParams (const Params& p)
    {
        params = p;
        if (sampleRate <= 0.0)
            return;
        lowSplit.setCrossover (sampleRate, juce::jlimit (40.0f, 400.0f, p.subFreq));
        highSplit.setCrossover (sampleRate, juce::jlimit (500.0f, 8000.0f, p.topFreq));
        tailCoeff = std::exp (-1.0f / (float) (0.001 * juce::jmax (5.0f, p.tailMs) * sampleRate));
        subGain = DspCommon::dbToGain (p.subGainDb);
        topGain = DspCommon::dbToGain (p.topGainDb);
    }

    // Detector runs on the mono sum once per sample, before the per-channel
    // processing, so both channels share one tail envelope and one transient
    // decision (a kick is a mono event; independent per-channel gating would
    // wobble the stereo image).
    void processDetector (float monoSample)
    {
        const float mag = std::abs (monoSample);
        const float fast = onsetFast.process (mag);
        const float slow = onsetSlow.process (mag);

        // A hit is when the fast envelope pulls clearly ahead of the slow one.
        const float ratio = slow > 1.0e-5f ? fast / slow : 0.0f;
        const bool hit = ratio > 1.35f && fast > 1.0e-3f;

        if (hit && ! gateOpen)
        {
            gateOpen = true;
            tailEnvelope = 1.0f;
        }
        else if (! hit && ratio < 1.1f)
        {
            gateOpen = false;
        }

        if (! gateOpen)
            tailEnvelope *= tailCoeff;

        transientStrength = transientEnv.process (juce::jlimit (0.0f, 1.0f, (ratio - 1.0f) * 1.5f));
    }

    float processSample (int channel, float x)
    {
        float low = 0.0f, aboveLow = 0.0f;
        lowSplit.split (channel, x, low, aboveLow);

        float body = 0.0f, top = 0.0f;
        highSplit.split (channel, aboveLow, body, top);

        // The tail envelope only touches the low band: shortening the low end
        // without touching the click is exactly the move that stops the kick
        // from covering the bass.
        const float tailGain = 1.0f - params.tailAmount * (1.0f - tailEnvelope);
        low *= subGain * tailGain;
        top *= topGain;

        float out = low + body + top;

        // Attack shaping rides the transient detector, boosting or softening the
        // leading edge without the level-dependence a compressor would bring.
        if (std::abs (params.attackAmount) > 0.001f)
            out *= 1.0f + params.attackAmount * transientStrength;

        return out;
    }

    float getTransientStrength() const { return transientStrength; }
    float getTailEnvelope() const { return tailEnvelope; }

private:
    double sampleRate = 44100.0;
    Params params;
    DspCommon::LR4Crossover lowSplit, highSplit;
    DspCommon::OnePoleEnvelope onsetFast, onsetSlow, transientEnv;
    float tailEnvelope = 1.0f, tailCoeff = 0.999f;
    float subGain = 1.0f, topGain = 1.0f, transientStrength = 0.0f;
    bool gateOpen = false;
};
