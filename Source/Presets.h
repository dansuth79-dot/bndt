#pragma once
#include "Common.h"
#include <vector>
#include <utility>

struct FactoryPreset
{
    juce::String name, category;
    std::vector<std::pair<const char*, float>> values;   // real-world values; choices as index, bools as 0/1
};

namespace Presets
{
    const juce::StringArray& categories();
    const std::vector<FactoryPreset>& factory();

    // Applies a preset: every parameter goes back to its default, then the preset's values are set.
    void apply (juce::AudioProcessorValueTreeState& state, const std::vector<std::pair<juce::String, float>>& values);
    void applyFactory (juce::AudioProcessorValueTreeState& state, int index);

    // User presets (XML files) live here.
    juce::File userFolder();
    bool saveUser (juce::AudioProcessorValueTreeState& state, const juce::String& name, const juce::String& category);
    bool loadUser (juce::AudioProcessorValueTreeState& state, const juce::File& file);
    juce::Array<juce::File> userPresets();

    // A musically sensible random patch.
    void randomise (juce::AudioProcessorValueTreeState& state, juce::Random& rng);
}
