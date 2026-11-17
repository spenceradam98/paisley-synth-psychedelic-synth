#include "PluginEditor.h"

// =========================================================== ParamControl
ParamControl::ParamControl (juce::AudioProcessorValueTreeState& s, const juce::String& id, const juce::String& text)
{
    auto* param = s.getParameter (id);
    jassert (param != nullptr);
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (11.5f));
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);

    if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (param))
    {
        combo = std::make_unique<juce::ComboBox>();
        combo->addItemList (choice->choices, 1);
        addAndMakeVisible (*combo);
        cAtt = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (s, id, *combo);
    }
    else if (dynamic_cast<juce::AudioParameterBool*> (param) != nullptr)
    {
        toggle = std::make_unique<juce::ToggleButton>();
        addAndMakeVisible (*toggle);
        bAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, id, *toggle);
    }
    else
    {
        slider = std::make_unique<juce::Slider> (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox);
        slider->setPopupDisplayEnabled (true, true, nullptr);
        slider->setTooltip (param->getName (64));
        addAndMakeVisible (*slider);
        sAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, id, *slider);
        slider->setDoubleClickReturnValue (true, (double) dynamic_cast<juce::RangedAudioParameter*> (param)
                                                     ->convertFrom0to1 (param->getDefaultValue()));
    }
}

void ParamControl::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromBottom (16));
    if (slider) slider->setBounds (r.withSizeKeepingCentre (juce::jmin (r.getWidth(), r.getHeight()), juce::jmin (r.getWidth(), r.getHeight())));
    if (combo)  combo->setBounds (r.withSizeKeepingCentre (r.getWidth() - 2, 22));
    if (toggle) toggle->setBounds (r.withSizeKeepingCentre (r.getWidth(), 24));
}

// ================================================================ Section
Section::Section (juce::AudioProcessorValueTreeState& s, const juce::String& t,
                  std::vector<std::pair<const char*, const char*>> list, const char* powerId)
    : apvts (s), title (t)
{
    for (auto& [id, name] : list)
        addAndMakeVisible (*controls.emplace_back (std::make_unique<ParamControl> (s, id, name)));
    if (powerId != nullptr)
    {
        powerParam = powerId;
        power = std::make_unique<juce::ToggleButton>();
        power->setTooltip ("Switch " + title + " on/off");
        addAndMakeVisible (*power);
        powerAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, powerParam, *power);
        apvts.addParameterListener (powerParam, this);
    }
}

Section::~Section() { if (powerParam.isNotEmpty()) apvts.removeParameterListener (powerParam, this); }

void Section::parameterChanged (const juce::String&, float)
{
    juce::MessageManager::callAsync ([sp = juce::Component::SafePointer<Section> (this)] { if (sp) sp->repaint(); });
}

void Section::paint (juce::Graphics& g)
{
    if (auto* l = dynamic_cast<PaisleyLookAndFeel*> (&getLookAndFeel()))
    {
        const bool active = powerParam.isEmpty() || apvts.getRawParameterValue (powerParam)->load() > 0.5f;
        l->drawPanel (g, getLocalBounds().toFloat(), title, active);
    }
}

void Section::resized()
{
    auto r = getLocalBounds().reduced (6, 4);
    auto header = r.removeFromTop (20);
    if (power) power->setBounds (header.removeFromRight (40));
    if (controls.empty()) return;
    const float w = (float) r.getWidth() / (float) controls.size();
    for (size_t i = 0; i < controls.size(); ++i)
        controls[i]->setBounds (juce::Rectangle<float> (r.getX() + w * (float) i, (float) r.getY(), w, (float) r.getHeight()).toNearestInt().reduced (2, 0));
}

