// Offline checks for the parts of the DSP whose correctness is not obvious by
// inspection. Everything downstream of the pitch tracker (the harmonic grid,
// note levelling, the exciter) is wrong if the tracker is wrong, so that gets
// the most attention here.
//
// Build and run:
//   cmake -B build-tests -S tests -DCMAKE_BUILD_TYPE=Release && cmake --build build-tests && ./build-tests/DspTests

#include "../Source/dsp/PitchTracker.h"
#include "../Source/dsp/SpectralTamer.h"
#include "../Source/dsp/HarmonicExciter.h"
#include "../Source/dsp/NoteLevelCompensator.h"
#include "../Source/dsp/KickDucker.h"
#include "../Source/dsp/KickShaper.h"
#include "../Source/dsp/LinkBus.h"

#include <cstdio>
#include <cmath>
#include <vector>
#include <string>

namespace
{
    int failures = 0;

    void check (bool condition, const std::string& what, const std::string& detail = {})
    {
        std::printf ("%-58s %s%s%s\n", what.c_str(), condition ? "PASS" : "FAIL",
                      detail.empty() ? "" : "  ", detail.c_str());
        if (! condition)
            ++failures;
    }

    constexpr double sr = 48000.0;

    // A sawtooth-ish bass note: fundamental plus a decaying harmonic series,
    // which is what the tracker will actually see on a real bassline.
    std::vector<float> makeBassNote (float hz, float seconds, int harmonics = 8, float extraHarmonic = 0.0f, int extraIndex = 4)
    {
        const int n = (int) (seconds * sr);
        std::vector<float> out ((size_t) n, 0.0f);
        for (int i = 0; i < n; ++i)
        {
            const double t = i / sr;
            double v = 0.0;
            for (int k = 1; k <= harmonics; ++k)
            {
                double amp = 1.0 / k;
                if (extraHarmonic > 0.0f && k == extraIndex)
                    amp *= (1.0 + extraHarmonic);
                v += amp * std::sin (2.0 * M_PI * hz * k * t);
            }
            out[(size_t) i] = (float) (0.25 * v);
        }
        return out;
    }

    // Mirrors how the processor actually drives the tamer: per-sample detector
    // and audio, control-rate update once per block. Running it any other way
    // hides block-size bugs, which is exactly how the smoothing-rate bug in
    // OnePoleEnvelope slipped through until these tests were written.
    void runInBlocks (SpectralTamer& tamer, const std::vector<float>& signal, float confidence, int blockSize = 512)
    {
        const int n = (int) signal.size();
        for (int start = 0; start < n; start += blockSize)
        {
            const int count = std::min (blockSize, n - start);
            for (int i = 0; i < count; ++i)
            {
                tamer.processDetector (signal[(size_t) (start + i)]);
                tamer.processSample (0, signal[(size_t) (start + i)]);
            }
            tamer.updateReductions (confidence, count);
        }
    }

    float trackPitch (PitchTracker& tracker, const std::vector<float>& signal, float& confidenceOut)
    {
        for (float s : signal)
            tracker.processSample (s);
        confidenceOut = tracker.getConfidence();
        return tracker.getFundamentalHz();
    }

