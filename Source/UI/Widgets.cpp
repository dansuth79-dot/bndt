#include "Widgets.h"
#include "BinaryData.h"

// ============================================================================
//  Theme
// ============================================================================
namespace
{
    juce::Typeface::Ptr loadTypeface()
    {
        // Prefer Fancy Footwork if it has been dropped into assets/, else the bundled hand-drawn font.
        for (int i = 0; i < BinaryData::namedResourceListSize; ++i)
        {
            const juce::String file (BinaryData::getNamedResourceOriginalFilename (BinaryData::namedResourceList[i]));
            if (file.containsIgnoreCase ("fancy") && (file.endsWithIgnoreCase (".ttf") || file.endsWithIgnoreCase (".otf")))
            {
                int size = 0;
                if (auto* data = BinaryData::getNamedResource (BinaryData::namedResourceList[i], size))
                    return juce::Typeface::createSystemTypefaceFor (data, (size_t) size);
            }
        }
        return juce::Typeface::createSystemTypefaceFor (BinaryData::GochiHandRegular_ttf, BinaryData::GochiHandRegular_ttfSize);
    }
}

juce::Font Theme::font (float height)
{
    static juce::Typeface::Ptr tf = loadTypeface();
    return juce::Font (juce::FontOptions (tf).withHeight (height));
}

void Theme::haloText (juce::Graphics& g, const juce::String& text, juce::Rectangle<float> r, juce::Justification j, juce::Colour c, float alpha)
{
    const juce::Colour halo = juce::Colour (0xffEAF7E8).withAlpha (0.42f * alpha);
    g.setColour (halo);
    for (int i = 0; i < 8; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * (float) i / 8.0f;
        g.drawText (text, r.translated (1.6f * std::cos (a), 1.6f * std::sin (a)), j, false);
    }
    g.setColour (c.withMultipliedAlpha (alpha));
    g.drawText (text, r, j, false);
}

juce::Image Theme::image (const char* resourceName)
{
    int size = 0;
    if (auto* data = BinaryData::getNamedResource (resourceName, size))
        return juce::ImageCache::getFromMemory (data, size);
    return {};
}

// ============================================================================
//  LookAndFeel
// ============================================================================
BndtLookAndFeel::BndtLookAndFeel()
{
    shadow = Theme::image ("knob_shadow_png");
    light = Theme::image ("knob_light_png");
    body[0] = Theme::image ("knob_body_0_png");
    body[1] = Theme::image ("knob_body_1_png");
    body[2] = Theme::image ("knob_body_2_png");

    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff121111));
    setColour (juce::PopupMenu::textColourId, Theme::cream);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Theme::teal.withAlpha (0.25f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
    setColour (juce::AlertWindow::backgroundColourId, juce::Colour (0xff151414));
    setColour (juce::AlertWindow::textColourId, Theme::cream);
    setColour (juce::AlertWindow::outlineColourId, Theme::pink);
    setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0b0b0b));
    setColour (juce::TextEditor::textColourId, Theme::teal);
    setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff3a3838));
    setColour (juce::TextEditor::focusedOutlineColourId, Theme::teal);
    setColour (juce::TextButton::textColourOffId, Theme::cream);
    setColour (juce::TextButton::textColourOnId, Theme::cream);
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff0b0b0b));
    setColour (juce::ComboBox::textColourId, Theme::teal);
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff3a3838));
    setColour (juce::Label::textColourId, Theme::cream);
}

void BndtLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider& s)
{
    const float size = (float) juce::jmin (w, h);
    const juce::Rectangle<float> r ((float) x + ((float) w - size) * 0.5f, (float) y + ((float) h - size) * 0.5f, size, size);
    const int variant = (int) s.getProperties().getWithDefault ("variant", 0) % 3;
    const float alpha = s.isEnabled() ? 1.0f : 0.45f;

    g.setImageResamplingQuality (juce::Graphics::highResamplingQuality);
    g.setOpacity (alpha);
    g.drawImage (shadow, r);

    // only the skirt/cap rotates; shadow and light stay put like the real thing
    const auto& img = body[variant];
    const float angle = pos * juce::MathConstants<float>::pi * (300.0f / 180.0f);
    const float sc = size / (float) img.getWidth();
    auto t = juce::AffineTransform::rotation (angle, (float) img.getWidth() * 0.5f, (float) img.getHeight() * 0.5f)
                 .scaled (sc).translated (r.getX(), r.getY());
    g.drawImageTransformed (img, t);
    g.setOpacity (alpha);
    g.drawImage (light, r);
    g.setOpacity (1.0f);
}

void BndtLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    g.fillAll (findColour (juce::PopupMenu::backgroundColourId));
    g.setColour (Theme::pink.withAlpha (0.6f));
    g.drawRect (0, 0, w, h, 1);
}

void BndtLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area, bool isSeparator, bool isActive,
                                         bool isHighlighted, bool isTicked, bool hasSubMenu, const juce::String& text,
                                         const juce::String&, const juce::Drawable*, const juce::Colour*)
{
    if (isSeparator)
    {
        g.setColour (Theme::ink.withAlpha (0.6f));
        g.fillRect (area.reduced (8, 0).withHeight (1).withY (area.getCentreY()));
        return;
    }
    auto r = area.reduced (2, 1);
    if (isHighlighted && isActive)
    {
        g.setColour (Theme::teal.withAlpha (0.22f));
        g.fillRoundedRectangle (r.toFloat(), 3.0f);
    }
    if (isTicked)
    {
        g.setColour (Theme::teal);
        g.fillEllipse (juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ (float) r.getX() + 12.0f, (float) r.getCentreY() }));
    }
    g.setColour (isActive ? (isTicked ? Theme::teal : Theme::cream) : Theme::ink);
    g.setFont (getPopupMenuFont());
    g.drawText (text, r.withTrimmedLeft (24).withTrimmedRight (20), juce::Justification::centredLeft, true);
    if (hasSubMenu)
    {
        juce::Path p;
        const float cx = (float) r.getRight() - 12.0f, cy = (float) r.getCentreY();
        p.addTriangle (cx - 3.0f, cy - 5.0f, cx - 3.0f, cy + 5.0f, cx + 4.0f, cy);
        g.setColour (Theme::pink);
        g.fillPath (p);
    }
}

void BndtLookAndFeel::drawPopupMenuSectionHeader (juce::Graphics& g, const juce::Rectangle<int>& area, const juce::String& text)
{
    g.setFont (Theme::font (18.0f));
    g.setColour (Theme::pink);
    g.drawText (text, area.reduced (12, 0), juce::Justification::bottomLeft, true);
}

void BndtLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0d0c0c));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (down ? Theme::pink : (over ? Theme::teal : juce::Colour (0xff3d3a38)));
    g.drawRoundedRectangle (r, 4.0f, 1.2f);
}

// ============================================================================
//  Knob
// ============================================================================
Knob::Knob (juce::AudioProcessorValueTreeState& s, const juce::String& paramID, const juce::String& label)
    : name (label)
{
    param = s.getParameter (paramID);
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    slider.setMouseDragSensitivity (220);
    slider.getProperties().set ("variant", (int) (paramID.hashCode() & 0x7fffffff) % 3);
    addAndMakeVisible (slider);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, paramID, slider);
    if (param != nullptr)
        slider.setDoubleClickReturnValue (true, param->convertFrom0to1 (param->getDefaultValue()));
    slider.onValueChange = [this] { repaint(); };
    slider.onDragStart = [this] { repaint(); };
    slider.onDragEnd = [this] { repaint(); };
    slider.addMouseListener (this, false);
    setRepaintsOnMouseActivity (true);
}

void Knob::resized()
{
    auto r = getLocalBounds();
    r.removeFromBottom (20);
    r.removeFromTop (6);
    slider.setBounds (r);
}

