#include "PluginEditor.h"

namespace {
const juce::Colour bg       (0xff090711);
const juce::Colour surface  (0xff11101b);
const juce::Colour card     (0xff191629);
const juce::Colour cardHi   (0xff25203a);
const juce::Colour border   (0xff3b3154);
const juce::Colour violet   (0xff9b6bff);
const juce::Colour violetHi (0xffc9a8ff);
const juce::Colour neon     (0xffe2c7ff);
const juce::Colour white    (0xfff5f1ff);
const juce::Colour muted    (0xff948ba8);

class RichLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override {
        // Keep the complete bezel, tick ring and glow inside the rotary drawing area.
        const float side = (float)juce::jmin(w, h);
        const float radius = juce::jmax(7.0f, side * 0.5f - 11.0f);
        const float size = radius * 2.0f;
        const float cx = x + w * 0.5f, cy = y + h * 0.5f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        g.setColour(juce::Colour(0xff08090d));
        g.fillEllipse(cx-radius-1.5f,cy-radius-1.5f,size+3.0f,size+3.0f);
        juce::ColourGradient bezel(juce::Colour(0xff858892),cx,cy-radius,
                                   juce::Colour(0xff20232a),cx,cy+radius,false);
        g.setGradientFill(bezel);
        g.fillEllipse(cx-radius,cy-radius,size,size);
        g.setColour(juce::Colour(0xffb6b8c0).withAlpha(0.55f));
        g.drawEllipse(cx-radius+1,cy-radius+1,size-2,size-2,1.0f);
        for(int i=0;i<=24;++i){
            const float a=startAngle+(float)i/24.0f*(endAngle-startAngle);
            const float inner=radius+2.0f, outer=inner+(i%3==0?3.5f:2.0f);
            juce::Path tick;
            tick.startNewSubPath(cx+std::cos(a)*inner,cy+std::sin(a)*inner);
            tick.lineTo(cx+std::cos(a)*outer,cy+std::sin(a)*outer);
            g.setColour((float)i/24.0f<=sliderPos?violetHi:juce::Colour(0xff77737f).withAlpha(0.8f));
            g.strokePath(tick,juce::PathStrokeType(i%3==0?1.35f:0.8f));
        }
        juce::Path arc;
        arc.addCentredArc(cx,cy,radius+3.0f,radius+3.0f,0.0f,startAngle,angle,true);
        g.setColour(violet.withAlpha(0.18f));
        g.strokePath(arc,juce::PathStrokeType(3.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        g.setColour(violetHi);
        g.strokePath(arc,juce::PathStrokeType(1.7f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        juce::ColourGradient cap(juce::Colour(0xff5b606b),cx-radius*0.35f,cy-radius*0.75f,
                                 juce::Colour(0xff171a22),cx+radius*0.45f,cy+radius*0.85f,false);
        g.setGradientFill(cap);
        g.fillEllipse(cx-radius+6,cy-radius+6,size-12,size-12);
        g.setColour(juce::Colour(0xffc1c3cb).withAlpha(0.3f));
        g.drawEllipse(cx-radius+7,cy-radius+7,size-14,size-14,1.0f);
        juce::Path pointer;
        pointer.addRoundedRectangle(-1.35f,-radius+8.0f,2.7f,radius*0.45f,1.1f);
        g.setColour(juce::Colour(0xff07090e).withAlpha(0.7f));
        g.fillPath(pointer,juce::AffineTransform::rotation(angle+0.04f).translated(cx+1,cy+1));
        g.setColour(juce::Colour(0xfff5efff));
        g.fillPath(pointer,juce::AffineTransform::rotation(angle).translated(cx,cy));
        g.setColour(violetHi); g.fillEllipse(cx-2.0f,cy-2.0f,4.0f,4.0f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool highlighted, bool down) override {
        auto r=button.getLocalBounds().toFloat().reduced(0.5f);
        const bool active=button.getToggleState();
        const auto top=active?juce::Colour(0xff7254ab):(down?juce::Colour(0xff252936):juce::Colour(0xff454954));
        const auto bottom=active?juce::Colour(0xff302044):juce::Colour(0xff171a21);
        juce::ColourGradient fill(top,r.getX(),r.getY(),bottom,r.getX(),r.getBottom(),false);
        g.setGradientFill(fill); g.fillRoundedRectangle(r,6.0f);
        g.setColour(active?violetHi.withAlpha(0.95f):(highlighted?violet.withAlpha(0.8f):juce::Colour(0xff777b86).withAlpha(0.75f)));
        g.drawRoundedRectangle(r.reduced(0.5f),6.0f,active?1.35f:0.85f);
        g.setColour(juce::Colours::white.withAlpha(active?0.12f:0.07f));
        g.drawLine(r.getX()+7,r.getY()+1.5f,r.getRight()-7,r.getY()+1.5f,0.8f);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override {
        g.setFont(juce::Font(juce::FontOptions(10.5f, juce::Font::bold)));
        g.setColour(button.getToggleState() ? white : muted);
        g.drawFittedText(button.getButtonText(), button.getLocalBounds().reduced(7, 0),
                         juce::Justification::centred, 1);
    }

    void drawComboBox(juce::Graphics& g, int width, int height, bool,
                      int, int, int, int, juce::ComboBox& box) override {
        auto r = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height);
        g.setColour(cardHi);
        g.fillRoundedRectangle(r, 7.0f);
        g.setColour(border);
        g.drawRoundedRectangle(r.reduced(0.5f), 7.0f, 1.0f);
        juce::Path arrow;
        const float cx = width - 16.0f, cy = height * 0.5f;
        arrow.startNewSubPath(cx - 3.5f, cy - 1.5f);
        arrow.lineTo(cx, cy + 2.0f);
        arrow.lineTo(cx + 3.5f, cy - 1.5f);
        g.setColour(violetHi);
        g.strokePath(arrow, juce::PathStrokeType(1.7f));
        juce::ignoreUnused(box);
    }
};
RichLookAndFeel richLook;

juce::Rectangle<int> scaledBounds(juce::Rectangle<int> r, float scale, int ox, int oy) {
    return { ox + juce::roundToInt(r.getX() * scale), oy + juce::roundToInt(r.getY() * scale),
             juce::roundToInt(r.getWidth() * scale), juce::roundToInt(r.getHeight() * scale) };
}

juce::String factoryPresetName(int id) {
    static const char* starterNames[25] = {
        "Violet Lead", "Neon Saw", "Hollow Lead", "Cloud Pad", "Glass Cathedral",
        "Slow Motion", "Digital Keys", "Soft Piano-ish", "Pluck Keys", "Sub Pressure",
        "Reese Machine", "Acid Bloom", "Neon Bass", "Glass Pluck", "Cyber Mallet",
        "Crystal Keys", "Aurora Drift", "Deep Space", "Ethereal Choir", "Frozen Sky",
        "Laser Bloom", "Retro Pulse", "Wide Unison", "Dirty Motion", "Dream Sequence"
    };
    if (id >= 1 && id <= 25) return starterNames[id - 1];

    static const char* prefixes[10][10] = {
        { "Neon", "Chrome", "Violet", "Voltage", "Afterglow", "Prism", "Pulse", "Midnight", "Arctic", "Solar" },
        { "Velvet", "Cloud", "Halo", "Opal", "Dream", "Silk", "Ethereal", "Lunar", "Horizon", "Softlight" },
        { "Astral", "Nebula", "Infinite", "Weightless", "Orbit", "Celestial", "Drift", "Nocturne", "Stellar", "Ambient" },
        { "Digital", "Ivory", "Vintage", "Soft", "Tape", "Felt", "Modern", "Warm", "Glass", "Electric" },
        { "Grand", "Felt", "Velour", "Noir", "Studio", "Cinematic", "Upright", "Velvet", "Chamber", "Sunday" },
        { "Crystal", "Plucked", "Bamboo", "Marble", "Tiny", "Bell", "Wooden", "Elastic", "Pixel", "Chime" },
        { "Sub", "Reese", "Mono", "Acid", "Rubber", "Heavy", "Deep", "Circuit", "Industrial", "Round" },
        { "Analog", "Modular", "Retro", "Digital", "Hybrid", "FM", "Wave", "Unison", "Future", "Classic" },
        { "Impact", "Reverse", "Riser", "Glitch", "Laser", "Transition", "Signal", "Noise", "Cinematic", "Spectral" },
        { "Organic", "Frozen", "Dusty", "Granular", "Shimmer", "Fractal", "Hollow", "Textured", "Worn", "Distant" }
    };
    static const char* flavours[5] = { "Motion", "Bloom", "Dream", "Signal", "Echo" };
    static const char* nouns[10] = {
        "Lead", "Pad", "Atmosphere", "Keys", "Piano", "Pluck", "Bass", "Synth", "FX", "Texture"
    };
    const int offset = juce::jlimit(0, 474, id - 26);
    const int group = juce::jmin(9, offset / 50);
    const int local = offset % 50;
    return juce::String(prefixes[group][local % 10]) + " "
        + flavours[local / 10] + " " + nouns[group] + " "
        + juce::String(local + 1).paddedLeft('0', 2);
}

juce::String factoryPresetCategory(int id) {
    if (id <= 3) return "LEADS";
    if (id <= 6) return "PADS";
    if (id <= 9) return "KEYS";
    if (id <= 13) return "BASS";
    if (id <= 16) return "PLUCKS";
    if (id <= 20) return "ATMOSPHERE";
    if (id <= 25) return "FX & SYNTH";
    static const char* groups[10] = {
        "LEADS / VOL. 2", "PADS / VOL. 2", "ATMOSPHERES", "KEYS",
        "PIANOS", "PLUCKS", "BASS", "SYNTHS", "SOUND FX", "TEXTURES"
    };
    return groups[juce::jlimit(0, 9, (id - 26) / 50)];
}
}

AuroraEditor::AuroraEditor(AuroraProcessor& p) : AudioProcessorEditor(&p), proc(p) {
    setSize(1200, 820);
    for (const auto* id : { "wave1", "wave2", "wtTable", "wtPosition" })
        proc.params.addParameterListener(id, this);
    setResizable(true, true);
    setResizeLimits(1080, 738, 1680, 1148);

    heading.setText("2rich", juce::dontSendNotification);
    heading.setFont(juce::Font(juce::FontOptions(32.0f, juce::Font::bold)));
    heading.setColour(juce::Label::textColourId, white);
    addAndMakeVisible(heading);

    auto& ap = proc.params;
    auto add = [&](juce::Slider& s, const juce::String& id, const juce::String& label) {
        addKnob(s, label, id);
        attachments.push_back(std::make_unique<Attach>(ap, id, s));
    };

    for (auto* box : { &wave1, &wave2, &presets }) {
        box->setLookAndFeel(&richLook);
        box->setColour(juce::ComboBox::textColourId, white);
        box->setColour(juce::ComboBox::arrowColourId, violetHi);
        addAndMakeVisible(*box);
    }
    wave1.addItemList({ "Saw", "Square", "Triangle", "Sine" }, 1);
    wave2.addItemList({ "Saw", "Square", "Triangle", "Sine" }, 1);
    wave1.onChange = [this] { repaint(); };
    wave2.onChange = [this] { repaint(); };
    aWave1 = std::make_unique<ChoiceAttach>(ap, "wave1", wave1);
    aWave2 = std::make_unique<ChoiceAttach>(ap, "wave2", wave2);

    presets.setTextWhenNothingSelected("SELECT FROM 500 FACTORY SOUNDS");
    rebuildPresetList();
    presets.onChange = [this] {
        if (presets.getSelectedId() > 0)
            applyPreset(presets.getSelectedId());
        updateFavoriteButton();
    };

    presetSearch.setTextToShowWhenEmpty("SEARCH / fav:", muted);
    presetSearch.setTooltip("Search factory presets by name. Type fav: to show only favourites, or fav:lead to narrow the favourite list.");
    presetSearch.setColour(juce::TextEditor::backgroundColourId, surface);
    presetSearch.setColour(juce::TextEditor::textColourId, white);
    presetSearch.setColour(juce::TextEditor::highlightColourId, violet);
    presetSearch.setColour(juce::TextEditor::outlineColourId, border);
    presetSearch.setColour(juce::TextEditor::focusedOutlineColourId, violetHi);
    presetSearch.onTextChange = [this] { rebuildPresetList(); };
    addAndMakeVisible(presetSearch);

    for (auto* button : { &presetFavoriteButton, &savePresetButton, &loadPresetButton }) {
        button->setLookAndFeel(&richLook);
        button->setColour(juce::TextButton::textColourOffId, white);
        button->setColour(juce::TextButton::textColourOnId, violetHi);
        addAndMakeVisible(*button);
    }
    presetFavoriteButton.setClickingTogglesState(true);
    presetFavoriteButton.setTooltip("Mark or unmark the selected factory preset as a favourite. Favourites are saved with the project.");
    presetFavoriteButton.onClick = [this] {
        const int id = presets.getSelectedId();
        if (id > 0)
            proc.setPresetFavorite(id, presetFavoriteButton.getToggleState());
        rebuildPresetList();
        updateFavoriteButton();
    };

    savePresetButton.setTooltip("Save the complete current sound, including imported wavetable, as a .2richpreset file.");
    savePresetButton.onClick = [this] {
        presetChooser = std::make_unique<juce::FileChooser>(
            "Save 2rich Preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("My 2rich Preset.2richpreset"), "*.2richpreset");
        presetChooser->launchAsync(juce::FileBrowserComponent::saveMode
                                   | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& chooser) {
                auto file = chooser.getResult();
                if (file == juce::File{})
                    return;
                if (file.getFileExtension().isEmpty())
                    file = file.withFileExtension(".2richpreset");
                juce::MemoryBlock state;
                proc.getStateInformation(state);
                if (auto output = file.createOutputStream()) {
                    output->write(state.getData(), state.getSize());
                    output->flush();
                }
            });
    };

    loadPresetButton.setTooltip("Load a previously saved .2richpreset file.");
    loadPresetButton.onClick = [this] {
        presetChooser = std::make_unique<juce::FileChooser>(
            "Load 2rich Preset", juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.2richpreset");
        presetChooser->launchAsync(juce::FileBrowserComponent::openMode
                                   | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& chooser) {
                const auto file = chooser.getResult();
                if (file == juce::File{})
                    return;
                if (auto input = file.createInputStream()) {
                    juce::MemoryBlock state;
                    if (input->readIntoMemoryBlock(state) && state.getSize() > 0)
                        proc.setStateInformation(state.getData(), (int)state.getSize());
                    rebuildPresetList();
                    updateFavoriteButton();
                    repaint();
                }
            });
    };

    wtTable.setLookAndFeel(&richLook); wtTable.setColour(juce::ComboBox::textColourId,white);
    wtTable.addItemList({"Basic","Harmonics","Triangle Morph","Digital","Octave","Fold","Imported WAV"},1);
    addAndMakeVisible(wtTable); aWtTable=std::make_unique<ChoiceAttach>(ap,"wtTable",wtTable);

    for (auto* button : { &importWavetableButton, &clearWavetableButton }) {
        button->setLookAndFeel(&richLook);
        button->setColour(juce::TextButton::textColourOffId, white);
        button->setColour(juce::TextButton::textColourOnId, violetHi);
        addAndMakeVisible(*button);
    }
    wavetableStatusLabel.setColour(juce::Label::textColourId, muted);
    wavetableStatusLabel.setFont(juce::Font(juce::FontOptions(8.0f)));
    wavetableStatusLabel.setText(proc.hasImportedWavetable()
        ? "RESTORED: " + proc.getImportedWavetableName()
        : "DRAW ON THE WAVEFORM OR IMPORT AUDIO", juce::dontSendNotification);
    addAndMakeVisible(wavetableStatusLabel);

    importWavetableButton.onClick = [this] {
        wavetableChooser = std::make_unique<juce::FileChooser>(
            "Import Wavetable Audio", juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*.wav;*.aif;*.aiff");
        wavetableChooser->launchAsync(juce::FileBrowserComponent::openMode
                                      | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& chooser) {
                const auto file = chooser.getResult();
                if (file == juce::File{})
                    return;
                juce::String error;
                if (proc.loadWavetableFromFile(file, error)) {
                    // Force the APVTS choice as well as the UI selection. This makes
                    // the imported table active even if the host's ComboBox attachment
                    // does not propagate a programmatic selection change.
                    if (auto* parameter = proc.params.getParameter("wtTable"))
                        parameter->setValueNotifyingHost(1.0f); // choice index 6 / item ID 7
                    wtTable.setSelectedId(7, juce::dontSendNotification);
                    wavetableStatusLabel.setText("ACTIVE WAVETABLE: " + file.getFileName(),
                                                 juce::dontSendNotification);
                } else {
                    wavetableStatusLabel.setText("IMPORT FAILED: " + error,
                                                 juce::dontSendNotification);
                }
                repaint();
            });
    };
    clearWavetableButton.onClick = [this] {
        proc.clearImportedWavetable();
        if (wtTable.getSelectedId() == 7)
            wtTable.setSelectedId(1, juce::sendNotification);
        wavetableStatusLabel.setText("IMPORTED TABLE CLEARED", juce::dontSendNotification);
        repaint();
    };

    auto addChoice = [&](juce::ComboBox& box, const juce::StringArray& items, const juce::String& id) {
        box.setLookAndFeel(&richLook);
        box.setColour(juce::ComboBox::textColourId, white);
        box.setColour(juce::ComboBox::arrowColourId, violetHi);
        box.addItemList(items, 1);
        addAndMakeVisible(box);
        choiceAttachments.push_back(std::make_unique<ChoiceAttach>(ap, id, box));
    };
    addChoice(oscModeBox, { "Classic", "FM", "Ring Mod", "Hard Sync", "Super Saw" }, "oscMode");
    addChoice(filterTypeBox, { "Low Pass", "Band Pass", "High Pass" }, "filterType");
    const juce::StringArray fxTypeNames = { "Off", "Compressor", "Saturation", "Phaser",
        "Ping-Pong", "Reverb", "Chorus", "Multiband Dist.", "Limiter",
        "Bitcrusher", "Stereo Width", "Tremolo", "Ring Mod", "Filter", "Flanger" };
    auto moveFxSlot = [this](int index, int direction) {
        const int other = index + direction;
        if (other < 0 || other >= 6) return;
        const auto idA = "fxSlot" + juce::String(index + 1);
        const auto idB = "fxSlot" + juce::String(other + 1);
        auto* parameterA = proc.params.getParameter(idA);
        auto* parameterB = proc.params.getParameter(idB);
        auto* valueA = proc.params.getRawParameterValue(idA);
        auto* valueB = proc.params.getRawParameterValue(idB);
        if (parameterA == nullptr || parameterB == nullptr || valueA == nullptr || valueB == nullptr) return;
        const float a = valueA->load(), b = valueB->load();
        parameterA->beginChangeGesture();
        parameterA->setValueNotifyingHost(parameterA->convertTo0to1(b));
        parameterA->endChangeGesture();
        parameterB->beginChangeGesture();
        parameterB->setValueNotifyingHost(parameterB->convertTo0to1(a));
        parameterB->endChangeGesture();
        selectedFxSlot = other;
        updateFxEditor();
        repaint();
    };

    for (int i = 0; i < 6; ++i) {
        addChoice(fxSlots[(size_t)i], fxTypeNames, "fxSlot" + juce::String(i + 1));
        fxSlots[(size_t)i].setTooltip("Choose the effect for this insert. Select OFF to bypass the slot.");
        fxSlots[(size_t)i].onChange = [this, i] {
            selectedFxSlot = i;
            updateFxEditor();
            repaint();
        };

        auto& select = fxSlotSelectButtons[(size_t)i];
        select.setButtonText(juce::String::formatted("%02d", i + 1));
        select.setLookAndFeel(&richLook);
        select.setClickingTogglesState(false);
        select.setColour(juce::TextButton::textColourOffId, white);
        select.setColour(juce::TextButton::textColourOnId, violetHi);
        select.setTooltip("Select insert " + juce::String(i + 1) + " to edit its effect.");
        select.onClick = [this, i] { selectedFxSlot = i; updateFxEditor(); repaint(); };
        addAndMakeVisible(select);

        auto& up = fxMoveUpButtons[(size_t)i];
        up.setButtonText("↑");
        up.setLookAndFeel(&richLook);
        up.setColour(juce::TextButton::textColourOffId, white);
        up.setTooltip("Move this effect earlier in the signal chain.");
        up.onClick = [moveFxSlot, i] { moveFxSlot(i, -1); };
        addAndMakeVisible(up);

        auto& down = fxMoveDownButtons[(size_t)i];
        down.setButtonText("↓");
        down.setLookAndFeel(&richLook);
        down.setColour(juce::TextButton::textColourOffId, white);
        down.setTooltip("Move this effect later in the signal chain.");
        down.onClick = [moveFxSlot, i] { moveFxSlot(i, 1); };
        addAndMakeVisible(down);
    }

    fxCompEnabledButton.setLookAndFeel(&richLook);
    fxCompEnabledButton.setColour(juce::ToggleButton::textColourId, white);
    fxCompEnabledButton.setTooltip("Enable or bypass the compressor wherever it appears in the FX chain.");
    addAndMakeVisible(fxCompEnabledButton);
    aFxCompEnabled = std::make_unique<ButtonAttach>(ap, "fxCompEnabled", fxCompEnabledButton);

    const juce::StringArray lfoShapes = { "Sine", "Triangle", "Saw", "Square", "Sample & Hold" };
    addChoice(lfo1Shape, lfoShapes, "lfo1Shape");
    addChoice(lfo2Shape, lfoShapes, "lfo2Shape");
    addChoice(lfo3Shape, lfoShapes, "lfo3Shape");
    // Preserve existing source indices in saved projects; append new sources.
    const juce::StringArray modSourceNames = { "Off", "LFO 1", "LFO 2", "Velocity", "Amp Env",
        "Filter Env", "Mod Wheel", "LFO 3", "Pitch Bend", "Aftertouch" };
    const juce::StringArray modTargetNames = { "Off", "Cutoff", "Osc Mix", "WT Position", "Pitch",
        "Pan", "Resonance", "Volume", "FX Chorus", "FX Delay", "FX Reverb" };
    for (int i = 0; i < 4; ++i) {
        const auto suffix = juce::String(i + 1);
        addChoice(this->modSources[(size_t)i], modSourceNames, "mod" + suffix + "Source");
        addChoice(this->modTargets[(size_t)i], modTargetNames, "mod" + suffix + "Target");
    }

    addChoice(seqModeBox, { "Off", "Arpeggiator", "Step Sequencer" }, "seqMode");
    addChoice(arpStyleBox, { "Up", "Down", "Up / Down", "Random" }, "arpStyle");
    addChoice(seqRateBox, { "1/4", "1/8", "1/16", "1/32", "1/8T" }, "seqRate");
    addChoice(seqDirectionBox, { "Forward", "Reverse", "Ping-Pong", "Random" }, "seqDirection");
    addChoice(seqLengthBox, { "4 Steps", "8 Steps", "16 Steps", "32 Steps" }, "seqLength");
    addChoice(seqPageBox, { "STEPS 01-16", "STEPS 17-32" }, "seqPage");
    addChoice(seqPatternSlotBox, { "PATTERN 01", "PATTERN 02", "PATTERN 03", "PATTERN 04",
                                   "PATTERN 05", "PATTERN 06", "PATTERN 07", "PATTERN 08" }, "seqPatternSlot");
    auto addExtra=[&](juce::Slider& s,const juce::String& id,const juce::String& label){addKnob(s,label,id);attachments.push_back(std::make_unique<Attach>(ap,id,s));};
    addExtra(wtPosition,"wtPosition","POSITION"); wtPosition.setSliderStyle(juce::Slider::LinearHorizontal); wtPosition.setTextBoxStyle(juce::Slider::TextBoxRight,false,42,18); addExtra(unison,"unison","VOICES"); addExtra(uniDetune,"uniDetune","DETUNE"); addExtra(uniSpread,"uniSpread","SPREAD");
    addExtra(fmAmount,"fmAmount","FM AMOUNT"); addExtra(syncRatio,"syncRatio","SYNC RATIO");
    for (auto* slider : { &fmAmount, &syncRatio }) {
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
    }
    addExtra(fxEq,"fxEq","EQ MIX"); addExtra(fxLow,"fxLow","LOW"); addExtra(fxMid,"fxMid","MID"); addExtra(fxHigh,"fxHigh","HIGH");
    for (auto* slider : { &fxEq, &fxLow, &fxMid, &fxHigh }) {
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 18);
        slider->setColour(juce::Slider::trackColourId, violet);
        slider->setColour(juce::Slider::thumbColourId, violetHi);
    }
    addExtra(fxDist,"fxDist","DRIVE"); addExtra(fxChorus,"fxChorus","MIX"); addExtra(fxFlange,"fxFlange","DEPTH"); addExtra(fxDelay,"fxDelay","MIX"); addExtra(fxReverb,"fxReverb","MIX");
    addExtra(fxCompThreshold,"fxCompThreshold","THRESHOLD");
    addExtra(fxCompRatio,"fxCompRatio","RATIO");
    addExtra(fxCompAttack,"fxCompAttack","ATTACK");
    addExtra(fxCompRelease,"fxCompRelease","RELEASE");
    addExtra(fxCompMakeup,"fxCompMakeup","MAKEUP");
    addExtra(fxCompMix,"fxCompMix","MIX");
    addExtra(fxPhaserRate,"fxPhaserRate","PHASER RATE");
    addExtra(fxPingTime,"fxPingTime","PING TIME");
    addExtra(fxPingFeedback,"fxPingFeedback","PING FB");
    addExtra(fxReverbRoom,"fxReverbRoom","ROOM SIZE");
    addExtra(fxReverbDamping,"fxReverbDamping","DAMPING");
    addExtra(fxLimiterCeiling,"fxLimiterCeiling","CEILING");

    const std::array<std::array<const char*, 2>, fxExtraCount> fxExtraDefinitions {{
        { "fxSatTone", "TONE" }, { "fxSatMix", "MIX" }, { "fxPhaserFeedback", "FEEDBACK" },
        { "fxPingTone", "TONE" }, { "fxReverbWidth", "WIDTH" },
        { "fxLimiterRelease", "RELEASE" }, { "fxLimiterInput", "INPUT" }, { "fxLimiterMix", "MIX" },
        { "fxMbLow", "LOW DRIVE" }, { "fxMbMid", "MID DRIVE" }, { "fxMbHigh", "HIGH DRIVE" }, { "fxMbMix", "MIX" },
        { "fxBitDepth", "BIT DEPTH" }, { "fxBitRate", "RATE REDUCE" }, { "fxBitMix", "MIX" },
        { "fxStereoWidth", "WIDTH" }, { "fxStereoPan", "BALANCE" }, { "fxStereoMix", "MIX" },
        { "fxTremRate", "RATE" }, { "fxTremDepth", "DEPTH" }, { "fxTremStereo", "STEREO PHASE" }, { "fxTremMix", "MIX" },
        { "fxRingFreq", "FREQUENCY" }, { "fxRingMix", "MIX" }, { "fxRingStereo", "STEREO" },
        { "fxFilterCutoff", "CUTOFF" }, { "fxFilterRes", "RESONANCE" }, { "fxFilterMix", "MIX" },
        { "fxFlangerRate", "RATE" }, { "fxFlangerDepth", "DEPTH" }, { "fxFlangerFeedback", "FEEDBACK" },
        { "fxFlangerTime", "TIME" }, { "fxFlangerMix", "MIX" },
        { "fxChorusRate", "RATE" }, { "fxChorusDepth", "DEPTH" }, { "fxChorusWidth", "WIDTH" }
    }};
    for (size_t i = 0; i < fxExtraKnobs.size(); ++i)
        addExtra(fxExtraKnobs[i], fxExtraDefinitions[i][0], fxExtraDefinitions[i][1]);
    addChoice(fxFilterTypeBox, { "Low Pass", "High Pass", "Band Pass" }, "fxFilterType");

    addExtra(lfo1Rate,"lfo1Rate","RATE"); addExtra(lfo2Rate,"lfo2Rate","RATE");
    addExtra(lfo3Rate,"lfo3Rate","RATE");
    addExtra(filterAttack,"filterAttack","ATTACK"); addExtra(filterDecay,"filterDecay","DECAY");
    addExtra(filterSustain,"filterSustain","SUSTAIN"); addExtra(filterRelease,"filterRelease","RELEASE");
    for (auto* slider : { &filterAttack, &filterDecay, &filterSustain, &filterRelease }) {
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 32, 16);
    }
    addExtra(filterEnvAmount,"filterEnvAmount","AMOUNT"); filterEnvAmount.setSliderStyle(juce::Slider::LinearHorizontal); filterEnvAmount.setTextBoxStyle(juce::Slider::TextBoxRight,false,42,18);
    addExtra(seqOctaves,"seqOctaves","OCTAVES");
    addExtra(seqGate,"seqGate","GATE");
    addExtra(seqSwing,"seqSwing","SWING");
    for (auto* slider : { &seqOctaves, &seqGate, &seqSwing }) {
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 42, 18);
    }
    seqOctaves.setNumDecimalPlacesToDisplay(0);
    for (int i = 0; i < 16; ++i) {
        auto& button = stepButtons[(size_t)i];
        button.setLookAndFeel(&richLook);
        button.setClickingTogglesState(false);
        button.setColour(juce::TextButton::textColourOffId, white);
        button.setColour(juce::TextButton::textColourOnId, violetHi);
        button.onClick = [this, i] {
            selectedStep = i;
            const int pageOffset = seqPageBox.getSelectedId() == 2 ? 16 : 0;
            const auto stepId = juce::String(pageOffset + i);
            const auto parameterId = "stepOn" + stepId;
            const bool isEnabled = proc.params.getRawParameterValue(parameterId)->load() > 0.5f;
            writeSelectedStepParameter("stepOn", isEnabled ? 0.0f : 1.0f);
            refreshSelectedStepControls();
            repaint();
        };
        addAndMakeVisible(button);
    }

