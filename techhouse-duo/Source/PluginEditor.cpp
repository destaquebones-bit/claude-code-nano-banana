#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr int headerHeight = 56;
    constexpr int spectrumHeight = 164;
    constexpr int statusHeight = 48;
    // Generous, and deliberately so: the cards no longer have outlines, so the
    // gap between them is the only thing separating one group of controls from
    // the next. Too tight and the panel reads as one undivided field.
    constexpr int spacing = 12;
}

void StatusStrip::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (1.0f, 0.0f);
    UiStyle::drawCard (g, bounds, UiStyle::cardRadius, 0.26f);

    auto area = bounds.reduced (14.0f, 6.0f);

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

    if (allAmountsZero)
    {
        verdict = kickMode ? "All controls at zero - the kick is passing through untouched."
                            : "All controls at zero - the bass is passing through untouched.";
        verdictColour = Palette::textDim;
    }
    else if (kickMode)
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

    // Embedded rather than read from disk: a plugin cannot rely on any
    // install-relative path once a DAW has relocated its bundle.
    sunWordmark = juce::ImageCache::getFromMemory (Assets::spectral_sun_wordmark_png,
                                                    Assets::spectral_sun_wordmark_pngSize);
    sunPlate = juce::ImageCache::getFromMemory (Assets::spectral_sun_plate_png,
                                                 Assets::spectral_sun_plate_pngSize);

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
    kickShapeSection.addKnob (ParamIDs::kickDrive, "DRIVE");

    kickToneSection.addKnob (ParamIDs::kickSubFreq, "SUB SPLIT");
    kickToneSection.addKnob (ParamIDs::kickSubGain, "SUB GAIN");
    kickToneSection.addKnob (ParamIDs::kickTopFreq, "CLICK SPLIT");
    kickToneSection.addKnob (ParamIDs::kickTopGain, "CLICK GAIN");
    kickToneSection.addKnob (ParamIDs::kickBoxiness, "BOXINESS");

    makerPlate.plate = sunPlate;
    content.addAndMakeVisible (makerPlate);
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
    UiStyle::paintBackdrop (g, getLocalBounds().toFloat());

    // Header: no plate and no hairline under it. The old version drew a filled
    // rectangle with a hard amber rule along the bottom, which put a box across
    // the top of the window. Here the header is only a slightly lifted wash that
    // fades out downward, plus a short amber mark that stops well before the
    // edges, so the eye reads a zone rather than a border.
    auto header = getLocalBounds().removeFromTop (headerHeight).toFloat();
    juce::ColourGradient wash (juce::Colours::white.withAlpha (0.035f), 0.0f, header.getY(),
                                juce::Colours::transparentBlack, 0.0f, header.getBottom(), false);
    g.setGradientFill (wash);
    g.fillRect (header);

    const float markInset = 14.0f;
    juce::ColourGradient rule (Palette::amber.withAlpha (0.0f), header.getX() + markInset, 0.0f,
                                Palette::amber.withAlpha (0.0f), header.getRight() - markInset, 0.0f, false);
    rule.addColour (0.18, Palette::amber.withAlpha (0.34f));
    rule.addColour (0.55, Palette::amber.withAlpha (0.10f));
    g.setGradientFill (rule);
    g.fillRect (juce::Rectangle<float> (header.getX() + markInset, header.getBottom() - 1.0f,
                                         header.getWidth() - 2.0f * markInset, 1.0f));

    // Spectral Sun mark, then the product name. The mark carries the amber so
    // the wordmark beside it stays quiet and the two do not compete.
    if (sunWordmark.isValid())
    {
        // The mark carries the identity on its own; a product wordmark beside
        // it competed with it for the same glance and added nothing the plugin
        // window title does not already say.
        const int logoHeight = headerHeight - 14;
        const int logoWidth = juce::roundToInt (logoHeight * sunWordmark.getWidth()
                                                 / (float) juce::jmax (1, sunWordmark.getHeight()));
        g.drawImageWithin (sunWordmark, 14, 7, logoWidth, logoHeight,
                            juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
    }
}

void TechHouseDuoEditor::resized()
{
    auto b = getLocalBounds();

    auto header = b.removeFromTop (headerHeight).reduced (10, 8);
    header.removeFromLeft (260); // Spectral Sun mark + product name
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

    constexpr int plateHeight = 66;
    makerPlate.setBounds (spacing, y, columnWidth, plateHeight);
    y += plateHeight + spacing;

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

    // Only the amounts belonging to the active mode count. Shape parameters
    // (splits, tolerances, release times) are excluded deliberately: they hold
    // musical values by default and are inert while their amount is zero, so
    // including them would never let the message appear.
    {
        auto amount = [this] (const char* id)
        {
            auto* p = processorRef.apvts.getRawParameterValue (id);
            return p != nullptr ? std::abs (p->load()) : 0.0f;
        };

        const float total = kickMode
            ? amount (ParamIDs::kickTailAmt) + amount (ParamIDs::kickAttack)
                + amount (ParamIDs::kickBoxiness) + amount (ParamIDs::kickBassAware)
                + amount (ParamIDs::kickDrive) + amount (ParamIDs::kickSubGain)
                + amount (ParamIDs::kickTopGain)
            : amount (ParamIDs::tameDepth) + amount (ParamIDs::mudDepth)
                + amount (ParamIDs::noteCompAmount) + amount (ParamIDs::duckAmount)
                + amount (ParamIDs::exciteAmount);

        status.allAmountsZero = total < 0.01f;
    }
    status.repaint();

    refreshVisualisers();
}
