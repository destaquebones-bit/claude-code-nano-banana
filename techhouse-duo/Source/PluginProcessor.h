#pragma once

#include <JuceHeader.h>
#include "dsp/ParameterIDs.h"
#include "dsp/LinkBus.h"
#include "dsp/SpectralTamer.h"
#include "dsp/PitchTracker.h"
#include "dsp/NoteLevelCompensator.h"
#include "dsp/HarmonicExciter.h"
#include "dsp/KickDucker.h"
#include "dsp/KickShaper.h"
#include "dsp/BandAnalyzer.h"

// One plugin, two cooperating modes.
//
// Drop an instance on the kick track in Kick mode and another on the bass track
// in Bass mode, set both to the same Link channel, and they exchange analysis
// directly (see LinkBus.h) -- no sidechain routing required, and the data
// crossing between them is measured features rather than raw audio.
class TechHouseDuoProcessor : public juce::AudioProcessor
{
public:
    TechHouseDuoProcessor();
    ~TechHouseDuoProcessor() override;

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

    bool isKickMode() const { return params.mode->load() > 0.5f; }

    // Editor-facing snapshots, written from the audio thread once per block.
    std::atomic<float> uiFundamentalHz { 0.0f };
    std::atomic<float> uiPitchConfidence { 0.0f };
    std::atomic<float> uiMaxTameDb { 0.0f };
    std::atomic<float> uiMaxDuckDb { 0.0f };
    std::atomic<float> uiNoteSpreadDb { 0.0f };
    std::atomic<float> uiNoteCompDb { 0.0f };
    std::atomic<int>   uiCurrentNote { -1 };
    std::atomic<int>   uiLearnedNotes { 0 };
    std::atomic<int>   uiWorstHarmonic { -1 };
    std::atomic<float> uiTransient { 0.0f };
    std::atomic<bool>  uiPartnerPresent { false };
    std::atomic<bool>  uiUsingSidechain { false };
    std::atomic<float> uiMudRatio { 0.0f };

    // Set from the editor's "Relearn" button; consumed by the audio thread.
    std::atomic<bool> requestNoteRelearn { false };

    // Per-band and per-note detail for the visualisers. The DSP already
    // computes all of this to do its job; publishing it is what lets the user
    // see *which* harmonic is being cut and *which* notes sit high, instead of
    // only being told a single "gain reduction" number.
    static constexpr int uiMaxBands = SpectralTamer::maxBands;
    std::array<std::atomic<float>, uiMaxBands> uiBandFreq {};
    std::array<std::atomic<float>, uiMaxBands> uiBandLevelDb {};
    std::array<std::atomic<float>, uiMaxBands> uiBandCutDb {};
    std::atomic<int> uiNumBands { 0 };

    std::array<std::atomic<float>, Link::numBands> uiDuckDb {};
    std::array<std::atomic<float>, Link::numBands> uiKickBandDb {};

    // 128 stores per refresh is wasteful every block, so the note table is
    // republished on a slower cadence than the rest.
    std::array<std::atomic<float>, 128> uiNoteLevelDb {};
    std::array<std::atomic<int>, 128> uiNoteObs {};
    std::atomic<float> uiNoteAverageDb { 0.0f };

private:
    void processBassMode (juce::AudioBuffer<float>&, const juce::AudioBuffer<float>* sidechain, int64_t hostPosition);
    void processKickMode (juce::AudioBuffer<float>&, int64_t hostPosition);
    void updateLinkRegistration();
    int64_t getHostPosition (int numSamples);

    // --- shared ---
    double currentSampleRate = 44100.0;
    DspCommon::StereoIIR inputHighPass;
    std::unique_ptr<Link::Registration> registration;
    int64_t fallbackPosition = 0;

    // --- bass mode ---
    PitchTracker pitchTracker;
    SpectralTamer harmonicTamer;   // grid = k * f0, follows the note
    SpectralTamer staticTamer;     // fixed grid, catches what the note-aware pass can't
    KickDucker ducker;
    HarmonicExciter exciter;
    NoteLevelCompensator noteComp;
    DspCommon::LR4Crossover monoCrossover;

    // Lookahead so the pitch estimate lines up with the audio it describes.
    juce::AudioBuffer<float> delayBuffer;
    int delayWritePos = 0;
    int delaySamples = 0;

    // --- kick mode ---
    KickShaper kickShaper;
    SpectralTamer kickTamer;
    BandAnalyzer kickAnalyzer;
    DspCommon::StereoIIR bassAwareNotch;
    DspCommon::OnePoleEnvelope bassAwareSmoother;

    // Publishing cadence: one Link frame per 64 samples gives the bass side
    // enough time resolution to follow a kick transient, without writing a
    // frame per sample.
    static constexpr int publishInterval = 64;
    int publishCounter = 0;
    int noteMapPublishCounter = 0;

    struct ParamPtrs
    {
        std::atomic<float>* mode = nullptr;
        std::atomic<float>* linkChannel = nullptr;
        std::atomic<float>* linkEnable = nullptr;
        std::atomic<float>* inputHpf = nullptr;
        std::atomic<float>* outputGain = nullptr;
        std::atomic<float>* dryWet = nullptr;
        std::atomic<float>* listenMode = nullptr;
        std::atomic<float>* bypass = nullptr;

        std::atomic<float>* tameDepth = nullptr;
        std::atomic<float>* tameTolerance = nullptr;
        std::atomic<float>* tameMaxCut = nullptr;
        std::atomic<float>* tameRelease = nullptr;
        std::atomic<float>* harmonicCount = nullptr;
        std::atomic<float>* mudDepth = nullptr;
        std::atomic<float>* noteCompAmount = nullptr;
        std::atomic<float>* duckAmount = nullptr;
        std::atomic<float>* duckThreshold = nullptr;
        std::atomic<float>* duckRelease = nullptr;
        std::atomic<float>* exciteAmount = nullptr;
        std::atomic<float>* exciteBalance = nullptr;
        std::atomic<float>* monoBelow = nullptr;

        std::atomic<float>* kickSubFreq = nullptr;
        std::atomic<float>* kickTopFreq = nullptr;
        std::atomic<float>* kickSubGain = nullptr;
        std::atomic<float>* kickTopGain = nullptr;
        std::atomic<float>* kickTailAmt = nullptr;
        std::atomic<float>* kickTailMs = nullptr;
        std::atomic<float>* kickAttack = nullptr;
        std::atomic<float>* kickBoxiness = nullptr;
        std::atomic<float>* kickBassAware = nullptr;
        std::atomic<float>* kickDrive = nullptr;
    } params;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TechHouseDuoProcessor)
};