    auto setupStepEditor = [this](juce::Slider& slider, float minimum, float maximum, float interval,
                                  const juce::String& baseId) {
        addKnob(slider, baseId, baseId);
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 44, 18);
        slider.setRange(minimum, maximum, interval);
        auto* sliderPtr = &slider;
        slider.onValueChange = [this, sliderPtr, baseId] {
            writeSelectedStepParameter(baseId, (float)sliderPtr->getValue());
        };
    };
    setupStepEditor(stepPitch, -12.0f, 12.0f, 1.0f, "stepPitch");
    setupStepEditor(stepVelocity, 1.0f, 127.0f, 1.0f, "stepVelocity");
    setupStepEditor(stepGate, 10.0f, 100.0f, 1.0f, "stepGate");
    stepPitch.setNumDecimalPlacesToDisplay(0);
    stepVelocity.setNumDecimalPlacesToDisplay(0);
    stepGate.setNumDecimalPlacesToDisplay(0);

    stepAccentButton.setLookAndFeel(&richLook);
    stepAccentButton.setClickingTogglesState(false);
    stepAccentButton.onClick = [this] {
        const int pageOffset = seqPageBox.getSelectedId() == 2 ? 16 : 0;
        const auto id = "stepAccent" + juce::String(pageOffset + selectedStep);
        const bool enabled = proc.params.getRawParameterValue(id)->load() > 0.5f;
        writeSelectedStepParameter("stepAccent", enabled ? 0.0f : 1.0f);
        refreshSelectedStepControls();
    };
    addAndMakeVisible(stepAccentButton);