// ================================================================= Editor
PaisleyEditor::PaisleyEditor (PaisleyProcessor& p)
    : AudioProcessorEditor (p), proc (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setLookAndFeel (&laf);
    auto& s = p.apvts;
    auto add = [&] (int row, std::unique_ptr<Section> sec) { content.addAndMakeVisible (*sec); rows[row].push_back (std::move (sec)); };

    // ---- Row 0: oscillators + mixer
    for (int o = 1; o <= 2; ++o)
    {
        static std::vector<std::string> ids[3];
        auto pre = std::string ("osc") + std::to_string (o);
        auto& v = ids[o];
        v = { pre + "Wave", pre + "Oct", pre + "Semi", pre + "Fine", pre + "Shape", pre + "Level", pre + "Unison", pre + "Detune" };
        add (0, std::make_unique<Section> (s, "Oscillator " + juce::String (o), std::vector<std::pair<const char*, const char*>> {
            { v[0].c_str(), "Wave" }, { v[1].c_str(), "Octave" }, { v[2].c_str(), "Semi" }, { v[3].c_str(), "Fine" },
            { v[4].c_str(), "Shape" }, { v[5].c_str(), "Level" }, { v[6].c_str(), "Unison" }, { v[7].c_str(), "Detune" } }));
    }
    add (0, std::make_unique<Section> (s, "Mixer", std::vector<std::pair<const char*, const char*>> {
        { "subLevel", "Sub" }, { "noiseLevel", "Noise" }, { "ringLevel", "Ring" } }));

    // ---- Row 1: filter + envelopes
    add (1, std::make_unique<Section> (s, "Filter", std::vector<std::pair<const char*, const char*>> {
        { "filtMode", "Mode" }, { "cutoff", "Cutoff" }, { "reso", "Reso" }, { "filtDrive", "Drive" },
        { "filtEnv", "Env Amt" }, { "keyTrack", "Key Trk" }, { "filtVel", "Velocity" } }));
    add (1, std::make_unique<Section> (s, "Filter Envelope", std::vector<std::pair<const char*, const char*>> {
        { "fA", "Attack" }, { "fD", "Decay" }, { "fS", "Sustain" }, { "fR", "Release" } }));
    add (1, std::make_unique<Section> (s, "Amp Envelope", std::vector<std::pair<const char*, const char*>> {
        { "aA", "Attack" }, { "aD", "Decay" }, { "aS", "Sustain" }, { "aR", "Release" }, { "ampVel", "Velocity" } }));

    // ---- Row 2: modulation + voice
    add (2, std::make_unique<Section> (s, "LFO 1  (vibrato / sweep)", std::vector<std::pair<const char*, const char*>> {
        { "lfo1Wave", "Wave" }, { "lfo1Rate", "Rate" }, { "lfo1Delay", "Fade In" }, { "lfo1Pitch", "> Pitch" }, { "lfo1Cutoff", "> Cutoff" } }));
    add (2, std::make_unique<Section> (s, "LFO 2  (motion)", std::vector<std::pair<const char*, const char*>> {
        { "lfo2Wave", "Wave" }, { "lfo2Rate", "Rate" }, { "lfo2Shape", "> Shape" }, { "lfo2Pan", "> Pan" }, { "lfo2Amp", "> Amp" } }));
    add (2, std::make_unique<Section> (s, "Voice", std::vector<std::pair<const char*, const char*>> {
        { "voiceMode", "Mode" }, { "glide", "Glide" }, { "drift", "Drift" }, { "warble", "Warble" },
        { "spread", "Spread" }, { "bendRange", "Bend" }, { "master", "Master" } }));

    // ---- Row 3: FX part 1
    add (3, std::make_unique<Section> (s, "Drive", std::vector<std::pair<const char*, const char*>> {
        { "drvType", "Type" }, { "drvAmt", "Amount" }, { "drvTone", "Tone" }, { "drvMix", "Mix" } }, "drvOn"));
    add (3, std::make_unique<Section> (s, "Phaser", std::vector<std::pair<const char*, const char*>> {
        { "phsRate", "Rate" }, { "phsDepth", "Depth" }, { "phsFb", "Feedback" }, { "phsMix", "Mix" } }, "phsOn"));
    add (3, std::make_unique<Section> (s, "Chorus / Flanger", std::vector<std::pair<const char*, const char*>> {
        { "choMode", "Mode" }, { "choRate", "Rate" }, { "choDepth", "Depth" }, { "choFb", "Feedback" }, { "choMix", "Mix" } }, "choOn"));
    add (3, std::make_unique<Section> (s, "Tremolo", std::vector<std::pair<const char*, const char*>> {
        { "trmShape", "Shape" }, { "trmRate", "Rate" }, { "trmDepth", "Depth" }, { "trmPan", "Auto-Pan" } }, "trmOn"));
    add (3, std::make_unique<Section> (s, "Tape", std::vector<std::pair<const char*, const char*>> {
        { "tapWow", "Wow" }, { "tapFlutter", "Flutter" }, { "tapSat", "Saturate" }, { "tapAge", "Age" } }, "tapOn"));

    // ---- Row 4: delay chain + reverb
    for (auto d : { "A", "B" })
    {
        static std::vector<std::string> ids[2];
        const int k = juce::String (d) == "A" ? 0 : 1;
        auto pre = std::string ("dly") + d;
        auto& v = ids[k];
        v = { pre + "Sync", pre + "Div", pre + "Ms", pre + "Fb", pre + "Tone", pre + "Mod", pre + "Ping", pre + "Mix" };
        add (4, std::make_unique<Section> (s, juce::String ("Delay ") + d + (k == 0 ? "  > " : ""), std::vector<std::pair<const char*, const char*>> {
            { v[0].c_str(), "Sync" }, { v[1].c_str(), "Division" }, { v[2].c_str(), "Time" }, { v[3].c_str(), "Feedback" },
            { v[4].c_str(), "Tone" }, { v[5].c_str(), "Wobble" }, { v[6].c_str(), "Ping-Pong" }, { v[7].c_str(), "Mix" } },
            (k == 0 ? "dlyAOn" : "dlyBOn")));
    }
    add (4, std::make_unique<Section> (s, "Reverb", std::vector<std::pair<const char*, const char*>> {
        { "revSize", "Size" }, { "revDamp", "Damping" }, { "revWidth", "Width" }, { "revMix", "Mix" } }, "revOn"));

    // ---- Header
    for (auto* c : { (juce::Component*) &categoryBox, (juce::Component*) &presetBox, (juce::Component*) &themeBox,
                     (juce::Component*) &prevButton, (juce::Component*) &nextButton, (juce::Component*) &saveButton,
                     (juce::Component*) &initButton, (juce::Component*) &keyboard })
        content.addAndMakeVisible (c);

    categoryBox.setTooltip ("Preset category");
    presetBox.setTooltip ("Preset");
    themeBox.setTooltip ("UI theme");
    for (int i = 0; i < (int) getThemes().size(); ++i) themeBox.addItem (getThemes()[(size_t) i].name, i + 1);
    themeBox.onChange = [this] { applyTheme (themeBox.getSelectedItemIndex()); };

    categoryBox.onChange = [this] { shownCategory = categoryBox.getText(); refreshPresetBox(); };
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id > 0 && id - 1 != proc.presets.getCurrentIndex()) proc.presets.load (id - 1);
    };
    prevButton.onClick = [this] { proc.presets.step (-1); };
    nextButton.onClick = [this] { proc.presets.step (1); };
    initButton.onClick = [this] { proc.presets.loadInit(); };
    saveButton.onClick = [this] { showSaveDialog(); };
    proc.presets.onPresetChanged = [sp = juce::Component::SafePointer<PaisleyEditor> (this)]
    {
        if (sp) { sp->refreshCategoryBox(); }
    };

    keyboard.setKeyWidth (22.0f);
    keyboard.setAvailableRange (24, 108);
    keyboard.setLowestVisibleKey (36);

    addAndMakeVisible (content);
    content.setSize (designW, designH);
    refreshCategoryBox();
    applyTheme (proc.getThemeIndex());

    setResizable (true, true);
    setResizeLimits (designW / 2, designH / 2, designW * 3 / 2, designH * 3 / 2);
    if (auto* c = getConstrainer()) c->setFixedAspectRatio ((double) designW / designH);
    setSize (designW * 4 / 5, designH * 4 / 5);
}

