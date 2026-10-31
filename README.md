# bndt by Switchkicker

A warm, mix-ready synth made for Tim "Bandit" Powles (bndt). Korg/Moog-style analogue voices, a tape-replay layer with Mellotron-style sounds, an Omnichord-style strum mode, and a built-in tape echo, reverb, saturation and shortwave radio.

## Build on Windows

1. Install Visual Studio 2022 (Desktop C++ workload) and CMake.
2. Double-click `build.bat`.
   - The first build downloads JUCE 8.0.4 automatically. To use your own copy, set `JUCE_DIR` first.
3. You get `dist\bndt.vst3` and `dist\bndt.exe` (standalone).
   - If `build.bat` was run as administrator, the VST3 is also copied into `C:\Program Files\Common Files\VST3`.

## Mac installer

GitHub builds the Mac installer for you, so no Mac is needed:

1. Push this folder to a GitHub repository. The workflow in `.github/workflows/build.yml` runs on every push.
2. When the run finishes (about 10 to 15 minutes), open it on the **Actions** tab and download **bndt-1.0.0-macOS** from Artifacts.
3. Inside is `bndt-1.0.0-macOS.dmg`. That's the file to send to Tim. It contains:
   - **Install bndt.pkg**: a branded installer with tick boxes for VST3, Audio Unit and the standalone app (universal: Apple Silicon and Intel, macOS 10.13 or later)
   - **Uninstall bndt.command**: removes everything except saved presets
   - **Read Me First.txt**: how to get past the "unidentified developer" warning

To make a tidy release instead, tag a version (`git tag v1.0.0 && git push --tags`). The DMG and the Windows zip are then attached to a GitHub Release with a shareable download link.

**About the macOS warning:** without a paid Apple Developer account the installer isn't notarised, so macOS warns the first time it's opened. On macOS 15 and later: System Settings > Privacy & Security > **Open Anyway**. On macOS 14 and earlier: right-click > Open. If you later get a Developer ID, add these repository secrets and the workflow signs and notarises automatically, so the warning goes away:
`MAC_CERTS_P12` (base64 of your exported .p12 with both Developer ID certificates), `MAC_CERTS_PASSWORD`, `SIGN_APP_IDENTITY`, `SIGN_INSTALLER_IDENTITY`, `NOTARY_APPLE_ID`, `NOTARY_TEAM_ID`, `NOTARY_PASSWORD` (an app-specific password).

The installer pieces live in `installer/mac/` (layout, pages, art, uninstaller). Re-run `python3 tools/make_installer_art.py` if you change the artwork.

## Using Fancy Footwork as the UI font

The UI ships with Gochi Hand (open font licence) as a stand-in. To use Fancy Footwork:

1. Drop the font file (any name containing "fancy", `.ttf` or `.otf`) into `assets/`.
2. Rebuild. The UI picks it up automatically.

## What's inside

| Section | Controls |
| --- | --- |
| Oscillators | 2 band-limited oscillators (saw, square/PWM, triangle, sine), octave, semitone, detune, sub, noise, analogue drift |
| Tape | Modelled tape-replay layer: strings, cello, flute, choir, brass, organ. Wow, age (hiss + dulling), optional 8-second tape limit |
| Filter | Smooth (4-pole ladder) or Scream (aggressive 2-pole); cutoff, resonance, drive, envelope, key tracking, velocity |
| Envelopes | Analogue-style exponential amp and filter ADSR with shape displays |
| LFO 1 and 2 | Sine, triangle, square, random; to pitch, cutoff, pulse width, volume, osc 2 pitch or pan; fade-in |
| Voice | Poly (16 voices), mono, legato; glide; Strum mode (major, minor, 7th, maj 7, min 7, sus 4; speed; 3 to 8 notes) |
| Saturation | Tape or tube, drive, mix |
| Chorus | Two-mode BBD-style chorus (I, II, I + II) |
| Tape Echo | 1 to 3 playback heads, sync or free time, repeats (can self-oscillate), wow/flutter, tone, mix |
| Reverb | Spring or hall; size, tone, mix |
| Radio | Shortwave band-limiting, fading, crackle and tuning whistle |
| Vocoder | Source = sidechain input (or your audio input in the standalone app); carrier = the synth (or synth + noise for crisper consonants). 8/16/24/32 bands, input gain + meter, formant shift, response, sibilance, mix |
| Output | Master, low cut, soft limiter at -1 dBFS |

Also: mod wheel vibrato, pitch bend (2 semitones), sustain pedal, on-screen keyboard.

## Using the vocoder

1. Load bndt on an instrument track and turn the Vocoder **ON** (or pick a preset from the Vocoder category).
2. Send a vocal (or drums, or anything) to bndt's **sidechain** input:
   - **Ableton Live:** in bndt's device header open the sidechain section and choose the vocal track as the source.
   - **Reaper:** route the vocal track to bndt's track on channels 3/4.
   - **Cubase / FL Studio / Studio One:** enable bndt's side-chain input and send the vocal to it.
   - **Standalone app:** pick your mic or audio interface input in Options.
3. Watch the input meter and set **INPUT** so it lights mostly teal. Then hold chords: the synth talks.

The vocoder presets are level-matched for a source peaking around -12 dBFS.

## Presets

54 factory presets in 9 categories (Bass, Keys, Pads, Tape, Leads, Plucks, Strum, Textures, Vocoder), all loudness-matched. A few nods to Tim's own work are in there: Endling, Hypnogogue, Goodtimes 4 Free, Spacejunk Drift, Downlights, Undersea Drone.

- Click the preset name to browse; use the arrows to step through.
- **Save** stores your own sounds in `Documents\Switchkicker\bndt\Presets` (they appear under User).
- **Surprise** rolls a new, musically sensible random patch.

Tips: double-click any knob to reset it. Hover over a knob to read its value. The number under the small printed arrow is the setting, like on the amp.

## Project layout

```
Source/            plugin code (engine, FX, presets, UI)
assets/            background photo, knob layers, fonts
tools/             knob renderer and the offline render/level-matching test
build.bat          Windows build
installer/mac/     macOS installer (built by GitHub Actions)
```
