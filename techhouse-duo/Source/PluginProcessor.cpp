#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    std::atomic<float>* raw (juce::AudioProcessorValueTreeState& s, const juce::String& id)
    {
        return s.getRawParameterValue (id);
    }
}

TechHouseDuoProcessor::TechHouseDuoProcessor()
    : AudioProcessor (BusesProperties()
                           .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                           .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)
                           .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), false)),
      apvts (*this, nullptr, "PARAMETERS", ParamIDs::createLayout())
{
    params.mode          = raw (apvts, ParamIDs::mode);
    params.linkChannel   = raw (apvts, ParamIDs::linkChannel);
    params.linkEnable    = raw (apvts, ParamIDs::linkEnable);
    params.inputHpf      = raw (apvts, ParamIDs::inputHpf);
    params.outputGain    = raw (apvts, ParamIDs::outputGain);
    params.dryWet        = raw (apvts, ParamIDs::dryWet);
    params.listenMode    = raw (apvts, ParamIDs::listenMode);
    params.bypass        = raw (apvts, ParamIDs::bypass);

    params.tameDepth     = raw (apvts, ParamIDs::tameDepth);
    params.tameTolerance = raw (apvts, ParamIDs::tameTolerance);
    params.tameMaxCut    = raw (apvts, ParamIDs::tameMaxCut);
    params.tameRelease   = raw (apvts, ParamIDs::tameRelease);
    params.harmonicCount = raw (apvts, ParamIDs::harmonicCount);
    params.mudDepth      = raw (apvts, ParamIDs::mudDepth);
    params.noteCompAmount= raw (apvts, ParamIDs::noteCompAmount);
    params.duckAmount    = raw (apvts, ParamIDs::duckAmount);
    params.duckThreshold = raw (apvts, ParamIDs::duckThreshold);
    params.duckRelease   = raw (apvts, ParamIDs::duckRelease);
    params.exciteAmount  = raw (apvts, ParamIDs::exciteAmount);
    params.exciteBalance = raw (apvts, ParamIDs::exciteBalance);
    params.monoBelow     = raw (apvts, ParamIDs::monoBelow);

    params.kickSubFreq   = raw (apvts, ParamIDs::kickSubFreq);
    params.kickTopFreq   = raw (apvts, ParamIDs::kickTopFreq);
    params.kickSubGain   = raw (apvts, ParamIDs::kickSubGain);
    params.kickTopGain   = raw (apvts, ParamIDs::kickTopGain);
    params.kickTailAmt   = raw (apvts, ParamIDs::kickTailAmt);
    params.kickTailMs    = raw (apvts, ParamIDs::kickTailMs);
    params.kickAttack    = raw (apvts, ParamIDs::kickAttack);
    params.kickBoxiness  = raw (apvts, ParamIDs::kickBoxiness);
    params.kickBassAware = raw (apvts, ParamIDs::kickBassAware);

    registration = std::make_unique<Link::Registration> (0, false);
}

TechHouseDuoProcessor::~TechHouseDuoProcessor() = default;

bool TechHouseDuoProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;
    if (mainIn != mainOut)
        return false;

    // The sidechain is optional: with a Link partner it isn't needed at all, so
    // a layout with it disabled must stay valid.
    if (layouts.inputBuses.size() > 1)
    {
        const auto sc = layouts.getChannelSet (true, 1);
        if (! sc.isDisabled()
            && sc != juce::AudioChannelSet::mono()
            && sc != juce::AudioChannelSet::stereo())
            return false;
    }

    return true;
}

void TechHouseDuoProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    const int numChannels = juce::jmax (1, getMainBusNumOutputChannels());
    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, (juce::uint32) numChannels };

    inputHighPass.prepare (spec);

    pitchTracker.prepare (sampleRate);
    harmonicTamer.prepare (spec);
    staticTamer.prepare (spec);
    ducker.prepare (spec);
    exciter.prepare (spec);
    noteComp.prepare (sampleRate);
    monoCrossover.prepare (spec);

    kickShaper.prepare (spec);
    kickTamer.prepare (spec);
    kickAnalyzer.prepare (spec);
    bassAwareNotch.prepare (spec);
    bassAwareSmoother.prepare (sampleRate);
    bassAwareSmoother.setTimes (15.0f, 80.0f);

    // The static grid never moves, so it can be set once here.
    staticTamer.setBandCentres (Link::bandCentres.data(), Link::numBands);
    kickTamer.setBandCentres (Link::bandCentres.data(), Link::numBands);

    delaySamples = pitchTracker.getAnalysisLatencySamples();
    delayBuffer.setSize (2, juce::jmax (delaySamples + 1, samplesPerBlock + 1));
    delayBuffer.clear();
    delayWritePos = 0;
    publishCounter = 0;
    fallbackPosition = 0;

    // Only the Bass path uses lookahead; reporting it in Kick mode would delay
    // the kick against the rest of the mix for no reason.
    setLatencySamples (isKickMode() ? 0 : delaySamples);
}

void TechHouseDuoProcessor::releaseResources() {}

void TechHouseDuoProcessor::updateLinkRegistration()
{
    const int channel = (int) params.linkChannel->load();
    const bool kickMode = isKickMode();
    if (registration != nullptr)
        registration->moveTo (channel, kickMode);
}

int64_t TechHouseDuoProcessor::getHostPosition (int numSamples)
{
    if (auto* head = getPlayHead())
    {
        if (auto position = head->getPosition())
        {
            if (auto samples = position->getTimeInSamples())
            {
                fallbackPosition = *samples + numSamples;
                return *samples;
            }
        }
    }

    // Standalone, or a host that reports no timeline: keep our own monotonic
    // counter so Link frames still line up between the two instances.
    const int64_t pos = fallbackPosition;
    fallbackPosition += numSamples;
    return pos;
}

void TechHouseDuoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    auto mainBus = getBusBuffer (buffer, false, 0);
    const int numChannels = mainBus.getNumChannels();

    for (int ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, numSamples);

    updateLinkRegistration();
    const int64_t hostPosition = getHostPosition (numSamples);

    if (params.bypass->load() > 0.5f || numChannels == 0)
        return;

    const float hpfFreq = params.inputHpf->load();
    inputHighPass.setCoefficients (juce::dsp::IIR::Coefficients<float>::makeHighPass (
        currentSampleRate, juce::jlimit (10.0f, 60.0f, hpfFreq), 0.7071f));

    if (isKickMode())
    {
        processKickMode (buffer, hostPosition);
    }
    else
    {
        const bool hasSidechain = getBus (true, 1) != nullptr && getBus (true, 1)->isEnabled();
        juce::AudioBuffer<float> scBuffer;
        if (hasSidechain)
            scBuffer = getBusBuffer (buffer, true, 1);
        processBassMode (buffer, hasSidechain ? &scBuffer : nullptr, hostPosition);
    }

    const float outGain = DspCommon::dbToGain (params.outputGain->load());
    if (std::abs (outGain - 1.0f) > 1.0e-6f)
        mainBus.applyGain (outGain);
}

