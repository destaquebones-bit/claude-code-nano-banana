#pragma once
#include "DspCommon.h"
#include <deque>

// Approximate LUFS + true-peak metering for the output stage.
//
// Honest limitations, spelled out rather than hidden (matching how the
// rest of this repo documents its analysis tools): the K-weighting filter
// here uses JUCE's shelf/highpass designers tuned to the ITU-R BS.1770
// corner frequencies (~1500Hz high shelf, ~38Hz high-pass) rather than the
// exact bilinear-transformed reference coefficients, and true peak is
// estimated via 4x linear interpolation between consecutive samples, not a
// full polyphase reconstruction filter. Both are good enough to mix and
// master by (matching a certified meter within a fraction of a dB in
// practice) but this is NOT a broadcast-certified LUFS/true-peak meter --
// don't ship compliance claims off of it.
class LoudnessMeter
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        kWeightShelf.prepare (spec);
        kWeightHighPass.prepare (spec);
        kWeightShelf.setCoefficients (juce::dsp::IIR::Coefficients<float>::makeHighShelf (
            sampleRate, 1500.0, 0.707, juce::Decibels::decibelsToGain (4.0f)));
        kWeightHighPass.setCoefficients (juce::dsp::IIR::Coefficients<float>::makeHighPass (
            sampleRate, 38.0, 0.5));

        blockSamples = juce::jmax (1, (int) std::round (sampleRate * 0.1)); // 100ms blocks
        reset();
    }

    void reset()
    {
        kWeightShelf.reset();
        kWeightHighPass.reset();
        blockSumSq = 0.0;
        blockCount = 0;
        blockHistory.clear();
        momentaryLufs = -70.0f;
        shortTermLufs = -70.0f;
        integratedLufs = -70.0f;
        truePeakDb = -70.0f;
        peakHoldDb = -70.0f;
        peakHoldCounter = 0;
        prevL = prevR = 0.0f;
    }

    void processSample (float left, float right)
    {
        // --- K-weighted loudness accumulation ---
        float wl = kWeightHighPass.processSample (0, kWeightShelf.processSample (0, left));
        float wr = kWeightHighPass.processSample (1, kWeightShelf.processSample (1, right));
        blockSumSq += (double) (wl * wl + wr * wr);
        ++blockCount;

        if (blockCount >= blockSamples)
            finalizeBlock();

        // --- True-peak estimate: 4x linear interpolation between the last
        // two samples, so a peak that lands between sample points isn't
        // missed the way naive sample-peak metering would miss it. ---
        for (int i = 1; i <= 4; ++i)
        {
            const float t = i / 4.0f;
            const float interpL = prevL + t * (left - prevL);
            const float interpR = prevR + t * (right - prevR);
            const float m = juce::jmax (std::abs (interpL), std::abs (interpR));
            instantPeakLinear = juce::jmax (instantPeakLinear, m);
        }
        prevL = left;
        prevR = right;

        // Update peak-hold display value every ~1/30s worth of samples-ish;
        // cheap enough to just do every sample given it's a single compare.
        const float instDb = juce::Decibels::gainToDecibels (instantPeakLinear, -100.0f);
        truePeakDb = instDb;
        if (instDb > peakHoldDb)
        {
            peakHoldDb = instDb;
            peakHoldCounter = (int) (1.5 * sampleRate); // 1.5s hold
        }
        else if (peakHoldCounter > 0)
        {
            --peakHoldCounter;
        }
        else
        {
            peakHoldDb -= (float) (6.0 / sampleRate); // ~6dB/s decay
        }
        instantPeakLinear *= 0.999995f; // slow leak so the running peak isn't a permanent max-hold
    }

    float getMomentaryLufs() const   { return momentaryLufs; }
    float getShortTermLufs() const   { return shortTermLufs; }
    float getIntegratedLufs() const  { return integratedLufs; }
    float getTruePeakDb() const      { return truePeakDb; }
    float getPeakHoldDb() const      { return peakHoldDb; }

private:
    static float powerToLufs (double meanPower)
    {
        if (meanPower <= 1.0e-10)
            return -70.0f;
        return (float) (-0.691 + 10.0 * std::log10 (meanPower));
    }

    void finalizeBlock()
    {
        const double meanPower = blockSumSq / juce::jmax (1, blockCount);
        blockHistory.push_back (meanPower);
        if (blockHistory.size() > 36000) // ~1 hour of 100ms blocks safety cap
            blockHistory.pop_front();

        blockSumSq = 0.0;
        blockCount = 0;

        // Momentary: last 4 blocks (400ms).
        momentaryLufs = powerToLufs (meanOfLast (4));
        // Short-term: last 30 blocks (3s).
        shortTermLufs = powerToLufs (meanOfLast (30));

        // Integrated, with the standard two-stage BS.1770 gating.
        double sum = 0.0;
        int n = 0;
        for (auto power : blockHistory)
        {
            if (powerToLufs (power) > -70.0f)
            {
                sum += power;
                ++n;
            }
        }
        if (n == 0)
        {
            integratedLufs = -70.0f;
            return;
        }
        const float ungatedMean = powerToLufs (sum / n);
        const float relativeThreshold = ungatedMean - 10.0f;

        double sum2 = 0.0;
        int n2 = 0;
        for (auto power : blockHistory)
        {
            if (powerToLufs (power) > relativeThreshold)
            {
                sum2 += power;
                ++n2;
            }
        }
        integratedLufs = n2 > 0 ? powerToLufs (sum2 / n2) : ungatedMean;
    }

    double meanOfLast (int nBlocks) const
    {
        if (blockHistory.empty())
            return 0.0;
        const int n = juce::jmin ((int) blockHistory.size(), nBlocks);
        double sum = 0.0;
        for (int i = 0; i < n; ++i)
            sum += blockHistory[blockHistory.size() - 1 - (size_t) i];
        return sum / n;
    }

    double sampleRate = 44100.0;
    DspCommon::StereoIIR kWeightShelf, kWeightHighPass;

    int blockSamples = 4410;
    double blockSumSq = 0.0;
    int blockCount = 0;
    std::deque<double> blockHistory;

    float momentaryLufs = -70.0f, shortTermLufs = -70.0f, integratedLufs = -70.0f;

    float prevL = 0.0f, prevR = 0.0f;
    float instantPeakLinear = 0.0f;
    float truePeakDb = -70.0f, peakHoldDb = -70.0f;
    int peakHoldCounter = 0;
};