void Knob::paint (juce::Graphics& g)
{
    // printed tick at 12 o'clock: the number under it is the setting
    const auto kb = slider.getBounds().toFloat();
    const float cx = kb.getCentreX();
    juce::Path tri;
    const float sz = juce::jmin (kb.getWidth(), kb.getHeight());
    const float ty = kb.getCentreY() - sz * 0.5f + 1.0f;
    tri.addTriangle (cx - 3.5f, ty - 5.0f, cx + 3.5f, ty - 5.0f, cx, ty + 0.5f);
    g.setColour (Theme::label.withAlpha (slider.isEnabled() ? 0.85f : 0.3f));
    g.fillPath (tri);

    const bool showValue = slider.isMouseOverOrDragging() && param != nullptr;
    auto label = getLocalBounds().removeFromBottom (22);
    g.setFont (Theme::font (17.0f));
    if (showValue)
    {
        Theme::haloText (g, param->getCurrentValueAsText() + (param->getLabel().isNotEmpty() ? " " + param->getLabel() : juce::String()),
                         label.toFloat(), juce::Justification::centred, Theme::tealDark);
    }
    else
    {
        Theme::haloText (g, name, label.toFloat(), juce::Justification::centred, Theme::label, slider.isEnabled() ? 1.0f : 0.4f);
    }
}

// ============================================================================
//  ChoiceBox
// ============================================================================
ChoiceBox::ChoiceBox (juce::AudioProcessorValueTreeState& s, const juce::String& paramID)
{
    param = dynamic_cast<juce::AudioParameterChoice*> (s.getParameter (paramID));
    jassert (param != nullptr);
    attachment = std::make_unique<juce::ParameterAttachment> (*param, [this] (float v) { index = (int) v; repaint(); });
    attachment->sendInitialUpdate();
}

void ChoiceBox::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (juce::Colour (0xff3a3838));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);
    // inner top shadow for an inset look
    g.setGradientFill (juce::ColourGradient (juce::Colours::black.withAlpha (0.6f), 0, r.getY(), juce::Colours::transparentBlack, 0, r.getY() + 6, false));
    g.fillRoundedRectangle (r.withHeight (6.0f), 4.0f);

    auto arrow = [&] (float cx, bool left)
    {
        juce::Path p;
        const float cy = r.getCentreY();
        if (left) p.addTriangle (cx + 3.0f, cy - 4.5f, cx + 3.0f, cy + 4.5f, cx - 3.0f, cy);
        else p.addTriangle (cx - 3.0f, cy - 4.5f, cx - 3.0f, cy + 4.5f, cx + 3.0f, cy);
        g.setColour (Theme::pink.withAlpha (0.85f));
        g.fillPath (p);
    };
    arrow (r.getX() + 10.0f, true);
    arrow (r.getRight() - 10.0f, false);

    g.setColour (Theme::teal);
    g.setFont (Theme::font (juce::jmin (19.0f, r.getHeight() * 0.8f)));
    g.drawFittedText (param->choices[index], r.reduced (18.0f, 0).toNearestInt(), juce::Justification::centred, 1, 0.8f);
}

void ChoiceBox::mouseDown (const juce::MouseEvent& e)
{
    const int n = param->choices.size();
    if (e.x < 20) { attachment->setValueAsCompleteGesture ((float) ((index + n - 1) % n)); return; }
    if (e.x > getWidth() - 20) { attachment->setValueAsCompleteGesture ((float) ((index + 1) % n)); return; }
    juce::PopupMenu m;
    for (int i = 0; i < n; ++i) m.addItem (i + 1, param->choices[i], true, i == index);
    juce::Component::SafePointer<ChoiceBox> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMinimumWidth (getWidth()),
                     [safe] (int result)
                     {
                         if (safe != nullptr && result > 0) safe->attachment->setValueAsCompleteGesture ((float) (result - 1));
                     });
}