    void testPitchTracker()
    {
        std::printf ("\n-- PitchTracker --\n");

        // Across the range a tech house bassline actually occupies.
        const float notes[] = { 41.2f, 49.0f, 55.0f, 65.4f, 82.4f, 110.0f, 146.8f };
        for (float hz : notes)
        {
            PitchTracker tracker;
            tracker.prepare (sr);
            float confidence = 0.0f;
            const float detected = trackPitch (tracker, makeBassNote (hz, 1.0f), confidence);
            const float centsOff = detected > 0.0f ? 1200.0f * std::log2 (detected / hz) : 9999.0f;
            check (std::abs (centsOff) < 50.0f && confidence > 0.5f,
                    "tracks " + std::to_string ((int) hz) + " Hz within 50 cents",
                    detected > 0.0f
                        ? "got " + std::to_string (detected) + " Hz (" + std::to_string ((int) centsOff) + " cents)"
                        : "no lock");
        }

        // Silence must not produce a confident answer -- a wrong note here would
        // drive the whole harmonic grid onto nonsense frequencies.
        {
            PitchTracker tracker;
            tracker.prepare (sr);
            std::vector<float> silence ((size_t) sr, 0.0f);
            float confidence = 1.0f;
            trackPitch (tracker, silence, confidence);
            check (confidence < 0.1f, "reports no confidence on silence",
                    "confidence " + std::to_string (confidence));
        }

        // White-ish noise is not periodic and must not read as a note.
        {
            PitchTracker tracker;
            tracker.prepare (sr);
            std::vector<float> noise ((size_t) sr);
            uint32_t seed = 12345;
            for (auto& v : noise)
            {
                seed = seed * 1664525u + 1013904223u;
                v = 0.2f * ((float) (seed >> 8) / 8388608.0f - 1.0f);
            }
            float confidence = 1.0f;
            trackPitch (tracker, noise, confidence);
            check (confidence < 0.6f, "low confidence on noise",
                    "confidence " + std::to_string (confidence));
        }

        // Octave errors are the classic autocorrelation failure; YIN's
        // cumulative-mean step exists to prevent them, so verify it works.
        {
            PitchTracker tracker;
            tracker.prepare (sr);
            float confidence = 0.0f;
            const float detected = trackPitch (tracker, makeBassNote (55.0f, 1.0f, 10), confidence);
            const bool octaveDown = detected > 0.0f && std::abs (detected - 27.5f) < 3.0f;
            const bool octaveUp = detected > 0.0f && std::abs (detected - 110.0f) < 6.0f;
            check (! octaveDown && ! octaveUp, "no octave error on a rich 55 Hz note",
                    "got " + std::to_string (detected) + " Hz");
        }

        // The reported latency must be a real, usable number: the processor
        // delays audio by exactly this much to align pitch with audio.
        {
            PitchTracker tracker;
            tracker.prepare (sr);
            const int latency = tracker.getAnalysisLatencySamples();
            check (latency > 0 && latency < (int) (sr * 0.12),
                    "analysis latency is sane (<120ms)",
                    std::to_string (latency) + " samples = "
                        + std::to_string ((int) (1000.0 * latency / sr)) + " ms");
        }
    }

