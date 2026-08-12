// Measures what happens to a bell filter's output when its centre frequency
// jumps -- which is what this plugin does every time the bassline changes note.
//
// Zavalishin, "The Art of VA Filter Design" section 3.11, on direct-form
// structures versus topology-preserving ones:
//
//   "In the time-invariant (unmodulated) case the performance of the direct
//    form filter [...] should be identical to the TPT filter [...] When the
//    cutoff is modulated, however, the performance will be different. [...]
//    the structure [...] contains a gain element at the output, the value of
//    this gain being approximately proportional to the cutoff [...] This will
//    particularly produce unsmoothed jumps in the output in response to jumps
//    in the cutoff value. In the [TPT] structure, on the other hand, the cutoff
//    jumps will be smoothed by the integrator."
//
// juce::dsp::IIR::Filter is a direct form, and SpectralTamer retunes it once
// per block onto k*f0 of the tracked note. So the claim, if it holds, predicts
// a step discontinuity in our output on every note change. This program checks
// whether that is actually true here and how big it is, before any rewrite.

#include "../Source/dsp/TptSvf.h"

#include <juce_dsp/juce_dsp.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <algorithm>

namespace
{
    constexpr double sr = 48000.0;
    constexpr int blockSize = 512;

    // A steady tone, unchanging throughout.
    //
    // The input deliberately does NOT change when the filter is retuned. An
    // earlier version of this experiment changed the note and the filter
    // together, which made the two effects impossible to separate: any step in
    // the output could have come from the signal rather than the filter. With
    // a constant input, every difference measured below is caused by the
    // retune alone, which is the thing under test.
    struct Signal { std::vector<float> samples; int changeAt; };

    Signal makeSteadyTone (float hz, float secondsEach)
    {
        const int n = (int) (secondsEach * sr);
        Signal s;
        s.samples.resize ((size_t) n * 2);
        s.changeAt = n;

        double phase = 0.0;
        for (int i = 0; i < 2 * n; ++i)
        {
            double v = 0.0;
            for (int k = 1; k <= 8; ++k)
                v += std::sin (phase * k) / k;
            s.samples[(size_t) i] = (float) (0.25 * v);
            phase += 2.0 * juce::MathConstants<double>::pi * hz / sr;
        }
        return s;
    }

    // Two numbers, both about the retune and nothing else.
    //
    // "jump" is the single-sample step at the instant of the retune measured
    // against the filter's own steady-state step just before it: 1.0 means the
    // waveform continued as if nothing happened, higher means a discontinuity
    // was injected -- a click.
    //
    // "settle" is how many milliseconds the output takes to come back within
    // 1% of its new steady-state peak, i.e. how long the filter rings or
    // lurches after being retuned.
    struct Result { double jump; double settleMs; };

    Result analyse (const std::vector<float>& out, int changeAt)
    {
        auto stepAt = [&] (int i) { return (double) std::abs (out[(size_t) i] - out[(size_t) (i - 1)]); };

        double beforeMax = 0.0;
        for (int i = changeAt - 400; i < changeAt; ++i)
            beforeMax = std::max (beforeMax, stepAt (i));

        double atChange = 0.0;
        for (int i = changeAt; i < changeAt + 4; ++i)
            atChange = std::max (atChange, stepAt (i));

        // Steady-state peak well after the retune.
        double finalPeak = 0.0;
        for (int i = changeAt + (int) (0.30 * sr); i < changeAt + (int) (0.45 * sr); ++i)
            finalPeak = std::max (finalPeak, (double) std::abs (out[(size_t) i]));

        int settled = changeAt;
        double running = 0.0;
        for (int i = changeAt; i < (int) out.size() - 1; ++i)
        {
            running = std::max (running * 0.9995, (double) std::abs (out[(size_t) i]));
            if (std::abs (running - finalPeak) > 0.01 * finalPeak)
                settled = i;
        }

        Result r;
        r.jump = beforeMax > 0.0 ? atChange / beforeMax : 0.0;
        r.settleMs = 1000.0 * (settled - changeAt) / sr;
        return r;
    }

