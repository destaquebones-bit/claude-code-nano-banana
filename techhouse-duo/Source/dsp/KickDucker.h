#pragma once
#include "DspCommon.h"
#include "LinkBus.h"

// Makes room for the kick by ducking only the bands the kick actually occupies,
// at the moment it occupies them.
//
// Full-band sidechain compression -- the default move in this genre -- ducks the
// entire bass including the 200-400 Hz body that gives it presence on small
// speakers, which is a large part of why heavily sidechained basslines sound
// thin and why the mud comes back when you dial the sidechain down. Ducking per
// band keeps the body and only clears the frequencies that are genuinely
// colliding.
//
// The band energies come from the Link bus (published by the Kick-mode instance)
// when a partner is present, and from the plugin's sidechain input otherwise, so
// it degrades to a conventional -- still spectral -- ducker with no partner.
class KickDucker
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        for (int b = 0; b < Link::numBands; ++b)
        {
            analysis[b].prepare (spec);
            analysis[b].setCoefficients (juce::dsp::IIR::Coefficients<float>::makeBandPass (
                sampleRate, Link::bandCentres[(size_t) b], bandQ (b)));
            duckFilter[b].prepare (spec);
            detector[b].prepare (spec.sampleRate);
            detector[b].setTimes (1.0f, 40.0f);
            smoother[b].prepare (spec.sampleRate);
            smoother[b].setTimes (1.5f, 90.0f);
            sidechainDb[b] = -120.0f;
            duckDb[b] = 0.0f;
        }
        reset();
    }

    void reset()
    {
        for (int b = 0; b < Link::numBands; ++b)
        {
            analysis[b].reset();
            duckFilter[b].reset();
            detector[b].reset();
            smoother[b].reset();
            duckDb[b] = 0.0f;
        }
    }

    struct Params
    {
        float amount = 0.0f;       // 0-1
        float thresholdDb = -40.0f; // kick level below this ducks nothing at all
        float maxDuckDb = 12.0f;
        float attackMs = 1.5f;
        float releaseMs = 90.0f;
        // How far below the kick's own peak band a band has to sit before it
        // stops being ducked. This is what sets the *shape* of the duck curve.
        float shapeRangeDb = 20.0f;
    };

    void setParams (const Params& p)
    {
        params = p;
        for (int b = 0; b < Link::numBands; ++b)
            smoother[b].setTimes (p.attackMs, p.releaseMs);
    }

    // Sidechain-audio path: feed the raw kick signal when no Link partner is
    // present. Measures the same bands the Link partner would have published.
    void processSidechainSample (float monoSidechain)
    {
        for (int b = 0; b < Link::numBands; ++b)
        {
            const float banded = analysis[b].processSample (0, monoSidechain);
            sidechainDb[b] = DspCommon::gainToDb (detector[b].process (std::abs (banded)));
        }
    }

    // Link path: take the band levels straight from the Kick instance's own
    // analysis, which is both cheaper and more accurate than re-analysing audio.
    void setBandLevelsFromLink (const Link::KickFrame& frame)
    {
        for (int b = 0; b < Link::numBands; ++b)
            sidechainDb[b] = frame.bandDb[b];
    }

    // Per block: turn band energies into duck amounts and re-design the cuts.
    // `samplesElapsed` keeps the attack/release times independent of buffer size.
    void updateDucking (int samplesElapsed)
    {
        if (sampleRate <= 0.0)
            return;

        // Ducking is driven by the *shape* of the kick's spectrum relative to
        // its own peak band, not by each band's absolute level over a
        // threshold. With an absolute test, every band carrying any real energy
        // runs far past the threshold and pins at maxDuckDb, so all eight bands
        // duck by the same amount -- which is full-band sidechain ducking wearing
        // a multiband costume, and throws away the entire reason for doing this
        // per band. Measuring each band against the peak instead produces the
        // inverse of the kick's own EQ curve: bands where the kick really lives
        // get pulled down, bands where it barely exists are left alone.
        float peakDb = -120.0f;
        for (int b = 0; b < Link::numBands; ++b)
            peakDb = juce::jmax (peakDb, sidechainDb[b]);

        // Overall gate so that between kicks nothing is ducked at all.
        const float gate = juce::jlimit (0.0f, 1.0f, (peakDb - params.thresholdDb) / 12.0f);
        const float range = juce::jmax (3.0f, params.shapeRangeDb);

        for (int b = 0; b < Link::numBands; ++b)
        {
            const float belowPeak = sidechainDb[b] - peakDb;   // <= 0
            const float prominence = juce::jlimit (0.0f, 1.0f, 1.0f + belowPeak / range);
            const float target = juce::jlimit (0.0f, params.maxDuckDb,
                                                params.amount * params.maxDuckDb * prominence * gate);
            duckDb[b] = smoother[b].processOverSamples (target, samplesElapsed);

            duckFilter[b].setCoefficients (juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                sampleRate, Link::bandCentres[(size_t) b], bandQ (b) * 0.8f,
                DspCommon::dbToGain (-duckDb[b])));
        }
    }

    float processSample (int channel, float x)
    {
        for (int b = 0; b < Link::numBands; ++b)
            x = duckFilter[b].processSample (channel, x);
        return x;
    }

    float getMaxDuckDb() const
    {
        float m = 0.0f;
        for (int b = 0; b < Link::numBands; ++b)
            m = juce::jmax (m, duckDb[b]);
        return m;
    }

    float getBandDuckDb (int b) const { return duckDb[juce::jlimit (0, Link::numBands - 1, b)]; }
    float getBandSidechainDb (int b) const { return sidechainDb[juce::jlimit (0, Link::numBands - 1, b)]; }

private:
    // Wide enough that the bands overlap into continuous coverage rather than
    // leaving gaps between centres.
    static float bandQ (int) { return 1.4f; }

    double sampleRate = 44100.0;
    Params params;
    std::array<DspCommon::StereoIIR, Link::numBands> analysis, duckFilter;
    std::array<DspCommon::OnePoleEnvelope, Link::numBands> detector, smoother;
    std::array<float, Link::numBands> sidechainDb {}, duckDb {};
};
