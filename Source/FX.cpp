#include "FX.h"

using namespace dsp;

// ============================================================================
//  Saturation: tape (soft, slightly dark) or tube (asymmetric, more 2nd harmonic)
// ============================================================================
void Saturator::prepare (float s)
{
    sr = s;
    mix.reset (sr, 0.05); drive.reset (sr, 0.05);
    toneL.reset(); toneR.reset();
}

void Saturator::process (float* l, float* r, int n, const FXParams& p)
{
    mix.setTargetValue (p.satOn ? p.satMix : 0.0f);
    drive.setTargetValue (p.satDrive);
    if (! p.satOn && ! mix.isSmoothing() && mix.getCurrentValue() < 1.0e-4f) return;
    toneL.setLowpass (16000.0f - p.satDrive * 7000.0f, sr);
    toneR.a = toneL.a;
    for (int i = 0; i < n; ++i)
    {
        const float m = mix.getNextValue();
        const float k = 1.0f + drive.getNextValue() * 8.0f;
        const float comp = 1.0f / std::pow (k, 0.62f);
        auto shape = [&] (float x)
        {
            if (p.satType == 0)
                return fastTanh (k * (x + 0.08f * x * x)) * comp;
            return (x > 0.0f ? fastTanh (k * x) : fastTanh (1.5f * k * x) / 1.5f) * comp * 1.1f;
        };
        float yl = shape (l[i]), yr = shape (r[i]);
        if (p.satType == 0) { yl = toneL.lp (yl); yr = toneR.lp (yr); }
        l[i] += (yl - l[i]) * m;
        r[i] += (yr - r[i]) * m;
    }
}

// ============================================================================
//  Chorus
// ============================================================================
void Chorus::prepare (float s)
{
    sr = s;
    dl.init ((int) (0.02f * sr));
    lpL.setLowpass (9000.0f, sr); lpR.setLowpass (9000.0f, sr);
    lpL.reset(); lpR.reset();
}

void Chorus::process (float* l, float* r, int n, int mode)
{
    const float target = mode == 0 ? 0.0f : 1.0f;
    if (mode == 0 && wet < 1.0e-4f) { wet = 0.0f; return; }
    float rate = 0.513f, lo = 1.66f, hi = 5.35f;
    if (mode == 2) rate = 0.863f;
    if (mode == 3) { rate = 9.75f; lo = 3.25f; hi = 3.75f; }
    const float msToS = sr * 0.001f;
    for (int i = 0; i < n; ++i)
    {
        wet += (target - wet) * 0.0015f;
        phase += rate / sr; if (phase >= 1.0f) phase -= 1.0f;
        const float tri = phase < 0.5f ? 4.0f * phase - 1.0f : 3.0f - 4.0f * phase;
        const float mid = (lo + hi) * 0.5f, dep = (hi - lo) * 0.5f;
        const float in = (l[i] + r[i]) * 0.5f;
        dl.push (in);
        const float a = lpL.lp (dl.tap ((mid + dep * tri) * msToS));
        const float b = lpR.lp (dl.tap ((mid - dep * tri) * msToS));
        l[i] = l[i] * (1.0f - 0.25f * wet) + a * 0.75f * wet;
        r[i] = r[i] * (1.0f - 0.25f * wet) + b * 0.75f * wet;
    }
}

// ============================================================================
//  Tape echo
// ============================================================================
void TapeEcho::prepare (float s)
{
    sr = s;
    line.init ((int) (3.2f * sr));
    fbHP.setLowpass (140.0f, sr);
    rng.seed (4242u);
    reset();
}

void TapeEcho::reset()
{
    line.clear();
    fbLP.reset(); fbHP.reset(); wetLP.reset();
    timeSmoothed = 0.0f;
}