// ============================================================================
//  Segmented
// ============================================================================
Segmented::Segmented (juce::AudioProcessorValueTreeState& s, const juce::String& paramID, int cols) : columns (cols)
{
    param = dynamic_cast<juce::AudioParameterChoice*> (s.getParameter (paramID));
    jassert (param != nullptr);
    attachment = std::make_unique<juce::ParameterAttachment> (*param, [this] (float v) { index = (int) v; repaint(); });
    attachment->sendInitialUpdate();
}

juce::Rectangle<float> Segmented::cell (int i) const
{
    const int n = param->choices.size();
    const int rows = (n + columns - 1) / columns;
    const float w = (float) getWidth() / (float) columns, h = (float) getHeight() / (float) rows;
    return { w * (float) (i % columns), h * (float) (i / columns), w, h };
}

void Segmented::paint (juce::Graphics& g)
{
    for (int i = 0; i < param->choices.size(); ++i)
    {
        auto r = cell (i).reduced (2.0f);
        const bool on = i == index;
        g.setColour (juce::Colour (on ? 0xff1d1b1b : 0xff0b0b0b));
        g.fillRoundedRectangle (r, 4.0f);
        g.setColour (on ? Theme::teal.withAlpha (0.7f) : juce::Colour (0xff353332));
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
        auto led = juce::Rectangle<float> (6.0f, 6.0f).withCentre ({ r.getX() + 9.0f, r.getCentreY() });
        if (on)
        {
            g.setColour (Theme::teal.withAlpha (0.35f));
            g.fillEllipse (led.expanded (4.0f));
        }
        g.setColour (on ? Theme::teal : juce::Colour (0xff2d3a37));
        g.fillEllipse (led);
        g.setColour (on ? Theme::cream : Theme::cream.withAlpha (0.55f));
        g.setFont (Theme::font (juce::jmin (18.0f, r.getHeight() * 0.78f)));
        g.drawFittedText (param->choices[i], r.withTrimmedLeft (15.0f).toNearestInt(), juce::Justification::centred, 1, 0.75f);
    }
}

void Segmented::mouseDown (const juce::MouseEvent& e)
{
    for (int i = 0; i < param->choices.size(); ++i)
        if (cell (i).contains (e.position)) { attachment->setValueAsCompleteGesture ((float) i); return; }
}

// ============================================================================
//  BatToggle
// ============================================================================
BatToggle::BatToggle (juce::AudioProcessorValueTreeState& s, const juce::String& paramID, const juce::String& label)
    : juce::Button (label), text (label)
{
    setClickingTogglesState (true);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, paramID, *this);
}

void BatToggle::paintButton (juce::Graphics& g, bool over, bool)
{
    const bool on = getToggleState();
    auto r = getLocalBounds().toFloat();
    auto sw = r.removeFromLeft (juce::jmin (r.getHeight(), 30.0f));
    const float cx = sw.getCentreX(), cy = sw.getCentreY();
    const float nutR = sw.getWidth() * 0.36f;

    // hex nut
    juce::Path hex;
    for (int i = 0; i < 6; ++i)
    {
        const float a = juce::MathConstants<float>::pi / 3.0f * (float) i + 0.52f;
        const juce::Point<float> p (cx + nutR * std::cos (a), cy + nutR * std::sin (a));
        if (i == 0) hex.startNewSubPath (p); else hex.lineTo (p);
    }
    hex.closeSubPath();
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillPath (hex, juce::AffineTransform::translation (1.5f, 2.0f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe8e8e4), cx - nutR, cy - nutR, juce::Colour (0xff5d5c5a), cx + nutR, cy + nutR, false));
    g.fillPath (hex);
    g.setColour (juce::Colour (0xff1c1c1c));
    g.fillEllipse (juce::Rectangle<float> (nutR * 0.9f, nutR * 0.9f).withCentre ({ cx, cy }));

    // bat lever pointing up (on) or down (off)
    const float len = sw.getHeight() * 0.42f;
    const float tipY = on ? cy - len : cy + len;
    juce::Path lever;
    lever.startNewSubPath (cx - 2.2f, cy);
    lever.lineTo (cx - 3.6f, tipY);
    lever.lineTo (cx + 3.6f, tipY);
    lever.lineTo (cx + 2.2f, cy);
    lever.closeSubPath();
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillPath (lever, juce::AffineTransform::translation (2.0f, 2.5f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff8c8b88), cx - 4.0f, 0, juce::Colour (0xfff4f3ef), cx + 1.0f, 0, false));
    g.fillPath (lever);
    auto tip = juce::Rectangle<float> (8.5f, 6.5f).withCentre ({ cx, tipY });
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, tip.getX(), tip.getY(), juce::Colour (0xff7d7c79), tip.getRight(), tip.getBottom(), false));
    g.fillEllipse (tip);

    // label + LED
    auto led = juce::Rectangle<float> (6.0f, 6.0f).withCentre ({ r.getX() + 7.0f, r.getCentreY() });
    if (on)
    {
        g.setColour (Theme::teal.withAlpha (0.35f));
        g.fillEllipse (led.expanded (4.0f));
    }
    g.setColour (on ? Theme::teal : juce::Colour (0xff2d3a37));
    g.fillEllipse (led);
    g.setFont (Theme::font (17.0f));
    Theme::haloText (g, text, r.withTrimmedLeft (14.0f), juce::Justification::centredLeft, Theme::label, on || over ? 1.0f : 0.65f);
}

