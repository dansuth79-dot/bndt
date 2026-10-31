#pragma once
#include "Common.h"

// Every parameter in one place: IDs, ranges, defaults and choice lists.
namespace P
{
    // Oscillators
    inline constexpr const char* osc1Wave = "osc1Wave";
    inline constexpr const char* osc1Oct = "osc1Oct";
    inline constexpr const char* osc1PW = "osc1PW";
    inline constexpr const char* osc1Level = "osc1Level";
    inline constexpr const char* osc2Wave = "osc2Wave";
    inline constexpr const char* osc2Oct = "osc2Oct";
    inline constexpr const char* osc2Semi = "osc2Semi";
    inline constexpr const char* osc2Detune = "osc2Detune";
    inline constexpr const char* osc2Level = "osc2Level";
    inline constexpr const char* subLevel = "subLevel";
    inline constexpr const char* noiseLevel = "noiseLevel";
    inline constexpr const char* drift = "drift";

    // Tape layer
    inline constexpr const char* tapeSound = "tapeSound";
    inline constexpr const char* tapeLevel = "tapeLevel";
    inline constexpr const char* tapeWow = "tapeWow";
    inline constexpr const char* tapeAge = "tapeAge";
    inline constexpr const char* tapeLimit = "tapeLimit";

    // Filter
    inline constexpr const char* filterType = "filterType";
    inline constexpr const char* cutoff = "cutoff";
    inline constexpr const char* resonance = "resonance";
    inline constexpr const char* filterDrive = "filterDrive";
    inline constexpr const char* envAmount = "envAmount";
    inline constexpr const char* keyTrack = "keyTrack";
    inline constexpr const char* velFilter = "velFilter";

    // Envelopes
    inline constexpr const char* ampA = "ampA";
    inline constexpr const char* ampD = "ampD";
    inline constexpr const char* ampS = "ampS";
    inline constexpr const char* ampR = "ampR";
    inline constexpr const char* velAmp = "velAmp";
    inline constexpr const char* fltA = "fltA";
    inline constexpr const char* fltD = "fltD";
    inline constexpr const char* fltS = "fltS";
    inline constexpr const char* fltR = "fltR";

    // LFOs
    inline constexpr const char* lfo1Shape = "lfo1Shape";
    inline constexpr const char* lfo1Rate = "lfo1Rate";
    inline constexpr const char* lfo1Dest = "lfo1Dest";
    inline constexpr const char* lfo1Amount = "lfo1Amount";
    inline constexpr const char* lfo1Fade = "lfo1Fade";
    inline constexpr const char* lfo2Shape = "lfo2Shape";
    inline constexpr const char* lfo2Rate = "lfo2Rate";
    inline constexpr const char* lfo2Dest = "lfo2Dest";
    inline constexpr const char* lfo2Amount = "lfo2Amount";
    inline constexpr const char* lfo2Fade = "lfo2Fade";

    // Voice / strum
    inline constexpr const char* voiceMode = "voiceMode";
    inline constexpr const char* glide = "glide";
    inline constexpr const char* strumOn = "strumOn";
    inline constexpr const char* strumChord = "strumChord";
    inline constexpr const char* strumSpeed = "strumSpeed";
    inline constexpr const char* strumSpan = "strumSpan";

    // FX
    inline constexpr const char* satOn = "satOn";
    inline constexpr const char* satType = "satType";
    inline constexpr const char* satDrive = "satDrive";
    inline constexpr const char* satMix = "satMix";
    inline constexpr const char* chorusMode = "chorusMode";
    inline constexpr const char* delayOn = "delayOn";
    inline constexpr const char* delayTime = "delayTime";
    inline constexpr const char* delaySync = "delaySync";
    inline constexpr const char* delayDiv = "delayDiv";
    inline constexpr const char* delayHeads = "delayHeads";
    inline constexpr const char* delayFeedback = "delayFeedback";
    inline constexpr const char* delayWow = "delayWow";
    inline constexpr const char* delayTone = "delayTone";
    inline constexpr const char* delayMix = "delayMix";
    inline constexpr const char* revOn = "revOn";
    inline constexpr const char* revType = "revType";
    inline constexpr const char* revSize = "revSize";
    inline constexpr const char* revTone = "revTone";
    inline constexpr const char* revMix = "revMix";
    inline constexpr const char* radioOn = "radioOn";
    inline constexpr const char* radioAmount = "radioAmount";
    inline constexpr const char* radioTune = "radioTune";

    // Vocoder
    inline constexpr const char* vocOn = "vocOn";
    inline constexpr const char* vocBands = "vocBands";
    inline constexpr const char* vocCarrier = "vocCarrier";
    inline constexpr const char* vocInput = "vocInput";
    inline constexpr const char* vocFormant = "vocFormant";
    inline constexpr const char* vocResponse = "vocResponse";
    inline constexpr const char* vocSibilance = "vocSibilance";
    inline constexpr const char* vocMix = "vocMix";