PaisleyEditor::~PaisleyEditor()
{
    proc.presets.onPresetChanged = nullptr;
    setLookAndFeel (nullptr);
}

void PaisleyEditor::applyTheme (int index)
{
    laf.setTheme (index);
    proc.setThemeIndex (laf.getThemeIndex());
    themeBox.setSelectedItemIndex (laf.getThemeIndex(), juce::dontSendNotification);
    sendLookAndFeelChange();
    keyboard.repaint();
    repaint();
}

void PaisleyEditor::refreshCategoryBox()
{
    auto& pm = proc.presets;
    const int cur = pm.getCurrentIndex();
    if (cur >= 0) shownCategory = pm.getEntries()[cur].category;
    auto cats = pm.getCategories();
    categoryBox.clear (juce::dontSendNotification);
    categoryBox.addItemList (cats, 1);
    if (shownCategory.isEmpty() && ! cats.isEmpty()) shownCategory = cats[0];
    categoryBox.setSelectedItemIndex (juce::jmax (0, cats.indexOf (shownCategory)), juce::dontSendNotification);
    refreshPresetBox();
}

void PaisleyEditor::refreshPresetBox()
{
    auto& pm = proc.presets;
    presetBox.clear (juce::dontSendNotification);
    for (int idx : pm.getIndicesInCategory (shownCategory))
        presetBox.addItem (pm.getEntries()[idx].name, idx + 1);
    if (pm.getCurrentIndex() >= 0 && pm.getEntries()[pm.getCurrentIndex()].category == shownCategory)
        presetBox.setSelectedId (pm.getCurrentIndex() + 1, juce::dontSendNotification);
    else
        presetBox.setText (pm.getCurrentIndex() < 0 ? pm.getCurrentName() : juce::String ("-- choose --"), juce::dontSendNotification);
}

