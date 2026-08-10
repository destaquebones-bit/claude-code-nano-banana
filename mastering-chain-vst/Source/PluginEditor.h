#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

// A single rotary knob + label + APVTS attachment. Building the UI out of
// these (driven from small per-section tables in PluginEditor.cpp) keeps
// ~35 parameters from turning into ~35 hand-written member variables.
struct KnobControl : public juce::Component
{
    KnobControl (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& displayName)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 16);
        addAndMakeVisible (slider);

        label.setText (displayName, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (11.0f));
        addAndMakeVisible (label);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramID, slider);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (14));
        slider.setBounds (b);
    }

    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// A titled group box containing bypass toggle + a wrapping grid of knobs.
struct ChainSection : public juce::Component
{
    ChainSection (juce::AudioProcessorValueTreeState& state, const juce::String& title,
                  const char* bypassParamID = nullptr)
        : apvts (state)
    {
        group.setText (title);
        addAndMakeVisible (group);

        if (bypassParamID != nullptr)
        {
            bypassButton.setButtonText ("Bypass");
            addAndMakeVisible (bypassButton);
            bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
                apvts, bypassParamID, bypassButton);
        }
    }

    void addKnob (const juce::String& paramID, const juce::String& displayName)
    {
        auto* k = new KnobControl (apvts, paramID, displayName);
        addAndMakeVisible (k);
        knobs.add (k);
    }

    // For boolean parameters that aren't the section's main bypass
    // (Phase Invert, Auto Release, Auto Makeup, Mono Safety...).
    void addToggle (const juce::String& paramID, const juce::String& displayText)
    {
        auto* button = new juce::ToggleButton (displayText);
        addAndMakeVisible (button);
        extraToggleButtons.add (button);
        extraToggleAttachments.add (new juce::AudioProcessorValueTreeState::ButtonAttachment (apvts, paramID, *button));
    }

    void resized() override
    {
        auto b = getLocalBounds();
        group.setBounds (b);
        b.removeFromTop (22); // room for the group's title
        b.reduce (8, 6);

        if (bypassAttachment != nullptr)
            bypassButton.setBounds (b.removeFromTop (20));

        if (! extraToggleButtons.isEmpty())
        {
            auto row = b.removeFromTop (20);
            const int w = row.getWidth() / extraToggleButtons.size();
            for (auto* t : extraToggleButtons)
                t->setBounds (row.removeFromLeft (w));
        }

        juce::Grid grid;
        grid.autoFlow = juce::Grid::AutoFlow::row;
        grid.templateColumns = { juce::Grid::TrackInfo (juce::Grid::Px (84)),
                                  juce::Grid::TrackInfo (juce::Grid::Px (84)),
                                  juce::Grid::TrackInfo (juce::Grid::Px (84)),
                                  juce::Grid::TrackInfo (juce::Grid::Px (84)) };
        grid.autoRows = juce::Grid::TrackInfo (juce::Grid::Px (92));
        grid.columnGap = juce::Grid::Px (4);
        grid.rowGap = juce::Grid::Px (4);

        for (auto* k : knobs)
            grid.items.add (juce::GridItem (*k));

        grid.performLayout (b);
    }

    int getPreferredHeight() const
    {
        constexpr int cols = 4; // must match the fixed templateColumns count in resized()
        const int rows = (knobs.size() + cols - 1) / cols;
        return 22 + (bypassAttachment != nullptr ? 22 : 0)
                   + (extraToggleButtons.isEmpty() ? 0 : 22)
                   + rows * 96 + 16;
    }

    juce::AudioProcessorValueTreeState& apvts;
    juce::GroupComponent group;
    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment;
    juce::OwnedArray<KnobControl> knobs;
    juce::OwnedArray<juce::ToggleButton> extraToggleButtons;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> extraToggleAttachments;
};

// Live LUFS / true-peak / gain-reduction readout, polled from the
// processor's atomics on a timer (never touches the audio thread).
struct MeterPanel : public juce::Component
{
    void paint (juce::Graphics& g) override
    {
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), 6.0f);

        g.setColour (juce::Colours::white);
        g.setFont (13.0f);

        auto line = [&] (int row, const juce::String& text)
        {
            g.drawFittedText (text, 10, 8 + row * 18, getWidth() - 20, 18, juce::Justification::centredLeft, 1);
        };

        line (0, juce::String::formatted ("Momentary: %.1f LUFS", momentaryLufs));
        line (1, juce::String::formatted ("Integrated: %.1f LUFS", integratedLufs));
        line (2, juce::String::formatted ("True Peak:  %.1f dBTP", truePeakDb));
        line (3, juce::String::formatted ("Comp GR: -%.1f dB   Limiter GR: -%.1f dB", compGrDb, limGrDb));
    }

    float momentaryLufs = -70.0f, integratedLufs = -70.0f, truePeakDb = -70.0f;
    float compGrDb = 0.0f, limGrDb = 0.0f;
};

class MasteringChainAudioProcessorEditor : public juce::AudioProcessorEditor,
                                            private juce::Timer
{
public:
    explicit MasteringChainAudioProcessorEditor (MasteringChainAudioProcessor&);
    ~MasteringChainAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    MasteringChainAudioProcessor& processorRef;

    juce::Viewport viewport;
    juce::Component content;

    ChainSection inputSection, eqSection, compSection, satSection, widthSection, limiterSection, outputSection;
    MeterPanel meterPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MasteringChainAudioProcessorEditor)
};
