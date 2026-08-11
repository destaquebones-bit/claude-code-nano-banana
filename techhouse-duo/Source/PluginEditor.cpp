#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr int contentWidth = 620;
    constexpr int spacing = 8;
    constexpr int diagnosisHeight = 132;
}

void DiagnosisPanel::paint (juce::Graphics& g)
{
    g.setColour (juce::Colour (0xff17181c));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);

    auto area = getLocalBounds().reduced (12, 10);
    int y = area.getY();

    auto line = [&] (const juce::String& text, juce::Colour colour, float size)
    {
        g.setColour (colour);
        g.setFont (size);
        g.drawFittedText (text, area.getX(), y, area.getWidth(), 17, juce::Justification::centredLeft, 1);
        y += 17;
    };

    const auto white = juce::Colours::white;
    const auto dim = juce::Colours::white.withAlpha (0.55f);
    const auto good = juce::Colour (0xff5fd38a);
    const auto warn = juce::Colour (0xffe6b34a);

    // Link status first: if the pair isn't actually talking, nothing else on
    // this panel means what the user thinks it means.
    if (partnerPresent)
        line ("Link " + linkName + ": connected to " + juce::String (kickMode ? "Bass" : "Kick") + " instance", good, 13.0f);
    else if (usingSidechain)
        line ("Link " + linkName + ": no partner - using sidechain input", warn, 13.0f);
    else
        line ("Link " + linkName + ": waiting for a " + juce::String (kickMode ? "Bass" : "Kick") + " instance", dim, 13.0f);

    if (kickMode)
    {
        line (juce::String::formatted ("Transient: %.0f%%    Boxiness cut: -%.1f dB", transient * 100.0f, tameDb), white, 13.0f);
        line (partnerPresent
                  ? "Bass-Aware Notch can follow the bass note."
                  : "Add a Bass instance on the same Link to enable Bass-Aware Notch.",
              dim, 12.0f);
        return;
    }

    // --- Bass mode diagnosis ---
    if (confidence > 0.5f && fundamentalHz > 0.0f)
        line (juce::String::formatted ("Note: %s (%.1f Hz)   confidence %.0f%%",
                                        DspCommon::midiNoteName (DspCommon::freqToMidiNote (fundamentalHz)).toRawUTF8(),
                                        fundamentalHz, confidence * 100.0f), white, 13.0f);
    else
        line ("Note: tracking... (note-aware cuts fade out while unsure)", dim, 13.0f);

    line (juce::String::formatted ("Resonance cut: -%.1f dB    Kick ducking: -%.1f dB", tameDb, duckDb), white, 13.0f);

    if (worstHarmonic >= 0)
        line (juce::String::formatted ("Worst offender: harmonic %d of the current note", worstHarmonic + 1), warn, 12.0f);
    else
        line ("No harmonic standing out right now.", dim, 12.0f);

    if (learnedNotes >= 2)
    {
        const bool uneven = noteSpreadDb > 3.0f;
        line (juce::String::formatted ("Note balance: %.1f dB spread across %d notes%s  (now %+.1f dB)",
                                        noteSpreadDb, learnedNotes,
                                        uneven ? "  <- one-note bass" : "",
                                        noteCompDb),
              uneven ? warn : good, 12.0f);
    }
    else
    {
        line ("Note balance: learning (play a few bars of the bassline)", dim, 12.0f);
    }
}

