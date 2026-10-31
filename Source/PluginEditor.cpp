#include "PluginEditor.h"
#include "Params.h"
#include "Presets.h"

using namespace P;

BndtContent::BndtContent (BndtProcessor& p)
    : proc (p), st (p.state),
      osc1Wave (st, P::osc1Wave), osc2Wave (st, P::osc2Wave), tapeSound (st, P::tapeSound),
      lfo1Shape (st, P::lfo1Shape), lfo1Dest (st, P::lfo1Dest), lfo2Shape (st, P::lfo2Shape), lfo2Dest (st, P::lfo2Dest),
      voiceMode (st, P::voiceMode), strumChord (st, P::strumChord), delayDiv (st, P::delayDiv), delayHeads (st, P::delayHeads),
      filterType (st, P::filterType, 2), satType (st, P::satType, 2), chorusMode (st, P::chorusMode, 2), revType (st, P::revType, 2),
      vocCarrier (st, P::vocCarrier, 2), vocBands (st, P::vocBands, 4),
      tapeLimit (st, P::tapeLimit, "8 SEC"), strumOn (st, P::strumOn, "STRUM"), satOn (st, P::satOn, "ON"),
      delayOn (st, P::delayOn, "ON"), delaySync (st, P::delaySync, "SYNC"), revOn (st, P::revOn, "ON"), radioOn (st, P::radioOn, "ON"), vocOn (st, P::vocOn, "ON"),
      ampEnv (st, P::ampA, P::ampD, P::ampS, P::ampR), fltEnv (st, P::fltA, P::fltD, P::fltS, P::fltR),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&lnf);
    background = Theme::image ("background_jpg");

    for (auto* c : { &pOsc, &pTape, &pFilter, &pVoice, &pAmp, &pFEnv, &pLfo1, &pLfo2, &pSat, &pChorus, &pEcho, &pRev, &pRadio, &pVoc, &pKeys, &pOut })
        addAndMakeVisible (c);
    pKeys.setOutlineOnly (true);
    pOut.setOutlineOnly (true);
    pVoice.setRightGap (96.0f);
    for (auto* pp : { &pSat, &pEcho, &pRev, &pRadio, &pVoc }) pp->setRightGap (72.0f);
    addAndMakeVisible (logo);
    logo.setInterceptsMouseClicks (false, false);

    knob (P::osc1Oct, "OCTAVE"); knob (P::osc1PW, "WIDTH"); knob (P::osc1Level, "LEVEL"); knob (P::subLevel, "SUB"); knob (P::noiseLevel, "NOISE");
    knob (P::osc2Oct, "OCTAVE"); knob (P::osc2Semi, "SEMI"); knob (P::osc2Detune, "DETUNE"); knob (P::osc2Level, "LEVEL"); knob (P::drift, "DRIFT");
    knob (P::tapeLevel, "LEVEL"); knob (P::tapeWow, "WOW"); knob (P::tapeAge, "AGE");
    knob (P::cutoff, "CUTOFF"); knob (P::resonance, "RESO"); knob (P::filterDrive, "DRIVE"); knob (P::envAmount, "ENV");
    knob (P::keyTrack, "KEY"); knob (P::velFilter, "VEL");
    knob (P::glide, "GLIDE"); knob (P::strumSpeed, "SPEED"); knob (P::strumSpan, "NOTES");
    knob (P::ampA, "A"); knob (P::ampD, "D"); knob (P::ampS, "S"); knob (P::ampR, "R"); knob (P::velAmp, "VEL");
    knob (P::fltA, "A"); knob (P::fltD, "D"); knob (P::fltS, "S"); knob (P::fltR, "R");
    knob (P::lfo1Rate, "RATE"); knob (P::lfo1Amount, "AMOUNT"); knob (P::lfo1Fade, "FADE IN");
    knob (P::lfo2Rate, "RATE"); knob (P::lfo2Amount, "AMOUNT"); knob (P::lfo2Fade, "FADE IN");
    knob (P::satDrive, "DRIVE"); knob (P::satMix, "MIX");
    knob (P::delayTime, "TIME"); knob (P::delayFeedback, "REPEATS"); knob (P::delayWow, "WOW"); knob (P::delayTone, "TONE"); knob (P::delayMix, "MIX");
    knob (P::revSize, "SIZE"); knob (P::revTone, "TONE"); knob (P::revMix, "MIX");
    knob (P::radioAmount, "AMOUNT"); knob (P::radioTune, "TUNING");
    knob (P::vocInput, "INPUT"); knob (P::vocFormant, "FORMANT"); knob (P::vocResponse, "RESPONSE");
    knob (P::vocSibilance, "SIBILANCE"); knob (P::vocMix, "MIX");
    knob (P::master, "MASTER"); knob (P::lowCut, "LOW CUT");

    for (juce::Component* c : std::initializer_list<juce::Component*> {
             &osc1Wave, &osc2Wave, &tapeSound, &lfo1Shape, &lfo1Dest, &lfo2Shape, &lfo2Dest, &voiceMode, &strumChord, &delayDiv, &delayHeads,
             &filterType, &satType, &chorusMode, &revType, &tapeLimit, &strumOn, &satOn, &delayOn, &delaySync, &revOn, &radioOn,
             &ampEnv, &fltEnv, &keyboard, &vocCarrier, &vocBands, &vocMeter, &vocOn, &jewel, &prevBtn, &nextBtn, &saveBtn, &surpriseBtn })
        addAndMakeVisible (c);

    keyboard.setAvailableRange (36, 96);
    keyboard.setOctaveForMiddleC (4);
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, Theme::cream);
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff1b1a1a));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0x55000000));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, Theme::teal.withAlpha (0.3f));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, Theme::pink.withAlpha (0.8f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colour (0x66000000));
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, Theme::panel);
    keyboard.setScrollButtonsVisible (false);

    prevBtn.onClick = [this] { stepPreset (-1); };
    nextBtn.onClick = [this] { stepPreset (1); };
    saveBtn.onClick = [this] { savePreset(); };
    surpriseBtn.onClick = [this] { proc.randomise(); };
    prevBtn.setTooltip ("Previous preset");
    surpriseBtn.setTooltip ("Roll a brand new sound");

    startTimerHz (30);
}

