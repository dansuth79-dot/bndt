#pragma once
#include "Common.h"
#include "DSP.h"
#include "Vocoder.h"
#include <array>

struct FXParams
{
    bool satOn = true; int satType = 0; float satDrive = 0.25f, satMix = 1.0f;
    int chorusMode = 0;
    bool delayOn = false, delaySync = true; float delayTimeMs = 375.0f; int delayDiv = 3, delayHeads = 0;
    float delayFeedback = 0.4f, delayWow = 0.3f, delayTone = 0.45f, delayMix = 0.25f;
    bool revOn = true; int revType = 1; float revSize = 0.5f, revTone = 0.5f, revMix = 0.2f;
    bool radioOn = false; float radioAmount = 0.6f, radioTune = 0.3f;
    float lowCut = 30.0f, masterDb = 0.0f, trimDb = 0.0f;
    double bpm = 120.0;
    VocoderParams voc;
};

class Saturator
{
public:
    void prepare (float sr);
    void process (float* l, float* r, int n, const FXParams& p);
private:
    float sr = 44100.0f;
    dsp::OnePole toneL, toneR;
    juce::SmoothedValue<float> mix, drive;
};

// Two-mode bucket-brigade style chorus (I, II, I+II).
class Chorus
{
public:
    void prepare (float sr);
    void process (float* l, float* r, int n, int mode);
private:
    float sr = 44100.0f, phase = 0.0f, wet = 0.0f;
    dsp::DelayLine dl;
    dsp::OnePole lpL, lpR;
};

// Multi-head tape echo with wow, flutter and a saturating feedback loop.
class TapeEcho
{
public:
    void prepare (float sr);
    void reset();
    void process (float* l, float* r, int n, const FXParams& p);
private:
    float sr = 44100.0f;
    dsp::DelayLine line;
    dsp::OnePole fbLP, fbHP, wetLP;
    float timeSmoothed = 0.0f, wowPhase = 0.0f, flutPhase = 0.0f, drift = 0.0f, mixS = 0.0f, fbS = 0.0f;
    dsp::Rng rng;
};

// Dispersive spring tank (two springs for stereo).
class SpringReverb
{
public:
    void prepare (float sr);
    void reset();
    void process (const float* inL, const float* inR, float* outL, float* outR, int n, float size, float tone);
private:
    struct Spring
    {
        dsp::DelayLine line;
        std::array<float, 16> apX {}, apY {};
        dsp::OnePole lp, hp;
        float fb = 0.0f, length = 0.0f;
        float modPhase = 0.0f;
    };
    Spring springs[2];
    float sr = 44100.0f;
};

// 8-line feedback delay network hall.
class HallReverb
{
public:
    void prepare (float sr);
    void reset();
    void process (const float* inL, const float* inR, float* outL, float* outR, int n, float size, float tone);
private:
    static constexpr int N = 8;
    dsp::DelayLine lines[N];
    dsp::DelayLine pre;
    dsp::OnePole damp[N];
    struct AP { dsp::DelayLine d; int len = 0; } diff[4];
    float baseLen[N] {};
    float sr = 44100.0f, modPhase = 0.0f;
};

// Bandwidth-limited "shortwave" colour: band-pass, crackle, fading and a tuning whistle.
class Radio
{
public:
    void prepare (float sr);
    void process (float* l, float* r, int n, const FXParams& p);
private:
    float sr = 44100.0f;
    dsp::SVF hpL, hpR, lpL, lpR, crackleF;
    float whistlePhase = 0.0f, whistleDrift = 0.0f, fade = 1.0f, fadeTarget = 1.0f, mixS = 0.0f;
    dsp::Rng rng;
    int fadeCounter = 0;
};

class OutputStage
{
public:
    void prepare (float sr);
    void process (float* l, float* r, int n, const FXParams& p);
private:
    float sr = 44100.0f, env = 0.0f;
    dsp::SVF hpL, hpR;
    juce::SmoothedValue<float> gain;
    float lastCut = -1.0f;
};

class FXChain
{
public:
    void prepare (double sr, int maxBlock);
    void reset();
    void process (juce::AudioBuffer<float>& buffer, int numSamples, const FXParams& p, const float* srcL, const float* srcR);
    float getVocoderInputLevel() const { return vocoder.getInputLevel(); }
private:
    Vocoder vocoder;
    Saturator sat; Chorus chorus; TapeEcho echo; SpringReverb spring; HallReverb hall; Radio radio; OutputStage out;
    juce::AudioBuffer<float> revBuf;
    float revMixS = 0.0f;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> trimGain { 1.0f };
};
