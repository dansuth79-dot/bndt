#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <memory>

// Small, allocation-free building blocks shared by the voice and the FX.
namespace dsp
{
    constexpr float kPi = 3.14159265358979f;
    constexpr float kTwoPi = 6.28318530717959f;

    inline float mtof (float midi) { return 440.0f * std::exp2 ((midi - 69.0f) / 12.0f); }
    inline float clampf (float x, float lo, float hi) { return std::min (hi, std::max (lo, x)); }

    // Rational tanh approximation: smooth, cheap, bounded to +-1.
    inline float fastTanh (float x)
    {
        x = clampf (x, -4.5f, 4.5f);
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    struct Rng
    {
        uint32_t s = 0x9E3779B9u;
        void seed (uint32_t v) { s = v ? v : 0x1234567u; }
        inline uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
        inline float bipolar() { return (float) (next() & 0xFFFFFF) / 8388608.0f - 1.0f; }
        inline float unipolar() { return (float) (next() & 0xFFFFFF) / 16777216.0f; }
    };

    inline float polyBlep (float t, float dt)
    {
        if (t < dt) { t /= dt; return t + t - t * t - 1.0f; }
        if (t > 1.0f - dt) { t = (t - 1.0f) / dt; return t * t + t + t + 1.0f; }
        return 0.0f;
    }

    // Band-limited (polyBLEP) oscillator: saw, pulse, triangle, sine.
    struct Osc
    {
        float phase = 0.0f, inc = 0.0f, tri = 0.0f;
        void setFreq (float hz, float sr) { inc = clampf (hz / sr, 0.0f, 0.45f); }
        void reset (float p) { phase = p; tri = 0.0f; }

        inline float process (int wave, float pw)
        {
            float out = 0.0f;
            switch (wave)
            {
                case 0: out = 2.0f * phase - 1.0f - polyBlep (phase, inc); break;
                case 1:
                case 2:
                {
                    float sq = phase < pw ? 1.0f : -1.0f;
                    sq += polyBlep (phase, inc);
                    float t2 = phase - pw; if (t2 < 0.0f) t2 += 1.0f;
                    sq -= polyBlep (t2, inc);
                    if (wave == 1) { out = sq; break; }
                    // leaky integration of a 50% square gives a clean triangle
                    tri = tri * 0.9995f + sq * 4.0f * inc;
                    out = tri;
                    break;
                }
                default: out = std::sin (kTwoPi * phase); break;
            }
            phase += inc;
            if (phase >= 1.0f) phase -= 1.0f;
            return out;
        }
    };

    struct OnePole
    {
        float z = 0.0f, a = 0.0f;
        void setLowpass (float hz, float sr) { a = 1.0f - std::exp (-kTwoPi * clampf (hz, 5.0f, sr * 0.49f) / sr); }
        inline float lp (float x) { z += a * (x - z); return z; }
        inline float hp (float x) { return x - lp (x); }
        void reset() { z = 0.0f; }
    };

    // Zero-delay-feedback state variable filter (TPT form).
    struct SVF
    {
        float g = 0.0f, k = 1.0f, a1 = 0.0f, s1 = 0.0f, s2 = 0.0f;
        float lpOut = 0.0f, bpOut = 0.0f, hpOut = 0.0f;
        void set (float hz, float q, float sr)
        {
            g = std::tan (kPi * clampf (hz, 10.0f, sr * 0.48f) / sr);
            k = 1.0f / std::max (0.05f, q);
            a1 = 1.0f / (1.0f + g * (g + k));
        }
        inline void process (float x)
        {
            hpOut = (x - (k + g) * s1 - s2) * a1;
            bpOut = g * hpOut + s1; s1 = g * hpOut + bpOut;
            lpOut = g * bpOut + s2; s2 = g * bpOut + lpOut;
        }
        inline float lp (float x) { process (x); return lpOut; }
        inline float bp (float x) { process (x); return bpOut; }
        inline float bpNorm (float x) { process (x); return bpOut * k; }   // unity gain at the centre
        inline float hp (float x) { process (x); return hpOut; }
        void reset() { s1 = s2 = 0.0f; }
    };

    // "Smooth": 4-pole transistor-ladder style low-pass, ZDF with saturated input.
    struct Ladder
    {
        float s[4] {}, G = 0.0f, g = 0.0f, k = 0.0f;
        void set (float hz, float res, float sr)
        {
            // x1.6 so the knob frequency is roughly the -3 dB point rather than -12 dB
            g = std::tan (kPi * clampf (hz * 1.6f, 15.0f, sr * 0.45f) / sr);
            G = g / (1.0f + g);
            k = res * 4.05f;
        }
        inline float process (float x, float drive)
        {
            const float inv = 1.0f / (1.0f + g);
            const float S = G * G * G * s[0] * inv + G * G * s[1] * inv + G * s[2] * inv + s[3] * inv;
            const float G4 = G * G * G * G;
            const float y4 = (G4 * x + S) / (1.0f + k * G4);
            float u = (x - k * y4) * (1.0f + k * 0.42f);         // bass compensation
            u = fastTanh (u * drive) / std::max (0.6f, drive * 0.85f);
            for (int i = 0; i < 4; ++i)
            {
                const float v = (u - s[i]) * G;
                const float y = v + s[i];
                s[i] = y + v;
                u = y;
            }
            return u;
        }
        void reset() { for (auto& v : s) v = 0.0f; }
    };

    // "Scream": aggressive 2-pole low-pass with a saturating resonance loop.
    struct Scream
    {
        float g = 0.0f, R = 1.0f, s1 = 0.0f, s2 = 0.0f;
        void set (float hz, float res, float sr)
        {
            g = std::tan (kPi * clampf (hz * 1.25f, 15.0f, sr * 0.45f) / sr);
            R = 1.0f - res * 0.985f;
        }
        inline float process (float x, float drive)
        {
            x = fastTanh (x * drive);
            const float hp = (x - (2.0f * R + g) * s1 - s2) / (1.0f + 2.0f * R * g + g * g);
            float bp = g * hp + s1;
            bp = fastTanh (bp * 0.7f) / 0.7f;                    // saturating feedback path
            s1 = g * hp + bp;
            const float lp = g * bp + s2;
            s2 = g * bp + lp;
            return fastTanh (lp * 0.9f) * 1.15f;
        }
        void reset() { s1 = s2 = 0.0f; }
    };

    // Analogue-style ADSR with exponential segments.
    struct Env
    {
        enum Stage { Idle, Attack, Decay, Sustain, Release };
        Stage stage = Idle;
        float value = 0.0f, aC = 0.0f, dC = 0.0f, rC = 0.0f, sus = 1.0f;

        static float coef (float seconds, float sr, float ratio)
        {
            return 1.0f - std::exp (-ratio / (std::max (0.0005f, seconds) * sr));
        }
        void set (float a, float d, float s, float r, float sr)
        {
            aC = coef (a, sr, 1.466f);   // overshoot target 1.3 reaches 1.0 at 'a'
            dC = coef (d, sr, 4.6f);
            rC = coef (r, sr, 4.6f);
            sus = s;
        }
        void noteOn() { stage = Attack; }
        void noteOff() { if (stage != Idle) stage = Release; }
        void reset() { stage = Idle; value = 0.0f; }
        bool active() const { return stage != Idle; }
        inline float process()
        {
            switch (stage)
            {
                case Attack:
                    value += (1.3f - value) * aC;
                    if (value >= 1.0f) { value = 1.0f; stage = Decay; }
                    break;
                case Decay:
                    value += (sus - value) * dC;
                    if (std::abs (value - sus) < 1.0e-4f) { value = sus; stage = Sustain; }
                    break;
                case Sustain: value = sus; break;
                case Release:
                    value += (0.0f - value) * rC;
                    if (value < 2.0e-5f) { value = 0.0f; stage = Idle; }
                    break;
                case Idle: break;
            }
            return value;
        }
    };

    // Fractional delay line with linear (or cubic) interpolation.
    struct DelayLine
    {
        float* buf = nullptr; int size = 0, w = 0;
        std::unique_ptr<float[]> mem;
        void init (int maxSamples)
        {
            size = 1; while (size < maxSamples + 4) size <<= 1;
            mem.reset (new float[(size_t) size]);
            buf = mem.get();
            clear();
        }
        void clear() { std::fill (buf, buf + size, 0.0f); w = 0; }
        inline void push (float x) { buf[w] = x; w = (w + 1) & (size - 1); }
        inline float tap (float delaySamples) const
        {
            const float d = clampf (delaySamples, 1.0f, (float) (size - 3));
            const float rp = (float) w - d;
            const int i0 = (int) std::floor (rp);
            const float f = rp - (float) i0;
            const int m = size - 1;
            const float xm1 = buf[(i0 - 1) & m], x0 = buf[i0 & m], x1 = buf[(i0 + 1) & m], x2 = buf[(i0 + 2) & m];
            // 4-point Hermite
            const float c1 = 0.5f * (x1 - xm1);
            const float c2 = xm1 - 2.5f * x0 + 2.0f * x1 - 0.5f * x2;
            const float c3 = 0.5f * (x2 - xm1) + 1.5f * (x0 - x1);
            return ((c3 * f + c2) * f + c1) * f + x0;
        }
        inline float tapInt (int d) const { return buf[(w - d) & (size - 1)]; }
    };
}
