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
        dcBlocker.prepare (spec);
        dcBlocker.setCoefficients (juce::dsp::IIR::Coefficients<float>::makeHighPass (
            spec.sampleRate, 20.0, 0.7071));
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
        dcBlocker.reset();
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
        float drive = 0.0f;          // 0-1 transformer-style saturation, body band only
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

        driveAmount = juce::jlimit (0.0f, 1.0f, p.drive);
        driveGain = 1.0f + driveAmount * 7.0f;
        // A constant operating-point offset, not a signal-dependent one: this
        // is how a biased valve or transformer stage actually behaves, and it
        // is what makes the curve clip harder on one half of the wave.
        driveBias = 0.45f * driveAmount;
        updateDriveCompensation();
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

        // Saturation is applied to the body band ONLY, and this is the whole
        // design of the stage rather than a shortcut:
        //
        //  - Driving the sub would generate harmonics straight into the
        //    150-400 Hz region this plugin exists to clear out, so the sub is
        //    left untouched and the low end stays exactly as clean as the tail
        //    control made it.
        //  - Driving the click band would be the one place aliasing is audible
        //    (a 3 kHz transient's harmonics land past Nyquist), and it would
        //    force oversampling and its latency onto a mode that currently
        //    reports none. The click already has its own gain control; what it
        //    needs is level, not harmonics.
        //  - The body band is where kick punch actually lives, and its 2nd and
        //    3rd harmonics stay comfortably under Nyquist, so this needs no
        //    oversampling to be clean.
        //
        // Saturation is instantaneous, so unlike a compressor it adds weight
        // without lengthening the tail the shaper just shortened.
        if (driveAmount > 0.0001f)
            body = saturateBody (channel, body);

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
    float shapeRaw (float x) const
    {
        return std::tanh (x * driveGain + driveBias);
    }

    // Antiderivative antialiasing (Zavalishin section 6.13) was implemented
    // here and then removed after measuring it: see tests/ModulationExperiment.
    // Folded partials sit at the noise floor anywhere below about 3 kHz, and
    // this stage only drives the body band, so the technique bought 0.0 dB
    // where it is actually used and 3.3 dB only at 5 kHz -- a frequency this
    // stage deliberately never touches. It costs a log and an exp per sample,
    // so it is not worth carrying. It would become worth revisiting if Drive
    // were ever extended to the click band.
    // Loudness-matched makeup, measured rather than guessed.
    //
    // A closed-form compensation is easy to get wrong: dividing by driveGain
    // only matches the small-signal slope, and a saturated signal is far
    // quieter than that predicts (an early version of this lost nearly 5 dB at
    // high drive). Instead the curve is evaluated over one cycle of a reference
    // sine here -- once per parameter change, never per sample -- and the
    // compensation is the ratio of RMS in to RMS out. The DC the bias creates
    // is subtracted first, because the blocker downstream removes it anyway.
    //
    // This matters beyond tidiness: without it, turning Drive up mostly makes
    // things louder, and louder always sounds better in an A/B. Matching the
    // level means the knob is judged on what it actually does to the tone.
    void updateDriveCompensation()
    {
        if (driveAmount <= 0.0001f)
        {
            driveComp = 1.0f;
            return;
        }

        constexpr int points = 256;
        constexpr float referenceAmplitude = 0.25f;

        float shaped[points];
        double mean = 0.0, inSum = 0.0;
        for (int i = 0; i < points; ++i)
        {
            const float x = referenceAmplitude
                * std::sin (juce::MathConstants<float>::twoPi * (float) i / (float) points);
            shaped[i] = shapeRaw (x);
            mean += shaped[i];
            inSum += (double) x * x;
        }
        mean /= points;

        double outSum = 0.0;
        for (int i = 0; i < points; ++i)
        {
            const double ac = shaped[i] - mean;
            outSum += ac * ac;
        }

        driveComp = outSum > 1.0e-12 ? (float) std::sqrt (inSum / outSum) : 1.0f;
    }

    // Transformer-style asymmetric soft saturation. Blending by driveAmount
    // makes the control exactly transparent at zero rather than merely close to
    // it, which an opt-in colouring stage has to be.
    float saturateBody (int channel, float x)
    {
        float shaped = shapeRaw (x) * driveComp;
        shaped = dcBlocker.processSample (channel, shaped);
        return x + driveAmount * (shaped - x);
    }

    double sampleRate = 44100.0;
    Params params;
    DspCommon::LR4Crossover lowSplit, highSplit;
    DspCommon::StereoIIR dcBlocker;
    DspCommon::OnePoleEnvelope onsetFast, onsetSlow, transientEnv;
    float tailEnvelope = 1.0f, tailCoeff = 0.999f;
    float subGain = 1.0f, topGain = 1.0f, transientStrength = 0.0f;
    float driveAmount = 0.0f, driveGain = 1.0f, driveBias = 0.0f, driveComp = 1.0f;
    bool gateOpen = false;
};