    void testSpectralTamer()
    {
        std::printf ("\n-- SpectralTamer --\n");

        juce::dsp::ProcessSpec spec { sr, 512, 2 };

        // A smooth harmonic series has no resonance, so nothing should be cut.
        // This is the property that makes the detector level-independent.
        {
            SpectralTamer tamer;
            tamer.prepare (spec);
            SpectralTamer::Params p;
            p.depth = 1.0f;
            p.toleranceDb = 3.0f;
            tamer.setParams (p);

            const float f0 = 55.0f;
            float grid[8];
            for (int k = 0; k < 8; ++k)
                grid[k] = f0 * (k + 1);
            tamer.setBandCentres (grid, 8);

            runInBlocks (tamer, makeBassNote (f0, 1.0f, 8), 1.0f);
            check (tamer.getMaxReductionDb() < 3.0f, "leaves a smooth harmonic series alone",
                    "max cut " + std::to_string (tamer.getMaxReductionDb()) + " dB");
        }

        // Now boost one harmonic well above its neighbours: that is a resonance
        // by definition and must be detected on the right band.
        {
            SpectralTamer tamer;
            tamer.prepare (spec);
            SpectralTamer::Params p;
            p.depth = 1.0f;
            p.toleranceDb = 3.0f;
            p.maxCutDb = 18.0f;
            tamer.setParams (p);

            const float f0 = 55.0f;
            float grid[8];
            for (int k = 0; k < 8; ++k)
                grid[k] = f0 * (k + 1);
            tamer.setBandCentres (grid, 8);

            runInBlocks (tamer, makeBassNote (f0, 1.5f, 8, 6.0f, 4), 1.0f); // 4th harmonic +~17dB

            const int worst = tamer.getWorstBandIndex();
            check (tamer.getMaxReductionDb() > 4.0f, "detects a boosted harmonic",
                    "max cut " + std::to_string (tamer.getMaxReductionDb()) + " dB");
            check (worst == 3, "attributes it to the 4th harmonic",
                    "worst band index " + std::to_string (worst));
        }

        // The same signal must reach the same reduction whatever buffer size the
        // host happens to use. This is the regression test for the control-rate
        // smoothing bug: before the fix, a 512-sample block ran the release 512x
        // too slowly and a 64-sample block 64x too slowly, so these diverged.
        {
            auto measureAt = [] (int blockSize)
            {
                juce::dsp::ProcessSpec s2 { sr, (juce::uint32) blockSize, 2 };
                SpectralTamer t;
                t.prepare (s2);
                SpectralTamer::Params p;
                p.depth = 1.0f;
                p.toleranceDb = 3.0f;
                p.maxCutDb = 18.0f;
                t.setParams (p);
                float grid[8];
                for (int k = 0; k < 8; ++k)
                    grid[k] = 55.0f * (k + 1);
                t.setBandCentres (grid, 8);
                runInBlocks (t, makeBassNote (55.0f, 1.5f, 8, 6.0f, 4), 1.0f, blockSize);
                return t.getMaxReductionDb();
            };

            const float small = measureAt (64);
            const float large = measureAt (1024);
            check (std::abs (small - large) < 1.0f, "reduction is independent of host block size",
                    std::to_string (small) + " dB @64 vs " + std::to_string (large) + " dB @1024");
        }

        // With zero confidence the note-aware pass must do nothing at all --
        // this is the graceful-degradation promise.
        {
            SpectralTamer tamer;
            tamer.prepare (spec);
            SpectralTamer::Params p;
            p.depth = 1.0f;
            tamer.setParams (p);
            float grid[8];
            for (int k = 0; k < 8; ++k)
                grid[k] = 55.0f * (k + 1);
            tamer.setBandCentres (grid, 8);

            runInBlocks (tamer, makeBassNote (55.0f, 1.0f, 8, 6.0f, 4), 0.0f);
            check (tamer.getMaxReductionDb() < 0.01f, "no action when confidence is zero",
                    std::to_string (tamer.getMaxReductionDb()) + " dB");
        }
    }

    // Measures the magnitude at a given frequency via a Goertzel-style
    // correlation, used to verify the exciter puts harmonics where it claims.
    double magnitudeAt (const std::vector<float>& signal, double hz, int skip)
    {
        double re = 0.0, im = 0.0;
        const int n = (int) signal.size();
        for (int i = skip; i < n; ++i)
        {
            const double t = i / sr;
            re += signal[(size_t) i] * std::cos (2.0 * M_PI * hz * t);
            im += signal[(size_t) i] * std::sin (2.0 * M_PI * hz * t);
        }
        const double count = n - skip;
        return 2.0 * std::sqrt (re * re + im * im) / count;
    }