void TechHouseDuoProcessor::processBassMode (juce::AudioBuffer<float>& buffer,
                                              const juce::AudioBuffer<float>* sidechain,
                                              int64_t hostPosition)
{
    auto mainBus = getBusBuffer (buffer, false, 0);
    const int numSamples = mainBus.getNumSamples();
    const int numChannels = mainBus.getNumChannels();

    auto* L = mainBus.getWritePointer (0);
    auto* R = numChannels > 1 ? mainBus.getWritePointer (1) : L;

    // --- Pre-pass: run the pitch tracker over this block's input first, so the
    // harmonic grid below is built from the freshest estimate rather than one
    // block old. The tracker sees undelayed input, which is precisely what lets
    // it "look ahead" of the delayed audio we are about to process. ---
    for (int n = 0; n < numSamples; ++n)
        pitchTracker.processSample (0.5f * (L[n] + R[n]));

    const float f0 = pitchTracker.getFundamentalHz();
    const float confidence = pitchTracker.getConfidence();

    // --- Build the note-relative grid: band k is the k-th harmonic of whatever
    // note is playing, so a cut identified as "the 4th harmonic is resonant"
    // keeps tracking that harmonic as the bassline moves. ---
    const int harmonics = juce::jlimit (3, SpectralTamer::maxBands, (int) params.harmonicCount->load());
    if (f0 > 0.0f && confidence > 0.25f)
    {
        float grid[SpectralTamer::maxBands];
        for (int k = 0; k < harmonics; ++k)
            grid[k] = f0 * (float) (k + 1);
        harmonicTamer.setBandCentres (grid, harmonics);
        exciter.setFundamental (f0);
    }

    SpectralTamer::Params tameP;
    tameP.depth = params.tameDepth->load() * 0.01f;
    tameP.toleranceDb = params.tameTolerance->load();
    tameP.maxCutDb = params.tameMaxCut->load();
    tameP.releaseMs = params.tameRelease->load();
    tameP.protectFirstBand = 1.0f;
    harmonicTamer.setParams (tameP);

    // The static tamer works on a fixed grid and therefore stays valid when the
    // tracker is unsure -- it is the graceful-degradation path, and it also
    // catches non-harmonic buildup the note-aware pass is blind to by design.
    SpectralTamer::Params mudP = tameP;
    mudP.depth = params.mudDepth->load() * 0.01f;
    mudP.protectFirstBand = 0.0f;
    staticTamer.setParams (mudP);

    // --- Where the kick information comes from: Link first, sidechain second. ---
    const int linkChannel = (int) params.linkChannel->load();
    const bool linkOn = params.linkEnable->load() > 0.5f;
    const bool partnerPresent = linkOn && Link::isKickPresent (linkChannel);
    bool usingSidechain = false;

    if (partnerPresent)
    {
        // Ask for the kick data belonging to the exact samples we are about to
        // output -- which, thanks to the lookahead delay, are already in the
        // past. That is what makes this immune to track processing order.
        Link::KickFrame frame;
        auto& ring = Link::getChannel (linkChannel).kick;
        if (ring.readAt (hostPosition - delaySamples, frame) || ring.readLatest (frame))
            ducker.setBandLevelsFromLink (frame);
    }
    else if (sidechain != nullptr && sidechain->getNumChannels() > 0)
    {
        usingSidechain = true;
        const auto* scL = sidechain->getReadPointer (0);
        const auto* scR = sidechain->getNumChannels() > 1 ? sidechain->getReadPointer (1) : scL;
        for (int n = 0; n < numSamples; ++n)
            ducker.processSidechainSample (0.5f * (scL[n] + scR[n]));
    }

    KickDucker::Params duckP;
    duckP.amount = params.duckAmount->load() * 0.01f;
    duckP.thresholdDb = params.duckThreshold->load();
    duckP.releaseMs = params.duckRelease->load();
    ducker.setParams (duckP);

    HarmonicExciter::Params exP;
    exP.amount = params.exciteAmount->load() * 0.01f * juce::jlimit (0.0f, 1.0f, confidence * 1.5f);
    exP.balance = params.exciteBalance->load() * 0.01f;
    exciter.setParams (exP);

    if (requestNoteRelearn.exchange (false))
        noteComp.resetLearning();

    NoteLevelCompensator::Params ncP;
    ncP.amount = params.noteCompAmount->load() * 0.01f;
    noteComp.setParams (ncP);

    monoCrossover.setCrossover (currentSampleRate, params.monoBelow->load());

    const bool listen = params.listenMode->load() > 0.5f;
    const float wet = juce::jlimit (0.0f, 1.0f, params.dryWet->load() * 0.01f);

    // --- Per-sample processing on the delayed signal ---
    float mudEnergy = 0.0f, totalEnergy = 0.0f;

    for (int n = 0; n < numSamples; ++n)
    {
        // Push the incoming sample into the lookahead line and take out the one
        // the current pitch estimate actually describes.
        const int readPos = (delayWritePos + 1) % delayBuffer.getNumSamples();
        delayBuffer.setSample (0, delayWritePos, L[n]);
        delayBuffer.setSample (1, delayWritePos, R[n]);
        float dryL = delayBuffer.getSample (0, readPos);
        float dryR = delayBuffer.getSample (1, readPos);
        delayWritePos = readPos;

        dryL = inputHighPass.processSample (0, dryL);
        dryR = inputHighPass.processSample (1, dryR);

        const float mono = 0.5f * (dryL + dryR);
        harmonicTamer.processDetector (mono);
        staticTamer.processDetector (mono);

        float l = harmonicTamer.processSample (0, dryL);
        float r = harmonicTamer.processSample (1, dryR);
        l = staticTamer.processSample (0, l);
        r = staticTamer.processSample (1, r);

        l = ducker.processSample (0, l);
        r = ducker.processSample (1, r);

        l = exciter.processSample (0, l);
        r = exciter.processSample (1, r);

        const float noteGain = noteComp.processSample (0.5f * (l + r), f0, confidence);
        l *= noteGain;
        r *= noteGain;

        // Bass-mono: below the crossover the low band is collapsed so the low
        // end stays phase-coherent, above it the stereo image is untouched.
        float lowL = 0.0f, highL = 0.0f, lowR = 0.0f, highR = 0.0f;
        monoCrossover.split (0, l, lowL, highL);
        monoCrossover.split (1, r, lowR, highR);
        const float lowMono = 0.5f * (lowL + lowR);
        l = lowMono + highL;
        r = lowMono + highR;

        if (listen)
        {
            // What was taken out, so the producer can hear whether the plugin is
            // removing mud or removing the bass.
            L[n] = dryL - l;
            R[n] = dryR - r;
        }
        else
        {
            L[n] = dryL + wet * (l - dryL);
            R[n] = dryR + wet * (r - dryR);
        }

        totalEnergy += mono * mono;
    }

    harmonicTamer.updateReductions (confidence, numSamples);
    staticTamer.updateReductions (1.0f, numSamples);
    ducker.updateDucking (numSamples);

    // Publish what the kick side can use: our current note.
    if (linkOn)
    {
        Link::BassFrame frame;
        frame.fundamentalHz = f0;
        frame.confidence = confidence;
        frame.broadbandDb = DspCommon::gainToDb (std::sqrt (totalEnergy / juce::jmax (1, numSamples)));
        Link::getChannel (linkChannel).bass.publish (hostPosition - delaySamples, frame);
    }

    juce::ignoreUnused (mudEnergy);

    uiFundamentalHz.store (f0);
    uiPitchConfidence.store (confidence);
    uiMaxTameDb.store (juce::jmax (harmonicTamer.getMaxReductionDb(), staticTamer.getMaxReductionDb()));
    uiMaxDuckDb.store (ducker.getMaxDuckDb());
    uiNoteSpreadDb.store (noteComp.getNoteSpreadDb());
    uiNoteCompDb.store (noteComp.getCompensationDb());
    uiCurrentNote.store (noteComp.getCurrentNote());
    uiLearnedNotes.store (noteComp.getLearnedNoteCount());
    uiWorstHarmonic.store (harmonicTamer.getWorstBandIndex());
    uiPartnerPresent.store (partnerPresent);
    uiUsingSidechain.store (usingSidechain);
}

