#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct KnobControl : public juce::Component
{
    KnobControl (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& displayName)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 16);
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

struct Section : public juce::Component
{
    Section (juce::AudioProcessorValueTreeState& state, const juce::String& title) : apvts (state)
    {
        group.setText (title);
        addAndMakeVisible (group);
    }

    void addKnob (const juce::String& paramID, const juce::String& displayName)
    {
        auto* k = new KnobControl (apvts, paramID, displayName);
        addAndMakeVisible (k);
        knobs.add (k);
    }

    void addToggle (const juce::String& paramID, const juce::String& text)
    {
        auto* b = new juce::ToggleButton (text);
        addAndMakeVisible (b);
        toggles.add (b);
        toggleAttachments.add (new juce::AudioProcessorValueTreeState::ButtonAttachment (apvts, paramID, *b));
    }

    void resized() override
    {
        auto b = getLocalBounds();
        group.setBounds (b);
        b.removeFromTop (22);
        b.reduce (8, 6);

        if (! toggles.isEmpty())
        {
            auto row = b.removeFromTop (22);
            const int w = row.getWidth() / toggles.size();
            for (auto* t : toggles)
                t->setBounds (row.removeFromLeft (w));
        }

        juce::Grid grid;
        grid.autoFlow = juce::Grid::AutoFlow::row;
        for (int i = 0; i < columns; ++i)
            grid.templateColumns.add (juce::Grid::TrackInfo (juce::Grid::Px (86)));
        grid.autoRows = juce::Grid::TrackInfo (juce::Grid::Px (92));
        grid.columnGap = juce::Grid::Px (4);
        grid.rowGap = juce::Grid::Px (4);
        for (auto* k : knobs)
            grid.items.add (juce::GridItem (*k));
        grid.performLayout (b);
    }

    int getPreferredHeight() const
    {
        const int rows = (knobs.size() + columns - 1) / columns;
        return 22 + (toggles.isEmpty() ? 0 : 24) + rows * 96 + 14;
    }

    static constexpr int columns = 4;
    juce::AudioProcessorValueTreeState& apvts;
    juce::GroupComponent group;
    juce::OwnedArray<KnobControl> knobs;
    juce::OwnedArray<juce::ToggleButton> toggles;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> toggleAttachments;
};

// The honest-diagnosis panel: rather than only moving gain reduction around
// silently, it names which of the muddy-bass causes is actually present.
struct DiagnosisPanel : public juce::Component
{
    void paint (juce::Graphics&) override;

    bool kickMode = false;
    bool partnerPresent = false;
    bool usingSidechain = false;
    float fundamentalHz = 0.0f;
    float confidence = 0.0f;
    float tameDb = 0.0f;
    float duckDb = 0.0f;
    float noteSpreadDb = 0.0f;
    float noteCompDb = 0.0f;
    int currentNote = -1;
    int learnedNotes = 0;
    int worstHarmonic = -1;
    float transient = 0.0f;
    juce::String linkName = "A";
};

class TechHouseDuoEditor : public juce::AudioProcessorEditor,
                            private juce::Timer
{
public:
    explicit TechHouseDuoEditor (TechHouseDuoProcessor&);
    ~TechHouseDuoEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void applyModeVisibility();

    TechHouseDuoProcessor& processorRef;

    juce::Viewport viewport;
    juce::Component content;

    juce::ComboBox modeBox, linkBox;
    juce::Label modeLabel, linkLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment, linkAttachment;

    juce::TextButton relearnButton { "Relearn notes" };

    DiagnosisPanel diagnosis;

    Section globalSection, tameSection, duckSection, noteSection, exciteSection;
    Section kickShapeSection, kickToneSection;

    bool lastKickMode = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TechHouseDuoEditor)
};
