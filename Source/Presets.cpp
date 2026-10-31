#include "Presets.h"
#include "Params.h"
#include "PresetTrims.h"

namespace
{
    // choice indices, so the preset table reads like a patch sheet
    enum { Saw = 0, Square, Tri, Sine };
    enum { Strings = 0, Cello, Flute, Choir, Brass, Organ };
    enum { Smooth = 0, Scream };
    enum { LSine = 0, LTri, LSquare, LRandom };
    enum { DPitch = 0, DCutoff, DPW, DVolume, DOsc2, DPan };
    enum { Poly = 0, Mono, Legato };
    enum { Major = 0, Minor, Seventh, Maj7, Min7, Sus4 };
    enum { SatTape = 0, SatTube };
    enum { ChOff = 0, ChI, ChII, ChBoth };
    enum { D16 = 0, D8T, D8, D8D, D4T, D4, D4D, D2 };
    enum { Spring = 0, Hall };

    using namespace P;

    std::vector<FactoryPreset> build()
    {
        std::vector<FactoryPreset> v;
        auto add = [&] (const char* cat, const char* name, std::vector<std::pair<const char*, float>> vals)
        {
            v.push_back ({ name, cat, std::move (vals) });
        };

        // ------------------------------------------------------------------ BASS
        add ("Bass", "Phat Bottom", {
            { osc1Wave, Saw }, { osc1Oct, -1 }, { osc2Wave, Square }, { osc2Oct, -1 }, { osc2Detune, 4 }, { osc2Level, 0.6f },
            { subLevel, 0.5f }, { cutoff, 380 }, { resonance, 0.25f }, { filterDrive, 0.45f }, { envAmount, 0.45f }, { keyTrack, 0.4f },
            { ampA, 0.002f }, { ampD, 0.6f }, { ampS, 0.75f }, { ampR, 0.12f }, { fltA, 0.001f }, { fltD, 0.35f }, { fltS, 0.15f },
            { voiceMode, Legato }, { glide, 0.04f }, { satDrive, 0.35f }, { revOn, 0 }, { lowCut, 20 } });
        add ("Bass", "Dub Cowboy", {
            { osc1Wave, Tri }, { osc1Oct, -1 }, { osc1Level, 0.9f }, { osc2Wave, Saw }, { osc2Oct, -1 }, { osc2Level, 0.2f },
            { cutoff, 600 }, { resonance, 0.1f }, { envAmount, 0.2f }, { ampD, 1.2f }, { ampS, 0.6f }, { ampR, 0.2f },
            { voiceMode, Mono }, { satType, SatTube }, { satDrive, 0.4f }, { revType, Spring }, { revSize, 0.3f }, { revMix, 0.08f }, { lowCut, 20 } });
        add ("Bass", "Snappy Pyramid", {
            { osc1Wave, Saw }, { osc1Oct, -1 }, { osc2Wave, Square }, { osc2Oct, -1 }, { osc2Detune, -6 }, { osc2Level, 0.5f },
            { cutoff, 330 }, { resonance, 0.35f }, { filterDrive, 0.6f }, { subLevel, 0.35f }, { osc1Level, 1.0f }, { envAmount, 0.7f }, { velFilter, 0.5f },
            { fltA, 0.001f }, { fltD, 0.18f }, { fltS, 0.0f }, { ampA, 0.001f }, { ampD, 0.45f }, { ampS, 0.55f }, { ampR, 0.08f },
            { voiceMode, Mono }, { satDrive, 0.3f }, { revOn, 0 }, { lowCut, 20 } });
        add ("Bass", "Scream Sub", {
            { filterType, Scream }, { osc1Wave, Saw }, { osc1Oct, -1 }, { osc2Wave, Saw }, { osc2Oct, -1 }, { osc2Detune, 10 },
            { cutoff, 300 }, { resonance, 0.62f }, { filterDrive, 0.55f }, { envAmount, 0.55f }, { fltD, 0.25f }, { fltS, 0.1f },
            { ampS, 0.8f }, { ampR, 0.1f }, { voiceMode, Legato }, { glide, 0.08f }, { satDrive, 0.4f }, { revOn, 0 }, { lowCut, 20 } });
        add ("Bass", "Rubber Funk", {
            { osc1Wave, Square }, { osc1PW, 0.35f }, { osc1Oct, -1 }, { osc2Wave, Saw }, { osc2Oct, -1 }, { osc2Level, 0.5f }, { subLevel, 0.3f }, { osc1Level, 1.0f },
            { cutoff, 600 }, { resonance, 0.55f }, { envAmount, 0.65f }, { velFilter, 0.6f }, { fltA, 0.001f }, { fltD, 0.22f }, { fltS, 0.05f },
            { ampA, 0.001f }, { ampD, 0.3f }, { ampS, 0.5f }, { ampR, 0.07f }, { voiceMode, Mono }, { glide, 0.03f },
            { satDrive, 0.3f }, { revOn, 0 }, { lowCut, 25 } });
        add ("Bass", "Cello Floor", {
            { osc1Wave, Sine }, { osc1Oct, -1 }, { osc1Level, 0.9f }, { osc2Level, 0.0f }, { tapeSound, Cello }, { tapeLevel, 0.35f },
            { tapeWow, 0.2f }, { tapeAge, 0.4f }, { cutoff, 900 }, { envAmount, 0.1f }, { ampD, 0.9f }, { ampS, 0.4f }, { ampR, 0.25f },
            { voiceMode, Mono }, { revType, Hall }, { revMix, 0.1f }, { lowCut, 20 } });

        // ------------------------------------------------------------------ KEYS
        add ("Keys", "Chord Machine Piano", {
            { osc1Wave, Tri }, { osc2Wave, Square }, { osc2Oct, 1 }, { osc2Level, 0.25f }, { osc2Detune, 3 },
            { cutoff, 2600 }, { envAmount, 0.4f }, { fltD, 0.7f }, { fltS, 0.1f }, { ampA, 0.002f }, { ampD, 1.4f }, { ampS, 0.0f },
            { ampR, 0.5f }, { velAmp, 0.7f }, { chorusMode, ChI }, { revType, Spring }, { revMix, 0.25f } });
        add ("Keys", "Melancholy Keys", {
            { osc1Wave, Saw }, { osc2Wave, Saw }, { osc2Detune, 9 }, { osc2Level, 0.7f }, { cutoff, 1500 }, { resonance, 0.15f },
            { envAmount, 0.35f }, { fltA, 0.01f }, { fltD, 0.9f }, { fltS, 0.35f }, { ampA, 0.01f }, { ampD, 1.0f }, { ampS, 0.7f }, { ampR, 0.6f },
            { chorusMode, ChII }, { delayOn, 1 }, { delayDiv, D4D }, { delayMix, 0.18f }, { delayFeedback, 0.35f }, { revMix, 0.25f } });
        add ("Keys", "Electric Tine", {
            { osc1Wave, Sine }, { osc2Wave, Sine }, { osc2Oct, 2 }, { osc2Level, 0.18f }, { osc2Detune, 2 }, { cutoff, 5000 },
            { envAmount, 0.1f }, { ampA, 0.002f }, { ampD, 2.5f }, { ampS, 0.15f }, { ampR, 0.5f }, { velAmp, 0.8f },
            { lfo1Dest, DPan }, { lfo1Rate, 3.5f }, { lfo1Amount, 0.5f }, { satType, SatTube }, { satDrive, 0.3f }, { revMix, 0.2f } });
        add ("Keys", "Pocket Wurly", {
            { osc1Wave, Tri }, { osc2Wave, Square }, { osc2Level, 0.3f }, { osc2Detune, 2 }, { cutoff, 2200 }, { envAmount, 0.3f },
            { fltD, 0.5f }, { fltS, 0.2f }, { ampA, 0.002f }, { ampD, 1.6f }, { ampS, 0.2f }, { ampR, 0.3f }, { velAmp, 0.7f },
            { satType, SatTube }, { satDrive, 0.45f }, { lfo1Dest, DVolume }, { lfo1Rate, 5.5f }, { lfo1Amount, 0.2f },
            { revType, Spring }, { revMix, 0.2f } });
        add ("Keys", "Combo Organ", {
            { osc1Wave, Square }, { osc1Level, 0.6f }, { osc2Wave, Square }, { osc2Oct, 1 }, { osc2Level, 0.4f }, { osc2Detune, 1 },
            { subLevel, 0.3f }, { cutoff, 4500 }, { envAmount, 0.0f }, { ampA, 0.002f }, { ampD, 0.1f }, { ampS, 1.0f }, { ampR, 0.05f },
            { velAmp, 0.1f }, { lfo1Dest, DPitch }, { lfo1Rate, 6.5f }, { lfo1Amount, 0.12f }, { chorusMode, ChII },
            { satType, SatTube }, { satDrive, 0.5f }, { revType, Spring }, { revMix, 0.18f } });
        add ("Keys", "Clavinet Kink", {
            { osc1Wave, Square }, { osc1PW, 0.15f }, { osc2Wave, Saw }, { osc2Level, 0.3f }, { filterType, Scream }, { cutoff, 1400 },
            { resonance, 0.4f }, { envAmount, 0.45f }, { fltD, 0.15f }, { fltS, 0.05f }, { ampA, 0.001f }, { ampD, 0.35f },
            { ampS, 0.25f }, { ampR, 0.06f }, { velFilter, 0.6f }, { satType, SatTube }, { satDrive, 0.4f }, { revMix, 0.12f } });

        // ------------------------------------------------------------------ PADS
        add ("Pads", "Dusk Machine", {
            { osc1Wave, Saw }, { osc2Wave, Saw }, { osc2Detune, 12 }, { osc2Level, 0.8f }, { subLevel, 0.2f }, { cutoff, 900 },
            { resonance, 0.2f }, { envAmount, 0.25f }, { fltA, 0.8f }, { fltD, 2.0f }, { fltS, 0.5f }, { fltR, 1.5f },
            { ampA, 0.9f }, { ampD, 1.5f }, { ampS, 0.85f }, { ampR, 2.0f }, { lfo2Dest, DCutoff }, { lfo2Rate, 0.15f }, { lfo2Amount, 0.25f },
            { chorusMode, ChII }, { delayOn, 1 }, { delayDiv, D4 }, { delayMix, 0.15f }, { revSize, 0.75f }, { revMix, 0.35f } });
        add ("Pads", "Plastic Sheen", {
            { osc1Wave, Square }, { osc1PW, 0.35f }, { osc2Wave, Saw }, { osc2Oct, 1 }, { osc2Detune, 6 }, { osc2Level, 0.5f },
            { cutoff, 2400 }, { resonance, 0.1f }, { envAmount, 0.15f }, { ampA, 0.35f }, { ampD, 1.0f }, { ampS, 0.8f }, { ampR, 1.4f },
            { lfo1Dest, DPW }, { lfo1Rate, 0.4f }, { lfo1Amount, 0.5f }, { chorusMode, ChBoth }, { revSize, 0.6f }, { revMix, 0.3f } });
        add ("Pads", "Downlights", {
            { osc1Wave, Tri }, { osc1Level, 0.6f }, { osc2Wave, Sine }, { osc2Oct, 1 }, { osc2Level, 0.4f }, { osc2Detune, 3 },
            { tapeSound, Choir }, { tapeLevel, 0.45f }, { tapeWow, 0.4f }, { tapeAge, 0.35f }, { cutoff, 1800 }, { envAmount, 0.1f },
            { ampA, 1.5f }, { ampS, 1.0f }, { ampR, 3.0f }, { delayOn, 1 }, { delayDiv, D2 }, { delayMix, 0.2f }, { delayFeedback, 0.5f },
            { delayWow, 0.5f }, { revSize, 0.9f }, { revMix, 0.45f } });
        add ("Pads", "Spacejunk Drift", {
            { osc1Wave, Saw }, { osc2Wave, Saw }, { osc2Detune, -14 }, { osc2Level, 0.9f }, { drift, 0.7f }, { filterType, Scream },
            { cutoff, 700 }, { resonance, 0.35f }, { envAmount, 0.3f }, { fltA, 2.0f }, { fltD, 3.0f }, { fltS, 0.4f }, { fltR, 2.0f },
            { ampA, 1.2f }, { ampS, 0.9f }, { ampR, 2.5f }, { lfo2Shape, LRandom }, { lfo2Dest, DCutoff }, { lfo2Rate, 0.5f }, { lfo2Amount, 0.3f },
            { chorusMode, ChI }, { delayOn, 1 }, { delayDiv, D4D }, { delayFeedback, 0.55f }, { delayMix, 0.25f }, { delayWow, 0.6f },
            { revSize, 0.8f }, { revMix, 0.35f } });
        add ("Pads", "Warm Blanket", {
            { osc1Wave, Saw }, { osc2Wave, Tri }, { osc2Oct, -1 }, { osc2Level, 0.5f }, { subLevel, 0.25f }, { cutoff, 650 },
            { resonance, 0.05f }, { envAmount, 0.15f }, { ampA, 0.6f }, { ampD, 1.0f }, { ampS, 0.9f }, { ampR, 1.5f },
            { chorusMode, ChII }, { satDrive, 0.35f }, { revSize, 0.55f }, { revMix, 0.25f } });
        add ("Pads", "Endling", {
            { osc1Wave, Saw }, { osc1Level, 0.35f }, { osc2Level, 0.0f }, { tapeSound, Strings }, { tapeLevel, 0.6f }, { tapeWow, 0.35f },
            { cutoff, 1100 }, { envAmount, 0.1f }, { ampA, 0.9f }, { ampS, 1.0f }, { ampR, 2.5f },
            { lfo1Dest, DPitch }, { lfo1Rate, 4.8f }, { lfo1Amount, 0.1f }, { lfo1Fade, 1.5f },
            { delayOn, 1 }, { delayDiv, D2 }, { delayMix, 0.12f }, { revSize, 0.85f }, { revMix, 0.4f } });

        // ------------------------------------------------------------------ TAPE
        auto tapeOnly = [] (std::vector<std::pair<const char*, float>> extra)
        {
            std::vector<std::pair<const char*, float>> base { { osc1Level, 0.0f }, { osc2Level, 0.0f }, { tapeLevel, 1.0f },
                                                              { tapeLimit, 1 }, { velAmp, 0.0f } };
            base.insert (base.end(), extra.begin(), extra.end());
            return base;
        };
        add ("Tape", "Tape Strings", tapeOnly ({ { tapeSound, Strings }, { tapeWow, 0.35f }, { tapeAge, 0.3f }, { revSize, 0.6f }, { revMix, 0.2f } }));
        add ("Tape", "Flute Fields", tapeOnly ({ { tapeSound, Flute }, { tapeWow, 0.4f }, { tapeAge, 0.25f }, { revMix, 0.25f } }));
        add ("Tape", "Tape Choir", tapeOnly ({ { tapeSound, Choir }, { tapeWow, 0.3f }, { tapeAge, 0.35f }, { revSize, 0.7f }, { revMix, 0.3f } }));
        add ("Tape", "Tape Trumpet", tapeOnly ({ { tapeSound, Brass }, { tapeWow, 0.3f }, { tapeAge, 0.4f }, { delayOn, 1 }, { delayDiv, D4 },
                                                 { delayMix, 0.12f }, { revMix, 0.2f } }));
        add ("Tape", "Tape Cello", tapeOnly ({ { tapeSound, Cello }, { tapeWow, 0.25f }, { tapeAge, 0.35f }, { revMix, 0.22f } }));
        add ("Tape", "Worn Organ", tapeOnly ({ { tapeSound, Organ }, { tapeWow, 0.45f }, { tapeAge, 0.55f }, { chorusMode, ChI },
                                               { revType, Spring }, { revMix, 0.25f } }));

        // ------------------------------------------------------------------ LEADS
        add ("Leads", "Whistle Cowboy", {
            { osc1Wave, Sine }, { osc2Wave, Tri }, { osc2Oct, 1 }, { osc2Level, 0.2f }, { cutoff, 6000 }, { envAmount, 0.0f },
            { ampA, 0.03f }, { ampS, 1.0f }, { ampR, 0.3f }, { lfo1Dest, DPitch }, { lfo1Rate, 5.5f }, { lfo1Amount, 0.1f }, { lfo1Fade, 0.4f },
            { voiceMode, Mono }, { glide, 0.08f }, { delayOn, 1 }, { delayDiv, D4D }, { delayMix, 0.25f }, { delayFeedback, 0.4f },
            { revType, Spring }, { revMix, 0.2f } });
        add ("Leads", "Phatty Lead", {
            { osc1Wave, Saw }, { osc2Wave, Square }, { osc2Detune, 8 }, { osc2Level, 0.7f }, { cutoff, 1300 }, { resonance, 0.35f },
            { filterDrive, 0.4f }, { envAmount, 0.45f }, { fltD, 0.4f }, { fltS, 0.4f }, { ampS, 0.9f }, { ampR, 0.2f },
            { voiceMode, Legato }, { glide, 0.06f }, { satDrive, 0.35f }, { delayOn, 1 }, { delayDiv, D8D }, { delayMix, 0.18f }, { revMix, 0.15f } });
        add ("Leads", "Scream Lead", {
            { filterType, Scream }, { osc1Wave, Saw }, { osc2Wave, Saw }, { osc2Detune, 5 }, { osc2Level, 0.6f }, { cutoff, 1800 },
            { resonance, 0.7f }, { filterDrive, 0.6f }, { envAmount, 0.3f }, { ampS, 0.9f }, { ampR, 0.2f },
            { lfo1Dest, DPitch }, { lfo1Rate, 5.8f }, { lfo1Amount, 0.1f }, { lfo1Fade, 0.6f }, { voiceMode, Legato }, { glide, 0.1f },
            { satType, SatTube }, { satDrive, 0.5f }, { delayOn, 1 }, { delayDiv, D4 }, { delayMix, 0.2f }, { revMix, 0.15f } });
        add ("Leads", "Velvet Brass", {
            { osc1Wave, Saw }, { osc2Wave, Saw }, { osc2Detune, 10 }, { osc2Level, 0.9f }, { cutoff, 700 }, { resonance, 0.1f },
            { envAmount, 0.55f }, { fltA, 0.06f }, { fltD, 0.5f }, { fltS, 0.45f }, { ampA, 0.03f }, { ampD, 0.5f }, { ampS, 0.8f },
            { ampR, 0.25f }, { velFilter, 0.5f }, { chorusMode, ChI }, { revMix, 0.2f } });
        add ("Leads", "Melodica Mood", {
            { osc1Wave, Square }, { osc1PW, 0.45f }, { osc2Wave, Saw }, { osc2Level, 0.2f }, { noiseLevel, 0.06f }, { cutoff, 2800 },
            { resonance, 0.05f }, { envAmount, 0.1f }, { ampA, 0.05f }, { ampS, 0.9f }, { ampR, 0.15f },
            { lfo1Dest, DVolume }, { lfo1Rate, 5.0f }, { lfo1Amount, 0.08f }, { voiceMode, Mono }, { revType, Spring }, { revMix, 0.25f } });
        add ("Leads", "Shortwave Lead", {
            { osc1Wave, Square }, { osc2Wave, Saw }, { osc2Oct, 1 }, { osc2Level, 0.3f }, { cutoff, 2000 }, { envAmount, 0.2f },
            { ampS, 0.9f }, { ampR, 0.2f }, { voiceMode, Mono }, { glide, 0.05f }, { radioOn, 1 }, { radioAmount, 0.7f }, { radioTune, 0.4f },
            { delayOn, 1 }, { delayDiv, D8D }, { delayMix, 0.3f }, { delayFeedback, 0.5f }, { revMix, 0.3f } });

        // ------------------------------------------------------------------ PLUCKS
        add ("Plucks", "Pocket Piano Pluck", {
            { osc1Wave, Square }, { osc1PW, 0.25f }, { osc1Level, 1.0f }, { osc2Wave, Square }, { osc2Oct, 1 }, { osc2Level, 0.45f }, { osc2Detune, 4 }, { cutoff, 2200 }, { envAmount, 0.5f }, { fltD, 0.2f },
            { fltS, 0.0f }, { ampA, 0.001f }, { ampD, 0.5f }, { ampS, 0.0f }, { ampR, 0.3f }, { delayOn, 1 }, { delayDiv, D8D },
            { delayMix, 0.25f }, { revType, Spring }, { revMix, 0.2f } });
        add ("Plucks", "Glass Drops", {
            { osc1Wave, Sine }, { osc2Wave, Sine }, { osc2Oct, 2 }, { osc2Semi, 7 }, { osc2Detune, 0 }, { osc2Level, 0.35f },
            { cutoff, 8000 }, { envAmount, 0.0f }, { ampA, 0.001f }, { ampD, 0.8f }, { ampS, 0.0f }, { ampR, 0.8f }, { velAmp, 0.7f },
            { delayOn, 1 }, { delayDiv, D4D }, { delayMix, 0.3f }, { delayFeedback, 0.45f }, { delayWow, 0.4f },
            { revSize, 0.6f }, { revMix, 0.3f } });
        add ("Plucks", "Snap Synth", {
            { osc1Wave, Saw }, { osc2Wave, Square }, { osc2Level, 0.5f }, { cutoff, 500 }, { resonance, 0.3f }, { envAmount, 0.7f },
            { fltD, 0.14f }, { fltS, 0.0f }, { ampA, 0.001f }, { ampD, 0.3f }, { ampS, 0.0f }, { ampR, 0.2f }, { velFilter, 0.7f },
            { chorusMode, ChI }, { delayOn, 1 }, { delayDiv, D16 }, { delayMix, 0.15f }, { revMix, 0.15f } });
        add ("Plucks", "Kalimba Tape", {
            { osc1Wave, Tri }, { osc1Level, 1.0f }, { osc2Wave, Sine }, { osc2Oct, 2 }, { osc2Level, 0.5f }, { cutoff, 3000 }, { envAmount, 0.1f },
            { ampA, 0.001f }, { ampD, 0.6f }, { ampS, 0.0f }, { ampR, 0.5f }, { satDrive, 0.5f }, { delayOn, 1 }, { delayDiv, D4 },
            { delayMix, 0.2f }, { delayWow, 0.7f }, { delayTone, 0.3f }, { revType, Spring }, { revMix, 0.2f } });
        add ("Plucks", "Muted Funk", {
            { filterType, Scream }, { osc1Wave, Saw }, { osc2Level, 0.3f }, { cutoff, 600 }, { resonance, 0.6f }, { filterDrive, 0.5f },
            { envAmount, 0.6f }, { fltD, 0.12f }, { fltS, 0.0f }, { ampA, 0.001f }, { ampD, 0.25f }, { ampS, 0.0f }, { ampR, 0.1f },
            { velFilter, 0.5f }, { revMix, 0.1f } });
        add ("Plucks", "Bubble Pluck", {
            { osc1Wave, Sine }, { osc1Oct, 1 }, { osc2Level, 0.0f }, { cutoff, 900 }, { resonance, 0.7f }, { envAmount, 0.5f },
            { fltD, 0.1f }, { fltS, 0.0f }, { ampA, 0.001f }, { ampD, 0.25f }, { ampS, 0.0f }, { ampR, 0.2f },
            { delayOn, 1 }, { delayDiv, D8 }, { delayMix, 0.3f }, { delayFeedback, 0.5f }, { revMix, 0.3f } });

        // ------------------------------------------------------------------ STRUM
        add ("Strum", "Harp Strum", {
            { strumOn, 1 }, { strumChord, Major }, { strumSpeed, 30 }, { strumSpan, 6 }, { osc1Wave, Tri }, { osc2Wave, Sine },
            { osc2Oct, 1 }, { osc2Level, 0.3f }, { cutoff, 4500 }, { envAmount, 0.1f }, { ampA, 0.001f }, { ampD, 1.6f }, { ampS, 0.0f },
            { ampR, 1.2f }, { chorusMode, ChI }, { revSize, 0.6f }, { revMix, 0.3f } });
        add ("Strum", "Minor Glass Strum", {
            { strumOn, 1 }, { strumChord, Minor }, { strumSpeed, 45 }, { strumSpan, 7 }, { osc1Wave, Sine }, { osc2Wave, Sine },
            { osc2Oct, 2 }, { osc2Level, 0.25f }, { cutoff, 8000 }, { envAmount, 0.0f }, { ampA, 0.001f }, { ampD, 2.0f }, { ampS, 0.0f },
            { ampR, 1.5f }, { delayOn, 1 }, { delayDiv, D4D }, { delayMix, 0.25f }, { revSize, 0.7f }, { revMix, 0.35f } });
        add ("Strum", "Seventh Heaven", {
            { strumOn, 1 }, { strumChord, Seventh }, { strumSpeed, 25 }, { strumSpan, 8 }, { osc1Wave, Square }, { osc1PW, 0.3f },
            { osc2Level, 0.0f }, { cutoff, 2200 }, { envAmount, 0.35f }, { fltD, 0.4f }, { fltS, 0.1f }, { ampA, 0.001f }, { ampD, 1.0f },
            { ampS, 0.0f }, { ampR, 0.8f }, { chorusMode, ChII }, { revType, Spring }, { revMix, 0.25f } });
        add ("Strum", "Tape Strum Strings", {
            { strumOn, 1 }, { strumChord, Maj7 }, { strumSpeed, 60 }, { strumSpan, 5 }, { osc1Level, 0.0f }, { osc2Level, 0.0f },
            { tapeSound, Strings }, { tapeLevel, 0.8f }, { tapeWow, 0.4f }, { velAmp, 0.0f }, { revSize, 0.6f }, { revMix, 0.3f } });
        add ("Strum", "Lo-Fi Autoharp", {
            { strumOn, 1 }, { strumChord, Major }, { strumSpeed, 18 }, { strumSpan, 8 }, { osc1Wave, Saw }, { osc2Level, 0.0f },
            { cutoff, 1600 }, { envAmount, 0.4f }, { fltD, 0.3f }, { fltS, 0.0f }, { ampA, 0.001f }, { ampD, 1.2f }, { ampS, 0.0f },
            { ampR, 0.6f }, { radioOn, 1 }, { radioAmount, 0.4f }, { radioTune, 0.2f }, { satDrive, 0.5f }, { revMix, 0.2f } });
        add ("Strum", "Sus Shimmer", {
            { strumOn, 1 }, { strumChord, Sus4 }, { strumSpeed, 80 }, { strumSpan, 6 }, { osc1Wave, Tri }, { osc2Wave, Saw },
            { osc2Oct, 1 }, { osc2Level, 0.2f }, { cutoff, 3500 }, { envAmount, 0.1f }, { ampA, 0.01f }, { ampD, 2.5f }, { ampS, 0.0f },
            { ampR, 2.0f }, { delayOn, 1 }, { delayDiv, D2 }, { delayMix, 0.3f }, { delayFeedback, 0.6f }, { revSize, 0.9f }, { revMix, 0.45f } });

        // ------------------------------------------------------------------ TEXTURES
        add ("Textures", "Shortwave Ghost", {
            { osc1Wave, Sine }, { osc1Level, 0.3f }, { osc2Level, 0.0f }, { noiseLevel, 0.6f }, { cutoff, 1200 }, { resonance, 0.8f },
            { keyTrack, 1.0f }, { envAmount, 0.0f }, { ampA, 1.5f }, { ampS, 1.0f }, { ampR, 2.0f }, { radioOn, 1 }, { radioAmount, 0.9f },
            { radioTune, 0.8f }, { delayOn, 1 }, { delayDiv, D2 }, { delayFeedback, 0.7f }, { delayMix, 0.3f }, { delayWow, 0.8f },
            { revSize, 0.9f }, { revMix, 0.5f } });
        add ("Textures", "Ghost Choir Tape", {
            { osc1Wave, Tri }, { osc1Level, 0.3f }, { osc2Level, 0.0f }, { tapeSound, Choir }, { tapeLevel, 0.7f }, { tapeWow, 0.8f },
            { tapeAge, 0.8f }, { cutoff, 1500 }, { ampA, 0.8f }, { ampS, 1.0f }, { ampR, 2.0f }, { delayOn, 1 }, { delayDiv, D4D },
            { delayFeedback, 0.6f }, { delayMix, 0.3f }, { delayWow, 0.6f }, { revSize, 0.8f }, { revMix, 0.45f } });
        add ("Textures", "Echo Rain", {
            { osc1Wave, Sine }, { osc1Level, 0.9f }, { osc2Level, 0.0f }, { noiseLevel, 0.35f }, { filterType, Scream }, { cutoff, 2500 },
            { resonance, 0.5f }, { envAmount, 0.0f }, { ampA, 0.001f }, { ampD, 0.08f }, { ampS, 0.0f }, { ampR, 0.1f },
            { lfo2Shape, LRandom }, { lfo2Dest, DCutoff }, { lfo2Rate, 6.0f }, { lfo2Amount, 0.5f },
            { delayOn, 1 }, { delayHeads, 2 }, { delayDiv, D8 }, { delayFeedback, 0.85f }, { delayMix, 0.5f }, { delayWow, 0.5f },
            { revType, Spring }, { revMix, 0.3f } });
        add ("Textures", "Undersea Drone", {
            { osc1Wave, Saw }, { osc1Oct, -1 }, { osc2Wave, Saw }, { osc2Oct, -1 }, { osc2Detune, -20 }, { subLevel, 0.5f },
            { cutoff, 300 }, { resonance, 0.45f }, { envAmount, 0.0f }, { lfo1Dest, DCutoff }, { lfo1Rate, 0.12f }, { lfo1Amount, 0.4f },
            { ampA, 2.0f }, { ampS, 1.0f }, { ampR, 4.0f }, { chorusMode, ChII }, { revSize, 0.95f }, { revMix, 0.5f }, { lowCut, 20 } });
        add ("Textures", "Hypnogogue", {
            { osc1Wave, Square }, { osc2Wave, Saw }, { osc2Oct, 1 }, { osc2Detune, 15 }, { osc2Level, 0.5f }, { cutoff, 1000 },
            { resonance, 0.3f }, { envAmount, 0.1f }, { lfo1Dest, DPW }, { lfo1Rate, 0.2f }, { lfo1Amount, 0.6f },
            { lfo2Shape, LTri }, { lfo2Dest, DCutoff }, { lfo2Rate, 0.07f }, { lfo2Amount, 0.5f },
            { ampA, 2.5f }, { ampS, 1.0f }, { ampR, 4.0f }, { chorusMode, ChI }, { delayOn, 1 }, { delayDiv, D2 },
            { delayFeedback, 0.6f }, { delayMix, 0.3f }, { revSize, 0.85f }, { revMix, 0.45f } });
        add ("Textures", "Goodtimes 4 Free", {
            { osc1Wave, Square }, { osc1PW, 0.3f }, { osc1Oct, 1 }, { osc2Level, 0.0f }, { cutoff, 2500 }, { envAmount, 0.3f },
            { fltD, 0.3f }, { fltS, 0.1f }, { ampA, 0.002f }, { ampD, 0.4f }, { ampS, 0.2f }, { ampR, 0.4f },
            { lfo1Shape, LRandom }, { lfo1Dest, DPitch }, { lfo1Rate, 8.0f }, { lfo1Amount, 0.07f },
            { satDrive, 0.4f }, { delayOn, 1 }, { delayDiv, D8D }, { delayMix, 0.3f }, { revType, Spring }, { revMix, 0.3f },
            { radioOn, 1 }, { radioAmount, 0.25f }, { radioTune, 0.15f } });

        // ------------------------------------------------------------------ VOCODER (needs audio on the sidechain)
        auto voc = [] (std::vector<std::pair<const char*, float>> extra)
        {
            std::vector<std::pair<const char*, float>> base { { vocOn, 1 }, { vocSibilance, 0.45f }, { vocMix, 1.0f },
                                                              { velAmp, 0.2f }, { envAmount, 0.0f }, { ampA, 0.01f },
                                                              { ampS, 1.0f }, { ampR, 0.3f } };
            base.insert (base.end(), extra.begin(), extra.end());
            return base;
        };
        add ("Vocoder", "Robot Choir", voc ({
            { osc1Wave, Saw }, { osc2Wave, Saw }, { osc2Detune, 8 }, { osc2Level, 0.8f }, { subLevel, 0.2f }, { cutoff, 6000 },
            { resonance, 0.1f }, { vocBands, 1 }, { vocCarrier, 1 }, { chorusMode, ChII }, { revMix, 0.25f } }));
        add ("Vocoder", "Plastic Voice", voc ({
            { osc1Wave, Square }, { osc1PW, 0.4f }, { osc2Wave, Saw }, { osc2Oct, 1 }, { osc2Level, 0.4f }, { cutoff, 5000 },
            { vocBands, 2 }, { vocFormant, 2 }, { vocResponse, 12 }, { vocCarrier, 1 }, { chorusMode, ChI },
            { delayOn, 1 }, { delayDiv, D8D }, { delayMix, 0.15f }, { revMix, 0.2f } }));
        add ("Vocoder", "Tape Talk", voc ({
            { osc1Wave, Saw }, { osc1Level, 0.5f }, { osc2Level, 0.0f }, { tapeSound, Strings }, { tapeLevel, 0.7f }, { tapeWow, 0.4f },
            { cutoff, 5000 }, { vocBands, 1 }, { vocFormant, -2 }, { vocCarrier, 1 }, { revSize, 0.6f }, { revMix, 0.3f } }));
        add ("Vocoder", "Shortwave Speaker", voc ({
            { osc1Wave, Saw }, { osc2Wave, Square }, { osc2Level, 0.5f }, { cutoff, 4000 }, { vocBands, 0 }, { vocResponse, 25 },
            { vocCarrier, 0 }, { radioOn, 1 }, { radioAmount, 0.6f }, { radioTune, 0.3f }, { revType, Spring }, { revMix, 0.2f } }));
        add ("Vocoder", "Deep Machine", voc ({
            { filterType, Scream }, { osc1Wave, Saw }, { osc1Oct, -1 }, { osc2Wave, Saw }, { osc2Detune, 6 }, { osc2Level, 0.7f },
            { cutoff, 3000 }, { resonance, 0.3f }, { vocBands, 3 }, { vocFormant, -5 }, { vocResponse, 10 }, { vocCarrier, 1 },
            { satType, SatTube }, { satDrive, 0.5f }, { revMix, 0.15f } }));
        add ("Vocoder", "Strum Speech", voc ({
            { strumOn, 1 }, { strumChord, Maj7 }, { strumSpeed, 40 }, { strumSpan, 6 }, { osc1Wave, Saw }, { osc2Wave, Square },
            { osc2Level, 0.5f }, { cutoff, 5000 }, { ampD, 2.5f }, { ampS, 0.3f }, { ampR, 1.0f }, { vocBands, 1 }, { vocCarrier, 1 },
            { delayOn, 1 }, { delayDiv, D4D }, { delayMix, 0.2f }, { revSize, 0.7f }, { revMix, 0.3f } }));

        // loudness-match: every factory preset gets a measured master trim
        for (auto& p : v)
        {
            bool hasMaster = false;
            for (auto& kv : p.values) if (juce::String (kv.first) == trim) hasMaster = true;
            if (hasMaster) continue;
            for (const auto& t : kTrims)
                if (p.name == t.first) { p.values.push_back ({ trim, t.second }); break; }
        }
        return v;
    }
}

