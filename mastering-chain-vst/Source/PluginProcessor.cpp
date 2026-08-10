#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    // Small helper so the constructor's parameter-pointer wiring is one
    // line per parameter instead of a getRawParameterValue() call buried
    // in a longer expression.
    std::atomic<float>* raw (juce::AudioProcessorValueTreeState& s, const juce::String& id)
    {
        return s.getRawParameterValue (id);
    }
}

MasteringChainAudioProcessor::MasteringChainAudioProcessor()
    : AudioProcessor (BusesProperties()
                           .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                           .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", ParamIDs::createLayout())
{
    params.inputGainDb      = raw (apvts, ParamIDs::inputGainDb);
    params.inputInvert      = raw (apvts, ParamIDs::inputInvert);

    params.eqBypass         = raw (apvts, ParamIDs::eqBypass);
    params.eqLowFreq        = raw (apvts, ParamIDs::eqLowFreq);
    params.eqLowBoostDb     = raw (apvts, ParamIDs::eqLowBoostDb);
    params.eqLowCutDb       = raw (apvts, ParamIDs::eqLowCutDb);
    params.eqLowMidFreq     = raw (apvts, ParamIDs::eqLowMidFreq);
    params.eqLowMidGainDb   = raw (apvts, ParamIDs::eqLowMidGainDb);
    params.eqLowMidQ        = raw (apvts, ParamIDs::eqLowMidQ);
    params.eqHighMidFreq    = raw (apvts, ParamIDs::eqHighMidFreq);
    params.eqHighMidGainDb  = raw (apvts, ParamIDs::eqHighMidGainDb);
    params.eqHighMidQ       = raw (apvts, ParamIDs::eqHighMidQ);
    params.eqAirFreq        = raw (apvts, ParamIDs::eqAirFreq);
    params.eqAirGainDb      = raw (apvts, ParamIDs::eqAirGainDb);
    params.eqIronAmount     = raw (apvts, ParamIDs::eqIronAmount);

    params.compBypass       = raw (apvts, ParamIDs::compBypass);
    params.compThreshold    = raw (apvts, ParamIDs::compThreshold);
    params.compRatio        = raw (apvts, ParamIDs::compRatio);
    params.compAttack       = raw (apvts, ParamIDs::compAttack);
    params.compRelease      = raw (apvts, ParamIDs::compRelease);
    params.compAutoRelease  = raw (apvts, ParamIDs::compAutoRelease);
    params.compKnee         = raw (apvts, ParamIDs::compKnee);
    params.compMakeup       = raw (apvts, ParamIDs::compMakeup);
    params.compAutoMakeup   = raw (apvts, ParamIDs::compAutoMakeup);
    params.compMix          = raw (apvts, ParamIDs::compMix);

    params.satBypass        = raw (apvts, ParamIDs::satBypass);
    params.satDrive         = raw (apvts, ParamIDs::satDrive);
    params.satWarmth        = raw (apvts, ParamIDs::satWarmth);
    params.satMix            = raw (apvts, ParamIDs::satMix);

    params.widthBypass      = raw (apvts, ParamIDs::widthBypass);
    params.widthAmount      = raw (apvts, ParamIDs::widthAmount);
    params.widthBassMonoHz  = raw (apvts, ParamIDs::widthBassMonoHz);

    params.limBypass        = raw (apvts, ParamIDs::limBypass);
    params.limCeiling       = raw (apvts, ParamIDs::limCeiling);
    params.limDrive         = raw (apvts, ParamIDs::limDrive);
    params.limRelease       = raw (apvts, ParamIDs::limRelease);

    params.outputGainDb     = raw (apvts, ParamIDs::outputGainDb);
    params.outputMonoSafety = raw (apvts, ParamIDs::outputMonoSafety);
    params.masterBypass     = raw (apvts, ParamIDs::masterBypass);
}

MasteringChainAudioProcessor::~MasteringChainAudioProcessor() = default;

bool MasteringChainAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    return out == layouts.getMainInputChannelSet();
}

void MasteringChainAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const int numChannels = juce::jmax (1, getTotalNumOutputChannels());

    juce::dsp::ProcessSpec baseSpec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) numChannels };
    eq.prepare (baseSpec);
    comp.prepare (baseSpec);
    widener.prepare (baseSpec);
    meter.prepare (baseSpec);

    // 1 stage of half-band FIR = 2x oversampling. The saturator and
    // limiter run at this rate (see processBlock) so their nonlinearities
    // don't fold aliased harmonics back into the audible band, and the
    // limiter's peak detector sees closer-to-true inter-sample peaks.
    oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
        (size_t) numChannels, 1, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, false);
    oversampler->initProcessing ((size_t) samplesPerBlock);

    juce::dsp::ProcessSpec osSpec { sampleRate * (double) oversampleFactor,
                                     (juce::uint32) (samplesPerBlock * oversampleFactor),
                                     (juce::uint32) numChannels };
    sat.prepare (osSpec);
    limiter.prepare (osSpec);

    setLatencySamples ((int) oversampler->getLatencyInSamples()
                        + (int) std::round (limiter.getLatencySamples() / (double) oversampleFactor));
}

void MasteringChainAudioProcessor::releaseResources()
{
    oversampler.reset();
}