void TapeEcho::process (float* l, float* r, int n, const FXParams& p)
{
    const float mixTarget = p.delayOn ? p.delayMix : 0.0f;
    if (! p.delayOn && mixS < 1.0e-4f) { mixS = 0.0f; return; }

    static const float divQ[8] = { 0.25f, 1.0f / 3.0f, 0.5f, 0.75f, 2.0f / 3.0f, 1.0f, 1.5f, 2.0f };
    float timeSec = p.delayTimeMs * 0.001f;
    if (p.delaySync)
        timeSec = (float) (60.0 / juce::jlimit (30.0, 300.0, p.bpm)) * divQ[juce::jlimit (0, 7, p.delayDiv)];
    const float target = clampf (timeSec, 0.02f, 1.5f) * sr;
    if (timeSmoothed <= 0.0f) timeSmoothed = target;

    static const float heads[3][3] = { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.5f, 0.0f }, { 1.0f, 0.75f, 0.5f } };
    static const float headGain[3] = { 1.0f, 0.75f, 0.6f };
    const int nh = juce::jlimit (0, 2, p.delayHeads) + 1;
    const float* h = heads[nh - 1];
    const float fbNorm = 1.0f / std::sqrt ((float) nh);

    const float toneHz = 1400.0f * std::exp2 (p.delayTone * 2.8f);
    fbLP.setLowpass (toneHz, sr);
    wetLP.setLowpass (toneHz * 1.3f, sr);
    const float slew = 1.0f - std::exp (-1.0f / (0.18f * sr));

    for (int i = 0; i < n; ++i)
    {
        mixS += (mixTarget - mixS) * 0.002f;
        fbS += (p.delayFeedback - fbS) * 0.002f;
        timeSmoothed += (target - timeSmoothed) * slew;   // tape-style pitch bend on time changes
        wowPhase += 0.55f / sr; if (wowPhase >= 1.0f) wowPhase -= 1.0f;
        flutPhase += 6.3f / sr; if (flutPhase >= 1.0f) flutPhase -= 1.0f;
        if ((i & 31) == 0) drift = clampf (drift * 0.995f + rng.bipolar() * 0.02f, -1.0f, 1.0f);
        const float mod = 1.0f + p.delayWow * (0.0045f * std::sin (kTwoPi * wowPhase) + 0.0012f * std::sin (kTwoPi * flutPhase) + 0.002f * drift);
        const float t = timeSmoothed * mod;

        float wl = 0.0f, wr = 0.0f, fb = 0.0f;
        for (int k = 0; k < nh; ++k)
        {
            const float tapL = line.tap (t * h[k]);
            const float tapR = k == 0 ? line.tap (t * h[k] * 1.013f) : tapL;
            const float g = headGain[k];
            fb += tapL * g;
            if (k == 1) { wl += tapL * g; wr += tapR * g * 0.45f; }
            else if (k == 2) { wl += tapL * g * 0.45f; wr += tapR * g; }
            else { wl += tapL * g; wr += tapR * g; }
        }
        fb *= fbNorm;

        const float in = (l[i] + r[i]) * 0.5f;
        float x = in + fb * fbS;
        x = fbLP.lp (x);
        x = x - fbHP.lp (x);
        x = fastTanh (x * 1.25f) / 1.25f;     // tape saturation keeps self-oscillation musical
        line.push (x);

        l[i] += wl * mixS;
        r[i] += wr * mixS;
    }
}

// ============================================================================
//  Spring reverb
// ============================================================================
void SpringReverb::prepare (float s)
{
    sr = s;
    for (int i = 0; i < 2; ++i)
    {
        springs[i].line.init ((int) (0.08f * sr));
        springs[i].hp.setLowpass (180.0f, sr);
    }
    reset();
}

void SpringReverb::reset()
{
    for (auto& sp : springs)
    {
        sp.line.clear(); sp.apX.fill (0.0f); sp.apY.fill (0.0f);
        sp.lp.reset(); sp.hp.reset(); sp.fb = 0.0f;
    }
}

void SpringReverb::process (const float* inL, const float* inR, float* outL, float* outR, int n, float size, float tone)
{
    const float lens[2] = { 0.0331f, 0.0397f };
    const float fbGain = 0.55f + size * 0.37f;
    const float a = 0.62f;
    for (int c = 0; c < 2; ++c)
    {
        auto& sp = springs[c];
        sp.length = lens[c] * (0.8f + size * 0.45f) * sr;
        sp.lp.setLowpass (2400.0f + tone * 4200.0f, sr);
    }
    for (int i = 0; i < n; ++i)
    {
        const float in = (inL[i] + inR[i]) * 0.5f;
        float outs[2];
        for (int c = 0; c < 2; ++c)
        {
            auto& sp = springs[c];
            sp.modPhase += (0.9f + 0.3f * (float) c) / sr; if (sp.modPhase > 1.0f) sp.modPhase -= 1.0f;
            float x = sp.line.tap (sp.length + 3.0f * std::sin (kTwoPi * sp.modPhase));
            // dispersion: cascade of first-order allpasses gives the "boing" chirp
            for (size_t k = 0; k < sp.apX.size(); ++k)
            {
                const float y = a * x + sp.apX[k] - a * sp.apY[k];
                sp.apX[k] = x; sp.apY[k] = y; x = y;
            }
            x = sp.lp.lp (x);
            x = x - sp.hp.lp (x);
            sp.line.push (fastTanh ((in + x * fbGain) * 0.9f));
            outs[c] = x;
        }
        outL[i] = outs[0] * 0.8f + outs[1] * 0.2f;
        outR[i] = outs[1] * 0.8f + outs[0] * 0.2f;
    }
}

