// What does the cut cascade do to the bass in the time domain?
//
// SpectralTamer applies its cuts as up to ten peaking biquads in series
// (SpectralTamer::processSample runs x through cut[0..numBands-1]). Every one
// of them is a minimum-phase filter, so each contributes phase shift, and phase
// shift that varies with frequency is group delay: different partials of the
// same note come out at different times. Julius Smith's "Introduction to
// Digital Filters" is the standard reference for this -- phase delay is what a
// single sinusoid experiences, group delay is what the *envelope* around a
// group of partials experiences, and it is the second one that decides whether
// a plucked note still sounds plucked.
//
// This matters here more than it would in a normal EQ for three reasons:
//
//   1. Group delay scales roughly as Q/f0, and this is a bass plugin. The same
//      filter shape that costs 0.2 ms at 2 kHz costs ten times that at 200 Hz.
//   2. The Q rises with band index by design (1.4 * k, up to 14), so the upper
//      bands are far narrower than a normal EQ band -- and narrow means slow.
//   3. Ten of them are in series, and group delay adds.
//
// None of that has ever been measured on this code, and the alternatives to it
// (fewer bands, lower Q, linear phase) all cost something real -- linear phase
// in particular costs latency and pre-ringing, which on a bass is a cure worse
// than the disease. So the question has to be settled by measurement.
//
// Group delay on its own is easy to misread: the cut bands show the largest
// numbers, but they are also the bands whose level has just been pulled down,
// so their contribution to the output is small. So the note is filtered twice --
//
//   min-phase   the real cascade, as shipped;
//   zero-phase  a filter with EXACTLY the same magnitude response and no phase
//               shift whatsoever, built by taking |H(f)| from the cascade's own
//               impulse response and applying it in the frequency domain.
//
// Both remove identical amounts of energy at identical frequencies, so every
// difference between the two outputs is caused by phase alone.
//
// HOW TO READ THAT COMPARISON, and it is not symmetric. A small difference
// would have proved the phase response is inaudible. A large one proves only
// that the two waveforms are not the same signal -- it does NOT prove they
// sound different, because the ear is largely insensitive to the relative
// phases of the harmonics of a sustained tone (Ohm's phase law; the classic
// demonstration is that a sawtooth and its phase-scrambled twin are hard to
// tell apart). A waveform can be visibly transformed and perceptually
// identical, so "the samples differ" is an upper bound on audibility, never a
// measurement of it.
//
// That is why the decision rests on the two columns that ARE perceptually
// meaningful regardless of phase sensitivity:
//
//   the group delay at the fundamental, which is when the note arrives, and
//   the crest factor, which is what a limiter downstream actually reacts to.
//
// The zero-phase column doubles as the test of the obvious "upgrade": linear
// phase. If linear phase left the note's crest factor closer to dry than the
// shipped cascade does, that would be an argument for paying its cost in
// latency and pre-ringing. If it does not, the argument collapses.

#include "../Source/dsp/DspCommon.h"

#include <juce_dsp/juce_dsp.h>
#include <cstdio>
#include <cmath>
#include <vector>
#include <array>
#include <algorithm>
#include <complex>
#include <string>

namespace
{
    constexpr double sr = 48000.0;
    // Not constant: the second table sweeps it, because a conclusion drawn from
    // one root note is a conclusion about one root note. Group delay scales with
    // 1/f0, so a low root is the hard case and has to be included.
    double f0 = 55.0;                  // A1, a common tech house bass root
    constexpr int numPartials = 10;

    constexpr int fftOrder = 17;
    constexpr int fftSize = 1 << fftOrder;      // 131072
    constexpr int irLength = 1 << 15;           // 32768, well past full decay
    constexpr int noteLength = (int) (1.2 * sr);
    // Linear convolution needs noteLength + irLength < fftSize, or the tail
    // wraps around and lands back on the attack.
    static_assert (noteLength + irLength < fftSize, "FFT too short, output would wrap");

