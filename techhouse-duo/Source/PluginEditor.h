#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/AnalogLookAndFeel.h"
#include "ui/Visualisers.h"

struct KnobControl : public juce::Component
{
    KnobControl (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& displayName)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 68, 15);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                     juce::MathConstants<float>::pi * 2.75f, true);
        addAndMakeVisible (slider);

        label.setText (displayName, juce::dontSendNotification);
        label.setJustificationType (juce::Justification::centred);
        label.setFont (juce::Font (10.5f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, Palette::textDim);
        addAndMakeVisible (label);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramID, slider);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        label.setBounds (b.removeFromTop (13));
        slider.setBounds (b);
    }

    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// A titled panel holding toggles, a grid of knobs, and optionally a visualiser
// belonging to that stage -- the ducking meter sits inside the ducking panel,
// the note map inside the levelling panel, so a control and the picture of what
// it does are never in different parts of the window.
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

    void setExtra (juce::Component* component, int height)
    {
        extra = component;
        extraHeight = height;
        if (extra != nullptr)
            addAndMakeVisible (extra);
    }

    void setFooter (juce::Component* component, int height)
    {
        footer = component;
        footerHeight = height;
        if (footer != nullptr)
            addAndMakeVisible (footer);
    }

    void resized() override
    {
        auto b = getLocalBounds();
        group.setBounds (b);
        b.removeFromTop (16);
        b.reduce (10, 8);

        if (! toggles.isEmpty())
        {
            auto row = b.removeFromTop (20);
            const int w = row.getWidth() / toggles.size();
            for (auto* t : toggles)
                t->setBounds (row.removeFromLeft (w));
            b.removeFromTop (4);
        }

        if (! knobs.isEmpty())
        {
            const int columns = juce::jmax (1, b.getWidth() / knobWidth);
            const int rows = (knobs.size() + columns - 1) / columns;
            auto grid = b.removeFromTop (rows * knobHeight);
            for (int r = 0; r < rows; ++r)
            {
                auto row = grid.removeFromTop (knobHeight);
                for (int c = 0; c < columns; ++c)
                {
                    const int index = r * columns + c;
                    if (index >= knobs.size())
                        break;
                    knobs[index]->setBounds (row.removeFromLeft (knobWidth).reduced (2));
                }
            }
        }

        if (extra != nullptr)
        {
            b.removeFromTop (4);
            extra->setBounds (b.removeFromTop (extraHeight));
        }

        if (footer != nullptr)
        {
            b.removeFromTop (6);
            footer->setBounds (b.removeFromTop (footerHeight).removeFromLeft (140));
        }
    }

    int getPreferredHeight (int availableWidth) const
    {
        const int usable = juce::jmax (knobWidth, availableWidth - 20);
        const int columns = juce::jmax (1, usable / knobWidth);
        const int rows = knobs.isEmpty() ? 0 : (knobs.size() + columns - 1) / columns;
        return 16 + 16
                + (toggles.isEmpty() ? 0 : 24)
                + rows * knobHeight
                + (extra != nullptr ? extraHeight + 4 : 0)
                + (footer != nullptr ? footerHeight + 6 : 0);
    }

    static constexpr int knobWidth = 84;
    static constexpr int knobHeight = 92;

    juce::AudioProcessorValueTreeState& apvts;
    juce::GroupComponent group;
    juce::OwnedArray<KnobControl> knobs;
    juce::OwnedArray<juce::ToggleButton> toggles;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ButtonAttachment> toggleAttachments;
    juce::Component* extra = nullptr;
    juce::Component* footer = nullptr;
    int extraHeight = 0, footerHeight = 0;
};

// Compact status strip: link state plus a plain-language verdict. The detail
// that used to live here as text is now drawn by the visualisers, so this only
// has to say what a picture cannot.
struct StatusStrip : public juce::Component
{
    void paint (juce::Graphics&) override;

    bool kickMode = false;
    bool partnerPresent = false;
    bool usingSidechain = false;
    float tameDb = 0.0f, duckDb = 0.0f, noteSpreadDb = 0.0f, noteCompDb = 0.0f;
    float transient = 0.0f;
    int learnedNotes = 0, worstHarmonic = -1;
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
    void refreshVisualisers();

    TechHouseDuoProcessor& processorRef;
    AnalogLookAndFeel lookAndFeel;

    juce::Viewport viewport;
    juce::Component content;

    juce::ComboBox modeBox, linkBox;
    juce::Label modeLabel, linkLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment, linkAttachment;

    juce::TextButton relearnButton { "Relearn notes" };

    SpectrumView spectrum;
    StatusStrip status;
    DuckMeter duckMeter;
    NoteMap noteMap;

    Section globalSection, tameSection, duckSection, noteSection, exciteSection;
    Section kickShapeSection, kickToneSection;

    bool lastKickMode = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TechHouseDuoEditor)
};