    for (auto* button : { &savePatternButton, &loadPatternButton }) {
        button->setLookAndFeel(&richLook);
        button->setColour(juce::TextButton::textColourOffId, white);
        button->setColour(juce::TextButton::textColourOnId, violetHi);
        addAndMakeVisible(*button);
    }
    savePatternButton.setTooltip("Save the current 32-step pattern into the selected project pattern slot.");
    loadPatternButton.setTooltip("Load the selected user pattern from this project.");
    savePatternButton.onClick = [this] {
        proc.saveUserPattern(juce::jmax(0, seqPatternSlotBox.getSelectedId() - 1));
        patternStatusLabel.setText("SAVED WITH PROJECT STATE", juce::dontSendNotification);
    };
    loadPatternButton.onClick = [this] {
        const bool loaded = proc.loadUserPattern(juce::jmax(0, seqPatternSlotBox.getSelectedId() - 1));
        refreshSelectedStepControls();
        patternStatusLabel.setText(loaded ? "PATTERN LOADED" : "EMPTY SLOT - SAVE FIRST",
                                   juce::dontSendNotification);
        repaint();
    };
    selectedStepLabel.setColour(juce::Label::textColourId, white);
    selectedStepLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    addAndMakeVisible(selectedStepLabel);
    patternStatusLabel.setColour(juce::Label::textColourId, muted);
    patternStatusLabel.setFont(juce::Font(juce::FontOptions(8.5f)));
    patternStatusLabel.setText("PATTERNS ARE STORED WITH THE DAW PROJECT", juce::dontSendNotification);
    addAndMakeVisible(patternStatusLabel);