// ============================================================================
//  Hall (FDN)
// ============================================================================
void HallReverb::prepare (float s)
{
    sr = s;
    const float ms[N] = { 29.7f, 37.1f, 41.1f, 43.7f, 53.0f, 59.9f, 67.7f, 73.1f };
    for (int i = 0; i < N; ++i)
    {
        baseLen[i] = ms[i] * 0.001f * sr;
        lines[i].init ((int) (baseLen[i] * 1.7f) + 64);
    }
    pre.init ((int) (0.05f * sr));
    const float dms[4] = { 4.7f, 3.6f, 12.7f, 9.3f };
    for (int i = 0; i < 4; ++i) { diff[i].len = (int) (dms[i] * 0.001f * sr); diff[i].d.init (diff[i].len + 4); }
    reset();
}

void HallReverb::reset()
{
    for (auto& l : lines) l.clear();
    for (auto& d : damp) d.reset();
    for (auto& d : diff) d.d.clear();
    pre.clear();
}

void HallReverb::process (const float* inL, const float* inR, float* outL, float* outR, int n, float size, float tone)
{
    const float scale = 0.6f + size * 1.0f;
    const float rt60 = 0.8f + std::pow (size, 1.5f) * 10.0f;
    float g[N];
    for (int i = 0; i < N; ++i)
    {
        g[i] = std::pow (10.0f, -3.0f * baseLen[i] * scale / (rt60 * sr));
        damp[i].setLowpass (2500.0f + tone * 12000.0f, sr);
    }
    const float preSamples = (0.012f + size * 0.025f) * sr;

    for (int s = 0; s < n; ++s)
    {
        modPhase += 0.31f / sr; if (modPhase > 1.0f) modPhase -= 1.0f;
        const float m = std::sin (kTwoPi * modPhase) * 7.0f;

        pre.push ((inL[s] + inR[s]) * 0.5f);
        float x = pre.tap (preSamples);
        for (auto& d : diff)
        {
            const float bufOut = d.d.tapInt (d.len);
            const float y = -0.7f * x + bufOut;
            d.d.push (x + 0.7f * bufOut);
            x = y;
        }

        float v[N];
        for (int i = 0; i < N; ++i)
        {
            const float mod = (i == 0 ? m : (i == 4 ? -m : 0.0f));
            v[i] = damp[i].lp (lines[i].tap (baseLen[i] * scale + mod)) * g[i];
        }
        // fast Walsh-Hadamard mixing
        for (int len = 1; len < N; len <<= 1)
            for (int i = 0; i < N; i += len << 1)
                for (int j = i; j < i + len; ++j)
                {
                    const float a = v[j], b = v[j + len];
                    v[j] = a + b; v[j + len] = a - b;
                }
        const float norm = 0.35355339f;
        float l = 0.0f, r = 0.0f;
        for (int i = 0; i < N; ++i)
        {
            const float y = v[i] * norm;
            lines[i].push (y + x * (i % 2 == 0 ? 0.5f : -0.5f));
            if (i % 2 == 0) l += y; else r += y;
        }
        outL[s] = l * 0.45f;
        outR[s] = r * 0.45f;
    }
}

// ============================================================================
//  Radio
// ============================================================================
void Radio::prepare (float s)
{
    sr = s;
    crackleF.set (3200.0f, 1.2f, sr);
    hpL.reset(); hpR.reset(); lpL.reset(); lpR.reset(); crackleF.reset();
    rng.seed (99u);
}

void Radio::process (float* l, float* r, int n, const FXParams& p)
{
    const float target = p.radioOn ? p.radioAmount : 0.0f;
    if (! p.radioOn && mixS < 1.0e-4f) { mixS = 0.0f; return; }
    const float amt = p.radioAmount, tune = p.radioTune;
    hpL.set (250.0f + amt * 350.0f, 0.8f, sr); hpR.set (250.0f + amt * 350.0f, 0.8f, sr);
    lpL.set (4800.0f - amt * 2000.0f, 1.1f, sr); lpR.set (4800.0f - amt * 2000.0f, 1.1f, sr);
    for (int i = 0; i < n; ++i)
    {
        mixS += (target - mixS) * 0.002f;
        if (--fadeCounter <= 0)
        {
            fadeCounter = (int) (0.2f * sr);
            fadeTarget = 1.0f - tune * 0.55f * rng.unipolar();
        }
        fade += (fadeTarget - fade) * 0.0004f;
        whistleDrift = clampf (whistleDrift * 0.9999f + rng.bipolar() * 0.002f, -1.0f, 1.0f);
        whistlePhase += (1100.0f + 500.0f * whistleDrift) / sr; if (whistlePhase > 1.0f) whistlePhase -= 1.0f;
        float crackle = 0.0f;
        if (rng.unipolar() < 0.0003f + tune * 0.0015f) crackle = rng.bipolar() * (0.2f + 0.4f * tune);
        crackle = crackleF.bp (crackle);
        const float noise = rng.bipolar() * tune * 0.012f + std::sin (kTwoPi * whistlePhase) * tune * tune * 0.03f + crackle;

        auto chain = [&] (float x, SVF& hp, SVF& lp)
        {
            x = hp.hp (x);
            x = lp.lp (x);
            return fastTanh (x * 1.8f) * 0.75f * fade;
        };
        const float wl = chain (l[i], hpL, lpL) + noise;
        const float wr = chain (r[i], hpR, lpR) + noise;
        l[i] += (wl - l[i]) * mixS;
        r[i] += (wr - r[i]) * mixS;
    }
}

