#pragma once
#include "Common.h"
#include "DSP.h"
#include <array>
#include <vector>

// Snapshot of every voice-level parameter, read once per block.
struct VoiceParams
{
    int osc1Wave = 0, osc1Oct = 0, osc2Wave = 0, osc2Oct = 0, osc2Semi = 0;
    float osc1PW = 0.5f, osc1Level = 0.8f, osc2Detune = 7.0f, osc2Level = 0.5f, subLevel = 0.0f, noiseLevel = 0.0f, drift = 0.25f;
    int tapeSound = 0; float tapeLevel = 0.0f, tapeWow = 0.3f, tapeAge = 0.3f; bool tapeLimit = false;
    int filterType = 0; float cutoff = 3000.0f, resonance = 0.2f, filterDrive = 0.2f, envAmount = 0.3f, keyTrack = 0.5f, velFilter = 0.3f;
    float ampA = 0.005f, ampD = 0.4f, ampS = 0.8f, ampR = 0.3f, velAmp = 0.5f;
    float fltA = 0.005f, fltD = 0.5f, fltS = 0.3f, fltR = 0.4f;
    int lfoShape[2] { 0, 1 }, lfoDest[2] { 0, 1 };
    float lfoRate[2] { 5.0f, 0.3f }, lfoAmount[2] { 0.0f, 0.0f }, lfoFade[2] { 0.0f, 0.0f };
    int voiceMode = 0; float glide = 0.0f;
    bool strumOn = false; int strumChord = 0; float strumSpeed = 35.0f; int strumSpan = 5;
};

// Modelled tape-replay instrument (strings, cello, flute, choir, brass, organ).
struct TapeVoice
{
    void prepare (float sampleRate, uint32_t seed);
    void start (int sound, float midiNote, float velocity);
    void release();
    void kill() { stage = 0; level = 0.0f; }
    bool active() const { return stage != 0; }
    float process (float pitchOffsetSemis, float age, bool limit);

    float sr = 44100.0f;
    int sound = 0, stage = 0;   // 0 idle, 1 playing, 2 releasing
    float note = 60.0f, vel = 1.0f, level = 0.0f, attackC = 0.0f, releaseC = 0.0f, t = 0.0f;
    std::array<dsp::Osc, 6> osc;
    std::array<float, 6> detune {}, vibPhase {};
    std::array<dsp::SVF, 3> formant;
    std::array<float, 3> formantGain {};
    dsp::SVF tone, breath, brassLP;
    dsp::OnePole ageLP, dcBlock;
    dsp::Rng rng;
    float clickEnv = 0.0f;
    int ctrlCounter = 0;
};

struct Voice
{
    void prepare (float sampleRate, uint32_t seed);
    void start (int note, int key, float velocity, bool retrigger, bool isStrum, float fromPitch);
    void release();
    void kill();
    bool active() const { return amp.active() || tape.active(); }
    bool releasing() const { return amp.stage == dsp::Env::Release || ! held; }

    float sr = 44100.0f;
    int note = -1, key = -1; bool held = false, sustained = false, strum = false;
    float velocity = 1.0f, pitch = 60.0f, targetPitch = 60.0f, timeOn = 0.0f;
    uint64_t age = 0;

    dsp::Osc o1, o2, sub;
    dsp::Ladder ladder;
    dsp::Scream scream;
    dsp::Env amp, flt;
    TapeVoice tape;
    dsp::Rng rng;
    float driftWalk[3] {}, driftStatic[3] {}, pan = 0.0f;
};

class SynthEngine
{
public:
    static constexpr int kVoices = 16;
    static constexpr int kCtrl = 16;   // control-rate block size in samples

    void prepare (double sampleRate, int maxBlock);
    void reset();
    void setParams (const VoiceParams& p) { params = p; }
    void handleMidi (const juce::MidiMessage& m);
    void render (float* left, float* right, int numSamples);
    void allNotesOff();

private:
    void noteOn (int note, float vel);
    void noteOff (int note);
    void startVoice (int note, int key, float vel, bool isStrum);
    Voice* findVoice (int key, bool isStrum);
    float lfoValue (int i) const { return lfoOut[i]; }
    void advanceLfos (int samples);

    VoiceParams params;
    std::array<Voice, kVoices> voices;
    float sr = 44100.0f;
    uint64_t counter = 0;
    float lastPitch = 60.0f;
    float bend = 0.0f, bendTarget = 0.0f, modWheel = 0.0f;
    bool sustainPedal = false;

    // mono note stack
    struct Held { int note; float vel; };
    std::vector<Held> stack;

    // strum scheduler
    struct Pending { int samplesLeft; int note; int key; float vel; };
    std::vector<Pending> pending;

    float lfoPhase[2] {}, lfoOut[2] {}, lfoHeld[2] {}, lfoSmooth[2] {};
    float vibPhase = 0.0f, wowPhase = 0.0f, flutterPhase = 0.0f, wowDrift = 0.0f;
    dsp::Rng rng;
};