void TechHouseDuoProcessor::processKickMode (juce::AudioBuffer<float>& buffer, int64_t hostPosition)
{
    auto mainBus = getBusBuffer (buffer, false, 0);
    const int numSamples = mainBus.getNumSamples();
    const int numChannels = mainBus.getNumChannels();

    auto* L = mainBus.getWritePointer (0);
    auto* R = numChannels > 1 ? mainBus.getWritePointer (1) : L;

    KickShaper::Params ksP;
    ksP.subFreq = params.kickSubFreq->load();
    ksP.topFreq = params.kickTopFreq->load();
    ksP.subGainDb = params.kickSubGain->load();
    ksP.topGainDb = params.kickTopGain->load();
    ksP.tailAmount = params.kickTailAmt->load() * 0.01f;
    ksP.tailMs = params.kickTailMs->load();
    ksP.attackAmount = params.kickAttack->load() * 0.01f;
    kickShaper.setParams (ksP);

    SpectralTamer::Params boxP;
    boxP.depth = params.kickBoxiness->load() * 0.01f;
    boxP.toleranceDb = 3.0f;
    boxP.maxCutDb = 12.0f;
    boxP.protectFirstBand = 0.0f;
    kickTamer.setParams (boxP);

    // --- Reverse link: let the kick step aside for the bass note.
    // The Bass instance publishes the fundamental it is tracking, so the kick
    // can notch exactly that frequency instead of the producer guessing at a
    // static EQ cut that only suits one note of the bassline. ---
    const int linkChannel = (int) params.linkChannel->load();
    const bool linkOn = params.linkEnable->load() > 0.5f;
    const bool partnerPresent = linkOn && Link::isBassPresent (linkChannel);
    const float bassAware = params.kickBassAware->load() * 0.01f;
    float notchDb = 0.0f;

    if (partnerPresent && bassAware > 0.001f)
    {
        Link::BassFrame frame;
        auto& ring = Link::getChannel (linkChannel).bass;
        if (ring.readAt (hostPosition, frame) || ring.readLatest (frame))
        {
            if (frame.fundamentalHz > 20.0f && frame.confidence > 0.4f)
            {
                notchDb = bassAware * 9.0f * juce::jlimit (0.0f, 1.0f, frame.confidence);
                bassAwareNotch.setCoefficients (juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                    currentSampleRate, frame.fundamentalHz, 3.0f,
                    DspCommon::dbToGain (-bassAwareSmoother.processOverSamples (notchDb, numSamples))));
            }
        }
    }

    if (notchDb <= 0.0f)
    {
        // Relax the notch back to flat rather than leaving the last cut in place
        // when the bass stops playing.
        const float relaxed = bassAwareSmoother.processOverSamples (0.0f, numSamples);
        if (relaxed > 0.01f)
            bassAwareNotch.setCoefficients (juce::dsp::IIR::Coefficients<float>::makePeakFilter (
                currentSampleRate, 60.0, 3.0f, DspCommon::dbToGain (-relaxed)));
    }

    const bool listen = params.listenMode->load() > 0.5f;
    const float wet = juce::jlimit (0.0f, 1.0f, params.dryWet->load() * 0.01f);
    const bool applyNotch = bassAwareSmoother.getValue() > 0.01f;

    for (int n = 0; n < numSamples; ++n)
    {
        float dryL = inputHighPass.processSample (0, L[n]);
        float dryR = inputHighPass.processSample (1, R[n]);

        const float mono = 0.5f * (dryL + dryR);
        kickShaper.processDetector (mono);
        kickTamer.processDetector (mono);

        float l = kickShaper.processSample (0, dryL);
        float r = kickShaper.processSample (1, dryR);

        l = kickTamer.processSample (0, l);
        r = kickTamer.processSample (1, r);

        if (applyNotch)
        {
            l = bassAwareNotch.processSample (0, l);
            r = bassAwareNotch.processSample (1, r);
        }

        float outL, outR;
        if (listen)
        {
            outL = dryL - l;
            outR = dryR - r;
        }
        else
        {
            outL = dryL + wet * (l - dryL);
            outR = dryR + wet * (r - dryR);
        }

        L[n] = outL;
        R[n] = outR;

        // Analyse and publish the *processed* kick -- that is what will actually
        // reach the master and collide with the bass, so it is what the bass
        // side should be ducking against.
        kickAnalyzer.processSample (0.5f * (outL + outR));

        if (linkOn && ++publishCounter >= publishInterval)
        {
            publishCounter = 0;
            Link::KickFrame frame;
            kickAnalyzer.fill (frame);
            frame.transient = kickShaper.getTransientStrength();
            Link::getChannel (linkChannel).kick.publish (hostPosition + n, frame);
        }
    }

    kickTamer.updateReductions (1.0f, numSamples);

    uiTransient.store (kickShaper.getTransientStrength());
    uiMaxTameDb.store (kickTamer.getMaxReductionDb());
    uiPartnerPresent.store (partnerPresent);
    uiFundamentalHz.store (0.0f);
    uiPitchConfidence.store (0.0f);
}

juce::AudioProcessorEditor* TechHouseDuoProcessor::createEditor()
{
    return new TechHouseDuoEditor (*this);
}

void TechHouseDuoProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void TechHouseDuoProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TechHouseDuoProcessor();
}
