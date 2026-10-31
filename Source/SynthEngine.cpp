#include "SynthEngine.h"

using namespace dsp;

// ============================================================================
//  Tape-replay layer
// ============================================================================
namespace
{
    struct TapeDef
    {
        int reeds; float spread; float attack, release;
        float f[3], q[3], g[3]; float dry;
        float lp, vibRate, vibCents, vibDelay, gain;
    };

    //                       reeds spread att   rel    formant freqs            Qs                 gains            dry   lp     vib   cents delay gain
    const TapeDef kTape[6] = {
        /* Strings */ { 5, 11.0f, 0.12f, 0.28f, { 450, 1400, 3000 }, { 1.6f, 2.0f, 2.5f }, { 0.9f, 0.6f, 0.3f }, 0.25f, 6500, 5.2f, 6.0f, 0.25f, 0.42f },
        /* Cello   */ { 3, 7.0f,  0.15f, 0.30f, { 260, 750, 1900 },  { 1.4f, 2.0f, 2.5f }, { 1.0f, 0.55f, 0.25f }, 0.30f, 3800, 4.8f, 8.0f, 0.30f, 0.50f },
        /* Flute   */ { 1, 0.0f,  0.07f, 0.18f, { 0, 0, 0 },         { 1, 1, 1 },          { 0, 0, 0 },          1.00f, 7000, 5.0f, 14.0f, 0.35f, 0.62f },
        /* Choir   */ { 4, 10.0f, 0.22f, 0.40f, { 750, 1150, 2800 }, { 4.0f, 5.0f, 6.0f }, { 1.0f, 0.7f, 0.3f }, 0.05f, 5000, 4.5f, 9.0f, 0.20f, 1.10f },
        /* Brass   */ { 2, 5.0f,  0.04f, 0.15f, { 0, 0, 0 },         { 1, 1, 1 },          { 0, 0, 0 },          1.00f, 7000, 5.5f, 5.0f, 0.40f, 0.42f },
        /* Organ   */ { 5, 0.0f,  0.01f, 0.08f, { 0, 0, 0 },         { 1, 1, 1 },          { 0, 0, 0 },          1.00f, 6000, 0.0f, 0.0f, 0.00f, 0.30f },
    };
    const float kOrganRatio[5] = { 0.5f, 1.0f, 2.0f, 3.0f, 4.0f };
    const float kOrganLevel[5] = { 0.5f, 1.0f, 0.6f, 0.35f, 0.3f };
}

void TapeVoice::prepare (float sampleRate, uint32_t seed)
{
    sr = sampleRate;
    rng.seed (seed * 7919u + 13u);
    dcBlock.setLowpass (25.0f, sr);
}

void TapeVoice::start (int s, float midiNote, float velocity)
{
    sound = juce::jlimit (0, 5, s);
    const auto& d = kTape[sound];
    note = midiNote;
    vel = 0.75f + 0.25f * velocity;   // tape replay is barely touch sensitive
    t = 0.0f;
    if (stage == 0)
    {
        level = 0.0f;
        for (auto& f : formant) f.reset();
        tone.reset(); breath.reset(); brassLP.reset(); ageLP.reset();
    }
    stage = 1;
    attackC = Env::coef (d.attack, sr, 3.0f);
    releaseC = Env::coef (d.release, sr, 4.6f);
    for (int i = 0; i < 6; ++i)
    {
        osc[(size_t) i].reset (rng.unipolar());
        const float spreadPos = d.reeds > 1 ? ((float) i / (float) (d.reeds - 1)) * 2.0f - 1.0f : 0.0f;
        detune[(size_t) i] = spreadPos * d.spread + rng.bipolar() * 1.5f;
        vibPhase[(size_t) i] = rng.unipolar();
    }
    for (int i = 0; i < 3; ++i)
    {
        if (d.f[i] > 0.0f) formant[(size_t) i].set (d.f[i] * (1.0f + rng.bipolar() * 0.03f), d.q[i], sr);
        formantGain[(size_t) i] = d.g[i];
    }
    clickEnv = sound == 5 ? 1.0f : 0.0f;
    ctrlCounter = 0;
}