BndtContent::~BndtContent()
{
    setLookAndFeel (nullptr);
}

Knob& BndtContent::knob (const char* id, const char* label)
{
    knobs.push_back (std::make_unique<Knob> (st, id, label));
    auto& k = *knobs.back();
    addAndMakeVisible (k);
    knobById[id] = &k;
    return k;
}

void BndtContent::paint (juce::Graphics& g)
{
    // the photo from the bndt artwork, logo removed, as the faceplate
    if (! backdrop.isValid()) buildBackdrop();
    g.drawImageAt (backdrop, 0, 0);

    // byline
    g.setFont (Theme::font (22.0f));
    Theme::haloText (g, "BY SWITCHKICKER", { 262.0f, 56.0f, 220.0f, 26.0f }, juce::Justification::centredLeft, Theme::label);

    // preset display
    {
        auto r = juce::Rectangle<float> (476.0f, 22.0f, 292.0f, 54.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.5f), 12, { 2, 4 }).drawForRectangle (g, r.toNearestInt());
        g.setColour (juce::Colour (0xff0a0a0a));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (juce::Colour (0xff3a3838));
        g.drawRoundedRectangle (r, 6.0f, 1.2f);
        g.setFont (Theme::font (16.0f));
        g.setColour (Theme::pink);
        g.drawText (proc.getPresetCategory().toUpperCase(), r.reduced (12.0f, 4.0f).removeFromTop (18.0f).toNearestInt(), juce::Justification::centredLeft, false);
        g.setFont (Theme::font (25.0f));
        g.setColour (Theme::teal);
        g.drawFittedText (proc.getPresetName(), r.reduced (12.0f, 4.0f).withTrimmedTop (14.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.8f);
        juce::Path tri;
        tri.addTriangle (r.getRight() - 20.0f, r.getCentreY() - 2.0f, r.getRight() - 10.0f, r.getCentreY() - 2.0f, r.getRight() - 15.0f, r.getCentreY() + 4.0f);
        g.setColour (Theme::pink.withAlpha (0.8f));
        g.fillPath (tri);
    }

    // small captions
    g.setFont (Theme::font (16.0f));
    Theme::haloText (g, "OSC 1", { 30.0f, 140.0f, 84.0f, 20.0f }, juce::Justification::centredLeft, Theme::label, 0.75f);
    Theme::haloText (g, "OSC 2", { 30.0f, 224.0f, 84.0f, 20.0f }, juce::Justification::centredLeft, Theme::label, 0.75f);
    Theme::haloText (g, "SOURCE  (SIDECHAIN / AUDIO IN)", { 30.0f, 750.0f, 260.0f, 20.0f }, juce::Justification::centredLeft, Theme::label, 0.75f);
    Theme::haloText (g, "CARRIER", { 330.0f, 750.0f, 200.0f, 20.0f }, juce::Justification::centredLeft, Theme::label, 0.75f);
    Theme::haloText (g, "BANDS", { 330.0f, 810.0f, 200.0f, 20.0f }, juce::Justification::centredLeft, Theme::label, 0.75f);
    g.setFont (Theme::font (17.0f));
    Theme::haloText (g, "Send a vocal or any track to bndt's", { 1000.0f, 790.0f, 260.0f, 20.0f }, juce::Justification::centredLeft, Theme::label, 0.8f);
    Theme::haloText (g, "sidechain input, then play the keys.", { 1000.0f, 810.0f, 260.0f, 20.0f }, juce::Justification::centredLeft, Theme::label, 0.8f);
    Theme::haloText (g, "Standalone: uses your audio input.", { 1000.0f, 836.0f, 260.0f, 20.0f }, juce::Justification::centredLeft, Theme::label, 0.6f);
}

