#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Hardware-style look: dark panel, machined metal knobs, amber indicators.
//
// Everything here is drawn rather than bitmap-based. That is a deliberate
// choice for a plugin UI: a drawn knob stays sharp on any display scaling and
// at any size the user resizes the window to, adds nothing to the binary, and
// keeps every control visually consistent -- none of which is true of generated
// or photographed knob images, which have to be produced at a fixed resolution
// and matched to each other by hand.
namespace Palette
{
    const juce::Colour panel        { 0xff1a1c20 };
    const juce::Colour panelDeep    { 0xff121418 };
    const juce::Colour section      { 0xff22252b };
    const juce::Colour sectionEdge  { 0xff33373f };
    // Silver hardware on a dark panel, the way an Apollo-style interface reads:
    // the knob is the bright object and the panel stays out of the way.
    const juce::Colour metalTop     { 0xfff2f4f6 };
    const juce::Colour metalUpper   { 0xffd2d7dd };
    const juce::Colour metalMid     { 0xffa8afb8 };
    const juce::Colour metalLower   { 0xff767d87 };
    const juce::Colour metalRim     { 0xff15181c };
    const juce::Colour pointerDark  { 0xff2a2f36 };
    const juce::Colour amber        { 0xfff0a63c };
    const juce::Colour amberDim     { 0xff7a5a2a };
    const juce::Colour cut          { 0xffe05c4a };
    const juce::Colour good         { 0xff5fd38a };
    const juce::Colour text         { 0xffe6e8ea };
    const juce::Colour textDim      { 0xff8b9098 };
    const juce::Colour grid         { 0xff2c3038 };
}

class AnalogLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AnalogLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, Palette::panel);
        setColour (juce::Label::textColourId, Palette::text);
        setColour (juce::Slider::textBoxTextColourId, Palette::text);
        setColour (juce::Slider::textBoxBackgroundColourId, Palette::panelDeep);
        setColour (juce::Slider::textBoxOutlineColourId, Palette::sectionEdge);
        setColour (juce::Slider::textBoxHighlightColourId, Palette::amber.withAlpha (0.3f));
        setColour (juce::ComboBox::backgroundColourId, Palette::panelDeep);
        setColour (juce::ComboBox::textColourId, Palette::text);
        setColour (juce::ComboBox::outlineColourId, Palette::sectionEdge);
        setColour (juce::ComboBox::arrowColourId, Palette::amber);
        setColour (juce::PopupMenu::backgroundColourId, Palette::section);
        setColour (juce::PopupMenu::textColourId, Palette::text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::amber.withAlpha (0.25f));
        setColour (juce::PopupMenu::highlightedTextColourId, Palette::text);
        setColour (juce::TextButton::buttonColourId, Palette::section);
        setColour (juce::TextButton::textColourOffId, Palette::text);
        setColour (juce::ToggleButton::textColourId, Palette::text);
        setColour (juce::GroupComponent::textColourId, Palette::textDim);
        setColour (juce::GroupComponent::outlineColourId, Palette::sectionEdge);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                            float sliderPos, float startAngle, float endAngle,
                            juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
        const auto centre = bounds.getCentre();
        const float outerRadius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const float bodyRadius = outerRadius * 0.70f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        const bool enabled = slider.isEnabled();

        // --- Value track: a dim full arc with the active portion lit in amber.
        // Bipolar parameters (a gain that can cut or boost) fill outward from
        // the centre instead of from the left, so "no change" reads as empty.
        const float trackRadius = outerRadius - 2.0f;
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, trackRadius, trackRadius, 0.0f, startAngle, endAngle, true);
        g.setColour (Palette::grid);
        g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const auto range = slider.getRange();
        const bool bipolar = range.getStart() < -0.01 && range.getEnd() > 0.01;
        const float originPos = bipolar
            ? (float) ((0.0 - range.getStart()) / juce::jmax (1.0e-9, range.getLength()))
            : 0.0f;
        const float originAngle = startAngle + originPos * (endAngle - startAngle);

        if (std::abs (angle - originAngle) > 0.001f)
        {
            juce::Path value;
            value.addCentredArc (centre.x, centre.y, trackRadius, trackRadius, 0.0f,
                                  juce::jmin (originAngle, angle), juce::jmax (originAngle, angle), true);
            g.setColour (enabled ? Palette::amber : Palette::amberDim);
            g.strokePath (value, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // --- Seated shadow under the body, so the knob reads as sitting on the
        // panel rather than floating on it.
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f)
                            .withCentre (centre.translated (0.0f, 1.5f)));

        // --- Body: a four-stop vertical gradient rather than a simple two-stop
        // one. Real turned aluminium has a bright band near the top, a rapid
        // falloff through the middle and a slight bounce-light lift at the very
        // bottom; two stops read as flat plastic, these read as metal.
        const auto bodyRect = juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f).withCentre (centre);
        juce::ColourGradient body (Palette::metalTop, centre.x, centre.y - bodyRadius,
                                    Palette::metalLower, centre.x, centre.y + bodyRadius, false);
        body.addColour (0.28, Palette::metalUpper);
        body.addColour (0.62, Palette::metalMid);
        body.addColour (0.93, Palette::metalLower.brighter (0.10f));
        g.setGradientFill (body);
        g.fillEllipse (bodyRect);

        // --- Machined grain: fine concentric turning marks rotating with the
        // knob. On a light body these read as darker lines, the opposite of
        // what worked on the dark version.
        {
            juce::Path grain;
            constexpr int strokes = 72;
            for (int i = 0; i < strokes; ++i)
            {
                const float a = angle + i * juce::MathConstants<float>::twoPi / strokes;
                const float c = std::cos (a - juce::MathConstants<float>::halfPi);
                const float s = std::sin (a - juce::MathConstants<float>::halfPi);
                grain.startNewSubPath (centre.x + c * bodyRadius * 0.34f, centre.y + s * bodyRadius * 0.34f);
                grain.lineTo (centre.x + c * bodyRadius * 0.95f, centre.y + s * bodyRadius * 0.95f);
            }
            g.setColour (juce::Colours::black.withAlpha (0.055f));
            g.strokePath (grain, juce::PathStrokeType (0.6f));
        }

        // --- Rim: a dark seat line so the bright body separates from the panel,
        // plus a specular arc across the top edge.
        g.setColour (Palette::metalRim.withAlpha (0.75f));
        g.drawEllipse (bodyRect, 1.0f);

        juce::Path highlight;
        highlight.addCentredArc (centre.x, centre.y, bodyRadius - 1.2f, bodyRadius - 1.2f, 0.0f,
                                  juce::MathConstants<float>::pi * 1.10f,
                                  juce::MathConstants<float>::pi * 1.90f, true);
        g.setColour (juce::Colours::white.withAlpha (0.75f));
        g.strokePath (highlight, juce::PathStrokeType (1.1f));

        // --- Pointer: a dark engraved line. On silver the indicator has to be
        // dark to be legible -- an amber pointer would disappear into the
        // bright body, so the amber stays on the value arc outside the knob.
        const float pointerLength = bodyRadius * 0.58f;
        const float pointerWidth = juce::jmax (1.8f, bodyRadius * 0.11f);
        juce::Path pointer;
        pointer.addRoundedRectangle (-pointerWidth * 0.5f, -bodyRadius * 0.90f,
                                      pointerWidth, pointerLength, pointerWidth * 0.5f);
        g.setColour (juce::Colours::white.withAlpha (0.5f));
        g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y + 0.9f));
        g.setColour (enabled ? Palette::pointerDark : Palette::pointerDark.withAlpha (0.45f));
        g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));

        // --- Centre cap: slightly darker brushed disc, lit the same way.
        const float capRadius = bodyRadius * 0.26f;
        juce::ColourGradient cap (Palette::metalUpper, centre.x, centre.y - capRadius,
                                   Palette::metalLower.darker (0.15f), centre.x, centre.y + capRadius, false);
        g.setGradientFill (cap);
        g.fillEllipse (juce::Rectangle<float> (capRadius * 2.0f, capRadius * 2.0f).withCentre (centre));
        g.setColour (Palette::metalRim.withAlpha (0.35f));
        g.drawEllipse (juce::Rectangle<float> (capRadius * 2.0f, capRadius * 2.0f).withCentre (centre), 0.8f);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool,
                        int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
        g.setColour (Palette::panelDeep);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (box.hasKeyboardFocus (true) ? Palette::amber : Palette::sectionEdge);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

        juce::Path arrow;
        const float cx = (float) width - 14.0f, cy = (float) height * 0.5f;
        arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
        arrow.lineTo (cx, cy + 3.0f);
        arrow.lineTo (cx + 4.0f, cy - 2.0f);
        g.setColour (Palette::amber);
        g.strokePath (arrow, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                            bool shouldDrawHighlighted, bool) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const float boxSize = juce::jmin (16.0f, bounds.getHeight() - 2.0f);
        auto box = juce::Rectangle<float> (boxSize, boxSize)
                        .withY (bounds.getY() + (bounds.getHeight() - boxSize) * 0.5f)
                        .withX (bounds.getX() + 1.0f);

        g.setColour (Palette::panelDeep);
        g.fillRoundedRectangle (box, 3.0f);
        g.setColour (shouldDrawHighlighted ? Palette::amber.withAlpha (0.7f) : Palette::sectionEdge);
        g.drawRoundedRectangle (box, 3.0f, 1.0f);

        if (button.getToggleState())
        {
            // A filled lamp rather than a tick: reads as a piece of hardware
            // with an indicator lit, and is legible at small sizes.
            g.setColour (Palette::amber);
            g.fillRoundedRectangle (box.reduced (4.0f), 2.0f);
            g.setColour (Palette::amber.withAlpha (0.25f));
            g.fillRoundedRectangle (box.reduced (2.0f), 3.0f);
        }

        g.setColour (button.getToggleState() ? Palette::text : Palette::textDim);
        g.setFont (12.0f);
        g.drawFittedText (button.getButtonText(),
                           box.getRight() + 6, (int) bounds.getY(),
                           (int) (bounds.getWidth() - boxSize - 8), (int) bounds.getHeight(),
                           juce::Justification::centredLeft, 1);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                bool highlighted, bool down) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        juce::ColourGradient grad (Palette::section.brighter (down ? 0.0f : 0.12f), 0.0f, bounds.getY(),
                                    Palette::section.darker (0.25f), 0.0f, bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (highlighted ? Palette::amber : Palette::sectionEdge);
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
    }

    void drawGroupComponentOutline (juce::Graphics& g, int width, int height,
                                     const juce::String& text, const juce::Justification&,
                                     juce::GroupComponent&) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (0.5f);
        bounds.removeFromTop (7.0f);

        g.setColour (Palette::section);
        g.fillRoundedRectangle (bounds, 6.0f);
        g.setColour (Palette::sectionEdge);
        g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

        if (text.isNotEmpty())
        {
            g.setFont (juce::Font (11.0f, juce::Font::bold));
            const int textWidth = juce::jmin (width - 20, (int) g.getCurrentFont().getStringWidthFloat (text) + 14);

            // Punch the label out of the border so it reads as silk-screened on
            // a panel rather than a caption floating above a box.
            g.setColour (Palette::panel);
            g.fillRect (12, 1, textWidth, 13);
            g.setColour (Palette::amber.withAlpha (0.85f));
            g.drawFittedText (text, 16, 0, textWidth, 15, juce::Justification::centredLeft, 1);
        }
    }

    juce::Font getLabelFont (juce::Label& label) override
    {
        return label.getFont().withHeight (juce::jmax (10.0f, label.getFont().getHeight()));
    }

    juce::Label* createSliderTextBox (juce::Slider& slider) override
    {
        auto* l = LookAndFeel_V4::createSliderTextBox (slider);
        l->setFont (juce::Font (11.0f));
        l->setJustificationType (juce::Justification::centred);
        l->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
        return l;
    }
};
