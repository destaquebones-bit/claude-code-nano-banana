#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr int headerHeight = 42;
    constexpr int spectrumHeight = 158;
    constexpr int statusHeight = 44;
    constexpr int spacing = 8;
}

void StatusStrip::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    g.setColour (Palette::panelDeep);
    g.fillRoundedRectangle (bounds, 5.0f);
    g.setColour (Palette::sectionEdge);
    g.drawRoundedRectangle (bounds.reduced (0.5f), 5.0f, 1.0f);

    auto area = bounds.reduced (10.0f, 6.0f);

    // Link state gets a lamp rather than a sentence: it is the one thing that
    // silently invalidates everything else on screen if it is wrong.
    const auto lampColour = partnerPresent ? Palette::good
                                            : (usingSidechain ? Palette::amber : Palette::textDim);
    auto lamp = juce::Rectangle<float> (9.0f, 9.0f).withCentre ({ area.getX() + 5.0f, area.getY() + 8.0f });
    g.setColour (lampColour.withAlpha (0.25f));
    g.fillEllipse (lamp.expanded (3.0f));
    g.setColour (lampColour);
    g.fillEllipse (lamp);

    juce::String linkText;
    if (partnerPresent)
        linkText = "LINK " + linkName + " connected to " + juce::String (kickMode ? "Bass" : "Kick");
    else if (usingSidechain)
        linkText = "LINK " + linkName + " no partner - using sidechain input";
    else
        linkText = "LINK " + linkName + " waiting for a " + juce::String (kickMode ? "Bass" : "Kick") + " instance";

    g.setColour (lampColour);
    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.drawFittedText (linkText, (int) area.getX() + 16, (int) area.getY(),
                       (int) area.getWidth() - 16, 14, juce::Justification::centredLeft, 1);

    // Second line: the plain-language verdict, the thing the pictures above
    // cannot state outright.
    juce::String verdict;
    juce::Colour verdictColour = Palette::textDim;

    if (kickMode)
    {
        verdict = juce::String::formatted ("Transient %.0f%%    Boxiness cut -%.1f dB", transient * 100.0f, tameDb);
        if (! partnerPresent)
            verdict += "    (add a Bass instance to enable Bass-Aware)";
    }
    else if (learnedNotes >= 2 && noteSpreadDb > 3.0f)
    {
        verdict = juce::String::formatted ("One-note bass: %.1f dB spread across %d notes  (correcting %+.1f dB)",
                                            noteSpreadDb, learnedNotes, noteCompDb);
        verdictColour = Palette::cut;
    }
    else if (worstHarmonic >= 0)
    {
        verdict = juce::String::formatted ("Harmonic %d is resonant - cutting %.1f dB", worstHarmonic + 1, tameDb);
        verdictColour = Palette::amber;
    }
    else if (duckDb > 0.5f)
    {
        verdict = juce::String::formatted ("Making room for the kick - up to %.1f dB", duckDb);
        verdictColour = Palette::amber;
    }
    else
    {
        verdict = learnedNotes >= 2 ? "Bassline is even, nothing standing out."
                                     : "Learning the bassline - play a few bars.";
        verdictColour = learnedNotes >= 2 ? Palette::good : Palette::textDim;
    }

    g.setColour (verdictColour);
    g.setFont (11.5f);
    g.drawFittedText (verdict, (int) area.getX() + 16, (int) area.getY() + 15,
                       (int) area.getWidth() - 16, 14, juce::Justification::centredLeft, 1);
}