const juce::StringArray& Presets::categories()
{
    static const juce::StringArray c { "Bass", "Keys", "Pads", "Tape", "Leads", "Plucks", "Strum", "Textures", "Vocoder" };
    return c;
}

const std::vector<FactoryPreset>& Presets::factory()
{
    static const std::vector<FactoryPreset> v = build();
    return v;
}

void Presets::apply (juce::AudioProcessorValueTreeState& state, const std::vector<std::pair<juce::String, float>>& values)
{
    for (auto* p : state.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            rp->setValueNotifyingHost (rp->getDefaultValue());

    for (const auto& [id, value] : values)
        if (auto* rp = state.getParameter (id))
            rp->setValueNotifyingHost (rp->convertTo0to1 (value));
}

void Presets::applyFactory (juce::AudioProcessorValueTreeState& state, int index)
{
    const auto& f = factory();
    if (! juce::isPositiveAndBelow (index, (int) f.size())) return;
    std::vector<std::pair<juce::String, float>> vals;
    for (const auto& [id, value] : f[(size_t) index].values) vals.push_back ({ id, value });
    Presets::apply (state, vals);
}

juce::File Presets::userFolder()
{
    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
                   .getChildFile ("Switchkicker").getChildFile ("bndt").getChildFile ("Presets");
    dir.createDirectory();
    return dir;
}

bool Presets::saveUser (juce::AudioProcessorValueTreeState& state, const juce::String& name, const juce::String& category)
{
    juce::XmlElement xml ("bndtPreset");
    xml.setAttribute ("name", name);
    xml.setAttribute ("category", category);
    for (auto* p : state.processor.getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            auto* e = xml.createNewChildElement ("param");
            e->setAttribute ("id", rp->getParameterID());
            e->setAttribute ("value", (double) rp->convertFrom0to1 (rp->getValue()));
        }
    const auto file = userFolder().getChildFile (juce::File::createLegalFileName (name) + ".bndt");
    return xml.writeTo (file);
}

bool Presets::loadUser (juce::AudioProcessorValueTreeState& state, const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName ("bndtPreset")) return false;
    std::vector<std::pair<juce::String, float>> vals;
    for (auto* e : xml->getChildWithTagNameIterator ("param"))
        vals.push_back ({ e->getStringAttribute ("id"), (float) e->getDoubleAttribute ("value") });
    Presets::apply (state, vals);
    return true;
}