    seqPageBox.onChange = [this] {
        selectedStep = juce::jlimit(0, 15, selectedStep);
        refreshSelectedStepControls();
        repaint();
    };
    for (int i = 0; i < 4; ++i) {
        addExtra(modAmounts[(size_t)i], "mod" + juce::String(i + 1) + "Amount", "AMOUNT");
        modAmounts[(size_t)i].setSliderStyle(juce::Slider::LinearHorizontal);
        modAmounts[(size_t)i].setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
    }
    for(auto* tab:{&synthTab,&wtTab,&fxTab,&modTab,&seqTab}){tab->setLookAndFeel(&richLook);tab->setColour(juce::TextButton::textColourOffId,white);tab->setColour(juce::TextButton::textColourOnId,violetHi);tab->setClickingTogglesState(false);addAndMakeVisible(*tab);}
    synthTab.onClick=[this]{setPage(0);};wtTab.onClick=[this]{setPage(1);};fxTab.onClick=[this]{setPage(2);};modTab.onClick=[this]{setPage(1);};seqTab.onClick=[this]{setPage(1);};
    unison.setRange(1,7,1); unison.setNumDecimalPlacesToDisplay(0);
    add(mix, "mix", "MIX"); add(detune, "detune", "DETUNE"); add(sub, "sub", "SUB");
    add(cutoff, "cutoff", "CUTOFF"); add(res, "res", "RES");
    add(attack, "attack", "ATTACK"); add(decay, "decay", "DECAY");
    add(sustain, "sustain", "SUSTAIN"); add(release, "release", "RELEASE");
    add(master, "master", "MASTER"); add(drive, "drive", "DRIVE"); add(width, "width", "WIDTH");
    setPage(0);
}

AuroraEditor::~AuroraEditor() {
    for (const auto* id : { "wave1", "wave2", "wtTable", "wtPosition" })
        proc.params.removeParameterListener(id, this);
    for (auto& slider : fxExtraKnobs)
        slider.setLookAndFeel(nullptr);
    for (auto* slider : { &mix, &detune, &sub, &cutoff, &res, &attack, &decay, &sustain,
                          &release, &master, &drive, &width, &wtPosition, &unison, &uniDetune,
                          &uniSpread, &fmAmount, &syncRatio, &fxEq, &fxLow, &fxMid, &fxHigh, &fxDist, &fxChorus,
                          &fxFlange, &fxDelay, &fxReverb, &fxCompThreshold, &fxCompRatio, &fxCompAttack,
                          &fxCompRelease, &fxCompMakeup, &fxCompMix, &fxPhaserRate, &fxPingTime,
                          &fxPingFeedback, &fxReverbRoom, &fxReverbDamping, &fxLimiterCeiling,
                          &lfo1Rate, &lfo2Rate, &lfo3Rate, &filterAttack,
                          &filterDecay, &filterSustain, &filterRelease, &filterEnvAmount,
                          &modAmounts[0], &modAmounts[1], &modAmounts[2], &modAmounts[3],
                          &seqOctaves, &seqGate, &seqSwing, &stepPitch, &stepVelocity, &stepGate })
        slider->setLookAndFeel(nullptr);
    wave1.setLookAndFeel(nullptr); wave2.setLookAndFeel(nullptr); presets.setLookAndFeel(nullptr); wtTable.setLookAndFeel(nullptr);
    for (auto& slot : fxSlots) slot.setLookAndFeel(nullptr);
    for (auto& button : fxSlotSelectButtons) button.setLookAndFeel(nullptr);
    for (auto& button : fxMoveUpButtons) button.setLookAndFeel(nullptr);
    for (auto& button : fxMoveDownButtons) button.setLookAndFeel(nullptr);
    for (auto* button : { &presetFavoriteButton, &savePresetButton, &loadPresetButton })
        button->setLookAndFeel(nullptr);
    fxCompEnabledButton.setLookAndFeel(nullptr);
    lfo1Shape.setLookAndFeel(nullptr); lfo2Shape.setLookAndFeel(nullptr); lfo3Shape.setLookAndFeel(nullptr);
    fxFilterTypeBox.setLookAndFeel(nullptr);
    oscModeBox.setLookAndFeel(nullptr); filterTypeBox.setLookAndFeel(nullptr);
    importWavetableButton.setLookAndFeel(nullptr); clearWavetableButton.setLookAndFeel(nullptr);
    wavetableChooser.reset();
    for (auto& box : modSources) box.setLookAndFeel(nullptr);
    for (auto& box : modTargets) box.setLookAndFeel(nullptr);
    for (auto& button : stepButtons) button.setLookAndFeel(nullptr);
    stepAccentButton.setLookAndFeel(nullptr);
    savePatternButton.setLookAndFeel(nullptr);
    loadPatternButton.setLookAndFeel(nullptr);
    seqModeBox.setLookAndFeel(nullptr); arpStyleBox.setLookAndFeel(nullptr); seqRateBox.setLookAndFeel(nullptr);
    seqDirectionBox.setLookAndFeel(nullptr); seqLengthBox.setLookAndFeel(nullptr);
    seqPageBox.setLookAndFeel(nullptr); seqPatternSlotBox.setLookAndFeel(nullptr);
    synthTab.setLookAndFeel(nullptr);wtTab.setLookAndFeel(nullptr);fxTab.setLookAndFeel(nullptr);modTab.setLookAndFeel(nullptr);seqTab.setLookAndFeel(nullptr);
}

void AuroraEditor::rebuildPresetList() {
    const auto rawQuery = presetSearch.getText().trim().toLowerCase();
    const bool favouritesOnly = rawQuery.startsWith("fav:");
    const auto query = favouritesOnly ? rawQuery.substring(4).trim() : rawQuery;
    const int previousId = presets.getSelectedId();
    presets.clear();

    struct Category { const char* name; int first; int last; };
    static const Category categories[] = {
        { "LEADS", 1, 3 }, { "PADS", 4, 6 }, { "KEYS", 7, 9 },
        { "BASS", 10, 13 }, { "PLUCKS", 14, 16 },
        { "ATMOSPHERE", 17, 20 }, { "FX & SYNTH", 21, 25 },
        { "LEADS / VOL. 2", 26, 75 }, { "PADS / VOL. 2", 76, 125 },
        { "ATMOSPHERES", 126, 175 }, { "KEYS", 176, 225 },
        { "PIANOS", 226, 275 }, { "PLUCKS", 276, 325 },
        { "BASS", 326, 375 }, { "SYNTHS", 376, 425 },
        { "SOUND FX", 426, 475 }, { "TEXTURES", 476, 500 }
    };

    if (query.isEmpty() && !favouritesOnly) {
        for (const auto& category : categories) {
            presets.addSectionHeading(category.name);
            for (int id = category.first; id <= category.last; ++id)
                presets.addItem(factoryPresetName(id), id);
        }
    } else {
        for (int id = 1; id <= 500; ++id) {
            const auto name = factoryPresetName(id);
            const bool matchesName = query.isEmpty() || name.toLowerCase().contains(query);
            if (matchesName && (!favouritesOnly || proc.isPresetFavorite(id)))
                presets.addItem(name, id);
        }
    }

    if (previousId > 0 && presets.indexOfItemId(previousId) >= 0)
        presets.setSelectedId(previousId, juce::dontSendNotification);
    updateFavoriteButton();
}

void AuroraEditor::updateFavoriteButton() {
    const int id = presets.getSelectedId();
    const bool favourite = id > 0 && proc.isPresetFavorite(id);
    presetFavoriteButton.setToggleState(favourite, juce::dontSendNotification);
    presetFavoriteButton.setButtonText(favourite ? "★ FAV" : "FAV");
}

void AuroraEditor::addKnob(juce::Slider& s, const juce::String&, const juce::String&) {
    s.setLookAndFeel(&richLook);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,50,16);
    s.setColour(juce::Slider::textBoxTextColourId,white);
    s.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff10131a));
    s.setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff555966).withAlpha(0.65f));
    s.setColour(juce::Slider::textBoxHighlightColourId,violet);
    addAndMakeVisible(s);
}

