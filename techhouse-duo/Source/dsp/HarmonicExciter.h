#pragma once
#include "DspCommon.h"

// Generates upper harmonics so the bass survives on phones, laptops and club
// tops -- the "missing fundamental" effect, where hearing 2f0/3f0/4f0 makes the
// brain perceive a fundamental the speaker never reproduced.
//
// What makes this different from running the bass through a saturator: a
// saturator distorts the *whole* signal, so every pair of input partials also
// produces sum and difference tones. Those intermodulation products are not
// harmonically related to the note and are a major cause of bass sounding
// muddy rather than bigger.
//
// Here the fundamental is first isolated to a near-pure sine with a narrow
// band-pass, and only that is shaped. A single sinusoid has nothing to
// intermodulate with, and passing it through Chebyshev polynomials of the first
// kind gives *exactly* the nth harmonic: T2(sin) -> 2f0, T3(sin) -> 3f0, and so
// on. So every generated partial lands precisely on the harmonic series of the
// note being played.
class HarmonicExciter
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        isolate1.prepare (spec);
        isolate2.prepare (spec);
        outputHighPass.prepare (spec);
        dcBlocker.prepare (spec);
        dcBlocker.setCoefficients (juce::dsp::IIR::Coefficients<float>::makeHighPass (sampleRate, 30.0, 0.7071));
        envelope.prepare (spec.sampleRate);
        envelope.setTimes (5.0f, 80.0f);
        reset();
    }

    void reset()
    {
        isolate1.reset();
        isolate2.reset();
        outputHighPass.reset();
        dcBlocker.reset();
        envelope.reset();
    }

    struct Params
    {
        float amount = 0.0f;   // 0-1 mix of the generated harmonics
        float balance = 0.5f;  // 0 = even harmonics (2nd/4th), 1 = odd (3rd/5th)
    };

    void setParams (const Params& p) { params = p; }

    // Re-tunes to the tracked note. Called once per block; when confidence is
    // low the caller should pass amount 0 rather than exciting a wrong note.
    void setFundamental (float hz)
    {
        if (sampleRate <= 0.0 || hz < 20.0f || hz > (float) (sampleRate * 0.2))
            return;

        fundamentalHz = hz;

        // Two cascaded band-passes: one is not selective enough to leave a
        // clean sine when neighbouring harmonics are strong.
        auto bp = juce::dsp::IIR::Coefficients<float>::makeBandPass (sampleRate, hz, 4.0f);
        isolate1.setCoefficients (bp);
        isolate2.setCoefficients (bp);

        // Keep only what we generated: everything at or below the fundamental
        // is already present in the dry signal and would just add level.
        outputHighPass.setCoefficients (juce::dsp::IIR::Coefficients<float>::makeHighPass (
            sampleRate, juce::jmin (hz * 1.6f, (float) (sampleRate * 0.4)), 0.7071f));
    }

    float processSample (int channel, float dry)
    {
        if (params.amount <= 0.0001f || fundamentalHz <= 0.0f)
            return dry;

        const float sine = isolate2.processSample (channel, isolate1.processSample (channel, dry));

        // Chebyshev polynomials need an input normalised to +/-1 to produce
        // clean harmonics, so divide by the running envelope and multiply back
        // afterwards. Below the floor the "sine" is just noise, so stay silent
        // rather than amplifying it into audible fizz.
        const float env = envelope.process (std::abs (sine));
        if (env < 1.0e-4f)
            return dry;

        const float s = juce::jlimit (-1.0f, 1.0f, sine / env);
        const float s2 = s * s;
        const float s3 = s2 * s;
        const float s4 = s2 * s2;
        const float s5 = s4 * s;

        const float t2 = 2.0f * s2 - 1.0f;                 // 2f0
        const float t3 = 4.0f * s3 - 3.0f * s;             // 3f0
        const float t4 = 8.0f * s4 - 8.0f * s2 + 1.0f;     // 4f0
        const float t5 = 16.0f * s5 - 20.0f * s3 + 5.0f * s; // 5f0

        const float odd = 0.65f * t3 + 0.35f * t5;
        const float even = 0.65f * t2 + 0.35f * t4;
        const float blend = juce::jlimit (0.0f, 1.0f, params.balance);

        float generated = env * ((1.0f - blend) * even + blend * odd);
        generated = dcBlocker.processSample (channel, generated);
        generated = outputHighPass.processSample (channel, generated);

        return dry + params.amount * generated;
    }

    float getFundamentalHz() const { return fundamentalHz; }

private:
    double sampleRate = 44100.0;
    float fundamentalHz = 0.0f;
    Params params;
    DspCommon::StereoIIR isolate1, isolate2, outputHighPass, dcBlocker;
    DspCommon::OnePoleEnvelope envelope;
};
