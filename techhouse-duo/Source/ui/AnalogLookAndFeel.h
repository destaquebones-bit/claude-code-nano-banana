#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

// Modern hardware look: soft dark field, rounded cards, silver knobs.
//
// Everything here is drawn rather than bitmap-based. That is a deliberate
// choice for a plugin UI: a drawn control stays sharp on any display scaling
// and at any size the user resizes the window to, adds nothing to the binary,
// and keeps every control visually consistent -- none of which is true of
// generated or photographed images, which have to be produced at a fixed
// resolution and matched to each other by hand.
//
// The backdrop palette and its light placement come from a generated reference
// image (Nano Banana 2, 3:4): near-black charcoal with a cool blue cast up top
// and a soft warm sun low in the frame. paintBackdrop() reproduces that as a
// gradient field, which also means it reflows correctly at any window size
// instead of stretching.
//
// The geometry rule for this revision: nothing is a plain rectangle. Cards get
// a large corner radius, every interactive control is a pill, and no element is
// outlined with a hard one-pixel border -- separation comes from a light top
// edge and a soft shadow underneath, the way a moulded surface separates from
// its background rather than the way a box is drawn around a region.
namespace Palette
{
    // Field
    const juce::Colour ink          { 0xff0d1015 };   // deepest, window edges
    const juce::Colour panel        { 0xff141820 };   // mid field
    const juce::Colour panelDeep    { 0xff0f1218 };
    const juce::Colour glowWarm     { 0xffc87a2e };   // the sun low in the frame
    const juce::Colour glowCool     { 0xff1b2530 };   // cool cast at the top

    // Cards
    const juce::Colour section      { 0xff1b1f27 };
    const juce::Colour sectionTop   { 0xff232833 };   // lit upper edge
    const juce::Colour sectionEdge  { 0xff2b313b };
    const juce::Colour trackDark    { 0xff0b0e13 };   // recessed slots and tracks

    // Silver hardware, the way an Apollo-style interface reads: the knob is the
    // bright object and the panel stays out of the way.
    const juce::Colour metalTop     { 0xfff2f4f6 };
    const juce::Colour metalUpper   { 0xffd2d7dd };
    const juce::Colour metalMid     { 0xffa8afb8 };
    const juce::Colour metalLower   { 0xff767d87 };
    const juce::Colour metalRim     { 0xff15181c };
    const juce::Colour pointerDark  { 0xff2a2f36 };

    // Signal
    const juce::Colour amber        { 0xfff0a63c };
    const juce::Colour amberDim     { 0xff7a5a2a };
    const juce::Colour cut          { 0xffe05c4a };
    const juce::Colour good         { 0xff5fd38a };
    const juce::Colour text         { 0xffe8eaee };
    const juce::Colour textDim      { 0xff868d99 };
    const juce::Colour grid         { 0xff262c35 };
}

namespace UiStyle
{
    constexpr float cardRadius = 14.0f;

    // Letterspaced small caps. JUCE has no tracking setting, so the string is
    // laid out a glyph at a time. Used only for short labels, where the extra
    // air is what makes a heading read as a heading without needing a box drawn
    // round it or a heavier weight.
    inline void drawTracked (juce::Graphics& g, const juce::String& text,
                              juce::Rectangle<int> area, float tracking,
                              juce::Justification justification = juce::Justification::centredLeft)
    {
        const auto font = g.getCurrentFont();
        float total = 0.0f;
        for (int i = 0; i < text.length(); ++i)
            total += font.getStringWidthFloat (text.substring (i, i + 1)) + tracking;
        total -= tracking;

        float x = (float) area.getX();
        if (justification.testFlags (juce::Justification::horizontallyCentred))
            x += ((float) area.getWidth() - total) * 0.5f;
        else if (justification.testFlags (juce::Justification::right))
            x += (float) area.getWidth() - total;

        const float y = (float) area.getCentreY() + font.getHeight() * 0.33f;
        for (int i = 0; i < text.length(); ++i)
        {
            const auto ch = text.substring (i, i + 1);
            g.drawSingleLineText (ch, juce::roundToInt (x), juce::roundToInt (y));
            x += font.getStringWidthFloat (ch) + tracking;
        }
    }