void TapeVoice::release()
{
    if (stage == 1) stage = 2;
}

float TapeVoice::process (float pitchOffset, float age, bool limit)
{
    if (stage == 0) return 0.0f;
    const auto& d = kTape[sound];
    t += 1.0f / sr;

    if (stage == 1) level += (1.0f - level) * attackC;
    else
    {
        level += (0.0f - level) * releaseC;
        if (level < 1.0e-4f) { stage = 0; return 0.0f; }
    }

    float limitGain = 1.0f, sag = 0.0f;
    if (limit)
    {
        if (t > 8.0f) { stage = 0; return 0.0f; }
        if (t > 7.4f) { limitGain = 1.0f - (t - 7.4f) / 0.6f; sag = -0.5f * (1.0f - limitGain); }
    }

    if (ctrlCounter-- <= 0)
    {
        ctrlCounter = 16;
        const float vibFade = d.vibDelay > 0.0f ? clampf ((t - d.vibDelay) / 0.4f, 0.0f, 1.0f) : 1.0f;
        for (int i = 0; i < d.reeds; ++i)
        {
            vibPhase[(size_t) i] += d.vibRate * (1.0f + 0.07f * (float) i) * 16.0f / sr;
            if (vibPhase[(size_t) i] > 1.0f) vibPhase[(size_t) i] -= 1.0f;
            const float vib = std::sin (kTwoPi * vibPhase[(size_t) i]) * d.vibCents * vibFade;
            float semis = note + pitchOffset + sag + (detune[(size_t) i] + vib) * 0.01f;
            float hz = mtof (semis);
            if (sound == 5) hz = mtof (note + pitchOffset + sag) * kOrganRatio[i];
            osc[(size_t) i].setFreq (hz, sr);
        }
        const float f0 = mtof (note + pitchOffset);
        if (sound == 2) breath.set (f0 * 2.0f, 2.5f, sr);
        if (sound == 4)
        {
            // brass "blat": brightness swells on the attack then settles
            const float b = t < 0.08f ? t / 0.08f : 0.55f + 0.45f * std::exp (-(t - 0.08f) / 0.25f);
            brassLP.set (f0 * (1.5f + 7.0f * b * vel), 0.9f, sr);
        }
        ageLP.setLowpass (d.lp * (1.0f - 0.6f * age), sr);
    }

    float x = 0.0f;
    switch (sound)
    {
        case 2: // flute: soft sine core, a little 2nd/3rd, breath noise
        {
            const float s = osc[0].process (3, 0.5f);
            const float p = osc[0].phase * kTwoPi;
            x = s + 0.22f * std::sin (2.0f * p) + 0.07f * std::sin (3.0f * p);
            const float chiff = t < 0.12f ? (1.0f - t / 0.12f) * 0.25f : 0.0f;
            x += breath.bp (rng.bipolar()) * (0.35f + chiff * 3.0f);
            break;
        }
        case 4: // brass
        {
            for (int i = 0; i < d.reeds; ++i) x += osc[(size_t) i].process (0, 0.5f);
            x = brassLP.lp (x * 0.6f) + rng.bipolar() * 0.01f;
            break;
        }
        case 5: // organ: additive drawbars + key click
        {
            for (int i = 0; i < 5; ++i) x += osc[(size_t) i].process (3, 0.5f) * kOrganLevel[i];
            if (clickEnv > 0.001f) { x += rng.bipolar() * clickEnv * 0.4f; clickEnv *= 0.993f; }
            break;
        }
        case 3: // choir: pulse + saw reeds into vowel formants
        {
            for (int i = 0; i < d.reeds; ++i) x += osc[(size_t) i].process (i % 2 == 0 ? 1 : 0, 0.3f);
            x *= 0.5f;
            float y = x * d.dry;
            for (int i = 0; i < 3; ++i) y += formant[(size_t) i].bp (x) * formantGain[(size_t) i];
            x = y + rng.bipolar() * 0.02f;
            break;
        }
        default: // strings / cello: detuned saws into body resonances
        {
            for (int i = 0; i < d.reeds; ++i) x += osc[(size_t) i].process (0, 0.5f);
            x *= 1.0f / std::sqrt ((float) d.reeds);
            float y = x * d.dry;
            for (int i = 0; i < 3; ++i) y += formant[(size_t) i].bp (x) * formantGain[(size_t) i];
            x = y;
            break;
        }
    }

    x = ageLP.lp (x);
    x = fastTanh (x * (1.0f + age)) / (1.0f + age * 0.5f);   // tape compression
    x += rng.bipolar() * age * 0.006f;                         // hiss
    x -= dcBlock.lp (x);
    return x * level * vel * limitGain * d.gain;
}

