#pragma once
#include "DspCommon.h"
#include "LinkBus.h"

// Measures energy across the eight band centres both modes agree on
// (Link::bandCentres). The Kick instance runs this on its own output and
// publishes the result, so the Bass instance gets a ready-made spectral picture
// of the kick instead of having to re-analyse audio that was already analysed
// one track over.
class BandAnalyzer
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        for (int b = 0; b < Link::numBands; ++b)
        {
            filters[b].prepare (spec);
            filters[b].setCoefficients (juce::dsp::IIR::Coefficients<float>::makeBandPass (
                spec.sampleRate, Link::bandCentres[(size_t) b], 1.4f));
            envelopes[b].prepare (spec.sampleRate);
            envelopes[b].setTimes (1.0f, 40.0f);
            levelDb[b] = -120.0f;
        }
        broadband.prepare (spec.sampleRate);
        broadband.setTimes (1.0f, 60.0f);
    }

    void reset()
    {
        for (int b = 0; b < Link::numBands; ++b)
        {
            filters[b].reset();
            envelopes[b].reset();
        }
        broadband.reset();
    }

    void processSample (float monoSample)
    {
        for (int b = 0; b < Link::numBands; ++b)
        {
            const float banded = filters[b].processSample (0, monoSample);
            levelDb[b] = DspCommon::gainToDb (envelopes[b].process (std::abs (banded)));
        }
        broadbandDb = DspCommon::gainToDb (broadband.process (std::abs (monoSample)));
    }

    void fill (Link::KickFrame& frame) const
    {
        for (int b = 0; b < Link::numBands; ++b)
            frame.bandDb[b] = levelDb[b];
        frame.broadbandDb = broadbandDb;
    }

    float getBandDb (int b) const { return levelDb[juce::jlimit (0, Link::numBands - 1, b)]; }
    float getBroadbandDb() const { return broadbandDb; }

private:
    std::array<DspCommon::StereoIIR, Link::numBands> filters;
    std::array<DspCommon::OnePoleEnvelope, Link::numBands> envelopes;
    DspCommon::OnePoleEnvelope broadband;
    std::array<float, Link::numBands> levelDb {};
    float broadbandDb = -120.0f;
};