    // Output
    inline constexpr const char* lowCut = "lowCut";
    inline constexpr const char* master = "master";
    inline constexpr const char* trim = "trim";   // hidden per-preset loudness match

    inline const juce::StringArray waveNames { "Saw", "Square", "Triangle", "Sine" };
    inline const juce::StringArray tapeNames { "Strings", "Cello", "Flute", "Choir", "Brass", "Organ" };
    inline const juce::StringArray filterNames { "Smooth", "Scream" };
    inline const juce::StringArray lfoShapeNames { "Sine", "Triangle", "Square", "Random" };
    inline const juce::StringArray lfoDestNames { "Pitch", "Cutoff", "Pulse Width", "Volume", "Osc 2 Pitch", "Pan" };
    inline const juce::StringArray voiceModeNames { "Poly", "Mono", "Legato" };
    inline const juce::StringArray chordNames { "Major", "Minor", "7th", "Maj 7", "Min 7", "Sus 4" };
    inline const juce::StringArray satTypeNames { "Tape", "Tube" };
    inline const juce::StringArray chorusNames { "Off", "I", "II", "I + II" };
    inline const juce::StringArray divNames { "1/16", "1/8 T", "1/8", "1/8 D", "1/4 T", "1/4", "1/4 D", "1/2" };
    inline const juce::StringArray headNames { "1 Head", "2 Heads", "3 Heads" };
    inline const juce::StringArray revTypeNames { "Spring", "Hall" };
    inline const juce::StringArray vocBandNames { "8", "16", "24", "32" };
    inline const juce::StringArray vocCarrierNames { "Synth", "Synth + Noise" };