juce::Array<juce::File> Presets::userPresets()
{
    auto files = userFolder().findChildFiles (juce::File::findFiles, false, "*.bndt");
    files.sort();
    return files;
}

void Presets::randomise (juce::AudioProcessorValueTreeState& state, juce::Random& r)
{
    auto pick = [&] (std::initializer_list<float> opts) { return *(opts.begin() + r.nextInt ((int) opts.size())); };
    auto range = [&] (float lo, float hi) { return lo + r.nextFloat() * (hi - lo); };
    auto logRange = [&] (float lo, float hi) { return lo * std::pow (hi / lo, r.nextFloat()); };
    auto chance = [&] (float p) { return r.nextFloat() < p; };

    std::vector<std::pair<juce::String, float>> v;
    auto set = [&] (const char* id, float value) { v.push_back ({ id, value }); };

    const int style = r.nextInt (4);   // 0 pluck, 1 keys, 2 pad, 3 lead/bass
    set (osc1Wave, (float) r.nextInt (4));
    set (osc1PW, range (0.2f, 0.6f));
    set (osc1Oct, style == 3 ? pick ({ -1.0f, 0.0f }) : pick ({ 0.0f, 0.0f, 1.0f, -1.0f }));
    set (osc2Wave, (float) r.nextInt (4));
    set (osc2Oct, pick ({ 0.0f, 0.0f, 1.0f, -1.0f }));
    set (osc2Semi, pick ({ 0.0f, 0.0f, 0.0f, 7.0f, 12.0f, 5.0f }));
    set (osc2Detune, range (-15.0f, 15.0f));
    set (osc2Level, range (0.0f, 0.8f));
    set (subLevel, chance (0.3f) ? range (0.1f, 0.5f) : 0.0f);
    set (noiseLevel, chance (0.15f) ? range (0.02f, 0.2f) : 0.0f);
    set (drift, range (0.1f, 0.6f));
    set (filterType, chance (0.35f) ? 1.0f : 0.0f);
    set (cutoff, logRange (style == 3 ? 250.0f : 500.0f, 5000.0f));
    set (resonance, range (0.0f, 0.6f));
    set (filterDrive, range (0.0f, 0.5f));
    set (envAmount, range (0.0f, 0.7f));
    set (fltA, style == 2 ? range (0.2f, 2.0f) : logRange (0.001f, 0.05f));
    set (fltD, logRange (0.1f, 2.0f));
    set (fltS, range (0.0f, 0.6f));

    switch (style)
    {
        case 0: set (ampA, 0.001f); set (ampD, range (0.2f, 1.5f)); set (ampS, 0.0f); set (ampR, range (0.2f, 1.0f)); break;
        case 1: set (ampA, logRange (0.002f, 0.02f)); set (ampD, range (0.5f, 2.0f)); set (ampS, range (0.3f, 0.8f)); set (ampR, range (0.2f, 0.8f)); break;
        case 2: set (ampA, range (0.3f, 1.5f)); set (ampD, 1.0f); set (ampS, range (0.7f, 1.0f)); set (ampR, range (1.0f, 3.0f)); break;
        default: set (ampA, 0.003f); set (ampD, range (0.3f, 1.0f)); set (ampS, range (0.5f, 1.0f)); set (ampR, range (0.08f, 0.3f));
                 set (voiceMode, pick ({ 1.0f, 2.0f })); set (glide, chance (0.5f) ? range (0.02f, 0.12f) : 0.0f); break;
    }

    if (chance (0.3f)) { set (tapeSound, (float) r.nextInt (6)); set (tapeLevel, range (0.3f, 0.7f)); set (tapeWow, range (0.2f, 0.7f)); set (tapeAge, range (0.1f, 0.6f)); }
    if (chance (0.4f)) { set (lfo1Dest, pick ({ 0.0f, 2.0f, 5.0f })); set (lfo1Rate, range (0.2f, 6.0f)); set (lfo1Amount, range (0.05f, 0.4f)); set (lfo1Fade, range (0.0f, 1.0f)); }
    if (chance (0.4f)) { set (lfo2Shape, (float) r.nextInt (4)); set (lfo2Dest, 1.0f); set (lfo2Rate, logRange (0.05f, 3.0f)); set (lfo2Amount, range (0.1f, 0.4f)); }
    if (style != 3 && chance (0.15f)) { set (strumOn, 1.0f); set (strumChord, (float) r.nextInt (6)); set (strumSpeed, range (15.0f, 80.0f)); set (strumSpan, (float) (4 + r.nextInt (5))); }

    set (satDrive, range (0.1f, 0.5f));
    set (satType, (float) r.nextInt (2));
    set (chorusMode, (float) r.nextInt (4));
    if (chance (0.55f)) { set (delayOn, 1.0f); set (delayDiv, (float) r.nextInt (8)); set (delayFeedback, range (0.2f, 0.6f)); set (delayMix, range (0.1f, 0.3f)); set (delayWow, range (0.1f, 0.6f)); set (delayHeads, (float) r.nextInt (3)); }
    set (revType, chance (0.35f) ? 0.0f : 1.0f);
    set (revSize, range (0.3f, 0.9f));
    set (revMix, range (0.1f, 0.4f));
    set (trim, style == 2 ? -9.0f : (style == 3 ? 2.0f : -4.0f));
    if (chance (0.15f)) { set (radioOn, 1.0f); set (radioAmount, range (0.2f, 0.7f)); set (radioTune, range (0.0f, 0.5f)); }
    Presets::apply (state, v);
}