void AuroraEditor::applyPreset(int id) {
    // Each preset is a complete patch: oscillator, envelope, filter, unison and FX.
    struct Patch {
        int w1, w2, table;
        float mix, detune, sub, cutoff, res, attack, decay, sustain, release;
        float master, drive, width, voices, uniDetune, uniSpread, position;
        float eq, low, mid, high, fxDist, chorus, flange, delay, reverb;
    };
    static const Patch patches[] = {
        {0,2,0,.42f,11,.18f,10500,.22f,.004f,.22f,.72f,.18f,.80f,.12f,.72f,1,12,.65f,.20f,.20f,0,0,.12f,.15f,0,.08f,.12f,0.0f},
        {0,1,3,.58f,18,.10f,8500,.35f,.002f,.16f,.62f,.12f,.78f,.24f,.85f,3,18,.85f,.28f,.35f,1,2,.24f,.10f,.18f,.16f,.10f,0.0f},
        {3,0,2,.25f,5,.05f,6200,.18f,.015f,.35f,.48f,.32f,.82f,.05f,.55f,1,8,.40f,.18f,.10f,0,1,.05f,.05f,.0f,.10f,.08f,0.0f},
        {2,3,2,.62f,7,.28f,5200,.28f,1.8f,3.2f,.82f,4.0f,.76f,.08f,.95f,3,18,.9f,.65f,.25f,2,3,.04f,.48f,.0f,.22f,.48f,0.0f},
        {3,2,4,.50f,3,.12f,4300,.42f,2.6f,3.8f,.76f,4.5f,.78f,.04f,.9f,5,24,1.0f,.72f,.35f,3,4,.02f,.62f,.0f,.28f,.62f,0.0f},
        {0,2,1,.38f,9,.20f,6800,.25f,1.2f,2.4f,.68f,3.8f,.76f,.10f,.8f,4,20,.9f,.58f,.22f,1,2,.10f,.55f,.0f,.25f,.52f,0.0f},
        {2,3,3,.32f,2,.10f,9200,.12f,.002f,.18f,.35f,.55f,.82f,.02f,.35f,1,5,.25f,.12f,.10f,0,1,.02f,.08f,.0f,.08f,.08f,0.0f},
        {3,2,4,.45f,4,.08f,7400,.20f,.008f,.42f,.48f,.8f,.82f,.02f,.45f,1,6,.35f,.16f,.12f,0,2,.02f,.10f,.0f,.12f,.12f,0.0f},
        {1,3,1,.22f,12,.05f,5800,.16f,.001f,.24f,.20f,.38f,.84f,.16f,.3f,1,10,.30f,.14f,.12f,0,1,.16f,.04f,.0f,.10f,.08f,0.0f},
        {0,1,5,.48f,24,.35f,2600,.62f,.01f,.42f,.50f,.8f,.78f,.38f,.72f,2,22,.75f,.35f,.32f,2,1,.38f,.12f,.15f,.22f,.18f,0.0f},
        {0,3,5,.55f,31,.18f,1800,.72f,.02f,.55f,.42f,.9f,.76f,.62f,.68f,4,30,.95f,.42f,.38f,3,2,.62f,.18f,.28f,.26f,.12f,0.0f},
        {1,0,3,.20f,8,.28f,1400,.82f,.003f,.28f,.58f,.35f,.82f,.72f,.42f,1,12,.40f,.12f,.20f,1,0,.72f,.0f,.10f,.05f,.08f,0.0f},
        {0,2,4,.42f,16,.32f,2200,.48f,.006f,.38f,.52f,.55f,.80f,.44f,.75f,3,25,.88f,.38f,.30f,1,2,.44f,.22f,.12f,.22f,.16f,0.0f},
        {3,2,2,.28f,6,.04f,7600,.16f,.001f,.20f,.18f,.24f,.84f,.08f,.38f,1,7,.35f,.18f,.12f,0,2,.08f,.08f,.0f,.14f,.10f,0.0f},
        {2,3,3,.36f,14,.06f,6400,.28f,.001f,.32f,.22f,.32f,.82f,.18f,.52f,2,14,.65f,.28f,.18f,1,3,.18f,.18f,.18f,.20f,.14f,0.0f},
        {3,1,4,.30f,9,.08f,8200,.22f,.002f,.26f,.24f,.40f,.82f,.12f,.58f,2,11,.72f,.30f,.22f,1,4,.12f,.24f,.08f,.18f,.18f,0.0f},
        {3,2,4,.52f,5,.12f,5400,.24f,1.1f,2.8f,.78f,4.2f,.76f,.04f,.95f,5,18,1.0f,.68f,.25f,1,3,.04f,.55f,.0f,.35f,.62f,0.0f},
        {0,3,5,.44f,20,.22f,3100,.38f,.8f,2.2f,.62f,3.6f,.78f,.20f,.88f,6,28,1.0f,.62f,.32f,2,4,.20f,.62f,.22f,.38f,.58f,0.0f},
        {3,2,4,.60f,4,.10f,4800,.30f,1.5f,3.4f,.90f,4.8f,.74f,.02f,.98f,5,16,1.0f,.74f,.28f,2,3,.02f,.72f,.0f,.34f,.72f,0.0f},
        {2,3,2,.48f,10,.18f,3900,.34f,2.2f,3.0f,.84f,4.5f,.76f,.08f,.92f,7,24,1.0f,.68f,.34f,2,3,.08f,.68f,.12f,.32f,.68f,0.0f},
        {1,0,3,.32f,22,.02f,9800,.12f,.001f,.12f,.05f,.45f,.82f,.48f,.72f,3,32,.95f,.22f,.15f,0,3,.48f,.04f,.55f,.25f,.10f,0.0f},
        {1,3,1,.40f,15,.10f,7200,.40f,.001f,.24f,.16f,.55f,.80f,.30f,.80f,4,26,.92f,.34f,.24f,2,2,.30f,.18f,.45f,.28f,.18f,0.0f},
        {0,2,4,.50f,8,.12f,6100,.22f,.006f,.45f,.72f,2.8f,.78f,.10f,.96f,7,28,1.0f,.72f,.30f,1,3,.10f,.62f,.0f,.35f,.62f,0.0f},
        {0,1,5,.58f,38,.28f,1700,.68f,.003f,.38f,.48f,.72f,.76f,.72f,.86f,7,42,1.0f,.48f,.42f,3,4,.72f,.25f,.48f,.30f,.24f,0.0f},
        {3,0,4,.46f,13,.16f,4600,.26f,.45f,2.6f,.86f,4.6f,.76f,.14f,.94f,6,30,1.0f,.70f,.30f,2,3,.14f,.58f,.30f,.48f,.70f,0.0f}
    };
    if (id < 1 || id > (int)std::size(patches)) return;
    const auto& p = patches[id - 1];
    auto set = [this](const char* key, float value) {
        if (auto* parameter = proc.params.getParameter(key))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };
    set("wave1",(float)p.w1); set("wave2",(float)p.w2); set("wtTable",(float)p.table);
    set("mix",p.mix); set("detune",p.detune); set("sub",p.sub);
    set("cutoff",p.cutoff); set("res",p.res);
    set("attack",p.attack); set("decay",p.decay); set("sustain",p.sustain); set("release",p.release);
    set("master",p.master); set("drive",p.drive); set("width",p.width);
    set("unison",p.voices); set("uniDetune",p.uniDetune); set("uniSpread",p.uniSpread);
    set("wtPosition",p.position);
    set("fxEq",p.eq); set("fxLow",p.low); set("fxMid",p.mid); set("fxHigh",p.high);
    set("fxDist",p.fxDist); set("fxChorus",p.chorus); set("fxFlange",p.flange);
    set("fxDelay",p.delay); set("fxReverb",p.reverb);
    set("fxSlot1", (p.drive > 0.01f || p.fxDist > 0.01f) ? 2.0f : 0.0f);
    set("fxSlot2", 1.0f);
    set("fxSlot3", p.flange > 0.01f ? 3.0f : 0.0f);
    set("fxSlot4", p.delay > 0.01f ? 4.0f : 0.0f);
    set("fxSlot5", p.reverb > 0.01f ? 5.0f : 0.0f);
    set("fxSlot6", p.chorus > 0.01f ? 6.0f : 0.0f);

    // Keep factory patches predictable: modulation is opt-in unless a patch explicitly defines it.
    set("filterEnvAmount", 0.0f);
    for (int route = 1; route <= 4; ++route) {
        const auto suffix = juce::String(route);
        set(("mod" + suffix + "Source").toRawUTF8(), 0.0f);
        set(("mod" + suffix + "Target").toRawUTF8(), 0.0f);
        set(("mod" + suffix + "Amount").toRawUTF8(), 0.0f);
    }
    repaint();
}

void AuroraEditor::refreshSelectedStepControls() {
    const int pageOffset = seqPageBox.getSelectedId() == 2 ? 16 : 0;
    const int globalStep = pageOffset + selectedStep;
    const auto suffix = juce::String(globalStep);
    auto read = [this, &suffix](const juce::String& baseId) {
        if (auto* raw = proc.params.getRawParameterValue(baseId + suffix))
            return raw->load();
        return 0.0f;
    };
    selectedStepLabel.setText("STEP " + juce::String(globalStep + 1).paddedLeft('0', 2),
                              juce::dontSendNotification);
    stepPitch.setValue(read("stepPitch"), juce::dontSendNotification);
    stepVelocity.setValue(read("stepVelocity"), juce::dontSendNotification);
    stepGate.setValue(read("stepGate"), juce::dontSendNotification);
    stepAccentButton.setToggleState(read("stepAccent") > 0.5f, juce::dontSendNotification);
    stepAccentButton.setButtonText(read("stepAccent") > 0.5f ? "ACCENT ON" : "ACCENT OFF");
    for (int i = 0; i < 16; ++i) {
        const auto id = "stepOn" + juce::String(pageOffset + i);
        const bool enabled = proc.params.getRawParameterValue(id)->load() > 0.5f;
        stepButtons[(size_t)i].setToggleState(enabled, juce::dontSendNotification);
        stepButtons[(size_t)i].setButtonText(juce::String(pageOffset + i + 1).paddedLeft('0', 2));
    }
}

void AuroraEditor::writeSelectedStepParameter(const juce::String& baseId, float valueToSet) {
    const int pageOffset = seqPageBox.getSelectedId() == 2 ? 16 : 0;
    const auto id = baseId + juce::String(pageOffset + selectedStep);
    if (auto* parameter = proc.params.getParameter(id)) {
        parameter->beginChangeGesture();
        parameter->setValueNotifyingHost(parameter->convertTo0to1(valueToSet));
        parameter->endChangeGesture();
    }
}

void AuroraEditor::mouseDown(const juce::MouseEvent& event) {
    if(currentPage!=0)return;
    const float scale=juce::jmin(getWidth()/1200.0f,getHeight()/820.0f);
    const int ox=(getWidth()-juce::roundToInt(1200.0f*scale))/2;
    const int oy=(getHeight()-juce::roundToInt(820.0f*scale))/2;
    const float x=((float)event.x-(float)ox)/scale;
    const float y=((float)event.y-(float)oy)/scale;
    if(x<628.0f||x>1148.0f||y<248.0f||y>378.0f)return;
    drawingWavetable=true;
    drawingFrame=juce::jlimit(0,63,juce::roundToInt((float)wtPosition.getValue()*63.0f));
    previousDrawIndex=juce::jlimit(0,2047,juce::roundToInt((x-638.0f)/500.0f*2047.0f));
    previousDrawValue=juce::jlimit(-1.0f,1.0f,(313.0f-y)/50.0f);
    wtTable.setSelectedId(7,juce::sendNotification);
    editWavetableFromMouse(event);
}

void AuroraEditor::mouseDrag(const juce::MouseEvent& event) {
    if (drawingWavetable)
        editWavetableFromMouse(event);
}

void AuroraEditor::mouseUp(const juce::MouseEvent&) {
    drawingWavetable = false;
}

void AuroraEditor::editWavetableFromMouse(const juce::MouseEvent& event) {
    const float scale=juce::jmin(getWidth()/1200.0f,getHeight()/820.0f);
    const int ox=(getWidth()-juce::roundToInt(1200.0f*scale))/2;
    const int oy=(getHeight()-juce::roundToInt(820.0f*scale))/2;
    const float x=((float)event.x-(float)ox)/scale, y=((float)event.y-(float)oy)/scale;
    const int sampleIndex=juce::jlimit(0,2047,juce::roundToInt((x-638.0f)/500.0f*2047.0f));
    const float value=juce::jlimit(-1.0f,1.0f,(313.0f-y)/50.0f);
    proc.editImportedWavetableRange(drawingFrame,previousDrawIndex,sampleIndex,previousDrawValue,value);
    previousDrawIndex=sampleIndex; previousDrawValue=value;
    wavetableStatusLabel.setText("EDITING FRAME "+juce::String(drawingFrame+1)+" / 64",juce::dontSendNotification);
    repaint();
}

void AuroraEditor::timerCallback() {
    bool changed = false;
    const float outputPeak = proc.getOutputPeak();
    if (std::abs(outputPeak - lastOutputPeak) > 0.004f) {
        lastOutputPeak = outputPeak;
        changed = true;
    }
    const int activeStep = proc.getSequencerCurrentStep();
    if (activeStep != lastActiveStep) {
        lastActiveStep = activeStep;
        changed = true;
    }

    if (currentPage == 1) {
        for (int route = 0; route < 4; ++route) {
            const float activity = proc.getModRouteActivity(route);
            if (std::abs(activity - lastModRouteActivity[(size_t)route]) > 0.012f) {
                lastModRouteActivity[(size_t)route] = activity;
                changed = true;
            }
        }
    }

    if (changed)
        repaint();
}

void AuroraEditor::setPage(int page) {
    currentPage=juce::jlimit(0,2,page);
    synthTab.setToggleState(currentPage==0,juce::dontSendNotification);
    wtTab.setToggleState(currentPage==1,juce::dontSendNotification);
    fxTab.setToggleState(currentPage==2,juce::dontSendNotification);
    modTab.setVisible(false); seqTab.setVisible(false);
    auto show=[](std::initializer_list<juce::Component*> list,bool visible){
        for(auto* c:list)c->setVisible(visible);
    };
    show({&mix,&detune,&sub,&cutoff,&res,&attack,&decay,&sustain,&release,&master,&drive,&width,
          &wave1,&wave2,&oscModeBox,&filterTypeBox,&fmAmount,&syncRatio,&wtTable,&wtPosition,
          &unison,&uniDetune,&uniSpread,&importWavetableButton,&clearWavetableButton,&wavetableStatusLabel},currentPage==0);
    show({&lfo1Rate,&lfo2Rate,&lfo3Rate,&filterAttack,&filterDecay,&filterSustain,&filterRelease,
          &filterEnvAmount,&lfo1Shape,&lfo2Shape,&lfo3Shape},currentPage==1);
    for(int i=0;i<4;++i){
        modSources[(size_t)i].setVisible(currentPage==1);
        modTargets[(size_t)i].setVisible(currentPage==1);
        modAmounts[(size_t)i].setVisible(currentPage==1);
    }
    show({&seqModeBox,&arpStyleBox,&seqRateBox,&seqDirectionBox,&seqLengthBox,&seqPageBox,
          &seqPatternSlotBox,&seqOctaves,&seqGate,&seqSwing,&stepPitch,&stepVelocity,&stepGate,
          &stepAccentButton,&savePatternButton,&loadPatternButton,&selectedStepLabel,&patternStatusLabel},currentPage==1);
    for(auto& b:stepButtons)b.setVisible(currentPage==1);
    updateFxEditor();
    if(currentPage==1)refreshSelectedStepControls();
    startTimerHz(30); repaint(); resized();
}