    void testHarmonicExciter()
    {
        std::printf ("\n-- HarmonicExciter --\n");

        juce::dsp::ProcessSpec spec { sr, 512, 2 };
        const float f0 = 55.0f;

        HarmonicExciter exciter;
        exciter.prepare (spec);
        exciter.setFundamental (f0);
        HarmonicExciter::Params p;
        p.amount = 1.0f;
        p.balance = 0.5f;
        exciter.setParams (p);

        // Feed a pure sine: any energy at non-integer multiples afterwards would
        // be intermodulation, which is exactly what this design avoids.
        const int n = (int) (2.0 * sr);
        std::vector<float> input ((size_t) n), output ((size_t) n);
        for (int i = 0; i < n; ++i)
            input[(size_t) i] = 0.4f * (float) std::sin (2.0 * M_PI * f0 * i / sr);

        for (int i = 0; i < n; ++i)
            output[(size_t) i] = exciter.processSample (0, input[(size_t) i]);

        const int skip = (int) (0.75 * sr); // let the envelope follower settle
        const double h2 = magnitudeAt (output, f0 * 2, skip);
        const double h3 = magnitudeAt (output, f0 * 3, skip);
        const double inharmonic = magnitudeAt (output, f0 * 2.5, skip);

        check (h2 > 0.01 && h3 > 0.01, "generates 2nd and 3rd harmonics",
                "h2 " + std::to_string (h2) + ", h3 " + std::to_string (h3));
        check (inharmonic < 0.1 * std::min (h2, h3), "nothing at non-integer multiples",
                "2.5*f0 " + std::to_string (inharmonic));

        // Silence in, silence out -- the normalisation divides by an envelope,
        // so this guards against it blowing up on a quiet passage.
        HarmonicExciter quiet;
        quiet.prepare (spec);
        quiet.setFundamental (f0);
        quiet.setParams (p);
        float peak = 0.0f;
        for (int i = 0; i < (int) sr; ++i)
            peak = std::max (peak, std::abs (quiet.processSample (0, 0.0f)));
        check (peak < 1.0e-6f, "stays silent on silence", "peak " + std::to_string (peak));
    }

    void testNoteLevelling()
    {
        std::printf ("\n-- NoteLevelCompensator --\n");

        NoteLevelCompensator comp;
        comp.prepare (sr);
        NoteLevelCompensator::Params p;
        p.amount = 1.0f;
        comp.setParams (p);

        // Two notes, one 6 dB louder: the levelling should learn both and pull
        // them toward each other.
        const float quietHz = 55.0f, loudHz = 65.4f;
        auto feedNote = [&] (float hz, float amplitude, float seconds)
        {
            const int n = (int) (seconds * sr);
            for (int i = 0; i < n; ++i)
            {
                const float s = amplitude * (float) std::sin (2.0 * M_PI * hz * i / sr);
                comp.processSample (s, hz, 0.9f);
            }
        };

        for (int repeat = 0; repeat < 6; ++repeat)
        {
            feedNote (quietHz, 0.1f, 0.35f);
            feedNote (loudHz, 0.2f, 0.35f);
        }

        check (comp.getLearnedNoteCount() >= 2, "learns both notes",
                std::to_string (comp.getLearnedNoteCount()) + " notes");
        check (comp.getNoteSpreadDb() > 3.0f, "reports the level spread it found",
                std::to_string (comp.getNoteSpreadDb()) + " dB");

        // The loud note should end up being pulled down.
        feedNote (loudHz, 0.2f, 0.5f);
        check (comp.getCompensationDb() < 0.0f, "attenuates the loud note",
                std::to_string (comp.getCompensationDb()) + " dB");

        feedNote (quietHz, 0.1f, 0.5f);
        check (comp.getCompensationDb() > 0.0f, "lifts the quiet note",
                std::to_string (comp.getCompensationDb()) + " dB");
    }

    // A short kick with a fast pitch drop: energy concentrated low, very little
    // above a few hundred Hz -- the shape the ducker is supposed to trace.
    std::vector<float> makeKick (float seconds)
    {
        const int n = (int) (seconds * sr);
        std::vector<float> out ((size_t) n, 0.0f);
        for (int i = 0; i < n; ++i)
        {
            const double t = i / sr;
            const double phase = 2.0 * M_PI * (55.0 * t + 40.0 * (1.0 - std::exp (-t * 25.0)) / 25.0);
            out[(size_t) i] = (float) (0.9 * std::exp (-t * 14.0) * std::sin (phase));
        }
        return out;
    }

