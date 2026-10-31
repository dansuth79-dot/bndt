// Offline checks: renders every factory preset to WAV, reports levels/NaNs/CPU, and snapshots the UI.
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Presets.h"
#include <chrono>

static void writeWav (const juce::File& f, const juce::AudioBuffer<float>& b, double sr)
{
    f.deleteFile();
    juce::WavAudioFormat fmt;
    std::unique_ptr<juce::OutputStream> os (f.createOutputStream().release());
    if (auto* w = fmt.createWriterFor (os.get(), sr, 2, 16, {}, 0))
    {
        os.release();
        std::unique_ptr<juce::AudioFormatWriter> writer (w);
        writer->writeFromAudioSampleBuffer (b, 0, b.getNumSamples());
    }
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File out (argc > 1 ? argv[1] : "/tmp/bndt_out");
    out.createDirectory();
    const double sr = 48000.0;
    const int block = 256;

    BndtProcessor proc;
    proc.prepareToPlay (sr, block);

    const auto& presets = Presets::factory();
    juce::String trimLines;
    double totalRenderSec = 0.0, totalAudioSec = 0.0;
    std::printf ("%-22s %-9s %8s %8s %8s  %s\n", "preset", "category", "peak", "momMax", "rmsTail", "flags");

    for (int i = 0; i < (int) presets.size() + 1; ++i)
    {
        const bool random = i == (int) presets.size();
        if (auto* only = std::getenv ("BNDT_ONLY")) if (! juce::StringArray::fromTokens (only, ",", "").contains (juce::String (i))) continue;
        if (random) proc.randomise(); else proc.loadFactory (i);
        const auto& cat = random ? juce::String ("Random") : presets[(size_t) i].category;
        const bool mono = cat == "Bass" || cat == "Leads";

        const int total = (int) (sr * 7.0);
        juce::AudioBuffer<float> all (2, total);
        all.clear();
        juce::AudioBuffer<float> buf (2, block);

        // fresh engine/fx state per preset
        proc.prepareToPlay (sr, block);

        // speech-like source for the vocoder: buzzy glottal pulses through moving vowel formants, plus "s" bursts
        juce::AudioBuffer<float> speech (1, total);
        {
            const float vowels[4][3] = { { 730, 1090, 2440 }, { 270, 2290, 3010 }, { 300, 870, 2240 }, { 570, 840, 2410 } };
            dsp::SVF f1, f2, f3; dsp::Rng rr; float ph = 0.0f;
            for (int s = 0; s < total; ++s)
            {
                const double t = s / sr;
                const int syl = (int) (t / 0.25);
                const double inSyl = t - syl * 0.25;
                const auto* v = vowels[syl % 4];
                if (s % 64 == 0) { f1.set (v[0], 6, (float) sr); f2.set (v[1], 8, (float) sr); f3.set (v[2], 10, (float) sr); }
                ph += (float) ((120.0 + 15.0 * std::sin (t * 3.0)) / sr); if (ph > 1.0f) ph -= 1.0f;
                const float glot = ph < 0.1f ? 1.0f : -0.11f;
                float x = 0.0f;
                if (inSyl < 0.19) x = f1.bp (glot) + 0.6f * f2.bp (glot) + 0.3f * f3.bp (glot);
                else if (syl % 3 == 1) x = rr.bipolar() * 0.15f;   // "s"
                speech.setSample (0, s, x);
            }
            speech.applyGain (0.126f / std::max (1.0e-6f, speech.getRMSLevel (0, 0, total)));   // -18 dBFS RMS
        }

        struct Ev { double t; int note; bool on; };
        std::vector<Ev> evs;
        if (mono)
        {
            const int line[] = { 36, 36, 43, 39, 41, 36, 48, 46 };
            const int base = cat == "Leads" ? 24 : 0;
            for (int k = 0; k < 8; ++k) { evs.push_back ({ 0.35 * k, line[k] + base, true }); evs.push_back ({ 0.35 * k + 0.3, line[k] + base, false }); }
        }
        else
        {
            for (int n : { 48, 52, 55, 60 }) { evs.push_back ({ 0.0, n, true }); evs.push_back ({ 3.0, n, false }); }
            for (int k = 0; k < 4; ++k) { evs.push_back ({ 3.2 + 0.25 * k, 67 + k * 2, true }); evs.push_back ({ 3.4 + 0.25 * k, 67 + k * 2, false }); }
        }

        double heldSum = 0.0, tailSum = 0.0; int heldN = 0, tailN = 0; float peak = 0.0f; bool nan = false;
        const auto t0 = std::chrono::high_resolution_clock::now();
        for (int pos = 0; pos < total; pos += block)
        {
            const int n = std::min (block, total - pos);
            juce::MidiBuffer midi;
            for (const auto& e : evs)
            {
                const int s = (int) (e.t * sr);
                if (s >= pos && s < pos + n)
                    midi.addEvent (e.on ? juce::MidiMessage::noteOn (1, e.note, (juce::uint8) 100) : juce::MidiMessage::noteOff (1, e.note), s - pos);
            }
            buf.setSize (2, n, false, false, true);
            for (int c = 0; c < 2; ++c) buf.copyFrom (c, 0, speech, 0, pos, n);
            proc.processBlock (buf, midi);
            if (std::getenv ("BNDT_ONLY") && pos < block * 3)
                std::printf ("block %d peak %.4f first %.4f %.4f %.4f\n", pos / block, buf.getMagnitude (0, n), buf.getSample (0, 0), buf.getSample (0, 1), buf.getSample (0, 2));
            for (int c = 0; c < 2; ++c)
            {
                all.copyFrom (c, pos, buf, c, 0, n);
                for (int s = 0; s < n; ++s)
                {
                    const float x = buf.getSample (c, s);
                    if (! std::isfinite (x)) nan = true;
                    peak = std::max (peak, std::abs (x));
                    const double t = (pos + s) / sr;
                    if (t > 0.3 && t < 2.8) { heldSum += x * x; ++heldN; }
                    if (t > 6.0) { tailSum += x * x; ++tailN; }
                }
            }
        }
        const auto t1 = std::chrono::high_resolution_clock::now();
        totalRenderSec += std::chrono::duration<double> (t1 - t0).count();
        totalAudioSec += total / sr;

        float momMax = 0.0f;
        {
            const int win = (int) (0.4 * sr);
            for (int st = 0; st + win < total; st += win / 4)
            {
                double acc = 0.0;
                for (int c = 0; c < 2; ++c) for (int s = st; s < st + win; ++s) { const float x = all.getSample (c, s); acc += x * x; }
                momMax = std::max (momMax, (float) std::sqrt (acc / (2.0 * win)));
            }
        }
        const float rmsH = momMax;
        const float rmsT = (float) std::sqrt (tailSum / std::max (1, tailN));
        juce::String flags;
        if (nan) flags << "NaN ";
        if (peak < 0.05f) flags << "QUIET ";
        if (peak > 0.95f) flags << "HOT ";
        const auto name = random ? juce::String ("Surprise") : presets[(size_t) i].name;
        std::printf ("%-22s %-9s %8.1f %8.1f %8.1f  %s\n", name.toRawUTF8(), cat.toRawUTF8(),
                     juce::Decibels::gainToDecibels (peak), juce::Decibels::gainToDecibels (rmsH),
                     juce::Decibels::gainToDecibels (rmsT), flags.toRawUTF8());
        if (! random) trimLines << "TRIM\t" << name << "\t" << juce::String (juce::Decibels::gainToDecibels (momMax), 2) << "\t"
                                 << juce::String (proc.state.getRawParameterValue ("trim")->load(), 2) << "\n";
        writeWav (out.getChildFile (juce::String (i).paddedLeft ('0', 2) + "_" + juce::File::createLegalFileName (name) + ".wav"), all, sr);
    }
    out.getChildFile ("trims.tsv").replaceWithText (trimLines);
    std::printf ("\nCPU: %.1f%% of real time (single thread, 4-note chords)\n", 100.0 * totalRenderSec / totalAudioSec);

    // UI snapshot at full design size
    {
        proc.loadFactory (0);
        for (int i = 0; i < (int) presets.size(); ++i) if (presets[(size_t) i].name == "Melancholy Keys") proc.loadFactory (i);
        std::unique_ptr<BndtEditor> ed (dynamic_cast<BndtEditor*> (proc.createEditor()));
        ed->setSize (BndtContent::W, BndtContent::H);
        auto& content = ed->getContent();
        auto img = content.createComponentSnapshot (content.getLocalBounds(), true, 1.0f);
        juce::PNGImageFormat png;
        auto f = out.getChildFile ("ui.png");
        f.deleteFile();
        if (auto s = f.createOutputStream()) png.writeImageToStream (img, *s);
        auto img2 = content.createComponentSnapshot (juce::Rectangle<int> (0, 0, 520, 330), true, 2.0f);
        auto f2 = out.getChildFile ("ui_zoom.png");
        f2.deleteFile();
        if (auto s = f2.createOutputStream()) png.writeImageToStream (img2, *s);
        std::printf ("UI snapshot written\n");
    }
    return 0;
}