// ============================================================================
//  Voice
// ============================================================================
void Voice::prepare (float sampleRate, uint32_t seed)
{
    sr = sampleRate;
    rng.seed (seed * 2654435761u + 1u);
    tape.prepare (sampleRate, seed + 101u);
    for (int i = 0; i < 3; ++i) driftStatic[i] = rng.bipolar();
    o1.reset (rng.unipolar()); o2.reset (rng.unipolar()); sub.reset (0.0f);
}

void Voice::start (int n, int k, float vel, bool retrigger, bool isStrum, float fromPitch)
{
    const bool wasIdle = ! active();
    note = n; key = k; velocity = vel; held = true; sustained = false; strum = isStrum;
    targetPitch = (float) n;
    pitch = fromPitch;
    if (wasIdle)
    {
        ladder.reset(); scream.reset();
        amp.reset(); flt.reset();
        o1.reset (rng.unipolar()); o2.reset (rng.unipolar()); sub.reset (o1.phase);
        pan = rng.bipolar();
    }
    if (retrigger || wasIdle)
    {
        amp.noteOn(); flt.noteOn();
        timeOn = 0.0f;
    }
}

void Voice::release()
{
    held = false;
    amp.noteOff(); flt.noteOff();
    tape.release();
}

void Voice::kill()
{
    held = false;
    amp.reset(); flt.reset(); tape.kill();
}

// ============================================================================
//  Engine
// ============================================================================
void SynthEngine::prepare (double sampleRate, int)
{
    sr = (float) sampleRate;
    for (int i = 0; i < kVoices; ++i) voices[(size_t) i].prepare (sr, (uint32_t) (i + 1));
    stack.reserve (128);
    pending.reserve (128);
    rng.seed (777u);
    reset();
}

void SynthEngine::reset()
{
    for (auto& v : voices) v.kill();
    stack.clear(); pending.clear();
    bend = bendTarget = 0.0f;
    sustainPedal = false;
}

void SynthEngine::allNotesOff()
{
    pending.clear(); stack.clear();
    for (auto& v : voices) if (v.active()) v.release();
}

void SynthEngine::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn()) noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff()) noteOff (m.getNoteNumber());
    else if (m.isPitchWheel()) bendTarget = ((float) m.getPitchWheelValue() - 8192.0f) / 8192.0f * 2.0f;
    else if (m.isControllerOfType (1)) modWheel = (float) m.getControllerValue() / 127.0f;
    else if (m.isSustainPedalOn()) sustainPedal = true;
    else if (m.isSustainPedalOff())
    {
        sustainPedal = false;
        for (auto& v : voices) if (v.sustained) { v.sustained = false; v.release(); }
    }
    else if (m.isAllNotesOff() || m.isAllSoundOff()) allNotesOff();
}