void AuroraEditor::updateFxEditor() {
    const bool fxPage = currentPage == 2;
    for (auto& slot : fxSlots) slot.setVisible(fxPage);
    for (auto& button : fxSlotSelectButtons) button.setVisible(fxPage);
    for (auto& button : fxMoveUpButtons) button.setVisible(fxPage);
    for (auto& button : fxMoveDownButtons) button.setVisible(fxPage);

    for (juce::Component* component : std::array<juce::Component*, 4>{ &fxEq, &fxLow, &fxMid, &fxHigh })
        component->setVisible(fxPage);

    for (juce::Component* component : std::array<juce::Component*, 18>{
            &fxDist, &fxChorus, &fxFlange, &fxDelay, &fxReverb,
            &fxCompThreshold, &fxCompRatio, &fxCompAttack, &fxCompRelease,
            &fxCompMakeup, &fxCompMix, &fxPhaserRate, &fxPingTime,
            &fxPingFeedback, &fxReverbRoom, &fxReverbDamping,
            &fxLimiterCeiling, &fxCompEnabledButton })
        component->setVisible(false);
    for (auto& knob : fxExtraKnobs) knob.setVisible(false);
    fxFilterTypeBox.setVisible(false);

    for (int i = 0; i < 6; ++i)
        fxSlotSelectButtons[(size_t)i].setToggleState(i == selectedFxSlot, juce::dontSendNotification);

    if (!fxPage) return;

    const int effect = juce::jlimit(0, 14, fxSlots[(size_t)selectedFxSlot].getSelectedId() - 1);
    auto show = [](std::initializer_list<juce::Component*> components) {
        for (auto* component : components) component->setVisible(true);
    };

    auto showExtra = [&](std::initializer_list<int> indices) {
        for (const int index : indices)
            if (index >= 0 && index < fxExtraCount)
                fxExtraKnobs[(size_t)index].setVisible(true);
    };
    switch (effect) {
        case 1: show({ &fxCompEnabledButton, &fxCompThreshold, &fxCompRatio, &fxCompAttack,
                       &fxCompRelease, &fxCompMakeup, &fxCompMix }); break;
        case 2: show({ &fxDist }); showExtra({ fxSatTone, fxSatMix }); break;
        case 3: show({ &fxPhaserRate, &fxFlange }); showExtra({ fxPhaserFeedback }); break;
        case 4: show({ &fxPingTime, &fxPingFeedback, &fxDelay }); showExtra({ fxPingTone }); break;
        case 5: show({ &fxReverbRoom, &fxReverbDamping, &fxReverb }); showExtra({ fxReverbWidth }); break;
        case 6: show({ &fxChorus }); showExtra({ fxChorusRate, fxChorusDepth, fxChorusWidth }); break;
        case 7: show({ &fxDist }); showExtra({ fxMbLow, fxMbMid, fxMbHigh, fxMbMix }); break;
        case 8: show({ &fxLimiterCeiling }); showExtra({ fxLimiterInput, fxLimiterRelease, fxLimiterMix }); break;
        case 9: showExtra({ fxBitDepth, fxBitRate, fxBitMix }); break;
        case 10: showExtra({ fxStereoWidth, fxStereoPan, fxStereoMix }); break;
        case 11: showExtra({ fxTremRate, fxTremDepth, fxTremStereo, fxTremMix }); break;
        case 12: showExtra({ fxRingFreq, fxRingStereo, fxRingMix }); break;
        case 13:
            showExtra({ fxFilterCutoff, fxFilterRes, fxFilterMix });
            fxFilterTypeBox.setVisible(true);
            break;
        case 14: showExtra({ fxFlangerRate, fxFlangerDepth, fxFlangerFeedback, fxFlangerTime, fxFlangerMix }); break;
        default: break;
    }
}

void AuroraEditor::parameterChanged(const juce::String& parameterID, float) {
    if (parameterID == "wave1" || parameterID == "wave2" ||
        parameterID == "wtTable" || parameterID == "wtPosition")
        repaint();
}

