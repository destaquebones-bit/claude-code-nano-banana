#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    constexpr int editorWidth = 640;
    constexpr int sectionSpacing = 8;
}

MasteringChainAudioProcessorEditor::MasteringChainAudioProcessorEditor (MasteringChainAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      inputSection   (p.apvts, "INPUT", nullptr),
      eqSection      (p.apvts, "ANALOG EQ", ParamIDs::eqBypass),
      compSection    (p.apvts, "GLUE COMPRESSOR", ParamIDs::compBypass),
      satSection     (p.apvts, "TAPE SATURATION", ParamIDs::satBypass),
      widthSection   (p.apvts, "STEREO WIDTH", ParamIDs::widthBypass),
      limiterSection (p.apvts, "LIMITER (TRUE PEAK)", ParamIDs::limBypass),
      outputSection  (p.apvts, "OUTPUT", ParamIDs::masterBypass)
{
    // --- Input ---
    inputSection.addKnob (ParamIDs::inputGainDb, "Gain");
    inputSection.addToggle (ParamIDs::inputInvert, "Phase Invert");

    // --- EQ ---
    eqSection.addKnob (ParamIDs::eqLowFreq, "Low Freq");
    eqSection.addKnob (ParamIDs::eqLowBoostDb, "Low Boost");
    eqSection.addKnob (ParamIDs::eqLowCutDb, "Low Tighten");
    eqSection.addKnob (ParamIDs::eqLowMidFreq, "LoMid Freq");
    eqSection.addKnob (ParamIDs::eqLowMidGainDb, "LoMid Gain");
    eqSection.addKnob (ParamIDs::eqLowMidQ, "LoMid Q");
    eqSection.addKnob (ParamIDs::eqHighMidFreq, "HiMid Freq");
    eqSection.addKnob (ParamIDs::eqHighMidGainDb, "HiMid Gain");
    eqSection.addKnob (ParamIDs::eqHighMidQ, "HiMid Q");
    eqSection.addKnob (ParamIDs::eqAirFreq, "Air Freq");
    eqSection.addKnob (ParamIDs::eqAirGainDb, "Air Gain");
    eqSection.addKnob (ParamIDs::eqIronAmount, "Iron");

    // --- Compressor ---
    compSection.addKnob (ParamIDs::compThreshold, "Threshold");
    compSection.addKnob (ParamIDs::compRatio, "Ratio");
    compSection.addKnob (ParamIDs::compAttack, "Attack");
    compSection.addKnob (ParamIDs::compRelease, "Release");
    compSection.addKnob (ParamIDs::compKnee, "Knee");
    compSection.addKnob (ParamIDs::compMakeup, "Makeup");
    compSection.addKnob (ParamIDs::compMix, "Mix");
    compSection.addToggle (ParamIDs::compAutoRelease, "Auto Release");
    compSection.addToggle (ParamIDs::compAutoMakeup, "Auto Makeup");

    // --- Saturator ---
    satSection.addKnob (ParamIDs::satDrive, "Drive");
    satSection.addKnob (ParamIDs::satWarmth, "Warmth");
    satSection.addKnob (ParamIDs::satMix, "Mix");

    // --- Width ---
    widthSection.addKnob (ParamIDs::widthAmount, "Width");
    widthSection.addKnob (ParamIDs::widthBassMonoHz, "Bass Mono <");

    // --- Limiter ---
    limiterSection.addKnob (ParamIDs::limCeiling, "Ceiling");
    limiterSection.addKnob (ParamIDs::limDrive, "Drive");
    limiterSection.addKnob (ParamIDs::limRelease, "Release");

    // --- Output ---
    outputSection.addKnob (ParamIDs::outputGainDb, "Gain");
    outputSection.addToggle (ParamIDs::outputMonoSafety, "Mono Safety Check");

    for (auto* section : { &inputSection, &eqSection, &compSection, &satSection,
                            &widthSection, &limiterSection, &outputSection })
        content.addAndMakeVisible (section);

    content.addAndMakeVisible (meterPanel);

    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, false);

    // Total content height = sum of each section's preferred height + the
    // meter panel + spacing between them -- computed once here since the
    // section list/knob counts never change after construction.
    int totalHeight = sectionSpacing;
    for (auto* section : { &inputSection, &eqSection, &compSection, &satSection,
                            &widthSection, &limiterSection, &outputSection })
        totalHeight += section->getPreferredHeight() + sectionSpacing;
    totalHeight += 100 + sectionSpacing; // meter panel

    content.setSize (editorWidth, totalHeight);

    setResizable (true, true);
    setResizeLimits (420, 400, 1200, 2000);
    setSize (editorWidth + 20, 760);

    startTimerHz (20);
}

MasteringChainAudioProcessorEditor::~MasteringChainAudioProcessorEditor()
{
    stopTimer();
}

void MasteringChainAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff2b2d31));
}

void MasteringChainAudioProcessorEditor::resized()
{
    viewport.setBounds (getLocalBounds());
    content.setSize (viewport.getWidth() - viewport.getScrollBarThickness(), content.getHeight());

    auto b = content.getLocalBounds().reduced (sectionSpacing, 0);
    int y = sectionSpacing;

    meterPanel.setBounds (b.getX(), y, b.getWidth(), 100);
    y += 100 + sectionSpacing;

    for (auto* section : { &inputSection, &eqSection, &compSection, &satSection,
                            &widthSection, &limiterSection, &outputSection })
    {
        const int h = section->getPreferredHeight();
        section->setBounds (b.getX(), y, b.getWidth(), h);
        y += h + sectionSpacing;
    }
}

void MasteringChainAudioProcessorEditor::timerCallback()
{
    meterPanel.momentaryLufs = processorRef.uiMomentaryLufs.load();
    meterPanel.integratedLufs = processorRef.uiIntegratedLufs.load();
    meterPanel.truePeakDb = processorRef.uiTruePeakDb.load();
    meterPanel.compGrDb = processorRef.uiCompGainReductionDb.load();
    meterPanel.limGrDb = processorRef.uiLimGainReductionDb.load();
    meterPanel.repaint();
}