    void testKickDucker()
    {
        std::printf ("\n-- KickDucker --\n");

        juce::dsp::ProcessSpec spec { sr, 512, 2 };

        auto run = [&] (KickDucker& ducker, const std::vector<float>& signal, int blockSize = 128)
        {
            const int n = (int) signal.size();
            for (int start = 0; start < n; start += blockSize)
            {
                const int count = std::min (blockSize, n - start);
                for (int i = 0; i < count; ++i)
                    ducker.processSidechainSample (signal[(size_t) (start + i)]);
                ducker.updateDucking (count);
            }
        };

        // Silence must duck nothing, or the bass is permanently thinned.
        {
            KickDucker ducker;
            ducker.prepare (spec);
            KickDucker::Params p;
            p.amount = 1.0f;
            ducker.setParams (p);
            run (ducker, std::vector<float> ((size_t) (0.2 * sr), 0.0f));
            check (ducker.getMaxDuckDb() < 0.1f, "no ducking on silence",
                    std::to_string (ducker.getMaxDuckDb()) + " dB");
        }

        // The whole justification for per-band ducking is that it does NOT duck
        // everything equally. Before the shape-based rewrite every band pinned
        // at the maximum, which is full-band ducking in disguise -- this locks
        // the corrected behaviour in.
        {
            KickDucker ducker;
            ducker.prepare (spec);
            KickDucker::Params p;
            p.amount = 0.7f;
            p.thresholdDb = -40.0f;
            p.maxDuckDb = 12.0f;
            ducker.setParams (p);
            run (ducker, makeKick (0.12f));

            float lowSum = 0.0f, highSum = 0.0f;
            float lo = 1.0e9f, hi = -1.0e9f;
            for (int b = 0; b < Link::numBands; ++b)
            {
                const float d = ducker.getBandDuckDb (b);
                lo = std::min (lo, d);
                hi = std::max (hi, d);
                (b < 4 ? lowSum : highSum) += d;
            }

            check (hi > 1.0f, "ducks where the kick lives",
                    "max " + std::to_string (hi) + " dB");
            check (hi - lo > 2.0f, "duck differs across bands (not full-band in disguise)",
                    "spread " + std::to_string (hi - lo) + " dB");
            check (lowSum > highSum, "ducks the low bands more than the high ones",
                    "low " + std::to_string (lowSum) + " vs high " + std::to_string (highSum));
        }
    }