// ============================================================================
//  Panel
// ============================================================================
Panel::Panel (const juce::String& t) : title (t)
{
    setInterceptsMouseClicks (false, true);
}

void Panel::paint (juce::Graphics& g)
{
    // no fill: the section is an engraved line drawn on the photo, broken by its title (like a fieldset)
    auto box = getLocalBounds().toFloat().reduced (4.0f).withTrimmedTop (noTitle ? 0.0f : 14.0f);
    const auto font = Theme::font (23.0f);
    const float titleW = noTitle ? 0.0f : juce::GlyphArrangement::getStringWidth (font, title) + 14.0f;
    const float gapStart = box.getX() + 10.0f, gapEnd = gapStart + titleW;
    const float rad = 12.0f;

    juce::Path p;
    if (noTitle) p.addRoundedRectangle (box, rad);
    else
    {
        if (rightGap > 0.0f)
        {
            p.startNewSubPath (gapEnd, box.getY());
            p.lineTo (box.getRight() - rightGap - 6.0f, box.getY());
            p.startNewSubPath (box.getRight() - rad - 2.0f, box.getY());
        }
        else
            p.startNewSubPath (gapEnd, box.getY());
        p.lineTo (box.getRight() - rad, box.getY());
        p.quadraticTo (box.getRight(), box.getY(), box.getRight(), box.getY() + rad);
        p.lineTo (box.getRight(), box.getBottom() - rad);
        p.quadraticTo (box.getRight(), box.getBottom(), box.getRight() - rad, box.getBottom());
        p.lineTo (box.getX() + rad, box.getBottom());
        p.quadraticTo (box.getX(), box.getBottom(), box.getX(), box.getBottom() - rad);
        p.lineTo (box.getX(), box.getY() + rad);
        p.quadraticTo (box.getX(), box.getY(), box.getX() + rad, box.getY());
        p.lineTo (gapStart, box.getY());
    }
    // engraved: dark groove with a light edge under it
    g.setColour (juce::Colour (0xffF4FFF2).withAlpha (0.45f));
    g.strokePath (p, juce::PathStrokeType (1.3f), juce::AffineTransform::translation (0.9f, 1.1f));
    g.setColour (Theme::label.withAlpha (0.55f));
    g.strokePath (p, juce::PathStrokeType (1.3f));
    if (noTitle) return;

    // title: pink face with teal offset, like the logo, sitting in the gap of the line
    auto tr = juce::Rectangle<int> ((int) gapStart + 7, 0, (int) titleW, 30);
    g.setFont (font);
    g.setColour (juce::Colour (0xffEAF7E8).withAlpha (0.35f));
    for (int i = 0; i < 8; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * (float) i / 8.0f;
        g.drawText (title, tr.toFloat().translated (2.0f * std::cos (a), 2.0f * std::sin (a)), juce::Justification::centredLeft, false);
    }
    g.setColour (Theme::teal);
    g.drawText (title, tr.translated (-1, -1), juce::Justification::centredLeft, false);
    g.setColour (Theme::pink);
    g.drawText (title, tr.translated (1, 0), juce::Justification::centredLeft, false);
}

