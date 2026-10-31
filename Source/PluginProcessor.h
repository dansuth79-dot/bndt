#pragma once
#include "Common.h"
#include "SynthEngine.h"
#include "FX.h"

class BndtProcessor : public juce::AudioProcessor
{
public:
    BndtProcessor();
    ~BndtProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "bndt"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentFactory; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // preset handling used by the editor
    void loadFactory (int index);
    void loadUserPreset (const juce::File& f);
    void saveUserPreset (const juce::String& name, const juce::String& category);
    void randomise();
    juce::String getPresetName() const { return presetName; }
    juce::String getPresetCategory() const { return presetCategory; }

    juce::AudioProcessorValueTreeState state;
    juce::MidiKeyboardState keyboardState;
    std::atomic<float> outputLevel { 0.0f };
    float getVocoderInputLevel() const { return fx.getVocoderInputLevel(); }
    std::atomic<int> presetVersion { 0 };   // bumps when a preset is loaded so the UI can refresh its display

private:
    void readParams();
    float getF (const char* id) const;
    int getI (const char* id) const;
    bool getB (const char* id) const;

    SynthEngine engine;
    FXChain fx;
    juce::AudioBuffer<float> sideBuf;
    VoiceParams vp;
    FXParams fp;
    std::unordered_map<std::string, std::atomic<float>*> raw;
    int currentFactory = 0;
    juce::String presetName { "Init" }, presetCategory { "" };
    juce::Random random;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BndtProcessor)
};