    // Current approach: a JUCE peak filter, coefficients rebuilt per block.
    std::vector<float> runDirectForm (const Signal& sig, float freqA, float freqB, float q, float cutDb)
    {
        juce::dsp::IIR::Filter<float> filter;
        filter.prepare ({ sr, (juce::uint32) blockSize, 1 });

        std::vector<float> out (sig.samples.size());
        for (int start = 0; start < (int) sig.samples.size(); start += blockSize)
        {
            const float freq = start < sig.changeAt ? freqA : freqB;
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                sr, freq, q, juce::Decibels::decibelsToGain (-cutDb));

            const int count = std::min (blockSize, (int) sig.samples.size() - start);
            for (int i = 0; i < count; ++i)
                out[(size_t) (start + i)] = filter.processSample (sig.samples[(size_t) (start + i)]);
        }
        return out;
    }

    // Proposed approach: the book's TPT state-variable filter in band-shelving
    // configuration, retuned the same way at the same moments.
    std::vector<float> runTpt (const Signal& sig, float freqA, float freqB, float q, float cutDb)
    {
        TptSvfBell bell;
        bell.prepare (sr);

        std::vector<float> out (sig.samples.size());
        for (int start = 0; start < (int) sig.samples.size(); start += blockSize)
        {
            const float freq = start < sig.changeAt ? freqA : freqB;
            bell.setBell (freq, q, -cutDb);

            const int count = std::min (blockSize, (int) sig.samples.size() - start);
            for (int i = 0; i < count; ++i)
                out[(size_t) (start + i)] = bell.processSample (sig.samples[(size_t) (start + i)]);
        }
        return out;
    }
}

int main()
{
    std::printf ("Steady 55 Hz tone throughout. Only the bell's centre frequency is\n"
                  "moved, 220.0 Hz -> 261.6 Hz, at one block boundary -- the same retune\n"
                  "SpectralTamer performs when the bassline changes note.\n\n");

    const auto sig = makeSteadyTone (55.0f, 0.5f);

    struct Case { const char* name; float q; float cutDb; };
    const Case cases[] = {
        { "gentle  (Q 5.6, cut 3 dB)",  5.6f,  3.0f },
        { "typical (Q 5.6, cut 8 dB)",  5.6f,  8.0f },
        { "deep    (Q 5.6, cut 14 dB)", 5.6f, 14.0f },
        { "narrow  (Q 12,  cut 12 dB)", 12.0f, 12.0f },
    };

    std::printf ("%-28s %11s %10s %11s %10s\n", "", "DF jump", "DF settle", "TPT jump", "TPT settle");
    std::printf ("-------------------------------------------------------------------------\n");

    for (const auto& c : cases)
    {
        const auto df = analyse (runDirectForm (sig, 220.0f, 261.64f, c.q, c.cutDb), sig.changeAt);
        const auto tpt = analyse (runTpt (sig, 220.0f, 261.64f, c.q, c.cutDb), sig.changeAt);
        std::printf ("%-28s %10.2fx %8.0f ms %10.2fx %8.0f ms\n",
                      c.name, df.jump, df.settleMs, tpt.jump, tpt.settleMs);
    }

    std::printf ("\njump   1.00x = the waveform carried on as if nothing happened.\n"
                  "       higher  = a step was injected into the signal, i.e. a click.\n"
                  "settle time for the output to reach its new steady state.\n");
    return 0;
}

// ---------------------------------------------------------------------------
// Second experiment: does the antiderivative antialiasing of section 6.13
// actually reduce aliasing for the curve this plugin uses?
//
// The book is careful about this -- "reduces aliasing by a noticeable amount"
// rather than removes it, and "44.1kHz would be usually insufficient". Worth
// measuring rather than assuming, since the technique costs an extra log and
// exp per sample.