// The faceplate is the photo itself; sections are drawn as engraved lines on top of it.
void BndtContent::buildBackdrop()
{
    backdrop = juce::Image (juce::Image::RGB, W, H, false);
    juce::Graphics g (backdrop);
    if (background.isValid()) g.drawImage (background, backdrop.getBounds().toFloat(), juce::RectanglePlacement::fillDestination);
    else g.fillAll (Theme::mint);
}

void BndtContent::resized()
{
    backdrop = {};
    auto place = [this] (const char* id, int x, int y, int w = 65, int h = 84)
    {
        if (auto* k = knobById[id]) k->setBounds (x, y, w, h);
    };
    auto row = [&] (std::initializer_list<const char*> ids, int x, int y, int totalW, int kw = 65, int kh = 84)
    {
        const int n = (int) ids.size();
        const float step = (float) totalW / (float) n;
        int i = 0;
        for (auto* id : ids)
        {
            const int cx = x + (int) (step * ((float) i + 0.5f));
            place (id, cx - kw / 2, y, kw, kh);
            ++i;
        }
    };

    // header
    logo.setBounds (16, 8, 244, 78);
    prevBtn.setBounds (436, 32, 34, 34);
    nextBtn.setBounds (774, 32, 34, 34);
    saveBtn.setBounds (818, 32, 70, 34);
    surpriseBtn.setBounds (896, 32, 108, 34);
    pOut.setBounds (1018, 6, 250, 90);
    jewel.setBounds (1030, 28, 44, 44);
    place (P::master, 1088, 8, 76, 84);
    place (P::lowCut, 1172, 8, 76, 84);

    // row A
    pOsc.setBounds (16, 100, 440, 212);
    osc1Wave.setBounds (28, 162, 84, 28);
    row ({ P::osc1Oct, P::osc1PW, P::osc1Level, P::subLevel, P::noiseLevel }, 116, 134, 330);
    osc2Wave.setBounds (28, 246, 84, 28);
    row ({ P::osc2Oct, P::osc2Semi, P::osc2Detune, P::osc2Level, P::drift }, 116, 218, 330);

    pTape.setBounds (472, 100, 230, 212);
    tapeSound.setBounds (484, 144, 124, 30);
    tapeLimit.setBounds (616, 142, 80, 34);
    row ({ P::tapeLevel, P::tapeWow, P::tapeAge }, 484, 204, 206);

    pFilter.setBounds (718, 100, 300, 212);
    filterType.setBounds (730, 146, 140, 30);
    place (P::cutoff, 872, 124, 72, 88);
    place (P::resonance, 942, 124, 72, 88);
    row ({ P::filterDrive, P::envAmount, P::keyTrack, P::velFilter }, 730, 218, 276);

    pVoice.setBounds (1034, 100, 230, 212);
    strumOn.setBounds (1170, 104, 88, 32);
    voiceMode.setBounds (1046, 146, 100, 30);
    strumChord.setBounds (1152, 146, 100, 30);
    row ({ P::glide, P::strumSpeed, P::strumSpan }, 1046, 204, 206);

    // row B
    pAmp.setBounds (16, 322, 260, 196);
    ampEnv.setBounds (28, 360, 236, 54);
    row ({ P::ampA, P::ampD, P::ampS, P::ampR, P::velAmp }, 28, 418, 236, 50, 80);

    pFEnv.setBounds (292, 322, 260, 196);
    fltEnv.setBounds (304, 360, 236, 54);
    row ({ P::fltA, P::fltD, P::fltS, P::fltR }, 304, 418, 236, 56, 80);

    pLfo1.setBounds (568, 322, 340, 196);
    lfo1Shape.setBounds (580, 364, 150, 30);
    lfo1Dest.setBounds (740, 364, 156, 30);
    row ({ P::lfo1Rate, P::lfo1Amount, P::lfo1Fade }, 580, 412, 316);

    pLfo2.setBounds (924, 322, 340, 196);
    lfo2Shape.setBounds (936, 364, 150, 30);
    lfo2Dest.setBounds (1096, 364, 156, 30);
    row ({ P::lfo2Rate, P::lfo2Amount, P::lfo2Fade }, 936, 412, 316);

    // row C (effects)
    pSat.setBounds (16, 528, 210, 176);
    satOn.setBounds (156, 532, 64, 32);
    satType.setBounds (28, 570, 186, 30);
    row ({ P::satDrive, P::satMix }, 28, 608, 186);

    pChorus.setBounds (242, 528, 140, 176);
    chorusMode.setBounds (252, 572, 120, 120);

    pEcho.setBounds (398, 528, 400, 176);
    delayOn.setBounds (728, 532, 64, 32);
    delayHeads.setBounds (410, 570, 112, 30);
    delaySync.setBounds (532, 568, 76, 34);
    delayDiv.setBounds (612, 570, 96, 30);
    row ({ P::delayTime, P::delayFeedback, P::delayWow, P::delayTone, P::delayMix }, 410, 608, 376);

    pRev.setBounds (814, 528, 250, 176);
    revOn.setBounds (994, 532, 64, 32);
    revType.setBounds (826, 570, 226, 30);
    row ({ P::revSize, P::revTone, P::revMix }, 826, 608, 226);

    pRadio.setBounds (1080, 528, 184, 176);
    radioOn.setBounds (1194, 532, 64, 32);
    row ({ P::radioAmount, P::radioTune }, 1092, 590, 160, 72, 92);

    // keyboard
    // row D (vocoder)
    pVoc.setBounds (16, 712, 1248, 176);
    vocOn.setBounds (1194, 716, 64, 32);
    vocMeter.setBounds (30, 774, 180, 16);
    place (P::vocInput, 214, 752, 76, 92);
    vocCarrier.setBounds (330, 772, 230, 30);
    vocBands.setBounds (330, 832, 230, 30);
    row ({ P::vocFormant, P::vocResponse, P::vocSibilance, P::vocMix }, 590, 772, 390);

    pKeys.setBounds (16, 894, 1248, 102);
    keyboard.setBounds (26, 902, 1228, 86);
    keyboard.setKeyWidth (1228.0f / 36.0f);
    keyboard.setLowestVisibleKey (36);
}