// ============================================================================
//  Output: low cut, master gain, soft limiter
// ============================================================================
void OutputStage::prepare (float s)
{
    sr = s;
    gain.reset (sr, 0.05);
    gain.setCurrentAndTargetValue (1.0f);
    lastCut = -1.0f;
    env = 0.0f;
    hpL.reset(); hpR.reset();
}

void OutputStage::process (float* l, float* r, int n, const FXParams& p)
{
    if (std::abs (p.lowCut - lastCut) > 0.01f)
    {
        hpL.set (p.lowCut, 0.707f, sr); hpR.set (p.lowCut, 0.707f, sr);
        lastCut = p.lowCut;
    }
    gain.setTargetValue (juce::Decibels::decibelsToGain (p.masterDb));
    const float att = 1.0f - std::exp (-1.0f / (0.0005f * sr));
    const float rel = 1.0f - std::exp (-1.0f / (0.12f * sr));
    const float ceiling = 0.891f;   // -1 dBFS
    for (int i = 0; i < n; ++i)
    {
        const float g = gain.getNextValue();
        float a = hpL.hp (l[i]) * g, b = hpR.hp (r[i]) * g;
        const float pk = std::max (std::abs (a), std::abs (b));
        env += (pk - env) * (pk > env ? att : rel);
        const float lim = env > ceiling ? ceiling / env : 1.0f;
        a *= lim; b *= lim;
        auto soft = [] (float x)
        {
            const float ax = std::abs (x);
            if (ax < 0.85f) return x;
            const float y = 0.85f + 0.13f * fastTanh ((ax - 0.85f) / 0.13f);
            return x < 0.0f ? -y : y;
        };
        l[i] = soft (a); r[i] = soft (b);
    }
}

// ============================================================================
//  Chain
// ============================================================================
void FXChain::prepare (double sampleRate, int maxBlock)
{
    const float s = (float) sampleRate;
    sat.prepare (s); chorus.prepare (s); echo.prepare (s); spring.prepare (s); hall.prepare (s);
    radio.prepare (s); out.prepare (s); vocoder.prepare (s);
    revBuf.setSize (2, maxBlock, false, true, true);
    trimGain.reset (sampleRate, 0.03);
}

void FXChain::reset()
{
    echo.reset(); spring.reset(); hall.reset(); vocoder.reset();
}

void FXChain::process (juce::AudioBuffer<float>& buffer, int n, const FXParams& p, const float* srcL, const float* srcR)
{
    if (revBuf.getNumSamples() < n) revBuf.setSize (2, n, false, true, true);
    float* l = buffer.getWritePointer (0);
    float* r = buffer.getWritePointer (1);

    vocoder.process (l, r, srcL, srcR, n, p.voc);

    // preset gain staging happens next, so saturation and echo see a consistent level
    trimGain.setTargetValue (juce::Decibels::decibelsToGain (p.trimDb));
    for (int i = 0; i < n; ++i) { const float g = trimGain.getNextValue(); l[i] *= g; r[i] *= g; }

    sat.process (l, r, n, p);
    chorus.process (l, r, n, p.chorusMode);
    echo.process (l, r, n, p);

    // reverb (wet added on top of the dry signal)
    const float revTarget = p.revOn ? p.revMix : 0.0f;
    if (p.revOn || revMixS > 1.0e-4f)
    {
        float* wl = revBuf.getWritePointer (0);
        float* wr = revBuf.getWritePointer (1);
        if (p.revType == 0) spring.process (l, r, wl, wr, n, p.revSize, p.revTone);
        else hall.process (l, r, wl, wr, n, p.revSize, p.revTone);
        for (int i = 0; i < n; ++i)
        {
            revMixS += (revTarget - revMixS) * 0.002f;
            const float dry = 1.0f - revMixS * 0.35f;
            l[i] = l[i] * dry + wl[i] * revMixS * 1.6f;
            r[i] = r[i] * dry + wr[i] * revMixS * 1.6f;
        }
    }
    else revMixS = 0.0f;

    radio.process (l, r, n, p);
    out.process (l, r, n, p);
}