// ============================================================================
//  Pixel logo
// ============================================================================
void PixelLogo::paint (juce::Graphics& g)
{
    struct R { float x, y, w, h; };
    const std::vector<std::vector<R>> letters {
        { { 0, 0, 1, 6 }, { 0, 1, 4, 1 }, { 3, 1, 1, 5 }, { 0, 5, 4, 1 } },        // b
        { { 0, 1, 4, 1 }, { 0, 1, 1, 5 }, { 3, 1, 1, 5 } },                         // n
        { { 3, 0, 1, 6 }, { 0, 1, 4, 1 }, { 0, 1, 1, 5 }, { 0, 5, 4, 1 } },        // d
        { { 0, 0, 1, 6 }, { 0, 5, 3, 1 }, { 1, 1, 0.55f, 1 } },                     // t
    };
    const float widths[4] = { 4, 4, 4, 3 };
    const float gap = 1.15f;
    const float totalUnits = 4 + 4 + 4 + 3 + gap * 3;
    const float u = juce::jmin ((float) getWidth() / (totalUnits + 0.6f), (float) getHeight() / 6.6f);
    const float ox = u * 0.35f, oy = u * 0.3f;

    auto layer = [&] (juce::Colour c, float dx, float dy)
    {
        g.setColour (c);
        float x0 = ox;
        for (size_t i = 0; i < letters.size(); ++i)
        {
            for (const auto& rc : letters[i])
                g.fillRect (juce::Rectangle<float> (x0 + (rc.x + dx) * u, oy + (rc.y + dy) * u, rc.w * u, rc.h * u));
            x0 += (widths[i] + gap) * u;
        }
    };
    layer (Theme::teal, -0.2f, -0.2f);
    layer (Theme::pinkDeep, 0.16f, 0.14f);
    layer (Theme::pinkLight, 0.08f, 0.07f);
    layer (Theme::pink, 0.0f, 0.0f);
}

// ============================================================================
//  Jewel lamp
// ============================================================================
void JewelLamp::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (4.0f);
    const auto c = r.getCentre();
    const float rad = r.getWidth() * 0.5f;
    const float glow = juce::jlimit (0.0f, 1.0f, 0.25f + level * 1.6f);

    // chrome bezel
    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (r.translated (1.5f, 2.0f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xffefefea), r.getX(), r.getY(), juce::Colour (0xff4b4a48), r.getRight(), r.getBottom(), false));
    g.fillEllipse (r);
    auto jewel = r.reduced (rad * 0.18f);
    const float jr = jewel.getWidth() * 0.5f;

    // halo
    g.setGradientFill (juce::ColourGradient (Theme::pinkDeep.withAlpha (0.55f * glow), c.x, c.y, Theme::pinkDeep.withAlpha (0.0f), c.x + jr * 2.2f, c.y, true));
    g.fillEllipse (jewel.expanded (jr * 1.2f));

    g.setGradientFill (juce::ColourGradient (Theme::pinkDeep.interpolatedWith (juce::Colours::white, 0.35f * glow), c.x - jr * 0.3f, c.y - jr * 0.3f,
                                             Theme::pinkDeep.darker (1.4f - glow), c.x + jr, c.y + jr, true));
    g.fillEllipse (jewel);
    // facets
    for (int i = 0; i < 12; ++i)
    {
        const float a0 = juce::MathConstants<float>::twoPi * (float) i / 12.0f;
        const float a1 = a0 + juce::MathConstants<float>::twoPi / 12.0f;
        juce::Path f;
        f.startNewSubPath (c.x + jr * 0.45f * std::cos (a0 + 0.26f), c.y + jr * 0.45f * std::sin (a0 + 0.26f));
        f.lineTo (c.x + jr * std::cos (a0), c.y + jr * std::sin (a0));
        f.lineTo (c.x + jr * std::cos (a1), c.y + jr * std::sin (a1));
        f.closeSubPath();
        g.setColour ((i % 2 == 0 ? juce::Colours::white : juce::Colours::black).withAlpha (i % 2 == 0 ? 0.10f + 0.12f * glow : 0.18f));
        g.fillPath (f);
    }
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.fillEllipse (juce::Rectangle<float> (jr * 0.35f, jr * 0.25f).withCentre ({ c.x - jr * 0.35f, c.y - jr * 0.45f }));
}