    // Rebuilt here exactly as SpectralTamer does it, so this measures the real
    // filter shapes rather than a stand-in:
    //   setBandCentres    q = jlimit (1, 14, 1.4 * (index + 1))
    //   updateReductions  cut Q = jmax (0.7, q * 0.85), gain = -reductionDb
    //
    // juce::dsp::IIR::Filter is non-copyable, so the filters live in a
    // fixed-size array and are configured in place rather than pushed back.
    struct Cascade
    {
        std::array<juce::dsp::IIR::Filter<float>, numPartials> filters;
        int count = 0;

        Cascade (const std::vector<float>& cutsDb)
            : count (juce::jmin ((int) cutsDb.size(), (int) numPartials))
        {
            for (int b = 0; b < count; ++b)
            {
                const float centre = (float) (f0 * (b + 1));
                const float q = juce::jlimit (1.0f, 14.0f, 1.4f * (float) (b + 1));

                filters[(size_t) b].prepare ({ sr, 512, 1 });
                filters[(size_t) b].coefficients
                    = juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                        sr, centre, juce::jmax (0.7f, q * 0.85f),
                        juce::Decibels::decibelsToGain (-cutsDb[(size_t) b]));
            }
        }

        float processSample (float x)
        {
            for (int b = 0; b < count; ++b)
                x = filters[(size_t) b].processSample (x);
            return x;
        }
    };

    std::vector<float> impulseResponse (const std::vector<float>& cutsDb)
    {
        Cascade c (cutsDb);
        std::vector<float> h ((size_t) irLength);
        for (int i = 0; i < irLength; ++i)
            h[(size_t) i] = c.processSample (i == 0 ? 1.0f : 0.0f);
        return h;
    }

    std::vector<float> runMinPhase (const std::vector<float>& in, const std::vector<float>& cutsDb)
    {
        Cascade c (cutsDb);
        std::vector<float> out (in.size());
        for (size_t i = 0; i < in.size(); ++i)
            out[i] = c.processSample (in[i]);
        return out;
    }

    // Same magnitude response, zero phase. |H| is measured from the cascade's
    // own impulse response, so this is not an approximation of the cascade --
    // it is the cascade's exact magnitude curve with the phase discarded.
    std::vector<float> runZeroPhase (const std::vector<float>& in, const std::vector<float>& cutsDb)
    {
        juce::dsp::FFT fft (fftOrder);

        const std::complex<float> zero (0.0f, 0.0f);
        std::vector<std::complex<float>> H ((size_t) fftSize, zero), X ((size_t) fftSize, zero);
        {
            const auto h = impulseResponse (cutsDb);
            std::vector<std::complex<float>> pad ((size_t) fftSize, std::complex<float> (0.0f, 0.0f));
            for (size_t i = 0; i < h.size(); ++i)
                pad[i] = { h[i], 0.0f };
            fft.perform (pad.data(), H.data(), false);
        }
        {
            std::vector<std::complex<float>> pad ((size_t) fftSize, std::complex<float> (0.0f, 0.0f));
            for (size_t i = 0; i < in.size(); ++i)
                pad[i] = { in[i], 0.0f };
            fft.perform (pad.data(), X.data(), false);
        }

        for (int k = 0; k < fftSize; ++k)
            X[(size_t) k] *= std::abs (H[(size_t) k]);   // real, non-negative: zero phase

        std::vector<std::complex<float>> y ((size_t) fftSize, std::complex<float> (0.0f, 0.0f));
        fft.perform (X.data(), y.data(), true);

        std::vector<float> out (in.size());
        for (size_t i = 0; i < in.size(); ++i)
            out[i] = y[i].real();
        return out;
    }

    std::complex<double> responseAt (const std::vector<float>& h, double hz)
    {
        std::complex<double> acc (0.0, 0.0);
        const double w = 2.0 * juce::MathConstants<double>::pi * hz / sr;
        for (int i = 0; i < (int) h.size(); ++i)
            acc += (double) h[(size_t) i] * std::polar (1.0, -w * i);
        return acc;
    }

    // Group delay by central difference of the phase. delta is small enough
    // that the phase change between the two probes stays well under pi, so no
    // unwrapping is needed -- at 20 ms of delay and delta = 1 Hz the step is
    // about 0.25 rad.
    double groupDelayMs (const std::vector<float>& h, double hz)
    {
        constexpr double delta = 1.0;
        const double lo = std::arg (responseAt (h, hz - delta));
        const double hi = std::arg (responseAt (h, hz + delta));

        double d = hi - lo;
        while (d >  juce::MathConstants<double>::pi) d -= 2.0 * juce::MathConstants<double>::pi;
        while (d < -juce::MathConstants<double>::pi) d += 2.0 * juce::MathConstants<double>::pi;

        return -1000.0 * d / (2.0 * juce::MathConstants<double>::pi * 2.0 * delta);
    }

    // A synth bass note: ten partials at 1/k, a 2 ms attack and a long decay.
    // Sharper than most real basslines, which is the point -- if a transient
    // survives this it survives the softer ones.
    std::vector<float> makeNote()
    {
        std::vector<float> s ((size_t) noteLength);
        for (int i = 0; i < noteLength; ++i)
        {
            const double t = i / sr;
            const double env = (1.0 - std::exp (-t / 0.002)) * std::exp (-t / 0.9);
            double v = 0.0;
            for (int k = 1; k <= numPartials; ++k)
                v += std::sin (2.0 * juce::MathConstants<double>::pi * f0 * k * t) / k;
            s[(size_t) i] = (float) (0.3 * env * v);
        }
        return s;
    }

    double crestDb (const std::vector<float>& s, int from, int to)
    {
        double peak = 0.0, sum = 0.0;
        for (int i = from; i < to; ++i)
        {
            peak = std::max (peak, (double) std::abs (s[(size_t) i]));
            sum += (double) s[(size_t) i] * s[(size_t) i];
        }
        const double rms = std::sqrt (sum / std::max (1, to - from));
        return 20.0 * std::log10 (std::max (peak, 1e-12) / std::max (rms, 1e-12));
    }

    // Largest sample-by-sample difference between two signals, in dB relative
    // to the peak of the reference. Applied to min-phase vs zero-phase output
    // it bounds how much of the note the phase response is responsible for
    // reshaping -- an upper bound on audibility, not a measure of it, for the
    // reason set out at the top of this file.
    double maxDiffDb (const std::vector<float>& a, const std::vector<float>& b)
    {
        double peak = 0.0, diff = 0.0;
        for (size_t i = 0; i < a.size(); ++i)
        {
            peak = std::max (peak, (double) std::abs (a[i]));
            diff = std::max (diff, (double) std::abs (a[i] - b[i]));
        }
        return 20.0 * std::log10 (std::max (diff, 1e-12) / std::max (peak, 1e-12));
    }

    void report (const char* name, const std::vector<float>& cutsDb)
    {
        const auto h = impulseResponse (cutsDb);

        double lo = 1e9, hi = -1e9;
        std::printf ("  %-20s", name);
        for (int k = 1; k <= numPartials; ++k)
        {
            const double g = groupDelayMs (h, f0 * k);
            lo = std::min (lo, g);
            hi = std::max (hi, g);
            std::printf ("%6.1f", g);
        }

        const auto dry = makeNote();
        const auto minPh = runMinPhase (dry, cutsDb);
        const auto zeroPh = runZeroPhase (dry, cutsDb);
        const int from = (int) (0.05 * sr), to = (int) (0.45 * sr);

        std::printf ("  |%6.1f  %5.2f %5.2f %5.2f  %8.1f\n",
                      hi - lo,
                      crestDb (dry, from, to), crestDb (minPh, from, to), crestDb (zeroPh, from, to),
                      maxDiffDb (minPh, zeroPh));
    }
}

