#include "Vocoder.h"
#include <cmath>

using namespace dsp;

void Vocoder::prepare (float sampleRate)
{
    sr = sampleRate;
    numBands = 0;
    curFormant = 1000.0f;
    sibHP1.set (5000.0f, 0.707f, sr); sibHP2.set (5000.0f, 0.707f, sr);
    noiseHP1.set (4500.0f, 0.707f, sr); noiseHP2.set (4500.0f, 0.707f, sr);
    rng.seed (31337u);
    reset();
}

void Vocoder::reset()
{
    for (auto& b : band)
    {
        b.m1.reset(); b.m2.reset(); b.cl1.reset(); b.cl2.reset(); b.cr1.reset(); b.cr2.reset();
        b.envM = b.envL = b.envR = 0.0f;
        b.gL = b.gR = b.gLt = b.gRt = 0.0f;
    }
    sibHP1.reset(); sibHP2.reset(); noiseHP1.reset(); noiseHP2.reset();
    sibEnv = 0.0f; dryEnv = 0.0f;
}

void Vocoder::setup (int bands, float formant)
{
    const bool sameBands = bands == numBands;
    numBands = juce::jlimit (4, kMax, bands);
    curFormant = formant;
    const float lo = 100.0f, hi = 7500.0f;
    const float ratio = std::pow (hi / lo, 1.0f / (float) (numBands - 1));
    const float q = clampf (std::sqrt (ratio) / (ratio - 1.0f), 1.2f, 30.0f);
    const float shift = std::exp2 (formant / 12.0f);
    for (int i = 0; i < numBands; ++i)
    {
        auto& b = band[(size_t) i];
        const float f = lo * std::pow (ratio, (float) i);
        const float fc = clampf (f * shift, 40.0f, sr * 0.45f);
        b.m1.set (f, q, sr); b.m2.set (f, q, sr);
        b.cl1.set (fc, q, sr); b.cl2.set (fc, q, sr);
        b.cr1.set (fc, q, sr); b.cr2.set (fc, q, sr);
        if (! sameBands) { b.envM = b.envL = b.envR = 0.0f; b.gL = b.gR = b.gLt = b.gRt = 0.0f; }
    }
}

void Vocoder::process (float* l, float* r, const float* srcL, const float* srcR, int n, const VocoderParams& p)
{
    const float target = p.on ? p.mix : 0.0f;
    if (! p.on && mixS < 1.0e-4f)
    {
        mixS = 0.0f;
        // keep the meter alive so routing can be checked before switching on
        float pk = 0.0f;
        for (int i = 0; i < n; ++i) pk = std::max (pk, std::abs ((srcL[i] + srcR[i]) * 0.5f));
        inputLevel.store (std::max (pk * juce::Decibels::decibelsToGain (p.inputDb), inputLevel.load() * 0.8f));
        return;
    }

    if (p.bands != numBands || std::abs (p.formant - curFormant) > 0.01f) setup (p.bands, p.formant);

    const float aAtt = 1.0f - std::exp (-1.0f / (0.0015f * sr));
    const float aRel = 1.0f - std::exp (-1.0f / (std::max (1.0f, p.responseMs) * 0.001f * sr));
    const float targetGain = juce::Decibels::decibelsToGain (p.inputDb);
    const float sibA = 1.0f - std::exp (-1.0f / (0.002f * sr));
    const float sibR = 1.0f - std::exp (-1.0f / (0.03f * sr));
    const float norm = 2.0f;   // calibrated so a source at -18 dBFS gives roughly the dry synth level
    float peak = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        mixS += (target - mixS) * 0.002f;
        inGain += (targetGain - inGain) * 0.002f;
        const float m = (srcL[i] + srcR[i]) * 0.5f * inGain;
        peak = std::max (peak, std::abs (m));

        float cl = l[i], cr = r[i];
        // level of the synth itself (before any added noise) drives the noise carrier and the sibilance gate
        const float ad = 0.5f * (std::abs (cl) + std::abs (cr));
        dryEnv += (ad - dryEnv) * (ad > dryEnv ? aAtt : aRel);

        // envelopes
        float carrierMax = 0.0f;
        for (int k = 0; k < numBands; ++k)
        {
            auto& b = band[(size_t) k];
            const float mb = std::abs (b.m2.bpNorm (b.m1.bpNorm (m)));
            b.envM += (mb - b.envM) * (mb > b.envM ? aAtt : aRel);
            carrierMax = std::max (carrierMax, std::max (b.envL, b.envR));
        }

        if (p.noiseCarrier)
        {
            const float nz = rng.bipolar() * dryEnv * 0.35f;
            cl += nz; cr += nz;
        }

        // gains at control rate, interpolated per sample
        if (ctrl-- <= 0)
        {
            ctrl = 15;
            const float floorV = carrierMax * 0.03f + 1.0e-4f;
            for (int k = 0; k < numBands; ++k)
            {
                auto& b = band[(size_t) k];
                b.gLt = std::min (60.0f, b.envM / (b.envL + floorV));
                b.gRt = std::min (60.0f, b.envM / (b.envR + floorV));
            }
        }

        float ol = 0.0f, orr = 0.0f;
        for (int k = 0; k < numBands; ++k)
        {
            auto& b = band[(size_t) k];
            const float xl = b.cl2.bpNorm (b.cl1.bpNorm (cl));
            const float xr = b.cr2.bpNorm (b.cr1.bpNorm (cr));
            const float al = std::abs (xl), ar = std::abs (xr);
            b.envL += (al - b.envL) * (al > b.envL ? aAtt : aRel);
            b.envR += (ar - b.envR) * (ar > b.envR ? aAtt : aRel);
            b.gL += (b.gLt - b.gL) * 0.0625f;
            b.gR += (b.gRt - b.gR) * 0.0625f;
            ol += xl * b.gL;
            orr += xr * b.gR;
        }

        // sibilance: let the source's "s" and "t" sounds through as filtered noise plus a little of the source itself
        const float hp = sibHP2.hp (sibHP1.hp (m));
        const float ah = std::abs (hp);
        sibEnv += (ah - sibEnv) * (ah > sibEnv ? sibA : sibR);
        const float nz = noiseHP2.hp (noiseHP1.hp (rng.bipolar()));
        // only while the synth is sounding, so the source never leaks through on its own
        const float gate = clampf (dryEnv * 40.0f, 0.0f, 1.0f);
        const float sib = p.sibilance * gate * (nz * sibEnv * 2.5f + hp * 0.5f);

        const float vl = (ol * norm + sib), vr = (orr * norm + sib);
        l[i] = l[i] * (1.0f - mixS) + vl * mixS;
        r[i] = r[i] * (1.0f - mixS) + vr * mixS;
    }
    inputLevel.store (std::max (peak, inputLevel.load() * 0.8f));
}