void BndtContent::timerCallback()
{
    jewel.setLevel (proc.outputLevel.load());
    vocMeter.setLevel (proc.getVocoderInputLevel());
    const int v = proc.presetVersion.load();
    if (v != lastPresetVersion) { lastPresetVersion = v; repaint (476, 22, 292, 54); }

    const bool sync = st.getRawParameterValue (P::delaySync)->load() > 0.5f;
    if (auto* k = knobById[P::delayTime]) k->slider.setEnabled (! sync);
    delayDiv.setEnabled (sync);
    delayDiv.setAlpha (sync ? 1.0f : 0.35f);
}

void BndtContent::stepPreset (int delta)
{
    const int n = (int) Presets::factory().size();
    const int cur = proc.getCurrentProgram();
    proc.loadFactory (cur < 0 ? 0 : (cur + delta + n) % n);
}

void BndtContent::showPresetMenu()
{
    juce::PopupMenu m;
    const auto& f = Presets::factory();
    for (const auto& cat : Presets::categories())
    {
        juce::PopupMenu sub;
        for (int i = 0; i < (int) f.size(); ++i)
            if (f[(size_t) i].category == cat)
                sub.addItem (i + 1, f[(size_t) i].name, true, i == proc.getCurrentProgram());
        m.addSubMenu (cat, sub);
    }
    const auto user = Presets::userPresets();
    juce::PopupMenu um;
    for (int i = 0; i < user.size(); ++i) um.addItem (1000 + i, user[i].getFileNameWithoutExtension());
    if (user.isEmpty()) um.addItem (999, "(none saved yet)", false);
    m.addSeparator();
    m.addSubMenu ("User", um);
    m.addItem (2000, "Open presets folder");

    juce::Component::SafePointer<BndtContent> safe (this);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (localAreaToGlobal (juce::Rectangle<int> (476, 22, 292, 54))),
                     [safe, user] (int r)
                     {
                         if (safe == nullptr || r <= 0) return;
                         if (r == 2000) Presets::userFolder().startAsProcess();
                         else if (r >= 1000) safe->proc.loadUserPreset (user[r - 1000]);
                         else safe->proc.loadFactory (r - 1);
                     });
}

void BndtContent::savePreset()
{
    auto* w = new juce::AlertWindow ("Save preset", "Name this sound", juce::MessageBoxIconType::NoIcon, this);
    w->setLookAndFeel (&lnf);
    w->addTextEditor ("name", proc.getPresetName(), "");
    w->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    w->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    juce::Component::SafePointer<BndtContent> safe (this);
    w->enterModalState (true, juce::ModalCallbackFunction::create ([safe, w] (int result)
    {
        if (safe != nullptr && result == 1)
        {
            const auto name = w->getTextEditorContents ("name").trim();
            if (name.isNotEmpty()) safe->proc.saveUserPreset (name, "User");
        }
    }), true);
}

// ============================================================================
BndtEditor::BndtEditor (BndtProcessor& p) : AudioProcessorEditor (p), content (p)
{
    addAndMakeVisible (content);
    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio ((double) BndtContent::W / (double) BndtContent::H);
    setResizeLimits (768, 600, 1920, 1500);
    setSize (1024, 800);
}

void BndtEditor::resized()
{
    content.setBounds (0, 0, BndtContent::W, BndtContent::H);
    content.setTransform (juce::AffineTransform::scale ((float) getWidth() / (float) BndtContent::W));
}