void AuroraEditor::paint(juce::Graphics& g) {
    const float scale=juce::jmin(getWidth()/1200.0f,getHeight()/820.0f);
    const int ox=(getWidth()-juce::roundToInt(1200.0f*scale))/2;
    const int oy=(getHeight()-juce::roundToInt(820.0f*scale))/2;
    juce::ColourGradient bgMetal(juce::Colour(0xff41444d),0,0,juce::Colour(0xff11141b),1200,820,false);
    g.setGradientFill(bgMetal); g.fillAll();
    g.addTransform(juce::AffineTransform::translation((float)ox,(float)oy).scaled(scale));

    juce::ColourGradient chassis(juce::Colour(0xff3b3e46),18,18,juce::Colour(0xff171a21),1180,800,false);
    g.setGradientFill(chassis); g.fillRoundedRectangle(12,12,1176,796,18);
    for(int y=18;y<800;y+=4){
        g.setColour(juce::Colours::white.withAlpha((y%16==0)?0.032f:0.012f));
        g.drawLine(20,(float)y,1180,(float)y,0.5f);
    }
    g.setColour(juce::Colour(0xff858994).withAlpha(0.45f));
    g.drawRoundedRectangle(12.5f,12.5f,1175,795,18,1.0f);
    g.setColour(juce::Colour(0xff050609).withAlpha(0.8f));
    g.drawRoundedRectangle(16,16,1168,788,15,1.0f);

    auto screw=[&](float x,float y){
        juce::ColourGradient metal(juce::Colour(0xffa0a3ac),x,y-4,juce::Colour(0xff252830),x,y+4,false);
        g.setGradientFill(metal);g.fillEllipse(x-4,y-4,8,8);
        g.setColour(juce::Colour(0xff111319));g.drawLine(x-2.1f,y+1.8f,x+2.1f,y-1.8f,0.9f);
    };
    screw(25,25);screw(1175,25);screw(25,795);screw(1175,795);

    g.setColour(juce::Colour(0xff0c0f15));g.fillRoundedRectangle(28,24,1144,78,10);
    g.setColour(juce::Colour(0xff777b86).withAlpha(0.6f));g.drawRoundedRectangle(28.5f,24.5f,1143,77,10,1);
    g.setColour(violet);g.fillRoundedRectangle(40,34,4,42,2);
    g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(8.0f,juce::Font::bold)));
    g.drawText("TIMAUDIO  /  VIRTUAL ANALOG SYNTH",52,67,195,12,juce::Justification::centredLeft);
    g.setColour(white);g.setFont(juce::Font(juce::FontOptions(9.5f,juce::Font::bold)));
    g.drawText("PATCH",758,17,54,12,juce::Justification::centredLeft);
    g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(8.0f,juce::Font::bold)));
    g.drawText("OUT",1008,67,30,12,juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff080b11));g.fillRoundedRectangle(1040,70,112,6,3);
    g.setColour(juce::Colour(0xff737986).withAlpha(0.6f));g.drawRoundedRectangle(1040,70,112,6,3,0.8f);
    g.setColour(violetHi);g.fillRoundedRectangle(1040,70,112*juce::jlimit(0.0f,1.0f,proc.getOutputPeak()),6,3);

    auto panel=[&](int x,int y,int w,int h,const juce::String& title,const juce::String& sub){
        juce::ColourGradient face(juce::Colour(0xff41444d),(float)x,(float)y,juce::Colour(0xff20232b),(float)x,(float)(y+h),false);
        g.setGradientFill(face);g.fillRoundedRectangle((float)x,(float)y,(float)w,(float)h,10);
        g.setColour(juce::Colour(0xff9296a1).withAlpha(0.45f));g.drawRoundedRectangle(x+0.5f,y+0.5f,w-1.0f,h-1.0f,10,1);
        g.setColour(juce::Colour(0xff090b10).withAlpha(0.8f));g.drawRoundedRectangle(x+3.0f,y+3.0f,w-6.0f,h-6.0f,8,0.8f);
        for(int yy=y+5;yy<y+h-5;yy+=5){
            g.setColour(juce::Colours::white.withAlpha((yy%20==0)?0.022f:0.008f));
            g.drawLine(x+5.0f,(float)yy,x+w-5.0f,(float)yy,0.45f);
        }
        g.setColour(juce::Colour(0xff0b0d12).withAlpha(0.92f));g.fillRoundedRectangle(x+10.0f,y+10.0f,w-20.0f,48.0f,5);
        g.setColour(violet);g.fillRoundedRectangle(x+15.0f,y+20.0f,3,17,1.4f);
        g.setColour(white);g.setFont(juce::Font(juce::FontOptions(12.0f,juce::Font::bold)));
        g.drawText(title,x+26,y+14,w-40,18,juce::Justification::centredLeft);
        g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(7.8f,juce::Font::bold)));
        g.drawFittedText(sub,x+26,y+34,w-42,14,juce::Justification::centredLeft,1);
        g.setColour(juce::Colour(0xffa1a5ae).withAlpha(0.25f));g.drawLine(x+12.0f,y+64.0f,x+w-12.0f,y+64.0f,0.8f);
        screw((float)x+12,(float)y+12);screw((float)(x+w-12),(float)y+12);
    };
    auto label=[&](int x,int y,int w,const juce::String& t){
        g.setColour(juce::Colour(0xffc7c8cf));g.setFont(juce::Font(juce::FontOptions(8.2f,juce::Font::bold)));
        g.drawFittedText(t,x,y,w,13,juce::Justification::centred,1);
    };
    auto waveform=[&](int x,int y,int w,int h,int shape,const juce::String& title){
        juce::ColourGradient screen(juce::Colour(0xff080d13),(float)x,(float)y,juce::Colour(0xff151c27),(float)x,(float)(y+h),false);
        g.setGradientFill(screen);g.fillRoundedRectangle(x,y,w,h,5);
        g.setColour(juce::Colour(0xff7b8796).withAlpha(0.55f));g.drawRoundedRectangle(x+0.5f,y+0.5f,w-1.0f,h-1.0f,5,0.8f);
        for(int i=1;i<8;++i){float gx=x+(float)i*w/8.0f;g.setColour(juce::Colour(0xff8290a1).withAlpha(0.2f));g.drawLine(gx,y+5.0f,gx,y+h-5.0f,0.55f);}
        const float mid=y+h*0.58f;g.setColour(juce::Colour(0xff8793a2).withAlpha(0.3f));g.drawLine(x+6.0f,mid,x+w-6.0f,mid,0.6f);
        juce::Path p;
        for(int i=0;i<=192;++i){
            float t=(float)i/192.0f,v=0;
            switch(shape){case 1:v=t<0.5f?1.0f:-1.0f;break;case 2:v=1.0f-4.0f*std::abs(t-0.5f);break;case 3:v=std::sin(t*juce::MathConstants<float>::twoPi);break;default:v=2.0f*t-1.0f;break;}
            float px=x+6.0f+t*(w-12.0f),py=mid-v*h*0.3f;
            if(i==0)p.startNewSubPath(px,py);else p.lineTo(px,py);
        }
        g.setColour(violet.withAlpha(0.2f));g.strokePath(p,juce::PathStrokeType(5.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        g.setColour(violetHi);g.strokePath(p,juce::PathStrokeType(1.7f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        g.setColour(juce::Colour(0xffaeb4c0));g.setFont(juce::Font(juce::FontOptions(7.0f,juce::Font::bold)));
        g.drawText(title,x+8,y+3,w-16,10,juce::Justification::centredLeft);
    };

    g.setColour(juce::Colour(0xff969ba5).withAlpha(0.25f));g.drawLine(748,20,1160,20,0.8f);
    g.setColour(violet.withAlpha(0.9f));g.fillRoundedRectangle(32,106,1136,2,1);

    if(currentPage==0){
        panel(32,128,560,300,"OSCILLATOR SECTION","DUAL VCO  /  PHASE / FM / HARD SYNC");
        panel(608,128,560,300,"WAVETABLE LAB","64 FRAME MORPH ENGINE  /  DRAW OR IMPORT");
        panel(32,448,300,278,"FILTER","STATE VARIABLE / RESONANCE");
        panel(348,448,480,278,"AMPLIFIER ENVELOPE","ADSR / AMPLITUDE SHAPING");
        panel(844,448,324,278,"VOICE / OUTPUT","MIX / UNISON / MASTER");

        label(52,194,155,"OSCILLATOR 1");label(216,194,155,"OSCILLATOR 2");label(380,194,190,"ENGINE MODE");
        const int w1=(int)proc.params.getRawParameterValue("wave1")->load();
        const int w2=(int)proc.params.getRawParameterValue("wave2")->load();
        waveform(52,239,230,74,w1,"VCO 1");waveform(302,239,248,74,w2,"VCO 2");
        label(52,324,230,"FM AMOUNT");label(310,324,240,"HARD SYNC RATIO");

        label(628,193,240,"SOURCE BANK");label(880,193,132,"IMPORT AUDIO");label(1022,193,126,"CLEAR TABLE");
        label(628,380,520,"MORPH POSITION  /  DRAW ON THE SCOPE TO EDIT THE ACTIVE FRAME");
        const int table=(int)proc.params.getRawParameterValue("wtTable")->load();
        const float position=proc.params.getRawParameterValue("wtPosition")->load();
        auto tableWave=[this,table,position](float t){
            if(table==6)return proc.getImportedWavetableSample(t,position);
            const float saw=2.0f*t-1.0f,sine=std::sin(t*juce::MathConstants<float>::twoPi);
            const float square=t<0.5f?1.0f:-1.0f,triangle=1.0f-4.0f*std::abs(t-0.5f);
            float a=saw,b=sine;
            switch(table){case 1:a=square;b=triangle;break;case 2:a=triangle;b=sine;break;case 3:a=saw;b=square;break;case 4:a=sine;b=std::sin(t*juce::MathConstants<float>::twoPi*2.0f);break;case 5:a=saw;b=std::tanh(saw*3.0f);break;default:break;}
            return juce::jmap(position,a,b);
        };
        juce::ColourGradient scope(juce::Colour(0xff080d13),628,248,juce::Colour(0xff141b25),628,378,false);
        g.setGradientFill(scope);g.fillRoundedRectangle(628,248,520,130,6);
        g.setColour(juce::Colour(0xff778393).withAlpha(0.55f));g.drawRoundedRectangle(628.5f,248.5f,519,129,6,0.8f);
        for(int i=1;i<9;++i){float gx=628.0f+(float)i*520.0f/9.0f;g.setColour(juce::Colour(0xff8793a2).withAlpha(0.18f));g.drawLine(gx,255,gx,371,0.6f);}
        g.setColour(juce::Colour(0xff8793a2).withAlpha(0.3f));g.drawLine(638,313,1138,313,0.7f);
        juce::Path scopePath;
        for(int i=0;i<=320;++i){float t=(float)i/320.0f,px=638.0f+t*500.0f,py=313.0f-tableWave(t)*50.0f;if(i==0)scopePath.startNewSubPath(px,py);else scopePath.lineTo(px,py);}
        g.setColour(violet.withAlpha(0.22f));g.strokePath(scopePath,juce::PathStrokeType(7.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        g.setColour(violetHi);g.strokePath(scopePath,juce::PathStrokeType(1.8f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));

        label(52,514,260,"FILTER TYPE");label(52,566,116,"CUTOFF");label(184,566,116,"RESONANCE");
        label(368,520,100,"ATTACK");label(480,520,100,"DECAY");label(592,520,100,"SUSTAIN");label(704,520,100,"RELEASE");
        label(858,514,92,"OSC MIX");label(958,514,92,"DETUNE");label(1058,514,92,"SUB");
        label(858,584,92,"UNISON");label(958,584,92,"UNI DETUNE");label(1058,584,92,"SPREAD");
        label(858,654,92,"MASTER");label(958,654,92,"DRIVE");label(1058,654,92,"WIDTH");
    }else if(currentPage==1){
        panel(32,128,272,190,"LFO 1","GLOBAL MODULATOR");panel(316,128,272,190,"LFO 2","GLOBAL MODULATOR");
        panel(600,128,272,190,"LFO 3","GLOBAL MODULATOR");panel(884,128,284,190,"FILTER ENVELOPE","PER-VOICE CUTOFF SHAPING");
        label(50,200,90,"RATE");label(150,200,122,"WAVE SHAPE");label(334,200,90,"RATE");label(434,200,122,"WAVE SHAPE");
        label(618,200,90,"RATE");label(718,200,122,"WAVE SHAPE");label(900,200,116,"ATTACK");label(1024,200,116,"DECAY");
        label(900,235,116,"SUSTAIN");label(1024,235,116,"RELEASE");label(910,277,230,"FILTER ENV AMOUNT");
        panel(32,326,1136,140,"MODULATION MATRIX","FOUR ASSIGNABLE ROUTES  /  SOURCE TO TARGET");
        for(int i=0;i<4;++i){
            int x=48+i*282;
            g.setColour(juce::Colour(0xff0d1017).withAlpha(0.7f));g.fillRoundedRectangle((float)x,390,268,62,5);
            g.setColour(juce::Colour(0xff808592).withAlpha(0.25f));g.drawRoundedRectangle(x+0.5f,390.5f,267,61,5,0.7f);
            float activity=proc.getModRouteActivity(i);
            g.setColour(juce::Colour(0xff252a34));g.fillRoundedRectangle(x+9.0f,442,248,3,1.5f);
            g.setColour(violetHi);g.fillRoundedRectangle(x+9.0f,442,248*juce::jlimit(0.0f,1.0f,activity),3,1.5f);
        }
        panel(32,472,1136,254,"PERFORMANCE SEQUENCER","ARPEGGIATOR / 32 STEP PATTERN  /  HOST SYNC");
        label(52,538,140,"MODE");label(200,538,130,"ARP ORDER");label(338,538,100,"RATE");label(446,538,130,"DIRECTION");
        label(584,538,110,"LENGTH");label(704,538,140,"OCTAVES");label(852,538,140,"GLOBAL GATE");label(1000,538,150,"SWING");
        label(580,586,330,"PITCH");label(580,628,330,"VELOCITY");label(580,670,330,"STEP GATE");
        const int pageOffset=seqPageBox.getSelectedId()==2?16:0;
        if(lastActiveStep>=pageOffset&&lastActiveStep<pageOffset+16){
            const int local=lastActiveStep-pageOffset,x=52+(local%8)*61,y=610+(local/8)*34;
            g.setColour(violetHi.withAlpha(0.8f));g.drawRoundedRectangle((float)x-2,(float)y-2,58,32,4,1.5f);
        }
        const int selectedX=52+(selectedStep%8)*61,selectedY=610+(selectedStep/8)*34;
        g.setColour(violet.withAlpha(0.95f));g.drawRoundedRectangle((float)selectedX-1.5f,(float)selectedY-1.5f,57,31,4,1.4f);
    }else{
        panel(32,128,300,598,"FX RACK","SIGNAL FLOW / TOP TO BOTTOM");
        panel(348,128,820,420,"MODULE EDITOR","SELECT AN INSERT TO EDIT ITS CONTROLS");
        panel(348,560,820,166,"MASTER EQ","GLOBAL TONE SHAPING AFTER THE FX CHAIN");

        label(48,194,264,"PROCESSING ORDER");
        const juce::StringArray fxNames = { "OFF", "COMPRESSOR", "SATURATION", "PHASER",
            "PING-PONG DELAY", "REVERB", "CHORUS", "MULTIBAND DIST.", "LIMITER",
            "BITCRUSHER", "STEREO WIDTH", "TREMOLO", "RING MOD", "FILTER", "FLANGER" };
        for (int i = 0; i < 6; ++i) {
            const int rowY = 208 + i * 70;
            const bool selected = i == selectedFxSlot;
            auto row = juce::Rectangle<float>(44.0f, (float)rowY, 276.0f, 44.0f);
            g.setColour(selected ? juce::Colour(0xff302540) : juce::Colour(0xff11151d));
            g.fillRoundedRectangle(row, 6.0f);
            g.setColour(selected ? violetHi.withAlpha(0.85f) : juce::Colour(0xff707784).withAlpha(0.38f));
            g.drawRoundedRectangle(row, 6.0f, selected ? 1.2f : 0.75f);
            g.setColour(selected ? violetHi : muted);
            g.fillRoundedRectangle(row.getX() + 4.0f, row.getY() + 8.0f, 3.0f, 28.0f, 1.4f);
            g.setColour(muted);
            g.setFont(juce::Font(juce::FontOptions(7.2f, juce::Font::bold)));
            g.drawText("INSERT " + juce::String::formatted("%02d", i + 1),
                       48, rowY - 13, 110, 11, juce::Justification::centredLeft);
        }

        const int effect = juce::jlimit(0, 14, fxSlots[(size_t)selectedFxSlot].getSelectedId() - 1);
        const juce::String selectedName = fxNames[effect];
        g.setColour(violetHi);
        g.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold)));
        g.drawText("INSERT " + juce::String::formatted("%02d", selectedFxSlot + 1)
                   + "  /  " + selectedName, 374, 207, 760, 24, juce::Justification::centredLeft);

        auto fxLabel = [&](int column, const juce::String& text) {
            label(370 + column * 126, 286, 110, text);
        };
        switch (effect) {
            case 0:
                g.setColour(muted); g.setFont(juce::Font(juce::FontOptions(11.0f)));
                g.drawFittedText("EMPTY INSERT  —  CHOOSE AN EFFECT FROM THE SLOT MENU",
                                 380, 330, 740, 42, juce::Justification::centred, 1);
                break;
            case 1:
                fxLabel(0,"THRESHOLD");fxLabel(1,"RATIO");fxLabel(2,"ATTACK");
                fxLabel(3,"RELEASE");fxLabel(4,"MAKEUP");fxLabel(5,"MIX"); break;
            case 2: fxLabel(0,"DRIVE");fxLabel(1,"TONE");fxLabel(2,"MIX"); break;
            case 3: fxLabel(0,"RATE");fxLabel(1,"DEPTH");fxLabel(2,"FEEDBACK"); break;
            case 4: fxLabel(0,"TIME");fxLabel(1,"FEEDBACK");fxLabel(2,"TONE");fxLabel(3,"MIX"); break;
            case 5: fxLabel(0,"ROOM SIZE");fxLabel(1,"DAMPING");fxLabel(2,"MIX");fxLabel(3,"WIDTH"); break;
            case 6: fxLabel(0,"RATE");fxLabel(1,"DEPTH");fxLabel(2,"WIDTH");fxLabel(3,"MIX"); break;
            case 7: fxLabel(0,"DRIVE");fxLabel(1,"LOW DRIVE");fxLabel(2,"MID DRIVE");fxLabel(3,"HIGH DRIVE");fxLabel(4,"MIX"); break;
            case 8: fxLabel(0,"INPUT");fxLabel(1,"CEILING");fxLabel(2,"RELEASE");fxLabel(3,"MIX"); break;
            case 9: fxLabel(0,"BIT DEPTH");fxLabel(1,"RATE REDUCE");fxLabel(2,"MIX"); break;
            case 10: fxLabel(0,"WIDTH");fxLabel(1,"BALANCE");fxLabel(2,"MIX"); break;
            case 11: fxLabel(0,"RATE");fxLabel(1,"DEPTH");fxLabel(2,"STEREO PHASE");fxLabel(3,"MIX"); break;
            case 12: fxLabel(0,"FREQUENCY");fxLabel(1,"STEREO");fxLabel(2,"MIX"); break;
            case 13: fxLabel(0,"CUTOFF");fxLabel(1,"RESONANCE");fxLabel(2,"TYPE");fxLabel(3,"MIX"); break;
            case 14: fxLabel(0,"RATE");fxLabel(1,"DEPTH");fxLabel(2,"FEEDBACK");fxLabel(3,"TIME");fxLabel(4,"MIX"); break;
            default: break;
        }
        label(420,636,150,"EQ AMOUNT");label(600,636,150,"LOW");label(780,636,150,"MID");label(960,636,150,"HIGH");
    }
    g.setColour(juce::Colour(0xff9398a3).withAlpha(0.28f));g.drawLine(32,778,1168,778,0.8f);
    g.setColour(muted);g.setFont(juce::Font(juce::FontOptions(7.8f,juce::Font::bold)));
    g.drawText("2RICH  /  TIMAUDIO",36,784,260,12,juce::Justification::centredLeft);
    g.drawText("VOICE  /  MOTION  /  FX",850,784,318,12,juce::Justification::centredRight);
}

void AuroraEditor::resized() {
    const float scale=juce::jmin(getWidth()/1200.0f,getHeight()/820.0f);
    const int ox=(getWidth()-juce::roundToInt(1200.0f*scale))/2,oy=(getHeight()-juce::roundToInt(820.0f*scale))/2;
    auto place=[&](juce::Component& c,juce::Rectangle<int> b){c.setBounds(scaledBounds(b,scale,ox,oy));};

    place(heading,{52,26,176,40});place(synthTab,{258,35,86,32});place(wtTab,{352,35,124,32});place(fxTab,{484,35,72,32});
    place(presetSearch,{758,34,142,29});place(presets,{908,34,244,29});
    place(presetFavoriteButton,{758,70,66,22});place(savePresetButton,{832,70,66,22});place(loadPresetButton,{906,70,66,22});
    modTab.setVisible(false);seqTab.setVisible(false);

    // VOICE: oscillator and wavetable top row.
    place(wave1,{52,207,155,28});place(wave2,{216,207,155,28});place(oscModeBox,{380,207,190,28});
    place(fmAmount,{52,343,230,24});place(syncRatio,{310,343,240,24});
    place(wtTable,{628,207,240,28});place(importWavetableButton,{880,207,132,28});place(clearWavetableButton,{1022,207,126,28});
    place(wavetableStatusLabel,{628,232,520,15});place(wtPosition,{628,393,520,23});
    place(filterTypeBox,{52,532,260,26});place(cutoff,{52,580,116,132});place(res,{184,580,116,132});
    place(attack,{368,535,100,172});place(decay,{480,535,100,172});place(sustain,{592,535,100,172});place(release,{704,535,100,172});
    const int vx[3]={858,958,1058},vy[3]={528,598,668};
    juce::Slider* voice[9]={&mix,&detune,&sub,&unison,&uniDetune,&uniSpread,&master,&drive,&width};
    for(int i=0;i<9;++i)place(*voice[i],{vx[i%3],vy[i/3],92,54});

    // MOTION: LFOs and filter envelope.
    place(lfo1Rate,{50,216,90,82});place(lfo1Shape,{150,224,122,30});
    place(lfo2Rate,{334,216,90,82});place(lfo2Shape,{434,224,122,30});
    place(lfo3Rate,{618,216,90,82});place(lfo3Shape,{718,224,122,30});
    place(filterAttack,{900,213,116,24});place(filterDecay,{1024,213,116,24});
    place(filterSustain,{900,249,116,24});place(filterRelease,{1024,249,116,24});
    place(filterEnvAmount,{910,292,230,22});
    for(int i=0;i<4;++i){
        const int x=48+i*282;
        place(modSources[(size_t)i],{x+8,398,124,28});
        place(modTargets[(size_t)i],{x+140,398,120,28});
        place(modAmounts[(size_t)i],{x+8,426,252,24});
    }

    // Sequencer performance controls, 16 pads and selected-step inspector.
    place(seqModeBox,{52,554,140,28});place(arpStyleBox,{200,554,130,28});place(seqRateBox,{338,554,100,28});
    place(seqDirectionBox,{446,554,130,28});place(seqLengthBox,{584,554,110,28});
    place(seqOctaves,{704,554,140,28});place(seqGate,{852,554,140,28});place(seqSwing,{1000,554,150,28});
    for(int i=0;i<16;++i){const int x=52+(i%8)*61,y=610+(i/8)*34;place(stepButtons[(size_t)i],{x,y,56,28});}
    place(stepPitch,{580,600,330,24});place(stepVelocity,{580,642,330,24});place(stepGate,{580,684,330,24});
    place(selectedStepLabel,{930,586,100,24});place(stepAccentButton,{1040,586,110,28});
    place(seqPageBox,{930,620,100,28});place(seqPatternSlotBox,{1040,620,110,28});
    place(savePatternButton,{930,652,100,28});place(loadPatternButton,{1040,652,110,28});place(patternStatusLabel,{930,690,220,18});

    // FX rack: safe, roomy rotary bounds and compact horizontal master-EQ faders.
    for (int i = 0; i < 6; ++i) {
        const int y = 214 + i * 70;
        place(fxSlotSelectButtons[(size_t)i], {48,y,28,32});
        place(fxSlots[(size_t)i], {80,y,154,32});
        place(fxMoveUpButtons[(size_t)i], {238,y,23,32});
        place(fxMoveDownButtons[(size_t)i], {265,y,23,32});
    }
    const int fxColumnX[6] = { 370, 496, 622, 748, 874, 1000 };
    auto placeFxKnob = [&](juce::Slider& slider, int column) {
        place(slider, {fxColumnX[column], 316, 110, 182});
    };
    place(fxCompEnabledButton,{370,238,180,28});
    placeFxKnob(fxCompThreshold,0);placeFxKnob(fxCompRatio,1);placeFxKnob(fxCompAttack,2);
    placeFxKnob(fxCompRelease,3);placeFxKnob(fxCompMakeup,4);placeFxKnob(fxCompMix,5);

    placeFxKnob(fxDist,0);placeFxKnob(fxExtraKnobs[(size_t)fxSatTone],1);placeFxKnob(fxExtraKnobs[(size_t)fxSatMix],2);
    placeFxKnob(fxPhaserRate,0);placeFxKnob(fxFlange,1);placeFxKnob(fxExtraKnobs[(size_t)fxPhaserFeedback],2);
    placeFxKnob(fxPingTime,0);placeFxKnob(fxPingFeedback,1);placeFxKnob(fxExtraKnobs[(size_t)fxPingTone],2);placeFxKnob(fxDelay,3);
    placeFxKnob(fxReverbRoom,0);placeFxKnob(fxReverbDamping,1);placeFxKnob(fxReverb,2);placeFxKnob(fxExtraKnobs[(size_t)fxReverbWidth],3);
    placeFxKnob(fxExtraKnobs[(size_t)fxChorusRate],0);placeFxKnob(fxExtraKnobs[(size_t)fxChorusDepth],1);
    placeFxKnob(fxExtraKnobs[(size_t)fxChorusWidth],2);placeFxKnob(fxChorus,3);
    placeFxKnob(fxDist,0);placeFxKnob(fxExtraKnobs[(size_t)fxMbLow],1);placeFxKnob(fxExtraKnobs[(size_t)fxMbMid],2);
    placeFxKnob(fxExtraKnobs[(size_t)fxMbHigh],3);placeFxKnob(fxExtraKnobs[(size_t)fxMbMix],4);
    placeFxKnob(fxExtraKnobs[(size_t)fxLimiterInput],0);placeFxKnob(fxLimiterCeiling,1);
    placeFxKnob(fxExtraKnobs[(size_t)fxLimiterRelease],2);placeFxKnob(fxExtraKnobs[(size_t)fxLimiterMix],3);

    placeFxKnob(fxExtraKnobs[(size_t)fxBitDepth],0);placeFxKnob(fxExtraKnobs[(size_t)fxBitRate],1);placeFxKnob(fxExtraKnobs[(size_t)fxBitMix],2);
    placeFxKnob(fxExtraKnobs[(size_t)fxStereoWidth],0);placeFxKnob(fxExtraKnobs[(size_t)fxStereoPan],1);placeFxKnob(fxExtraKnobs[(size_t)fxStereoMix],2);
    placeFxKnob(fxExtraKnobs[(size_t)fxTremRate],0);placeFxKnob(fxExtraKnobs[(size_t)fxTremDepth],1);
    placeFxKnob(fxExtraKnobs[(size_t)fxTremStereo],2);placeFxKnob(fxExtraKnobs[(size_t)fxTremMix],3);
    placeFxKnob(fxExtraKnobs[(size_t)fxRingFreq],0);placeFxKnob(fxExtraKnobs[(size_t)fxRingStereo],1);placeFxKnob(fxExtraKnobs[(size_t)fxRingMix],2);
    placeFxKnob(fxExtraKnobs[(size_t)fxFilterCutoff],0);placeFxKnob(fxExtraKnobs[(size_t)fxFilterRes],1);
    placeFxKnob(fxExtraKnobs[(size_t)fxFilterMix],3);place(fxFilterTypeBox,{622,382,110,30});
    placeFxKnob(fxExtraKnobs[(size_t)fxFlangerRate],0);placeFxKnob(fxExtraKnobs[(size_t)fxFlangerDepth],1);
    placeFxKnob(fxExtraKnobs[(size_t)fxFlangerFeedback],2);placeFxKnob(fxExtraKnobs[(size_t)fxFlangerTime],3);
    placeFxKnob(fxExtraKnobs[(size_t)fxFlangerMix],4);

    place(fxEq,{420,660,150,28});place(fxLow,{600,660,150,28});
    place(fxMid,{780,660,150,28});place(fxHigh,{960,660,150,28});
}