TechHouseDuoEditor::TechHouseDuoEditor (TechHouseDuoProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      globalSection    (p.apvts, "GLOBAL"),
      tameSection      (p.apvts, "RESONANCE TAMING (note-aware)"),
      duckSection      (p.apvts, "KICK DUCKING (spectral)"),
      noteSection      (p.apvts, "NOTE LEVELLING"),
      exciteSection    (p.apvts, "HARMONIC EXCITE (translation)"),
      kickShapeSection (p.apvts, "KICK SHAPE"),
      kickToneSection  (p.apvts, "KICK TONE")
{
    modeLabel.setText ("Mode", juce::dontSendNotification);
    modeLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (modeLabel);
    modeBox.addItemList ({ "Bass", "Kick" }, 1);
    addAndMakeVisible (modeBox);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.apvts, ParamIDs::mode, modeBox);
    modeBox.onChange = [this] { applyModeVisibility(); };

    linkLabel.setText ("Link", juce::dontSendNotification);
    linkLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (linkLabel);
    linkBox.addItemList (ParamIDs::linkChannelNames(), 1);
    addAndMakeVisible (linkBox);
    linkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.apvts, ParamIDs::linkChannel, linkBox);

    globalSection.addToggle (ParamIDs::linkEnable, "Link");
    globalSection.addToggle (ParamIDs::listenMode, "Listen");
    globalSection.addToggle (ParamIDs::bypass, "Bypass");
    globalSection.addKnob (ParamIDs::inputHpf, "Input HPF");
    globalSection.addKnob (ParamIDs::dryWet, "Dry/Wet");
    globalSection.addKnob (ParamIDs::outputGain, "Output");

    tameSection.addKnob (ParamIDs::tameDepth, "Depth");
    tameSection.addKnob (ParamIDs::tameTolerance, "Tolerance");
    tameSection.addKnob (ParamIDs::tameMaxCut, "Max Cut");
    tameSection.addKnob (ParamIDs::tameRelease, "Release");
    tameSection.addKnob (ParamIDs::harmonicCount, "Harmonics");
    tameSection.addKnob (ParamIDs::mudDepth, "Mud (fixed)");

    duckSection.addKnob (ParamIDs::duckAmount, "Amount");
    duckSection.addKnob (ParamIDs::duckThreshold, "Threshold");
    duckSection.addKnob (ParamIDs::duckRelease, "Release");

    noteSection.addKnob (ParamIDs::noteCompAmount, "Levelling");
    addAndMakeVisible (relearnButton);
    relearnButton.onClick = [this] { processorRef.requestNoteRelearn.store (true); };

    exciteSection.addKnob (ParamIDs::exciteAmount, "Amount");
    exciteSection.addKnob (ParamIDs::exciteBalance, "Even/Odd");
    exciteSection.addKnob (ParamIDs::monoBelow, "Mono Below");

    kickShapeSection.addKnob (ParamIDs::kickTailAmt, "Tail Tighten");
    kickShapeSection.addKnob (ParamIDs::kickTailMs, "Tail Length");
    kickShapeSection.addKnob (ParamIDs::kickAttack, "Attack");
    kickShapeSection.addKnob (ParamIDs::kickBassAware, "Bass-Aware");

    kickToneSection.addKnob (ParamIDs::kickSubFreq, "Sub Split");
    kickToneSection.addKnob (ParamIDs::kickSubGain, "Sub Gain");
    kickToneSection.addKnob (ParamIDs::kickTopFreq, "Click Split");
    kickToneSection.addKnob (ParamIDs::kickTopGain, "Click Gain");
    kickToneSection.addKnob (ParamIDs::kickBoxiness, "Boxiness");

    content.addAndMakeVisible (diagnosis);
    for (auto* s : { &globalSection, &tameSection, &duckSection, &noteSection,
                      &exciteSection, &kickShapeSection, &kickToneSection })
        content.addAndMakeVisible (s);
    content.addAndMakeVisible (relearnButton);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, false);

    applyModeVisibility();
    setResizable (true, true);
    setResizeLimits (460, 420, 1100, 1600);
    setSize (contentWidth + 24, 720);
    startTimerHz (20);
}

TechHouseDuoEditor::~TechHouseDuoEditor()
{
    stopTimer();
}

void TechHouseDuoEditor::applyModeVisibility()
{
    const bool kickMode = processorRef.isKickMode();
    lastKickMode = kickMode;

    tameSection.setVisible (! kickMode);
    duckSection.setVisible (! kickMode);
    noteSection.setVisible (! kickMode);
    exciteSection.setVisible (! kickMode);
    relearnButton.setVisible (! kickMode);
    kickShapeSection.setVisible (kickMode);
    kickToneSection.setVisible (kickMode);

    resized();
}

void TechHouseDuoEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff232529));
}

void TechHouseDuoEditor::resized()
{
    auto b = getLocalBounds();

    auto header = b.removeFromTop (34).reduced (10, 4);
    modeLabel.setBounds (header.removeFromLeft (46));
    modeBox.setBounds (header.removeFromLeft (96));
    header.removeFromLeft (14);
    linkLabel.setBounds (header.removeFromLeft (40));
    linkBox.setBounds (header.removeFromLeft (64));

    viewport.setBounds (b);

    const int innerWidth = juce::jmax (300, viewport.getWidth() - viewport.getScrollBarThickness());
    auto area = juce::Rectangle<int> (spacing, spacing, innerWidth - 2 * spacing, 0);
    int y = spacing;

    diagnosis.setBounds (area.getX(), y, area.getWidth(), diagnosisHeight);
    y += diagnosisHeight + spacing;

    for (auto* s : { &globalSection, &tameSection, &duckSection, &noteSection,
                      &exciteSection, &kickShapeSection, &kickToneSection })
    {
        if (! s->isVisible())
            continue;
        const int h = s->getPreferredHeight();
        s->setBounds (area.getX(), y, area.getWidth(), h);
        y += h + spacing;

        if (s == &noteSection && relearnButton.isVisible())
        {
            relearnButton.setBounds (area.getX() + 8, y, 130, 24);
            y += 24 + spacing;
        }
    }

    content.setSize (innerWidth, y);
}

void TechHouseDuoEditor::timerCallback()
{
    const bool kickMode = processorRef.isKickMode();
    if (kickMode != lastKickMode)
        applyModeVisibility();

    diagnosis.kickMode = kickMode;
    diagnosis.partnerPresent = processorRef.uiPartnerPresent.load();
    diagnosis.usingSidechain = processorRef.uiUsingSidechain.load();
    diagnosis.fundamentalHz = processorRef.uiFundamentalHz.load();
    diagnosis.confidence = processorRef.uiPitchConfidence.load();
    diagnosis.tameDb = processorRef.uiMaxTameDb.load();
    diagnosis.duckDb = processorRef.uiMaxDuckDb.load();
    diagnosis.noteSpreadDb = processorRef.uiNoteSpreadDb.load();
    diagnosis.noteCompDb = processorRef.uiNoteCompDb.load();
    diagnosis.currentNote = processorRef.uiCurrentNote.load();
    diagnosis.learnedNotes = processorRef.uiLearnedNotes.load();
    diagnosis.worstHarmonic = processorRef.uiWorstHarmonic.load();
    diagnosis.transient = processorRef.uiTransient.load();
    diagnosis.linkName = ParamIDs::linkChannelNames()[linkBox.getSelectedItemIndex()];
    diagnosis.repaint();
}