    void testKickDrive()
    {
        std::printf ("\n-- KickShaper drive --\n");

        juce::dsp::ProcessSpec spec { sr, 512, 2 };

        // A body-band tone: this is the band the drive is allowed to touch.
        const float toneHz = 300.0f;
        const int n = (int) (1.0 * sr);
        std::vector<float> tone ((size_t) n);
        for (int i = 0; i < n; ++i)
            tone[(size_t) i] = 0.4f * (float) std::sin (2.0 * M_PI * toneHz * i / sr);

        auto run = [&] (float drive)
        {
            KickShaper shaper;
            shaper.prepare (spec);
            KickShaper::Params p;
            p.drive = drive;
            shaper.setParams (p);
            std::vector<float> out ((size_t) n);
            for (int i = 0; i < n; ++i)
            {
                shaper.processDetector (tone[(size_t) i]);
                out[(size_t) i] = shaper.processSample (0, tone[(size_t) i]);
            }
            return out;
        };

        const int skip = (int) (0.5 * sr);
        auto mag = [&] (const std::vector<float>& sig, double hz)
        {
            double re = 0.0, im = 0.0;
            for (int i = skip; i < n; ++i)
            {
                const double t = i / sr;
                re += sig[(size_t) i] * std::cos (2.0 * M_PI * hz * t);
                im += sig[(size_t) i] * std::sin (2.0 * M_PI * hz * t);
            }
            return 2.0 * std::sqrt (re * re + im * im) / (n - skip);
        };

        auto dry = run (0.0f);
        auto wet = run (0.8f);

        // Off means off. An opt-in colouring stage that quietly changes the
        // sound at zero would make every A/B against it dishonest.
        const double dry2 = mag (dry, toneHz * 2), dry3 = mag (dry, toneHz * 3);
        check (dry2 < 0.002 && dry3 < 0.002, "drive at 0 adds no harmonics",
                "h2 " + std::to_string (dry2) + ", h3 " + std::to_string (dry3));

        const double wet2 = mag (wet, toneHz * 2), wet3 = mag (wet, toneHz * 3);
        check (wet2 > 0.004, "drive adds even harmonics (transformer asymmetry)",
                "h2 " + std::to_string (wet2));
        check (wet3 > 0.004, "drive adds odd harmonics", "h3 " + std::to_string (wet3));

        // Gain compensation: driving must change the tone, not just the volume,
        // or the knob only sounds better because it is louder.
        auto rms = [&] (const std::vector<float>& sig)
        {
            double sum = 0.0;
            for (int i = skip; i < n; ++i)
                sum += (double) sig[(size_t) i] * sig[(size_t) i];
            return std::sqrt (sum / (n - skip));
        };
        const double ratioDb = 20.0 * std::log10 (rms (wet) / std::max (1e-9, rms (dry)));
        check (std::abs (ratioDb) < 3.0, "level stays within 3 dB when driven",
                std::to_string (ratioDb) + " dB");

        // The sub band must stay clean: harmonics generated down there would
        // land straight in the mud region this plugin exists to clear.
        {
            const float subHz = 50.0f;
            std::vector<float> sub ((size_t) n);
            for (int i = 0; i < n; ++i)
                sub[(size_t) i] = 0.5f * (float) std::sin (2.0 * M_PI * subHz * i / sr);

            KickShaper shaper;
            shaper.prepare (spec);
            KickShaper::Params p;
            p.drive = 1.0f;
            shaper.setParams (p);
            std::vector<float> out ((size_t) n);
            for (int i = 0; i < n; ++i)
            {
                shaper.processDetector (sub[(size_t) i]);
                out[(size_t) i] = shaper.processSample (0, sub[(size_t) i]);
            }
            double re = 0.0, im = 0.0;
            for (int i = skip; i < n; ++i)
            {
                const double t = i / sr;
                re += out[(size_t) i] * std::cos (2.0 * M_PI * subHz * 3 * t);
                im += out[(size_t) i] * std::sin (2.0 * M_PI * subHz * 3 * t);
            }
            const double h3 = 2.0 * std::sqrt (re * re + im * im) / (n - skip);
            check (h3 < 0.01, "sub band is left clean even at full drive",
                    "150 Hz content " + std::to_string (h3));
        }
    }

    void testLinkBus()
    {
        std::printf ("\n-- LinkBus --\n");

        auto& channel = Link::getChannel (0);

        check (! Link::isKickPresent (3), "no partner reported on an unused channel");

        {
            Link::Registration reg (3, true);
            check (Link::isKickPresent (3), "registration makes the partner visible");
        }
        check (! Link::isKickPresent (3), "unregistration clears it");

        // Position-addressed reads are what make the link immune to track
        // processing order, so verify a reader gets the frame for the position
        // it asks about, not merely the newest one.
        for (int i = 0; i < 20; ++i)
        {
            Link::KickFrame f;
            f.bandDb[0] = (float) i;
            channel.kick.publish (i * 64, f);
        }

        Link::KickFrame got;
        const bool found = channel.kick.readAt (10 * 64, got);
        check (found && std::abs (got.bandDb[0] - 10.0f) < 0.001f,
                "reads the frame belonging to a requested position",
                found ? "band0 " + std::to_string (got.bandDb[0]) : "not found");

        const bool foundBetween = channel.kick.readAt (10 * 64 + 30, got);
        check (foundBetween && std::abs (got.bandDb[0] - 10.0f) < 0.001f,
                "falls back to the most recent frame at or before the position");

        Link::KickFrame future;
        check (! channel.kick.readAt (-1, future), "returns nothing for a position with no history");
    }
}

int main()
{
    std::printf ("SS BUMBO - DSP checks (sample rate %.0f Hz)\n", sr);

    testPitchTracker();
    testSpectralTamer();
    testHarmonicExciter();
    testNoteLevelling();
    testKickDucker();
    testKickDrive();
    testLinkBus();

    std::printf ("\n%s (%d failure%s)\n", failures == 0 ? "ALL PASSED" : "FAILURES PRESENT",
                  failures, failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