    // A raised card: soft shadow underneath, a lit top edge, no hard outline.
    // Drawn with stacked translucent rounded rectangles rather than
    // juce::DropShadow so it costs the same offscreen as on screen and needs no
    // intermediate image.
    inline void drawCard (juce::Graphics& g, juce::Rectangle<float> bounds,
                           float radius = cardRadius, float shadowAlpha = 0.32f)
    {
        for (int i = 3; i >= 1; --i)
        {
            g.setColour (juce::Colours::black.withAlpha (shadowAlpha / (float) (i * 2)));
            g.fillRoundedRectangle (bounds.translated (0.0f, (float) i * 0.9f)
                                            .expanded ((float) i * 0.7f), radius + (float) i);
        }

        juce::ColourGradient face (Palette::sectionTop, bounds.getCentreX(), bounds.getY(),
                                    Palette::section, bounds.getCentreX(), bounds.getBottom(), false);
        face.addColour (0.14, Palette::section);
        g.setGradientFill (face);
        g.fillRoundedRectangle (bounds, radius);

        // A single lit hairline along the top curve, which is what separates the
        // card from the field. An outline all the way round would put a box back.
        juce::Path topEdge;
        topEdge.addRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                      radius, radius, true, true, false, false);
        g.setColour (juce::Colours::white.withAlpha (0.055f));
        g.strokePath (topEdge, juce::PathStrokeType (1.0f));
    }

    // A recessed slot: the inverse of a card, for anything a control sits in.
    inline void drawWell (juce::Graphics& g, juce::Rectangle<float> bounds, float radius)
    {
        g.setColour (Palette::trackDark);
        g.fillRoundedRectangle (bounds, radius);
        juce::Path lip;
        lip.addRoundedRectangle (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                                  radius, radius, true, true, false, false);
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.strokePath (lip, juce::PathStrokeType (1.2f));
    }

    // The window field. Reproduces the generated reference: cool dark wash at
    // the top, a wide warm sun low and slightly left of centre, vignette at the
    // edges. Every stop is proportional to the bounds, so it reflows on resize
    // rather than stretching a fixed image.
    inline void paintBackdrop (juce::Graphics& g, juce::Rectangle<float> b)
    {
        juce::ColourGradient field (Palette::glowCool, b.getCentreX(), b.getY(),
                                     Palette::ink, b.getCentreX(), b.getBottom(), false);
        field.addColour (0.30, Palette::panel);
        field.addColour (0.72, Palette::panelDeep);
        g.setGradientFill (field);
        g.fillRect (b);

        // The sun sits mostly below the bottom edge, so only its upper falloff
        // reaches the panel. A first pass placed it inside the frame at three
        // times this strength and it washed warm over the lower readouts, which
        // made them look dirty rather than lit -- a backdrop has to stay behind
        // the content, and the way to keep it there is to let the light arrive
        // from off-frame.
        const float sunX = b.getX() + b.getWidth() * 0.46f;
        const float sunY = b.getBottom() + b.getHeight() * 0.10f;
        const float sunR = juce::jmax (b.getWidth(), b.getHeight()) * 0.70f;
        juce::ColourGradient sun (Palette::glowWarm.withAlpha (0.17f), sunX, sunY,
                                   Palette::glowWarm.withAlpha (0.0f), sunX, sunY - sunR, true);
        sun.addColour (0.55, Palette::glowWarm.withAlpha (0.045f));
        g.setGradientFill (sun);
        g.fillRect (b);

        // Radial gradients in JUCE run from the centre outward, so the corner
        // distance -- not the edge distance -- sets where the outer colour is
        // reached. Using half the diagonal puts full darkness in the corners and
        // leaves the edge midpoints partly shaded, which is what a vignette does.
        const float corner = std::sqrt (b.getWidth() * b.getWidth() + b.getHeight() * b.getHeight()) * 0.5f;
        juce::ColourGradient vignette (juce::Colours::transparentBlack, b.getCentreX(), b.getCentreY(),
                                        juce::Colours::black.withAlpha (0.62f),
                                        b.getCentreX(), b.getCentreY() - corner, true);
        vignette.addColour (0.55, juce::Colours::transparentBlack);
        vignette.addColour (0.80, juce::Colours::black.withAlpha (0.22f));
        g.setGradientFill (vignette);
        g.fillRect (b);
    }
}

class AnalogLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AnalogLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId, Palette::panel);
        setColour (juce::Label::textColourId, Palette::text);
        setColour (juce::Slider::textBoxTextColourId, Palette::text);
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxHighlightColourId, Palette::amber.withAlpha (0.3f));
        setColour (juce::ComboBox::backgroundColourId, Palette::trackDark);
        setColour (juce::ComboBox::textColourId, Palette::text);
        setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ComboBox::arrowColourId, Palette::amber);
        setColour (juce::PopupMenu::backgroundColourId, Palette::section);
        setColour (juce::PopupMenu::textColourId, Palette::text);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::amber.withAlpha (0.25f));
        setColour (juce::PopupMenu::highlightedTextColourId, Palette::text);
        setColour (juce::TextButton::buttonColourId, Palette::section);
        setColour (juce::TextButton::textColourOffId, Palette::text);
        setColour (juce::ToggleButton::textColourId, Palette::text);
        setColour (juce::GroupComponent::textColourId, Palette::textDim);
        setColour (juce::GroupComponent::outlineColourId, juce::Colours::transparentBlack);
        setColour (juce::ScrollBar::thumbColourId, Palette::sectionEdge);
        setColour (juce::ScrollBar::trackColourId, juce::Colours::transparentBlack);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                            float sliderPos, float startAngle, float endAngle,
                            juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
        const auto centre = bounds.getCentre();
        const float outerRadius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const float bodyRadius = outerRadius * 0.68f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        const bool enabled = slider.isEnabled();

        // --- Value track: a recessed full arc with the active portion lit.
        // Bipolar parameters (a gain that can cut or boost) fill outward from
        // the centre instead of from the left, so "no change" reads as empty.
        const float trackRadius = outerRadius - 2.5f;
        const float trackWidth = juce::jmax (3.0f, outerRadius * 0.13f);
        juce::Path track;
        track.addCentredArc (centre.x, centre.y, trackRadius, trackRadius, 0.0f, startAngle, endAngle, true);
        g.setColour (Palette::trackDark);
        g.strokePath (track, juce::PathStrokeType (trackWidth + 1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (Palette::grid);
        g.strokePath (track, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

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
            // Bloom under the lit arc, then the arc itself. The bloom is what
            // makes the indicator read as emitting light rather than as a
            // coloured line, and it is the one place colour is allowed to spread.
            if (enabled)
            {
                g.setColour (Palette::amber.withAlpha (0.16f));
                g.strokePath (value, juce::PathStrokeType (trackWidth * 2.6f, juce::PathStrokeType::curved,
                                                            juce::PathStrokeType::rounded));
            }
            g.setColour (enabled ? Palette::amber : Palette::amberDim);
            g.strokePath (value, juce::PathStrokeType (trackWidth, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));
        }

        // --- Seated shadow under the body, so the knob reads as sitting on the
        // panel rather than floating on it.
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillEllipse (juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f)
                            .withCentre (centre.translated (0.0f, 2.0f)));

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
        const float radius = bounds.getHeight() * 0.5f;   // full pill

        UiStyle::drawWell (g, bounds, radius);
        if (box.hasKeyboardFocus (true))
        {
            g.setColour (Palette::amber.withAlpha (0.55f));
            g.drawRoundedRectangle (bounds.reduced (0.5f), radius, 1.0f);
        }

        juce::Path arrow;
        const float cx = (float) width - 14.0f, cy = (float) height * 0.5f;
        arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
        arrow.lineTo (cx, cy + 3.0f);
        arrow.lineTo (cx + 4.0f, cy - 2.0f);
        g.setColour (Palette::amber);
        g.strokePath (arrow, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // A sliding switch rather than a checkbox. This is the single biggest
    // departure from the old panel: a tick in a square box is the most
    // rectangular thing a UI can contain, and a switch says on/off faster
    // because the knob's position carries the state even out of focus.
    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                            bool shouldDrawHighlighted, bool) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        const bool on = button.getToggleState();

        const float trackHeight = juce::jlimit (13.0f, 17.0f, bounds.getHeight() - 3.0f);
        const float trackWidth = trackHeight * 1.85f;
        auto track = juce::Rectangle<float> (trackWidth, trackHeight)
                        .withY (bounds.getY() + (bounds.getHeight() - trackHeight) * 0.5f)
                        .withX (bounds.getX() + 1.0f);
        const float radius = trackHeight * 0.5f;

        UiStyle::drawWell (g, track, radius);
        if (on)
        {
            g.setColour (Palette::amber.withAlpha (0.22f));
            g.fillRoundedRectangle (track.expanded (2.0f), radius + 2.0f);
            g.setColour (Palette::amber.withAlpha (0.85f));
            g.fillRoundedRectangle (track, radius);
        }

        // The moving part is metal, matching the knobs, so the whole panel
        // reads as one set of hardware.
        const float knobDiameter = trackHeight - 3.0f;
        const float knobX = on ? track.getRight() - knobDiameter - 1.5f : track.getX() + 1.5f;
        auto knob = juce::Rectangle<float> (knobDiameter, knobDiameter)
                        .withY (track.getY() + 1.5f).withX (knobX);

        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (knob.translated (0.0f, 1.0f));
        juce::ColourGradient metal (Palette::metalTop, knob.getCentreX(), knob.getY(),
                                     Palette::metalMid, knob.getCentreX(), knob.getBottom(), false);
        g.setGradientFill (metal);
        g.fillEllipse (knob);
        if (shouldDrawHighlighted)
        {
            g.setColour (juce::Colours::white.withAlpha (0.35f));
            g.drawEllipse (knob, 1.0f);
        }

        g.setColour (on ? Palette::text : Palette::textDim);
        g.setFont (12.0f);
        g.drawFittedText (button.getButtonText(),
                           juce::roundToInt (track.getRight() + 8.0f), (int) bounds.getY(),
                           juce::roundToInt (bounds.getWidth() - trackWidth - 10.0f), (int) bounds.getHeight(),
                           juce::Justification::centredLeft, 1);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                bool highlighted, bool down) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
        const float radius = bounds.getHeight() * 0.5f;   // full pill

        g.setColour (juce::Colours::black.withAlpha (down ? 0.15f : 0.35f));
        g.fillRoundedRectangle (bounds.translated (0.0f, 1.2f), radius);

        juce::ColourGradient grad (Palette::sectionTop.brighter (down ? 0.0f : 0.10f), 0.0f, bounds.getY(),
                                    Palette::section.darker (0.15f), 0.0f, bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, radius);

        g.setColour (highlighted ? Palette::amber.withAlpha (0.75f)
                                  : juce::Colours::white.withAlpha (0.07f));
        g.drawRoundedRectangle (bounds.reduced (0.5f), radius, 1.0f);
    }

    // The section heading sits above its card in letterspaced caps with a small
    // amber marker, instead of being punched out of a border. Nothing is
    // enclosed, so the eye groups the controls by the card's shape and the
    // heading's proximity rather than by a line drawn around them.
    void drawGroupComponentOutline (juce::Graphics& g, int width, int height,
                                     const juce::String& text, const juce::Justification&,
                                     juce::GroupComponent&) override
    {
        auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat();
        auto titleStrip = bounds.removeFromTop (16.0f);

        UiStyle::drawCard (g, bounds.reduced (1.0f, 0.0f));

        if (text.isNotEmpty())
        {
            auto dot = juce::Rectangle<float> (4.0f, 4.0f)
                            .withCentre ({ titleStrip.getX() + 6.0f, titleStrip.getCentreY() });
            g.setColour (Palette::amber.withAlpha (0.30f));
            g.fillEllipse (dot.expanded (2.5f));
            g.setColour (Palette::amber);
            g.fillEllipse (dot);

            g.setColour (Palette::textDim);
            g.setFont (juce::Font (9.5f, juce::Font::bold));
            UiStyle::drawTracked (g, text.toUpperCase(),
                                   titleStrip.toNearestInt().withTrimmedLeft (15), 1.4f);
        }
    }

    void drawScrollbar (juce::Graphics& g, juce::ScrollBar&, int x, int y, int width, int height,
                         bool isVertical, int thumbStart, int thumbSize,
                         bool isMouseOver, bool isMouseDown) override
    {
        if (thumbSize <= 0)
            return;

        auto thumb = isVertical
            ? juce::Rectangle<float> ((float) x + width * 0.35f, (float) thumbStart,
                                       width * 0.3f, (float) thumbSize)
            : juce::Rectangle<float> ((float) thumbStart, (float) y + height * 0.35f,
                                       (float) thumbSize, height * 0.3f);

        g.setColour (Palette::sectionEdge.withAlpha (isMouseDown ? 0.95f : (isMouseOver ? 0.8f : 0.5f)));
        g.fillRoundedRectangle (thumb, thumb.getWidth() * 0.5f);
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
