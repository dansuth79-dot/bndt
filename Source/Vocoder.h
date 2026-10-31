#pragma once
#include "Common.h"
#include "DSP.h"
#include <array>
#include <atomic>

struct VocoderParams
{
    bool on = false;
    int bands = 16;            // 8, 16, 24 or 32
    bool noiseCarrier = false; // "Synth + Noise"
    float inputDb = 0.0f;      // source gain
    float formant = 0.0f;      // semitones the carrier bands are shifted by
    float responseMs = 15.0f;  // envelope release
    float sibilance = 0.4f;
    float mix = 1.0f;
};

// Channel vocoder: the sidechain input (source / modulator) shapes the synth (carrier).
class Vocoder
{
public:
    void prepare (float sampleRate);
    void reset();
    // l/r: carrier in, vocoded out. srcL/srcR: source (modulator) signal.
    void process (float* l, float* r, const float* srcL, const float* srcR, int n, const VocoderParams& p);
    float getInputLevel() const { return inputLevel.load(); }

private:
    void setup (int bands, float formant);

    static constexpr int kMax = 32;
    struct Band
    {
        dsp::SVF m1, m2, cl1, cl2, cr1, cr2;
        float envM = 0.0f, envL = 0.0f, envR = 0.0f;
        float gL = 0.0f, gR = 0.0f, gLt = 0.0f, gRt = 0.0f;
    };
    std::array<Band, kMax> band;
    int numBands = 0;
    float sr = 44100.0f, curFormant = 1000.0f;
    dsp::SVF sibHP1, sibHP2, noiseHP1, noiseHP2;
    float sibEnv = 0.0f, dryEnv = 0.0f, mixS = 0.0f, inGain = 1.0f;
    int ctrl = 0;
    dsp::Rng rng;
    std::atomic<float> inputLevel { 0.0f };
};