namespace
{
    constexpr float kDriveGain = 8.0f;   // drive at maximum
    constexpr float kDriveBias = 0.45f;

    inline float curve (float x) { return std::tanh (x * kDriveGain + kDriveBias); }

    inline float curveAntiderivative (float x)
    {
        const float u = x * kDriveGain + kDriveBias;
        const float a = std::abs (u);
        return (a + std::log1p (std::exp (-2.0f * a)) - 0.6931472f) / kDriveGain;
    }

    double aliasingDb (bool useAdaa, double f0)
    {
        const int n = (int) sr;
        const int skip = (int) (0.4 * sr);
        std::vector<float> out ((size_t) n);

        float prevX = 0.0f, prevF = curveAntiderivative (0.0f);
        for (int i = 0; i < n; ++i)
        {
            const float x = 0.5f * (float) std::sin (2.0 * juce::MathConstants<double>::pi * f0 * i / sr);
            if (useAdaa)
            {
                const float f = curveAntiderivative (x);
                const float dx = x - prevX;
                out[(size_t) i] = std::abs (dx) < 1.0e-5f ? curve (0.5f * (x + prevX)) : (f - prevF) / dx;
                prevX = x;
                prevF = f;
            }
            else
            {
                out[(size_t) i] = curve (x);
            }
        }

        auto magAt = [&] (double hz)
        {
            double re = 0.0, im = 0.0;
            for (int i = skip; i < n; ++i)
            {
                const double t = i / sr;
                re += out[(size_t) i] * std::cos (2.0 * juce::MathConstants<double>::pi * hz * t);
                im += out[(size_t) i] * std::sin (2.0 * juce::MathConstants<double>::pi * hz * t);
            }
            return 2.0 * std::sqrt (re * re + im * im) / (n - skip);
        };

        const double harmonic = magAt (f0 * 3.0);

        // Correct folding: a partial above Nyquist wraps around the sample rate
        // and then reflects about Nyquist. An earlier version of this probed
        // |fs - k*f0|, which is only the alias for partials between Nyquist and
        // fs -- for anything else it lands back on a real harmonic, so it
        // measured the harmonic and reported it as aliasing.
        auto aliasOf = [] (double h)
        {
            double m = std::fmod (h, sr);
            if (m > sr * 0.5) m = sr - m;
            return m;
        };

        double worstFold = 0.0;
        for (int k = 2; k <= 40; ++k)
        {
            const double h = f0 * k;
            if (h <= sr * 0.5)
                continue;                     // not folded, it is a real partial
            const double a = aliasOf (h);
            if (a < 20.0)
                continue;
            // Skip anything landing on a genuine harmonic, where the two are
            // indistinguishable by this measurement.
            const double nearestHarmonic = std::round (a / f0) * f0;
            if (std::abs (a - nearestHarmonic) < f0 * 0.15)
                continue;
            worstFold = std::max (worstFold, magAt (a));
        }
        return 20.0 * std::log10 (std::max (worstFold, 1e-12) / std::max (harmonic, 1e-12));
    }
}

struct RunAliasing
{
    RunAliasing()
    {
        std::printf ("\n\nAliasing of the kick Drive curve at maximum, as folded-partial level\n"
                      "relative to a genuine harmonic. Lower is better.\n\n");
        std::printf ("%-14s %14s %14s %12s\n", "input", "plain tanh", "with ADAA", "improvement");
        std::printf ("------------------------------------------------------------\n");
        for (double f0 : { 200.0, 800.0, 1500.0, 3000.0, 5000.0 })
        {
            const double plain = aliasingDb (false, f0);
            const double adaa = aliasingDb (true, f0);
            std::printf ("%9.0f Hz %12.1f dB %12.1f dB %10.1f dB\n", f0, plain, adaa, plain - adaa);
        }
    }
};
static RunAliasing runAliasingAtExit;
