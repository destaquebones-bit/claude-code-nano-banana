#pragma once

#include <JuceHeader.h>
#include "dsp/ParameterIDs.h"
#include "dsp/AnalogEQ.h"
#include "dsp/GlueCompressor.h"
#include "dsp/TapeSaturator.h"
#include "dsp/StereoWidener.h"
#include "dsp/AnalogLimiter.h"
#include "dsp/LoudnessMeter.h"

// One VST3/Standalone plugin standing in for a whole analog mastering rack:
// input trim -> Pultec-style program EQ -> VCA glue bus compressor -> tape
// saturation -> M/S stereo widener (bass-mono safe) -> lookahead true-peak
// limiter -> output trim, with LUFS/true-peak metering on the way out.
// See Source/dsp/*.h for the DSP behind each stage and README.md for the
// signal-flow rationale.
class MasteringChainAudioProcessor : public juce::AudioProcessor
{
public:
    MasteringChainAudioProcessor();
    ~MasteringChainAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Snapshot values the editor polls on a timer for metering -- written
    // from the audio thread at the end of every block, read from the
    // message thread. Plain atomics are enough since these are single
    // float values, not structures needing multi-field consistency.
    std::atomic<float> uiMomentaryLufs { -70.0f };
    std::atomic<float> uiIntegratedLufs { -70.0f };
    std::atomic<float> uiTruePeakDb { -70.0f };
    std::atomic<float> uiCompGainReductionDb { 0.0f };
    std::atomic<float> uiLimGainReductionDb { 0.0f };

private:
    void updateModuleParameters();

    // DSP chain, in signal-flow order.
    AnalogEQ eq;
    GlueCompressor comp;
    TapeSaturator sat;
    StereoWidener widener;
    AnalogLimiter limiter;
    LoudnessMeter meter;

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    static constexpr int oversampleFactor = 2; // must match the oversampling stage count below (1 stage = 2x)

    // Cached raw parameter pointers, fetched once in the constructor so
    // processBlock() only does atomic loads, never string lookups.
    struct ParamPtrs
    {
        std::atomic<float>* inputGainDb = nullptr;
        std::atomic<float>* inputInvert = nullptr;

        std::atomic<float>* eqBypass = nullptr;
        std::atomic<float>* eqLowFreq = nullptr;
        std::atomic<float>* eqLowBoostDb = nullptr;
        std::atomic<float>* eqLowCutDb = nullptr;
        std::atomic<float>* eqLowMidFreq = nullptr;
        std::atomic<float>* eqLowMidGainDb = nullptr;
        std::atomic<float>* eqLowMidQ = nullptr;
        std::atomic<float>* eqHighMidFreq = nullptr;
        std::atomic<float>* eqHighMidGainDb = nullptr;
        std::atomic<float>* eqHighMidQ = nullptr;
        std::atomic<float>* eqAirFreq = nullptr;
        std::atomic<float>* eqAirGainDb = nullptr;
        std::atomic<float>* eqIronAmount = nullptr;

        std::atomic<float>* compBypass = nullptr;
        std::atomic<float>* compThreshold = nullptr;
        std::atomic<float>* compRatio = nullptr;
        std::atomic<float>* compAttack = nullptr;
        std::atomic<float>* compRelease = nullptr;
        std::atomic<float>* compAutoRelease = nullptr;
        std::atomic<float>* compKnee = nullptr;
        std::atomic<float>* compMakeup = nullptr;
        std::atomic<float>* compAutoMakeup = nullptr;
        std::atomic<float>* compMix = nullptr;

        std::atomic<float>* satBypass = nullptr;
        std::atomic<float>* satDrive = nullptr;
        std::atomic<float>* satWarmth = nullptr;
        std::atomic<float>* satMix = nullptr;

        std::atomic<float>* widthBypass = nullptr;
        std::atomic<float>* widthAmount = nullptr;
        std::atomic<float>* widthBassMonoHz = nullptr;

        std::atomic<float>* limBypass = nullptr;
        std::atomic<float>* limCeiling = nullptr;
        std::atomic<float>* limDrive = nullptr;
        std::atomic<float>* limRelease = nullptr;

        std::atomic<float>* outputGainDb = nullptr;
        std::atomic<float>* outputMonoSafety = nullptr;
        std::atomic<float>* masterBypass = nullptr;
    } params;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasteringChainAudioProcessor)
};
