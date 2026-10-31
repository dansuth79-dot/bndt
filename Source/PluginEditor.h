#pragma once
#include "Common.h"
#include "PluginProcessor.h"
#include "UI/Widgets.h"

// Everything is laid out on a fixed 1280 x 820 canvas and scaled to the window.
class BndtContent : public juce::Component, private juce::Timer
{
public:
    static constexpr int W = 1280, H = 1000;
    explicit BndtContent (BndtProcessor&);
    ~BndtContent() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (juce::Rectangle<int> (476, 22, 292, 54).contains (e.getPosition())) showPresetMenu();
    }

private:
    void timerCallback() override;
    void buildBackdrop();
    void showPresetMenu();
    void stepPreset (int delta);
    void savePreset();
    Knob& knob (const char* id, const char* label);

    BndtProcessor& proc;
    juce::AudioProcessorValueTreeState& st;
    BndtLookAndFeel lnf;
    juce::Image background, backdrop;

    PixelLogo logo;
    JewelLamp jewel;
    juce::TextButton prevBtn { "<" }, nextBtn { ">" }, saveBtn { "SAVE" }, surpriseBtn { "SURPRISE" };
    juce::Label presetLabel;
    int lastPresetVersion = -1;

    Panel pOsc { "OSCILLATORS" }, pTape { "TAPE" }, pFilter { "FILTER" }, pVoice { "VOICE" },
          pAmp { "AMP ENVELOPE" }, pFEnv { "FILTER ENVELOPE" }, pLfo1 { "LFO 1" }, pLfo2 { "LFO 2" },
          pSat { "SATURATION" }, pChorus { "CHORUS" }, pEcho { "TAPE ECHO" }, pRev { "REVERB" }, pRadio { "RADIO" },
          pVoc { "VOCODER" }, pKeys { "" }, pOut { "" };

    std::vector<std::unique_ptr<Knob>> knobs;
    std::map<juce::String, Knob*> knobById;

    ChoiceBox osc1Wave, osc2Wave, tapeSound, lfo1Shape, lfo1Dest, lfo2Shape, lfo2Dest, voiceMode, strumChord, delayDiv, delayHeads;
    Segmented filterType, satType, chorusMode, revType, vocCarrier, vocBands;
    LedMeter vocMeter;
    BatToggle tapeLimit, strumOn, satOn, delayOn, delaySync, revOn, radioOn, vocOn;
    EnvDisplay ampEnv, fltEnv;
    juce::MidiKeyboardComponent keyboard;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BndtContent)
};

class BndtEditor : public juce::AudioProcessorEditor
{
public:
    explicit BndtEditor (BndtProcessor&);
    void resized() override;
    void paint (juce::Graphics& g) override { g.fillAll (juce::Colours::black); }
    BndtContent& getContent() { return content; }
private:
    BndtContent content;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BndtEditor)
};