int main()
{
    // Self-check first: with every cut at 0 dB the filters are unity, so the
    // zero-phase path must return the input untouched. If JUCE's inverse FFT
    // scaling were not what this code assumes, it would show up here as a large
    // error rather than silently biasing every row below.
    {
        const std::vector<float> unity (numPartials, 0.0f);
        const auto dry = makeNote();
        const auto err = maxDiffDb (dry, runZeroPhase (dry, unity));
        std::printf ("FFT round-trip self-check (unity filter): %.1f dB error\n", err);
        if (err > -100.0)
            std::printf ("  WARNING: round-trip is not clean, the numbers below are unreliable.\n");
    }

    std::printf (
        "\nGroup delay through the SpectralTamer cut cascade, 55 Hz bass note.\n"
        "Ten peaking biquads in series at k*55 Hz, Q = 1.4*k * 0.85.\n\n"
        "Columns 1-10: group delay in ms at each partial (55..550 Hz). Negative\n"
        "at a cut band is expected -- a dip advances phase where a peak delays it.\n"
        "sprd  = max-min group delay across the partials.\n"
        "crest = peak-to-RMS in dB: dry, min-phase (shipped), zero-phase.\n"
        "phase = min-phase vs zero-phase difference, dB below the note's peak.\n"
        "        Same magnitude response in both, so this column is phase ALONE.\n\n");

    std::printf ("  %-20s%6s%6s%6s%6s%6s%6s%6s%6s%6s%6s  |%6s  %17s  %8s\n",
                  "profile", "55", "110", "165", "220", "275", "330", "385", "440", "495", "550",
                  "sprd", "crest dry/min/zero", "phase");
    std::printf ("  %s\n", std::string (122, '-').c_str());

    // Band 0 is never cut (protectFirstBand), so every profile leaves it at 0.
    report ("one band -4 dB",     { 0, 0, 0, 4, 0, 0, 0, 0, 0, 0 });
    report ("two bands -8 dB",    { 0, 0, 8, 8, 0, 0, 0, 0, 0, 0 });
    report ("three bands -12 dB", { 0, 0, 12, 12, 12, 0, 0, 0, 0, 0 });
    report ("all bands -6 dB",    { 0, 6, 6, 6, 6, 6, 6, 6, 6, 6 });
    report ("worst case -12 dB",  { 0, 12, 12, 12, 12, 12, 12, 12, 12, 12 });

    // Second table: the two decision-relevant numbers across the range of root
    // notes this plugin actually sees, so neither conclusion rests on one pitch.
    // The cut profile is held at the realistic "two bands -8 dB" setting.
    std::printf (
        "\n\nAcross root notes, at the realistic two-bands -8 dB setting.\n"
        "t(f0) = group delay at the fundamental, i.e. how much later the note\n"
        "        lands. crest is dry -> min-phase (shipped) -> zero-phase.\n\n");
    std::printf ("  %-12s%10s%10s%10s%10s\n", "root", "t(f0) ms", "crest dry", "min-phase", "zero-ph");
    std::printf ("  %s\n", std::string (52, '-').c_str());

    const std::vector<float> realistic { 0, 0, 8, 8, 0, 0, 0, 0, 0, 0 };
    struct Root { const char* name; double hz; };
    for (const auto& r : { Root { "E1", 41.20 }, Root { "A1", 55.00 },
                            Root { "D2", 73.42 }, Root { "G2", 98.00 },
                            Root { "C3", 130.81 } })
    {
        f0 = r.hz;
        const auto h = impulseResponse (realistic);
        const auto dry = makeNote();
        const auto minPh = runMinPhase (dry, realistic);
        const auto zeroPh = runZeroPhase (dry, realistic);
        const int from = (int) (0.05 * sr), to = (int) (0.45 * sr);

        std::printf ("  %-4s%7.1f Hz%10.2f%10.2f%10.2f%10.2f\n",
                      r.name, r.hz, groupDelayMs (h, f0),
                      crestDb (dry, from, to), crestDb (minPh, from, to), crestDb (zeroPh, from, to));
    }

    return 0;
}
