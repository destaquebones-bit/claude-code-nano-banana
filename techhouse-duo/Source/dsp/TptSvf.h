#pragma once
#include <juce_dsp/juce_dsp.h>

// Topology-preserving-transform state variable filter, in band-shelving (bell)
// configuration.
//
// From Zavalishin, "The Art of VA Filter Design" rev. 2.1.0 -- the free book by
// the author of Reaktor's filters. Two things in it apply directly here.
//
// 1. Section 3.11, on direct-form structures versus topology-preserving ones:
//    a direct form "contains a gain element at the output, the value of this
//    gain being approximately proportional to the cutoff", so it "will
//    particularly produce unsmoothed jumps in the output in response to jumps
//    in the cutoff value", whereas in the TPT structure "the cutoff jumps will
//    be smoothed by the integrator". This plugin retunes its filters onto the
//    harmonics of whatever note is playing, so every note change is a cutoff
//    jump -- precisely the case the direct form handles worst.
//
// 2. Section 4.4 gives the TPT SVF's per-sample form, and 4.7 gives the
//    band-shelving filter as input plus a scaled unit-gain bandpass:
//    H_BS = 1 + K * 2R * H_BP, with the boost in decibels being
//    G_dB = 20*log10(K + 1).
//
// The state is two integrator values rather than a coefficients object, so
// retuning writes three floats and allocates nothing -- which also removes the
// per-block allocation the JUCE Coefficients path performed on the audio
// thread.
//
// Known limitation, stated in the book (section 4.7, figure 4.20): in this
// simple form the bandwidth varies with the amount of boost or cut. Chapter 10
// gives a constant-bandwidth construction. For taming resonances that is
// acceptable -- a deeper cut being slightly wider is benign, and arguably
// desirable -- but it is a real difference from a textbook RBJ peaking filter,
// not an equivalence.
class TptSvfBell
{
public:
    void prepare (double sampleRateIn)
    {
        sampleRate = sampleRateIn;
        reset();
    }

    void reset()
    {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    // freq in Hz, q as the usual filter Q, gainDb negative to cut.
    void setBell (float freq, float q, float gainDb)
    {
        if (sampleRate <= 0.0)
            return;

        const float nyquistLimit = (float) (sampleRate * 0.49);
        const float f = juce::jlimit (5.0f, nyquistLimit, freq);

        // Prewarped integration gain: g = tan(pi * fc / fs). Prewarping is what
        // keeps the digital corner at the frequency actually asked for rather
        // than one progressively flattened toward Nyquist.
        g = std::tan (juce::MathConstants<float>::pi * f / (float) sampleRate);

        // The book parameterises damping as R, with 2R = 1/Q.
        const float r = 1.0f / (2.0f * juce::jmax (0.05f, q));
        twoR = 2.0f * r;

        g1 = twoR + g;
        d = 1.0f / (1.0f + twoR * g + g * g);

        // G_dB = 20*log10(K + 1)  =>  K = 10^(G_dB/20) - 1.
        k = juce::Decibels::decibelsToGain (gainDb) - 1.0f;
    }

    float processSample (float x)
    {
        // Section 4.4, solving the zero-delay feedback for the highpass:
        //   HP = (x - g1*s1 - s2) * d
        // then running both trapezoidal integrators.
        const float hp = (x - g1 * s1 - s2) * d;

        const float v1 = g * hp;
        const float bp = v1 + s1;
        s1 = bp + v1;

        const float v2 = g * bp;
        const float lp = v2 + s2;
        s2 = lp + v2;

        // Band-shelving: input plus K times the unit-gain bandpass, where the
        // unit-gain bandpass is 2R * BP.
        return x + k * (twoR * bp);
    }

private:
    double sampleRate = 44100.0;
    float g = 0.0f, twoR = 1.0f, g1 = 1.0f, d = 1.0f, k = 0.0f;
    float s1 = 0.0f, s2 = 0.0f;
};

// The same core, tapped at the bandpass output, for the tamer's detectors.
class TptSvfBandpass
{
public:
    void prepare (double sampleRateIn)
    {
        sampleRate = sampleRateIn;
        reset();
    }

    void reset()
    {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    void setBandpass (float freq, float q)
    {
        if (sampleRate <= 0.0)
            return;

        const float nyquistLimit = (float) (sampleRate * 0.49);
        const float f = juce::jlimit (5.0f, nyquistLimit, freq);

        g = std::tan (juce::MathConstants<float>::pi * f / (float) sampleRate);
        const float r = 1.0f / (2.0f * juce::jmax (0.05f, q));
        twoR = 2.0f * r;
        g1 = twoR + g;
        d = 1.0f / (1.0f + twoR * g + g * g);
    }

    // Returns the unit-gain bandpass, so a band's measured level does not
    // change just because its Q did.
    float processSample (float x)
    {
        const float hp = (x - g1 * s1 - s2) * d;

        const float v1 = g * hp;
        const float bp = v1 + s1;
        s1 = bp + v1;

        const float v2 = g * bp;
        const float lp = v2 + s2;
        s2 = lp + v2;

        juce::ignoreUnused (lp);
        return twoR * bp;
    }

private:
    double sampleRate = 44100.0;
    float g = 0.0f, twoR = 1.0f, g1 = 1.0f, d = 1.0f;
    float s1 = 0.0f, s2 = 0.0f;
};