Voice* SynthEngine::findVoice (int k, bool isStrum)
{
    if (! isStrum)
        for (auto& v : voices)
            if (v.active() && v.key == k && ! v.strum) return &v;

    for (auto& v : voices) if (! v.active()) return &v;

    Voice* best = nullptr;
    for (auto& v : voices)   // oldest released voice first
        if (! v.held && ! v.sustained && (best == nullptr || v.age < best->age)) best = &v;
    if (best == nullptr)
        for (auto& v : voices)
            if (best == nullptr || v.age < best->age) best = &v;
    return best;
}

void SynthEngine::startVoice (int n, int k, float vel, bool isStrum)
{
    Voice* v = findVoice (k, isStrum);
    const float from = params.glide > 0.0005f ? lastPitch : (float) n;
    v->start (n, k, vel, true, isStrum, from);
    v->age = ++counter;
    if (params.tapeLevel > 0.001f) v->tape.start (params.tapeSound, (float) n, vel);
    else v->tape.kill();
    lastPitch = (float) n;
}

void SynthEngine::noteOn (int n, float vel)
{
    const bool mono = params.voiceMode != 0 && ! params.strumOn;
    if (mono)
    {
        const bool wasHeld = ! stack.empty();
        stack.erase (std::remove_if (stack.begin(), stack.end(), [n] (const Held& h) { return h.note == n; }), stack.end());
        stack.push_back ({ n, vel });
        Voice& v = voices[0];
        const bool retrig = params.voiceMode == 1 || ! wasHeld || ! v.amp.active() || v.amp.stage == Env::Release;
        const float from = (v.active() && params.glide > 0.0005f) ? v.pitch : (params.glide > 0.0005f ? lastPitch : (float) n);
        v.start (n, n, vel, retrig, false, from);
        v.age = ++counter;
        if (params.tapeLevel > 0.001f && (retrig || ! v.tape.active())) v.tape.start (params.tapeSound, (float) n, vel);
        else if (params.tapeLevel > 0.001f) v.tape.note = (float) n;
        lastPitch = (float) n;
        return;
    }

    if (params.strumOn)
    {
        static const int chords[6][4] = { { 0, 4, 7, -1 }, { 0, 3, 7, -1 }, { 0, 4, 7, 10 }, { 0, 4, 7, 11 }, { 0, 3, 7, 10 }, { 0, 5, 7, -1 } };
        const int* c = chords[juce::jlimit (0, 5, params.strumChord)];
        const int len = c[3] < 0 ? 3 : 4;
        pending.erase (std::remove_if (pending.begin(), pending.end(), [n] (const Pending& p) { return p.key == n; }), pending.end());
        const int step = (int) (params.strumSpeed * 0.001f * sr);
        for (int i = 0; i < params.strumSpan; ++i)
        {
            const int pitch = n + c[i % len] + 12 * (i / len);
            if (pitch > 127) break;
            const float v = vel * std::pow (0.94f, (float) i);
            if (i == 0) startVoice (pitch, n, v, true);
            else pending.push_back ({ step * i, pitch, n, v });
        }
        return;
    }

    startVoice (n, n, vel, false);
}

void SynthEngine::noteOff (int n)
{
    // pending strum notes for this key never sound
    pending.erase (std::remove_if (pending.begin(), pending.end(), [n] (const Pending& p) { return p.key == n; }), pending.end());

    if (params.voiceMode != 0 && ! params.strumOn)
    {
        stack.erase (std::remove_if (stack.begin(), stack.end(), [n] (const Held& h) { return h.note == n; }), stack.end());
        Voice& v = voices[0];
        if (stack.empty())
        {
            if (v.key == n || v.note == n)
            {
                if (sustainPedal) { v.sustained = true; v.held = false; }
                else v.release();
            }
        }
        else if (v.note == n)
        {
            const auto& h = stack.back();
            v.note = h.note; v.key = h.note; v.targetPitch = (float) h.note;
            if (params.glide <= 0.0005f) v.pitch = v.targetPitch;
            if (params.voiceMode == 1) { v.amp.noteOn(); v.flt.noteOn(); }
            v.tape.note = (float) h.note;
            lastPitch = (float) h.note;
        }
    }

    for (auto& v : voices)
    {
        if (! v.held || v.key != n) continue;
        if (sustainPedal) { v.sustained = true; v.held = false; }
        else v.release();
    }
}