// ============================================================================
//  LED meter
// ============================================================================
void LedMeter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (r, 3.0f);
    const int segs = 16;
    const float db = juce::Decibels::gainToDecibels (level, -60.0f);
    const float lit = juce::jlimit (0.0f, 1.0f, (db + 54.0f) / 54.0f) * (float) segs;
    auto inner = r.reduced (3.0f);
    const float w = inner.getWidth() / (float) segs;
    for (int i = 0; i < segs; ++i)
    {
        auto c = juce::Rectangle<float> (inner.getX() + w * (float) i + 1.0f, inner.getY(), w - 2.0f, inner.getHeight());
        const auto col = i < segs - 3 ? Theme::teal : (i < segs - 1 ? Theme::pink : Theme::pinkDeep);
        g.setColour ((float) i < lit ? col : col.withAlpha (0.13f));
        g.fillRoundedRectangle (c, 1.5f);
    }
}

// ============================================================================
//  Envelope display
// ============================================================================
EnvDisplay::EnvDisplay (juce::AudioProcessorValueTreeState& s, const char* a, const char* d, const char* su, const char* r)
{
    p[0] = s.getRawParameterValue (a); p[1] = s.getRawParameterValue (d);
    p[2] = s.getRawParameterValue (su); p[3] = s.getRawParameterValue (r);
    for (int i = 0; i < 4; ++i) last[i] = p[i]->load();
    startTimerHz (15);
}

void EnvDisplay::timerCallback()
{
    bool changed = false;
    for (int i = 0; i < 4; ++i)
        if (std::abs (p[i]->load() - last[i]) > 1.0e-5f) { last[i] = p[i]->load(); changed = true; }
    if (changed) repaint();
}

void EnvDisplay::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (juce::Colour (0xff0a0a0a));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (juce::Colour (0xff2c2a29));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    auto plot = r.reduced (8.0f, 6.0f);
    const float a = std::sqrt (last[0]), d = std::sqrt (last[1]), s = last[2], rel = std::sqrt (last[3]);
    const float hold = 0.6f;
    const float total = a + d + hold + rel + 1.0e-3f;
    const float sx = plot.getWidth() / total;
    const float x0 = plot.getX(), yb = plot.getBottom(), yt = plot.getY(), h = plot.getHeight();

    juce::Path path;
    path.startNewSubPath (x0, yb);
    path.lineTo (x0 + a * sx, yt);
    const float xd = x0 + (a + d) * sx;
    for (int i = 1; i <= 12; ++i)
    {
        const float t = (float) i / 12.0f;
        path.lineTo (x0 + (a + d * t) * sx, yt + (1.0f - (s + (1.0f - s) * std::exp (-4.0f * t))) * h);
    }
    path.lineTo (xd + hold * sx, yb - s * h);
    for (int i = 1; i <= 12; ++i)
    {
        const float t = (float) i / 12.0f;
        path.lineTo (xd + (hold + rel * t) * sx, yb - s * h * std::exp (-4.0f * t));
    }
    juce::Path fill (path);
    fill.lineTo (plot.getRight(), yb);
    fill.closeSubPath();
    g.setColour (Theme::teal.withAlpha (0.13f));
    g.fillPath (fill);
    g.setColour (Theme::teal);
    g.strokePath (path, juce::PathStrokeType (1.8f, juce::PathStrokeType::curved));
}