    inline juce::NormalisableRange<float> skewed (float lo, float hi, float centre)
    {
        juce::NormalisableRange<float> r (lo, hi);
        r.setSkewForCentre (centre);
        return r;
    }

    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> p;
        auto F = [&] (const char* id, const char* name, NormalisableRange<float> r, float def, const char* unit = "")
        {
            p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, r, def,
                                                                AudioParameterFloatAttributes().withLabel (unit)));
        };
        auto C = [&] (const char* id, const char* name, const StringArray& items, int def)
        {
            p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, items, def));
        };
        auto I = [&] (const char* id, const char* name, int lo, int hi, int def)
        {
            p.push_back (std::make_unique<AudioParameterInt> (ParameterID { id, 1 }, name, lo, hi, def));
        };
        auto B = [&] (const char* id, const char* name, bool def)
        {
            p.push_back (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def));
        };
        NormalisableRange<float> unit (0.0f, 1.0f);

        C (osc1Wave, "Osc 1 Wave", waveNames, 0);
        I (osc1Oct, "Osc 1 Octave", -2, 2, 0);
        F (osc1PW, "Osc 1 Pulse Width", { 0.05f, 0.95f }, 0.5f);
        F (osc1Level, "Osc 1 Level", unit, 0.8f);
        C (osc2Wave, "Osc 2 Wave", waveNames, 0);
        I (osc2Oct, "Osc 2 Octave", -2, 2, 0);
        I (osc2Semi, "Osc 2 Semitones", -12, 12, 0);
        F (osc2Detune, "Osc 2 Detune", { -50.0f, 50.0f }, 7.0f, "ct");
        F (osc2Level, "Osc 2 Level", unit, 0.5f);
        F (subLevel, "Sub Level", unit, 0.0f);
        F (noiseLevel, "Noise Level", unit, 0.0f);
        F (drift, "Drift", unit, 0.25f);

        C (tapeSound, "Tape Sound", tapeNames, 0);
        F (tapeLevel, "Tape Level", unit, 0.0f);
        F (tapeWow, "Tape Wow", unit, 0.3f);
        F (tapeAge, "Tape Age", unit, 0.3f);
        B (tapeLimit, "8 Second Tape", false);

        C (filterType, "Filter Type", filterNames, 0);
        F (cutoff, "Cutoff", skewed (20.0f, 20000.0f, 1200.0f), 3000.0f, "Hz");
        F (resonance, "Resonance", unit, 0.2f);
        F (filterDrive, "Filter Drive", unit, 0.2f);
        F (envAmount, "Filter Env", { -1.0f, 1.0f }, 0.3f);
        F (keyTrack, "Key Track", unit, 0.5f);
        F (velFilter, "Velocity > Filter", unit, 0.3f);

        auto timeRange = skewed (0.001f, 10.0f, 0.5f);
        F (ampA, "Amp Attack", timeRange, 0.005f, "s");
        F (ampD, "Amp Decay", timeRange, 0.4f, "s");
        F (ampS, "Amp Sustain", unit, 0.8f);
        F (ampR, "Amp Release", timeRange, 0.3f, "s");
        F (velAmp, "Velocity > Amp", unit, 0.5f);
        F (fltA, "Filter Attack", timeRange, 0.005f, "s");
        F (fltD, "Filter Decay", timeRange, 0.5f, "s");
        F (fltS, "Filter Sustain", unit, 0.3f);
        F (fltR, "Filter Release", timeRange, 0.4f, "s");

        auto rate = skewed (0.05f, 20.0f, 2.0f);
        C (lfo1Shape, "LFO 1 Shape", lfoShapeNames, 0);
        F (lfo1Rate, "LFO 1 Rate", rate, 5.0f, "Hz");
        C (lfo1Dest, "LFO 1 Destination", lfoDestNames, 0);
        F (lfo1Amount, "LFO 1 Amount", unit, 0.0f);
        F (lfo1Fade, "LFO 1 Fade In", { 0.0f, 4.0f }, 0.0f, "s");
        C (lfo2Shape, "LFO 2 Shape", lfoShapeNames, 1);
        F (lfo2Rate, "LFO 2 Rate", rate, 0.3f, "Hz");
        C (lfo2Dest, "LFO 2 Destination", lfoDestNames, 1);
        F (lfo2Amount, "LFO 2 Amount", unit, 0.0f);
        F (lfo2Fade, "LFO 2 Fade In", { 0.0f, 4.0f }, 0.0f, "s");

        C (voiceMode, "Voice Mode", voiceModeNames, 0);
        F (glide, "Glide", skewed (0.0f, 2.0f, 0.25f), 0.0f, "s");
        B (strumOn, "Strum", false);
        C (strumChord, "Strum Chord", chordNames, 0);
        F (strumSpeed, "Strum Speed", skewed (5.0f, 200.0f, 40.0f), 35.0f, "ms");
        I (strumSpan, "Strum Notes", 3, 8, 5);

        B (satOn, "Saturation On", true);
        C (satType, "Saturation Type", satTypeNames, 0);
        F (satDrive, "Saturation Drive", unit, 0.25f);
        F (satMix, "Saturation Mix", unit, 1.0f);
        C (chorusMode, "Chorus", chorusNames, 0);
        B (delayOn, "Echo On", false);
        F (delayTime, "Echo Time", skewed (20.0f, 1500.0f, 350.0f), 375.0f, "ms");
        B (delaySync, "Echo Sync", true);
        C (delayDiv, "Echo Division", divNames, 3);
        C (delayHeads, "Echo Heads", headNames, 0);
        F (delayFeedback, "Echo Feedback", { 0.0f, 1.1f }, 0.4f);
        F (delayWow, "Echo Wow", unit, 0.3f);
        F (delayTone, "Echo Tone", unit, 0.45f);
        F (delayMix, "Echo Mix", unit, 0.25f);
        B (revOn, "Reverb On", true);
        C (revType, "Reverb Type", revTypeNames, 1);
        F (revSize, "Reverb Size", unit, 0.5f);
        F (revTone, "Reverb Tone", unit, 0.5f);
        F (revMix, "Reverb Mix", unit, 0.2f);
        B (radioOn, "Radio On", false);
        F (radioAmount, "Radio Amount", unit, 0.6f);
        F (radioTune, "Radio Tuning", unit, 0.3f);

        B (vocOn, "Vocoder On", false);
        C (vocBands, "Vocoder Bands", vocBandNames, 1);
        C (vocCarrier, "Vocoder Carrier", vocCarrierNames, 1);
        F (vocInput, "Vocoder Input Gain", { -12.0f, 24.0f }, 0.0f, "dB");
        F (vocFormant, "Vocoder Formant", { -12.0f, 12.0f }, 0.0f, "st");
        F (vocResponse, "Vocoder Response", skewed (2.0f, 120.0f, 20.0f), 15.0f, "ms");
        F (vocSibilance, "Vocoder Sibilance", unit, 0.4f);
        F (vocMix, "Vocoder Mix", unit, 1.0f);

        F (lowCut, "Low Cut", skewed (10.0f, 400.0f, 60.0f), 30.0f, "Hz");
        F (master, "Master", { -24.0f, 6.0f }, 0.0f, "dB");
        p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { trim, 1 }, "Preset Trim", NormalisableRange<float> (-30.0f, 12.0f), 0.0f,
                                                            AudioParameterFloatAttributes().withLabel ("dB").withAutomatable (false)));

        return { p.begin(), p.end() };
    }
}