void MasteringChainAudioProcessor::updateModuleParameters()
{
    AnalogEQ::Params eqP;
    eqP.lowFreq = params.eqLowFreq->load();
    eqP.lowBoostDb = params.eqLowBoostDb->load();
    eqP.lowCutDb = params.eqLowCutDb->load();
    eqP.lowMidFreq = params.eqLowMidFreq->load();
    eqP.lowMidGainDb = params.eqLowMidGainDb->load();
    eqP.lowMidQ = params.eqLowMidQ->load();
    eqP.highMidFreq = params.eqHighMidFreq->load();
    eqP.highMidGainDb = params.eqHighMidGainDb->load();
    eqP.highMidQ = params.eqHighMidQ->load();
    eqP.airFreq = params.eqAirFreq->load();
    eqP.airGainDb = params.eqAirGainDb->load();
    eqP.ironAmount = params.eqIronAmount->load();
    eq.updateParameters (eqP);

    GlueCompressor::Params compP;
    compP.thresholdDb = params.compThreshold->load();
    compP.ratio = params.compRatio->load();
    compP.attackMs = params.compAttack->load();
    compP.releaseMs = params.compRelease->load();
    compP.autoRelease = params.compAutoRelease->load() > 0.5f;
    compP.kneeDb = params.compKnee->load();
    compP.makeupDb = params.compMakeup->load();
    compP.autoMakeup = params.compAutoMakeup->load() > 0.5f;
    compP.mixPercent = params.compMix->load();
    comp.updateParameters (compP);

    TapeSaturator::Params satP;
    satP.driveDb = params.satDrive->load();
    satP.warmthPercent = params.satWarmth->load();
    satP.mixPercent = params.satMix->load();
    sat.updateParameters (satP);

    StereoWidener::Params widthP;
    widthP.widthPercent = params.widthAmount->load();
    widthP.bassMonoHz = params.widthBassMonoHz->load();
    widener.updateParameters (widthP);

    AnalogLimiter::Params limP;
    limP.ceilingDb = params.limCeiling->load();
    limP.driveDb = params.limDrive->load();
    limP.releaseMs = params.limRelease->load();
    limiter.updateParameters (limP);
}

void MasteringChainAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    if (params.masterBypass->load() > 0.5f)
        return;

    updateModuleParameters();

    auto* L = buffer.getWritePointer (0);
    auto* R = numChannels > 1 ? buffer.getWritePointer (1) : L;

    const float inGain = juce::Decibels::decibelsToGain (params.inputGainDb->load());
    const float invertSign = params.inputInvert->load() > 0.5f ? -1.0f : 1.0f;
    const bool eqBypassed = params.eqBypass->load() > 0.5f;
    const bool compBypassed = params.compBypass->load() > 0.5f;
    const bool widthBypassed = params.widthBypass->load() > 0.5f;

    // --- Pre-oversampling stages: input trim, EQ, glue compression, width ---
    for (int n = 0; n < numSamples; ++n)
    {
        float l = L[n] * inGain * invertSign;
        float r = R[n] * inGain * invertSign;

        if (! eqBypassed)
        {
            l = eq.processSample (0, l);
            r = eq.processSample (1, r);
        }

        if (! compBypassed)
        {
            const float linked = juce::jmax (std::abs (l), std::abs (r));
            const float g = comp.computeGain (linked);
            l *= g;
            r *= g;
        }

        if (! widthBypassed && widener.isUsable())
            widener.processSample (l, r);

        L[n] = l;
        R[n] = r;
    }

    uiCompGainReductionDb.store (comp.getCurrentGainReductionDb());

    // --- Oversampled stages: tape saturation + lookahead true-peak limiter ---
    juce::dsp::AudioBlock<float> block (buffer);
    auto osBlock = oversampler->processSamplesUp (block);

    const bool satBypassed = params.satBypass->load() > 0.5f;
    const bool limBypassed = params.limBypass->load() > 0.5f;
    const int osNumSamples = (int) osBlock.getNumSamples();
    auto* osL = osBlock.getChannelPointer (0);
    auto* osR = osBlock.getNumChannels() > 1 ? osBlock.getChannelPointer (1) : osL;

    for (int n = 0; n < osNumSamples; ++n)
    {
        float l = osL[n];
        float r = osR[n];

        if (! satBypassed)
        {
            l = sat.processSample (0, l);
            r = sat.processSample (1, r);
        }

        if (! limBypassed)
            limiter.processStereoSample (l, r);

        osL[n] = l;
        osR[n] = r;
    }

    oversampler->processSamplesDown (block);

    uiLimGainReductionDb.store (limiter.getCurrentGainReductionDb());

    // --- Output trim, mono-safety check, metering ---
    const float outGain = juce::Decibels::decibelsToGain (params.outputGainDb->load());
    const bool monoSafety = params.outputMonoSafety->load() > 0.5f;

    for (int n = 0; n < numSamples; ++n)
    {
        float l = L[n] * outGain;
        float r = R[n] * outGain;

        if (monoSafety)
        {
            const float m = 0.5f * (l + r);
            l = m;
            r = m;
        }

        L[n] = l;
        R[n] = r;
        meter.processSample (l, r);
    }

    uiMomentaryLufs.store (meter.getMomentaryLufs());
    uiIntegratedLufs.store (meter.getIntegratedLufs());
    uiTruePeakDb.store (meter.getPeakHoldDb());
}

juce::AudioProcessorEditor* MasteringChainAudioProcessor::createEditor()
{
    return new MasteringChainAudioProcessorEditor (*this);
}

void MasteringChainAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); true)
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    }
}

void MasteringChainAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MasteringChainAudioProcessor();
}