TechHouseDuoEditor::TechHouseDuoEditor (TechHouseDuoProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      globalSection    (p.apvts, "GLOBAL"),
      tameSection      (p.apvts, "RESONANCE TAMING  -  note-aware"),
      duckSection      (p.apvts, "KICK DUCKING  -  spectral"),
      noteSection      (p.apvts, "NOTE LEVELLING"),
      exciteSection    (p.apvts, "HARMONIC EXCITE  -  translation"),
      kickShapeSection (p.apvts, "KICK SHAPE"),
      kickToneSection  (p.apvts, "KICK TONE")
{
    setLookAndFeel (&lookAndFeel);

    modeLabel.setText ("MODE", juce::dontSendNotification);
    modeLabel.setJustificationType (juce::Justification::centredRight);
    modeLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    modeLabel.setColour (juce::Label::textColourId, Palette::textDim);
    addAndMakeVisible (modeLabel);
    modeBox.addItemList ({ "Bass", "Kick" }, 1);
    addAndMakeVisible (modeBox);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.apvts, ParamIDs::mode, modeBox);
    modeBox.onChange = [this] { applyModeVisibility(); };

    linkLabel.setText ("LINK", juce::dontSendNotification);
    linkLabel.setJustificationType (juce::Justification::centredRight);
    linkLabel.setFont (juce::Font (10.0f, juce::Font::bold));
    linkLabel.setColour (juce::Label::textColourId, Palette::textDim);
    addAndMakeVisible (linkLabel);
    linkBox.addItemList (ParamIDs::linkChannelNames(), 1);
    addAndMakeVisible (linkBox);
    linkAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.apvts, ParamIDs::linkChannel, linkBox);

    globalSection.addToggle (ParamIDs::linkEnable, "Link");
    globalSection.addToggle (ParamIDs::listenMode, "Listen");
    globalSection.addToggle (ParamIDs::bypass, "Bypass");
    globalSection.addKnob (ParamIDs::inputHpf, "INPUT HPF");
    globalSection.addKnob (ParamIDs::dryWet, "DRY/WET");
    globalSection.addKnob (ParamIDs::outputGain, "OUTPUT");

    tameSection.addKnob (ParamIDs::tameDepth, "DEPTH");
    tameSection.addKnob (ParamIDs::tameTolerance, "TOLERANCE");
    tameSection.addKnob (ParamIDs::tameMaxCut, "MAX CUT");
    tameSection.addKnob (ParamIDs::tameRelease, "RELEASE");
    tameSection.addKnob (ParamIDs::harmonicCount, "HARMONICS");
    tameSection.addKnob (ParamIDs::mudDepth, "MUD FIXED");

    duckSection.addKnob (ParamIDs::duckAmount, "AMOUNT");
    duckSection.addKnob (ParamIDs::duckThreshold, "THRESHOLD");
    duckSection.addKnob (ParamIDs::duckRelease, "RELEASE");
    duckSection.setExtra (&duckMeter, 62);

    noteSection.addKnob (ParamIDs::noteCompAmount, "LEVELLING");
    noteSection.setExtra (&noteMap, 76);
    noteSection.setFooter (&relearnButton, 24);
    relearnButton.onClick = [this] { processorRef.requestNoteRelearn.store (true); };

    exciteSection.addKnob (ParamIDs::exciteAmount, "AMOUNT");
    exciteSection.addKnob (ParamIDs::exciteBalance, "EVEN/ODD");
    exciteSection.addKnob (ParamIDs::monoBelow, "MONO BELOW");

    kickShapeSection.addKnob (ParamIDs::kickTailAmt, "TAIL TIGHTEN");
    kickShapeSection.addKnob (ParamIDs::kickTailMs, "TAIL LENGTH");
    kickShapeSection.addKnob (ParamIDs::kickAttack, "ATTACK");
    kickShapeSection.addKnob (ParamIDs::kickBassAware, "BASS-AWARE");

    kickToneSection.addKnob (ParamIDs::kickSubFreq, "SUB SPLIT");
    kickToneSection.addKnob (ParamIDs::kickSubGain, "SUB GAIN");
    kickToneSection.addKnob (ParamIDs::kickTopFreq, "CLICK SPLIT");
    kickToneSection.addKnob (ParamIDs::kickTopGain, "CLICK GAIN");
    kickToneSection.addKnob (ParamIDs::kickBoxiness, "BOXINESS");

    content.addAndMakeVisible (spectrum);
    content.addAndMakeVisible (status);
    for (auto* s : { &globalSection, &tameSection, &duckSection, &noteSection,
                      &exciteSection, &kickShapeSection, &kickToneSection })
        content.addAndMakeVisible (s);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, false);

    applyModeVisibility();
    setResizable (true, true);
    setResizeLimits (560, 480, 1200, 1800);
    setSize (720, 800);
    startTimerHz (24);
}

TechHouseDuoEditor::~TechHouseDuoEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void TechHouseDuoEditor::applyModeVisibility()
{
    const bool kickMode = processorRef.isKickMode();
    lastKickMode = kickMode;

    tameSection.setVisible (! kickMode);
    duckSection.setVisible (! kickMode);
    noteSection.setVisible (! kickMode);
    exciteSection.setVisible (! kickMode);
    kickShapeSection.setVisible (kickMode);
    kickToneSection.setVisible (kickMode);

    resized();
}

