#pragma once
#include "../Common.h"

namespace Theme
{
    // sampled from the bndt logo artwork
    const juce::Colour pink      { 0xffFC6CA8 };
    const juce::Colour pinkLight { 0xffF999CD };
    const juce::Colour pinkDeep  { 0xffF01C63 };
    const juce::Colour teal      { 0xff1EC6A6 };
    const juce::Colour mint      { 0xffCFF5D1 };
    const juce::Colour rust      { 0xff8A6355 };
    const juce::Colour ink       { 0xff66605B };
    const juce::Colour panel     { 0xff151414 };
    const juce::Colour cream     { 0xffECE6D2 };
    const juce::Colour label     { 0xff1E1918 };   // dark ink printed straight onto the photo
    const juce::Colour tealDark  { 0xff0B7A66 };

    juce::Font font (float height);
    // dark text with a soft light halo, readable over both the mint and the rust parts of the photo
    void haloText (juce::Graphics&, const juce::String&, juce::Rectangle<float>, juce::Justification, juce::Colour, float alpha = 1.0f);
    juce::Image image (const char* resourceName);
}

class BndtLookAndFeel : public juce::LookAndFeel_V4
{
public:
    BndtLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float, float, juce::Slider&) override;
    void drawPopupMenuBackground (juce::Graphics&, int w, int h) override;
    void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>&, bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColour) override;
    juce::Font getPopupMenuFont() override { return Theme::font (19.0f); }
    void drawPopupMenuSectionHeader (juce::Graphics&, const juce::Rectangle<int>&, const juce::String&) override;
    juce::Font getAlertWindowTitleFont() override { return Theme::font (24.0f); }
    juce::Font getAlertWindowMessageFont() override { return Theme::font (18.0f); }
    juce::Font getTextButtonFont (juce::TextButton&, int) override { return Theme::font (18.0f); }
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;

private:
    juce::Image shadow, light, body[3];
};

// Worn '65 blackface-style skirted knob with its name (or value while touched) underneath.
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState& s, const juce::String& paramID, const juce::String& label);
    void resized() override;
    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override { repaint(); }
    void mouseExit (const juce::MouseEvent&) override { repaint(); }
    juce::Slider slider;
private:
    juce::String name;
    juce::RangedAudioParameter* param = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// Dark display showing a choice; click for the list, arrows to step.
class ChoiceBox : public juce::Component
{
public:
    ChoiceBox (juce::AudioProcessorValueTreeState& s, const juce::String& paramID);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    juce::AudioParameterChoice* param = nullptr;
    std::unique_ptr<juce::ParameterAttachment> attachment;
    int index = 0;
};

// Row of small buttons with LEDs, one per choice.
class Segmented : public juce::Component
{
public:
    Segmented (juce::AudioProcessorValueTreeState& s, const juce::String& paramID, int columns);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
private:
    juce::Rectangle<float> cell (int i) const;
    juce::AudioParameterChoice* param = nullptr;
    std::unique_ptr<juce::ParameterAttachment> attachment;
    int index = 0, columns = 4;
};

// Chrome bat-handle toggle switch like the amp's bright switches.
class BatToggle : public juce::Button
{
public:
    BatToggle (juce::AudioProcessorValueTreeState& s, const juce::String& paramID, const juce::String& label);
    void paintButton (juce::Graphics&, bool over, bool down) override;
private:
    juce::String text;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
};

// Blackface panel with a title in the logo's pink-on-teal offset style.
class Panel : public juce::Component
{
public:
    explicit Panel (const juce::String& title);
    void paint (juce::Graphics&) override;
    juce::Rectangle<int> content() const { return getLocalBounds().reduced (12, 0).withTrimmedTop (36).withTrimmedBottom (8); }
    void setOutlineOnly (bool b) { noTitle = b; }
    void setRightGap (float w) { rightGap = w; }   // leaves room in the top line for a header switch
private:
    juce::String title;
    bool noTitle = false;
    float rightGap = 0.0f;
};

// "bndt" rebuilt as pixel blocks: teal offset shadow, pink face, light/deep pink edges.
class PixelLogo : public juce::Component
{
public:
    void paint (juce::Graphics&) override;
};

// Faceted jewel pilot lamp that glows with the output level.
class JewelLamp : public juce::Component
{
public:
    void setLevel (float l) { if (std::abs (l - level) > 0.01f) { level = l; repaint(); } }
    void paint (juce::Graphics&) override;
private:
    float level = 0.0f;
};

// Horizontal LED ladder showing the vocoder source level.
class LedMeter : public juce::Component
{
public:
    void setLevel (float l) { if (std::abs (l - level) > 0.003f) { level = l; repaint(); } }
    void paint (juce::Graphics&) override;
private:
    float level = 0.0f;
};

// Small envelope shape display (reads the four ADSR parameters).
class EnvDisplay : public juce::Component, private juce::Timer
{
public:
    EnvDisplay (juce::AudioProcessorValueTreeState& s, const char* a, const char* d, const char* su, const char* r);
    void paint (juce::Graphics&) override;
private:
    void timerCallback() override;
    std::atomic<float>* p[4] {};
    float last[4] {};
};