void PaisleyEditor::showSaveDialog()
{
    saveDialog = std::make_unique<juce::AlertWindow> ("Save Preset", "Saved to " + PresetManager::getUserDir().getFullPathName(),
                                                      juce::MessageBoxIconType::NoIcon, this);
    saveDialog->addTextEditor ("name", proc.presets.getCurrentName(), "Name");
    saveDialog->addTextEditor ("cat", "User", "Folder");
    saveDialog->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    saveDialog->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));
    saveDialog->enterModalState (true, juce::ModalCallbackFunction::create ([this] (int result)
    {
        if (result == 1 && saveDialog != nullptr)
            proc.presets.saveUser (saveDialog->getTextEditorContents ("name"), saveDialog->getTextEditorContents ("cat"));
        saveDialog.reset();
    }), false);
}

void PaisleyEditor::paint (juce::Graphics& g) { g.fillAll (laf.theme().bgBottom); }

void PaisleyEditor::resized()
{
    const float scale = (float) getWidth() / (float) designW;
    content.setTransform (juce::AffineTransform::scale (scale));
    content.setBounds (0, 0, designW, designH);
}

// ---------------------------------------------------------------- Content
void PaisleyEditor::Content::paint (juce::Graphics& g)
{
    auto& laf = owner.laf;
    const auto& t = laf.theme();
    laf.drawBackground (g, getLocalBounds().toFloat());

    // Header strip + logo
    auto header = getLocalBounds().removeFromTop (64).toFloat().reduced (10, 8);
    g.setColour (t.panel);
    g.fillRoundedRectangle (header, t.corner);
    g.setColour (t.panelEdge.withAlpha (0.6f));
    g.drawRoundedRectangle (header, t.corner, 1.4f);

    auto logo = header.removeFromLeft (250).reduced (14, 0);
    juce::String word = t.uppercaseTitles ? "PAISLEY" : "paisley";
    auto sub = logo.removeFromBottom (14);
    g.setFont (laf.titleFont (32.0f));
    // offset "trails" for a psychedelic echo effect
    for (int i = 3; i > 0; --i)
    {
        g.setColour ((i % 2 ? t.accent2 : t.accent).withAlpha (0.25f + 0.1f * (3 - i)));
        g.drawText (word, logo.translated ((float) i * 2.0f, (float) i * 1.5f), juce::Justification::centredLeft);
    }
    g.setColour (t.title);
    g.drawText (word, logo, juce::Justification::centredLeft);
    g.setColour (t.dimText);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText (t.uppercaseTitles ? "PSYCHEDELIC SYNTHESISER" : "psychedelic synthesiser", sub, juce::Justification::centredLeft);
}

void PaisleyEditor::Content::resized()
{
    auto& e = owner;
    auto r = getLocalBounds().reduced (10, 0);
    auto header = r.removeFromTop (64).reduced (0, 8);
    header.removeFromLeft (260);
    auto hdr = header.reduced (6, 10);
    e.themeBox.setBounds (hdr.removeFromRight (190));
    hdr.removeFromRight (20);
    e.saveButton.setBounds (hdr.removeFromRight (60)); hdr.removeFromRight (6);
    e.initButton.setBounds (hdr.removeFromRight (50)); hdr.removeFromRight (16);
    e.nextButton.setBounds (hdr.removeFromRight (30)); hdr.removeFromRight (4);
    e.prevButton.setBounds (hdr.removeFromRight (30)); hdr.removeFromRight (6);
    e.presetBox.setBounds (hdr.removeFromRight (260)); hdr.removeFromRight (6);
    e.categoryBox.setBounds (hdr.removeFromRight (150));

    e.keyboard.setBounds (r.removeFromBottom (78).reduced (0, 6));
    e.keyboard.setKeyWidth ((float) e.keyboard.getWidth() / 50.0f); // 24..108 = 50 white keys
    e.keyboard.setLowestVisibleKey (24);

    const int rowH = (r.getHeight() - 4) / 5;
    for (auto& row : e.rows)
    {
        auto rr = r.removeFromTop (rowH).reduced (0, 4);
        int total = 0;
        for (auto& s : row) total += s->numControls();
        const int gap = 10;
        const float unit = (float) (rr.getWidth() - gap * ((int) row.size() - 1)) / (float) total;
        float x = (float) rr.getX();
        for (auto& s : row)
        {
            const float w = unit * (float) s->numControls();
            s->setBounds (juce::Rectangle<float> (x, (float) rr.getY(), w, (float) rr.getHeight()).toNearestInt());
            x += w + (float) gap;
        }
    }
}
