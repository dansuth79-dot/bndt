#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Params.h"
#include "Presets.h"

BndtProcessor::BndtProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), false)
                          .withInput ("Sidechain", juce::AudioChannelSet::stereo(), true)   // vocoder source
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      state (*this, nullptr, "BNDT", P::createLayout())
{
    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            raw[rp->getParameterID().toStdString()] = state.getRawParameterValue (rp->getParameterID());

    // open on something warm
    const auto& f = Presets::factory();
    for (int i = 0; i < (int) f.size(); ++i)
        if (f[(size_t) i].name == "Melancholy Keys") { loadFactory (i); break; }
}

bool BndtProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
    for (int i = 0; i < layouts.inputBuses.size(); ++i)
    {
        const auto set = layouts.getChannelSet (true, i);
        if (! set.isDisabled() && set != juce::AudioChannelSet::mono() && set != juce::AudioChannelSet::stereo()) return false;
    }
    return true;
}

void BndtProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    engine.prepare (sampleRate, samplesPerBlock);
    fx.prepare (sampleRate, samplesPerBlock);
    sideBuf.setSize (2, samplesPerBlock, false, true, true);
    readParams();
}

float BndtProcessor::getF (const char* id) const { return raw.at (id)->load(); }
int BndtProcessor::getI (const char* id) const { return (int) std::lround (raw.at (id)->load()); }
bool BndtProcessor::getB (const char* id) const { return raw.at (id)->load() > 0.5f; }

void BndtProcessor::readParams()
{
    using namespace P;
    vp.osc1Wave = getI (osc1Wave); vp.osc1Oct = getI (osc1Oct); vp.osc1PW = getF (osc1PW); vp.osc1Level = getF (osc1Level);
    vp.osc2Wave = getI (osc2Wave); vp.osc2Oct = getI (osc2Oct); vp.osc2Semi = getI (osc2Semi);
    vp.osc2Detune = getF (osc2Detune); vp.osc2Level = getF (osc2Level);
    vp.subLevel = getF (subLevel); vp.noiseLevel = getF (noiseLevel); vp.drift = getF (drift);
    vp.tapeSound = getI (tapeSound); vp.tapeLevel = getF (tapeLevel); vp.tapeWow = getF (tapeWow);
    vp.tapeAge = getF (tapeAge); vp.tapeLimit = getB (tapeLimit);
    vp.filterType = getI (filterType); vp.cutoff = getF (cutoff); vp.resonance = getF (resonance);
    vp.filterDrive = getF (filterDrive); vp.envAmount = getF (envAmount); vp.keyTrack = getF (keyTrack); vp.velFilter = getF (velFilter);
    vp.ampA = getF (ampA); vp.ampD = getF (ampD); vp.ampS = getF (ampS); vp.ampR = getF (ampR); vp.velAmp = getF (velAmp);
    vp.fltA = getF (fltA); vp.fltD = getF (fltD); vp.fltS = getF (fltS); vp.fltR = getF (fltR);
    vp.lfoShape[0] = getI (lfo1Shape); vp.lfoRate[0] = getF (lfo1Rate); vp.lfoDest[0] = getI (lfo1Dest);
    vp.lfoAmount[0] = getF (lfo1Amount); vp.lfoFade[0] = getF (lfo1Fade);
    vp.lfoShape[1] = getI (lfo2Shape); vp.lfoRate[1] = getF (lfo2Rate); vp.lfoDest[1] = getI (lfo2Dest);
    vp.lfoAmount[1] = getF (lfo2Amount); vp.lfoFade[1] = getF (lfo2Fade);
    vp.voiceMode = getI (voiceMode); vp.glide = getF (glide);
    vp.strumOn = getB (strumOn); vp.strumChord = getI (strumChord); vp.strumSpeed = getF (strumSpeed); vp.strumSpan = getI (strumSpan);

    fp.satOn = getB (satOn); fp.satType = getI (satType); fp.satDrive = getF (satDrive); fp.satMix = getF (satMix);
    fp.chorusMode = getI (chorusMode);
    fp.delayOn = getB (delayOn); fp.delayTimeMs = getF (delayTime); fp.delaySync = getB (delaySync); fp.delayDiv = getI (delayDiv);
    fp.delayHeads = getI (delayHeads); fp.delayFeedback = getF (delayFeedback); fp.delayWow = getF (delayWow);
    fp.delayTone = getF (delayTone); fp.delayMix = getF (delayMix);
    fp.revOn = getB (revOn); fp.revType = getI (revType); fp.revSize = getF (revSize); fp.revTone = getF (revTone); fp.revMix = getF (revMix);
    fp.radioOn = getB (radioOn); fp.radioAmount = getF (radioAmount); fp.radioTune = getF (radioTune);
    fp.voc.on = getB (vocOn); fp.voc.bands = 8 * (getI (vocBands) + 1); fp.voc.noiseCarrier = getI (vocCarrier) == 1;
    fp.voc.inputDb = getF (vocInput); fp.voc.formant = getF (vocFormant); fp.voc.responseMs = getF (vocResponse);
    fp.voc.sibilance = getF (vocSibilance); fp.voc.mix = getF (vocMix);
    fp.lowCut = getF (lowCut); fp.masterDb = getF (master); fp.trimDb = getF (trim);
}

void BndtProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();

    // grab the vocoder source (sidechain, or the audio input in the standalone app) before the buffer is reused
    if (sideBuf.getNumSamples() < n) sideBuf.setSize (2, n, false, true, true);
    sideBuf.clear();
    for (int b = 0; b < getBusCount (true); ++b)
    {
        auto* bus = getBus (true, b);
        if (bus == nullptr || ! bus->isEnabled() || bus->getNumberOfChannels() == 0) continue;
        auto in = getBusBuffer (buffer, true, b);
        const int ch = in.getNumChannels();
        for (int c = 0; c < 2; ++c)
            sideBuf.addFrom (c, 0, in, juce::jmin (c, ch - 1), 0, n);
    }
    buffer.clear();
    keyboardState.processNextMidiBuffer (midi, 0, n, true);

    readParams();
    engine.setParams (vp);

    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto bpm = pos->getBpm()) fp.bpm = *bpm;

    float* l = buffer.getWritePointer (0);
    float* r = buffer.getWritePointer (1);
    int pos = 0;
    for (const auto meta : midi)
    {
        const int at = juce::jlimit (0, n, meta.samplePosition);
        if (at > pos) { engine.render (l + pos, r + pos, at - pos); pos = at; }
        engine.handleMidi (meta.getMessage());
    }
    if (pos < n) engine.render (l + pos, r + pos, n - pos);

    fx.process (buffer, n, fp, sideBuf.getReadPointer (0), sideBuf.getReadPointer (1));

    const float pk = buffer.getMagnitude (0, n);
    outputLevel.store (std::max (pk, outputLevel.load() * 0.85f));
}

int BndtProcessor::getNumPrograms() { return (int) Presets::factory().size(); }

void BndtProcessor::setCurrentProgram (int index) { loadFactory (index); }

const juce::String BndtProcessor::getProgramName (int index)
{
    const auto& f = Presets::factory();
    return juce::isPositiveAndBelow (index, (int) f.size()) ? f[(size_t) index].name : juce::String();
}

void BndtProcessor::loadFactory (int index)
{
    const auto& f = Presets::factory();
    if (! juce::isPositiveAndBelow (index, (int) f.size())) return;
    Presets::applyFactory (state, index);
    currentFactory = index;
    presetName = f[(size_t) index].name;
    presetCategory = f[(size_t) index].category;
    ++presetVersion;
}

void BndtProcessor::loadUserPreset (const juce::File& file)
{
    if (Presets::loadUser (state, file))
    {
        presetName = file.getFileNameWithoutExtension();
        presetCategory = "User";
        currentFactory = -1;
        ++presetVersion;
    }
}

void BndtProcessor::saveUserPreset (const juce::String& name, const juce::String& category)
{
    if (Presets::saveUser (state, name, category))
    {
        presetName = name;
        presetCategory = "User";
        currentFactory = -1;
        ++presetVersion;
    }
}

void BndtProcessor::randomise()
{
    Presets::randomise (state, random);
    presetName = "Surprise";
    presetCategory = "Random";
    currentFactory = -1;
    ++presetVersion;
}

void BndtProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto s = state.copyState();
    s.setProperty ("presetName", presetName, nullptr);
    s.setProperty ("presetCategory", presetCategory, nullptr);
    s.setProperty ("factoryIndex", currentFactory, nullptr);
    if (auto xml = s.createXml()) copyXmlToBinary (*xml, destData);
}

void BndtProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (state.state.getType()))
        {
            auto s = juce::ValueTree::fromXml (*xml);
            presetName = s.getProperty ("presetName", "Init").toString();
            presetCategory = s.getProperty ("presetCategory", "").toString();
            currentFactory = (int) s.getProperty ("factoryIndex", -1);
            state.replaceState (s);
            ++presetVersion;
        }
}

juce::AudioProcessorEditor* BndtProcessor::createEditor() { return new BndtEditor (*this); }

#ifndef BNDT_TEST
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new BndtProcessor(); }
#endif