void SynthEngine::advanceLfos (int samples)
{
    for (int i = 0; i < 2; ++i)
    {
        lfoPhase[i] += params.lfoRate[i] * (float) samples / sr;
        bool wrapped = false;
        while (lfoPhase[i] >= 1.0f) { lfoPhase[i] -= 1.0f; wrapped = true; }
        const float p = lfoPhase[i];
        float v = 0.0f;
        switch (params.lfoShape[i])
        {
            case 0: v = std::sin (kTwoPi * p); break;
            case 1: v = p < 0.5f ? 4.0f * p - 1.0f : 3.0f - 4.0f * p; break;
            case 2: v = p < 0.5f ? 1.0f : -1.0f; break;
            default:
                if (wrapped) lfoHeld[i] = rng.bipolar();
                v = lfoHeld[i];
                break;
        }
        // light smoothing keeps square / random from clicking
        const float a = 1.0f - std::exp (-(float) samples / (0.003f * sr));
        lfoSmooth[i] += (v - lfoSmooth[i]) * a;
        lfoOut[i] = lfoSmooth[i];
    }
}

void SynthEngine::render (float* left, float* right, int numSamples)
{
    const auto& P = params;
    const float glideCoef = P.glide > 0.0005f ? 1.0f - std::exp (-(float) kCtrl / (P.glide * sr / 3.0f)) : 1.0f;
    const float drive = 1.0f + P.filterDrive * 5.0f;
    const float maxCut = sr * 0.45f;

    for (auto& v : voices)
    {
        v.amp.set (P.ampA, P.ampD, P.ampS, P.ampR, sr);
        v.flt.set (P.fltA, P.fltD, P.fltS, P.fltR, sr);
    }

    for (int pos = 0; pos < numSamples; pos += kCtrl)
    {
        const int len = std::min (kCtrl, numSamples - pos);

        for (size_t i = 0; i < pending.size();)
        {
            pending[i].samplesLeft -= len;
            if (pending[i].samplesLeft <= 0)
            {
                const auto p = pending[i];
                pending.erase (pending.begin() + (long) i);
                startVoice (p.note, p.key, p.vel, true);
            }
            else ++i;
        }

        advanceLfos (len);
        bend += (bendTarget - bend) * 0.15f;
        vibPhase += 5.6f * (float) len / sr; if (vibPhase > 1.0f) vibPhase -= 1.0f;
        wowPhase += 0.62f * (float) len / sr; if (wowPhase > 1.0f) wowPhase -= 1.0f;
        flutterPhase += 8.3f * (float) len / sr; if (flutterPhase > 1.0f) flutterPhase -= 1.0f;
        wowDrift = clampf (wowDrift * 0.999f + rng.bipolar() * 0.01f, -1.0f, 1.0f);
        const float wowSemis = P.tapeWow * (0.17f * std::sin (kTwoPi * wowPhase) + 0.035f * std::sin (kTwoPi * flutterPhase) + 0.07f * wowDrift);
        const float wheelVib = modWheel * 0.45f * std::sin (kTwoPi * vibPhase);

        for (auto& v : voices)
        {
            if (! v.active()) continue;

            // ---- control rate ----
            v.pitch += (v.targetPitch - v.pitch) * glideCoef;
            float lfoPitch = 0.0f, lfoCut = 0.0f, lfoPW = 0.0f, lfoVol = 0.0f, lfoOsc2 = 0.0f, lfoPan = 0.0f;
            for (int i = 0; i < 2; ++i)
            {
                if (P.lfoAmount[i] <= 0.0f) continue;
                const float fade = P.lfoFade[i] > 0.001f ? clampf (v.timeOn / P.lfoFade[i], 0.0f, 1.0f) : 1.0f;
                const float val = lfoOut[i] * fade;
                const float amt = P.lfoAmount[i];
                switch (P.lfoDest[i])
                {
                    case 0: lfoPitch += val * amt * amt * 12.0f; break;
                    case 1: lfoCut += val * amt * 4.0f; break;
                    case 2: lfoPW += val * amt * 0.42f; break;
                    case 3: lfoVol += (0.5f + 0.5f * val) * amt; break;
                    case 4: lfoOsc2 += val * amt * amt * 12.0f; break;
                    default: lfoPan += val * amt; break;
                }
            }
            for (int i = 0; i < 3; ++i)
            {
                v.driftWalk[i] = clampf (v.driftWalk[i] * 0.997f + v.rng.bipolar() * 0.03f, -1.0f, 1.0f);
            }
            const float dr = P.drift;
            const float d1 = dr * (v.driftStatic[0] * 3.0f + v.driftWalk[0] * 5.0f) * 0.01f;
            const float d2 = dr * (v.driftStatic[1] * 3.0f + v.driftWalk[1] * 5.0f) * 0.01f;
            const float dCut = dr * v.driftWalk[2] * 0.12f;

            const float base = v.pitch + bend + lfoPitch + wheelVib;
            const bool oscOn = v.amp.active();
            if (oscOn)
            {
                const float p1 = base + (float) P.osc1Oct * 12.0f + d1;
                const float p2 = base + (float) P.osc2Oct * 12.0f + (float) P.osc2Semi + P.osc2Detune * 0.01f + d2 + lfoOsc2;
                v.o1.setFreq (mtof (p1), sr);
                v.o2.setFreq (mtof (p2), sr);
                v.sub.setFreq (mtof (p1 - 12.0f), sr);

                const float keyOct = P.keyTrack * ((float) v.note - 60.0f) / 12.0f;
                const float velOct = P.velFilter * (v.velocity - 0.75f) * 2.5f;
                const float envOct = P.envAmount * v.flt.value * 7.0f;
                const float cut = clampf (P.cutoff * std::exp2 (envOct + keyOct + velOct + lfoCut + dCut), 20.0f, maxCut);
                if (P.filterType == 0) v.ladder.set (cut, P.resonance, sr);
                else v.scream.set (cut, P.resonance, sr);
            }

            const float pw = clampf (P.osc1PW + lfoPW, 0.05f, 0.95f);
            const float velGain = 1.0f - P.velAmp + P.velAmp * v.velocity * v.velocity;
            const float trem = 1.0f - lfoVol;
            const float panPos = clampf (v.pan * P.drift * 0.35f + lfoPan, -1.0f, 1.0f);
            const float th = (panPos + 1.0f) * kPi * 0.25f;
            const float gl = std::cos (th) * 1.41421f * 0.36f, gr = std::sin (th) * 1.41421f * 0.36f;
            const float tapeOffset = (v.pitch - (float) v.note) + bend + lfoPitch + wheelVib + wowSemis;
            const bool tapeOn = v.tape.active();

            // ---- audio rate ----
            for (int s = 0; s < len; ++s)
            {
                float out = 0.0f;
                if (oscOn)
                {
                    float mix = v.o1.process (P.osc1Wave, pw) * P.osc1Level
                              + v.o2.process (P.osc2Wave, 0.5f) * P.osc2Level
                              + v.sub.process (1, 0.5f) * P.subLevel * 0.8f
                              + v.rng.bipolar() * P.noiseLevel * 0.5f;
                    mix *= 0.6f;
                    v.flt.process();
                    const float y = P.filterType == 0 ? v.ladder.process (mix, drive) : v.scream.process (mix, drive);
                    out = y * v.amp.process() * velGain * trem;
                }
                if (tapeOn) out += v.tape.process (tapeOffset, P.tapeAge, P.tapeLimit) * P.tapeLevel;
                left[pos + s] += out * gl;
                right[pos + s] += out * gr;
            }
            v.timeOn += (float) len / sr;
        }
    }
}