void TechHouseDuoEditor::paint (juce::Graphics& g)
{
    juce::ColourGradient bg (Palette::panel, 0.0f, 0.0f,
                              Palette::panelDeep, 0.0f, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    // Header plate, with a hairline to separate it from the scrolling body.
    auto header = getLocalBounds().removeFromTop (headerHeight).toFloat();
    juce::ColourGradient plate (Palette::section, 0.0f, header.getY(),
                                 Palette::section.darker (0.35f), 0.0f, header.getBottom(), false);
    g.setGradientFill (plate);
    g.fillRect (header);
    g.setColour (Palette::amber.withAlpha (0.5f));
    g.drawHorizontalLine ((int) header.getBottom() - 1, header.getX(), header.getRight());

    g.setColour (Palette::text);
    g.setFont (juce::Font (15.0f, juce::Font::bold));
    g.drawFittedText ("TECHHOUSE", 14, 0, 110, headerHeight, juce::Justification::centredLeft, 1);
    g.setColour (Palette::amber);
    g.setFont (juce::Font (15.0f, juce::Font::bold));
    g.drawFittedText ("DUO", 108, 0, 60, headerHeight, juce::Justification::centredLeft, 1);
}

void TechHouseDuoEditor::resized()
{
    auto b = getLocalBounds();

    auto header = b.removeFromTop (headerHeight).reduced (10, 8);
    header.removeFromLeft (150); // title
    linkBox.setBounds (header.removeFromRight (60));
    linkLabel.setBounds (header.removeFromRight (38));
    header.removeFromRight (10);
    modeBox.setBounds (header.removeFromRight (90));
    modeLabel.setBounds (header.removeFromRight (44));

    viewport.setBounds (b);

    const int innerWidth = juce::jmax (320, viewport.getWidth() - viewport.getScrollBarThickness());
    const int columnWidth = innerWidth - 2 * spacing;
    int y = spacing;

    spectrum.setBounds (spacing, y, columnWidth, spectrumHeight);
    y += spectrumHeight + spacing;

    status.setBounds (spacing, y, columnWidth, statusHeight);
    y += statusHeight + spacing;

    for (auto* s : { &globalSection, &tameSection, &duckSection, &noteSection,
                      &exciteSection, &kickShapeSection, &kickToneSection })
    {
        if (! s->isVisible())
            continue;
        const int h = s->getPreferredHeight (columnWidth);
        s->setBounds (spacing, y, columnWidth, h);
        y += h + spacing;
    }

    content.setSize (innerWidth, y);
}

void TechHouseDuoEditor::refreshVisualisers()
{
    const bool kickMode = processorRef.isKickMode();
    const float confidence = processorRef.uiPitchConfidence.load();
    const float f0 = processorRef.uiFundamentalHz.load();

    spectrum.numBands = juce::jmin (SpectrumView::maxBands, processorRef.uiNumBands.load());
    spectrum.noteAware = ! kickMode && confidence > 0.5f && f0 > 0.0f;
    spectrum.fundamentalHz = f0;
    spectrum.noteName = spectrum.noteAware
        ? DspCommon::midiNoteName (DspCommon::freqToMidiNote (f0))
        : juce::String ("--");

    for (int b = 0; b < SpectrumView::maxBands; ++b)
    {
        spectrum.bandFreq[b] = processorRef.uiBandFreq[(size_t) b].load();
        spectrum.bandLevelDb[b] = processorRef.uiBandLevelDb[(size_t) b].load();
        spectrum.bandCutDb[b] = processorRef.uiBandCutDb[(size_t) b].load();
    }
    spectrum.repaint();

    for (int b = 0; b < Link::numBands; ++b)
    {
        duckMeter.duckDb[b] = processorRef.uiDuckDb[(size_t) b].load();
        duckMeter.kickBandDb[b] = processorRef.uiKickBandDb[(size_t) b].load();
    }
    if (duckMeter.isShowing())
        duckMeter.repaint();

    // Note map: collect the learned notes, sorted by pitch, expressed as a
    // deviation from the average so the picture is about balance, not level.
    if (noteMap.isShowing())
    {
        const float average = processorRef.uiNoteAverageDb.load();
        int count = 0;
        for (int n = 0; n < 128 && count < NoteMap::maxEntries; ++n)
        {
            if (processorRef.uiNoteObs[(size_t) n].load() < 3)
                continue;
            noteMap.notes[count].midiNote = n;
            noteMap.notes[count].deviationDb = processorRef.uiNoteLevelDb[(size_t) n].load() - average;
            noteMap.notes[count].name = DspCommon::midiNoteName (n);
            ++count;
        }
        noteMap.noteCount = count;
        noteMap.currentNote = processorRef.uiCurrentNote.load();
        noteMap.repaint();
    }
}

void TechHouseDuoEditor::timerCallback()
{
    const bool kickMode = processorRef.isKickMode();
    if (kickMode != lastKickMode)
        applyModeVisibility();

    status.kickMode = kickMode;
    status.partnerPresent = processorRef.uiPartnerPresent.load();
    status.usingSidechain = processorRef.uiUsingSidechain.load();
    status.tameDb = processorRef.uiMaxTameDb.load();
    status.duckDb = processorRef.uiMaxDuckDb.load();
    status.noteSpreadDb = processorRef.uiNoteSpreadDb.load();
    status.noteCompDb = processorRef.uiNoteCompDb.load();
    status.transient = processorRef.uiTransient.load();
    status.learnedNotes = processorRef.uiLearnedNotes.load();
    status.worstHarmonic = processorRef.uiWorstHarmonic.load();
    status.linkName = ParamIDs::linkChannelNames()[juce::jmax (0, linkBox.getSelectedItemIndex())];
    status.repaint();

    refreshVisualisers();
}
