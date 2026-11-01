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
        const float size = juce::jmax(28.0f, (float)juce::jmin(w, h) - 25.0f);
        const float cx = x + w * 0.5f;
        const float cy = y + h * 0.42f;
        const float radius = size * 0.5f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);

        g.setColour(juce::Colour(0xff2e2841));
        g.drawEllipse(cx - radius, cy - radius, size, size, 4.0f);

        juce::Path arc;
        arc.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, angle, true);
        g.setColour(violet);
        g.strokePath(arc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        g.setColour(juce::Colour(0xff211c33));
        g.fillEllipse(cx - radius + 6.0f, cy - radius + 6.0f, size - 12.0f, size - 12.0f);
        g.setColour(juce::Colour(0xff302748));
        g.drawEllipse(cx - radius + 8.0f, cy - radius + 8.0f, size - 16.0f, size - 16.0f, 1.0f);

        juce::Path pointer;
        pointer.addRoundedRectangle(-1.6f, -radius + 10.0f, 3.2f, radius * 0.48f, 1.6f);
        g.setColour(violetHi);
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(cx, cy));
        g.setColour(white.withAlpha(0.9f));
        g.fillEllipse(cx - 2.0f, cy - 2.0f, 4.0f, 4.0f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool highlighted, bool down) override {
        auto r = button.getLocalBounds().toFloat().reduced(0.5f);
        const bool active = button.getToggleState();
        if (active) {
            juce::ColourGradient fill(juce::Colour(0xff6742a7), r.getX(), r.getY(),
                                      juce::Colour(0xff39265f), r.getRight(), r.getBottom(), false);
            g.setGradientFill(fill);
        } else {
            g.setColour(down ? cardHi : surface);
        }
        g.fillRoundedRectangle(r, 8.0f);
        g.setColour(active ? violetHi : (highlighted ? violet.withAlpha(0.8f) : border));
        g.drawRoundedRectangle(r, 8.0f, active ? 1.5f : 1.0f);
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

    presets.addSectionHeading("LEADS");
    presets.addItem("Violet Lead", 1);
    presets.addItem("Neon Saw", 2);
    presets.addItem("Hollow Lead", 3);
    presets.addSectionHeading("PADS");
    presets.addItem("Cloud Pad", 4);
    presets.addItem("Glass Cathedral", 5);
    presets.addItem("Slow Motion", 6);
    presets.addSectionHeading("KEYS");
    presets.addItem("Digital Keys", 7);
    presets.addItem("Soft Piano-ish", 8);
    presets.addItem("Pluck Keys", 9);
    presets.addSectionHeading("BASS");
    presets.addItem("Sub Pressure",10); presets.addItem("Reese Machine",11); presets.addItem("Acid Bloom",12); presets.addItem("Neon Bass",13);
    presets.addSectionHeading("PLUCKS");
    presets.addItem("Glass Pluck",14); presets.addItem("Cyber Mallet",15); presets.addItem("Crystal Keys",16);
    presets.addSectionHeading("ATMOSPHERE");
    presets.addItem("Aurora Drift",17); presets.addItem("Deep Space",18); presets.addItem("Ethereal Choir",19); presets.addItem("Frozen Sky",20);
    presets.addSectionHeading("FX & SYNTH");
    presets.addItem("Laser Bloom",21); presets.addItem("Retro Pulse",22); presets.addItem("Wide Unison",23); presets.addItem("Dirty Motion",24); presets.addItem("Dream Sequence",25);
    // Expansion categories are reserved for future user-created banks.

    presets.setTextWhenNothingSelected("SELECT PRESET");
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
        "Ping-Pong", "Reverb", "Chorus", "Multiband Dist.", "Limiter" };
    for (int i = 0; i < 6; ++i) {
        addChoice(fxSlots[(size_t)i], fxTypeNames, "fxSlot" + juce::String(i + 1));
        fxSlots[(size_t)i].setTooltip("FX Chain slot " + juce::String(i + 1) + ". Signal flows from slot 1 to slot 6.");
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
    addExtra(wtPosition,"wtPosition","POSITION"); addExtra(unison,"unison","VOICES"); addExtra(uniDetune,"uniDetune","DETUNE"); addExtra(uniSpread,"uniSpread","SPREAD");
    addExtra(fmAmount,"fmAmount","FM AMOUNT"); addExtra(syncRatio,"syncRatio","SYNC RATIO");
    for (auto* slider : { &fmAmount, &syncRatio }) {
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
    }
    addExtra(fxEq,"fxEq","EQ MIX"); addExtra(fxLow,"fxLow","LOW"); addExtra(fxMid,"fxMid","MID"); addExtra(fxHigh,"fxHigh","HIGH");
    addExtra(fxDist,"fxDist","SATURATION"); addExtra(fxChorus,"fxChorus","CHORUS"); addExtra(fxFlange,"fxFlange","FLANGER"); addExtra(fxDelay,"fxDelay","DELAY"); addExtra(fxReverb,"fxReverb","REVERB");
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
    for (auto* slider : { &fxPhaserRate, &fxPingTime, &fxPingFeedback, &fxReverbRoom, &fxReverbDamping, &fxLimiterCeiling }) {
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 38, 18);
    }
    addExtra(lfo1Rate,"lfo1Rate","RATE"); addExtra(lfo2Rate,"lfo2Rate","RATE");
    addExtra(lfo3Rate,"lfo3Rate","RATE");
    addExtra(filterAttack,"filterAttack","ATTACK"); addExtra(filterDecay,"filterDecay","DECAY");
    addExtra(filterSustain,"filterSustain","SUSTAIN"); addExtra(filterRelease,"filterRelease","RELEASE");
    addExtra(filterEnvAmount,"filterEnvAmount","AMOUNT");
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
    synthTab.onClick=[this]{setPage(0);};wtTab.onClick=[this]{setPage(1);};fxTab.onClick=[this]{setPage(2);};modTab.onClick=[this]{setPage(3);};seqTab.onClick=[this]{setPage(4);};
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
    for (auto* button : { &presetFavoriteButton, &savePresetButton, &loadPresetButton })
        button->setLookAndFeel(nullptr);
    fxCompEnabledButton.setLookAndFeel(nullptr);
    lfo1Shape.setLookAndFeel(nullptr); lfo2Shape.setLookAndFeel(nullptr); lfo3Shape.setLookAndFeel(nullptr);
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
    static const char* names[] = {
        "Violet Lead", "Neon Saw", "Hollow Lead",
        "Cloud Pad", "Glass Cathedral", "Slow Motion",
        "Digital Keys", "Soft Piano-ish", "Pluck Keys",
        "Sub Pressure", "Reese Machine", "Acid Bloom", "Neon Bass",
        "Glass Pluck", "Cyber Mallet", "Crystal Keys",
        "Aurora Drift", "Deep Space", "Ethereal Choir", "Frozen Sky",
        "Laser Bloom", "Retro Pulse", "Wide Unison", "Dirty Motion", "Dream Sequence"
    };
    const auto rawQuery = presetSearch.getText().trim().toLowerCase();
    const bool favouritesOnly = rawQuery.startsWith("fav:");
    const auto query = favouritesOnly ? rawQuery.substring(4).trim() : rawQuery;
    const int previousId = presets.getSelectedId();
    presets.clear();

    const struct Category { const char* name; int first; int last; } categories[] = {
        { "LEADS", 1, 3 }, { "PADS", 4, 6 }, { "KEYS", 7, 9 },
        { "BASS", 10, 13 }, { "PLUCKS", 14, 16 },
        { "ATMOSPHERE", 17, 20 }, { "FX & SYNTH", 21, 25 }
    };

    if (query.isEmpty() && !favouritesOnly) {
        for (const auto& category : categories) {
            presets.addSectionHeading(category.name);
            for (int id = category.first; id <= category.last; ++id)
                presets.addItem(names[id - 1], id);
        }
    } else {
        for (int id = 1; id <= 25; ++id) {
            const bool matchesName = query.isEmpty()
                || juce::String(names[id - 1]).toLowerCase().contains(query);
            if (matchesName && (!favouritesOnly || proc.isPresetFavorite(id)))
                presets.addItem(names[id - 1], id);
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
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 66, 19);
    s.setColour(juce::Slider::textBoxTextColourId, white);
    s.setColour(juce::Slider::textBoxBackgroundColourId, cardHi);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    s.setColour(juce::Slider::textBoxHighlightColourId, violet);
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
    if (currentPage != 1)
        return;

    const float scale = juce::jmin(getWidth() / 1200.0f, getHeight() / 820.0f);
    const int ox = (getWidth() - juce::roundToInt(1200.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(820.0f * scale)) / 2;
    const float x = ((float)event.x - (float)ox) / scale;
    const float y = ((float)event.y - (float)oy) / scale;
    if (x < 342.0f || x > 1144.0f || y < 210.0f || y > 366.0f)
        return;

    drawingWavetable = true;
    drawingFrame = juce::jlimit(0, 63, juce::roundToInt((float)wtPosition.getValue() * 63.0f));
    previousDrawIndex = juce::jlimit(0, 2047,
        juce::roundToInt((x - 352.0f) / 782.0f * 2047.0f));
    previousDrawValue = juce::jlimit(-1.0f, 1.0f, (296.0f - y) / 58.0f);
    wtTable.setSelectedId(7, juce::sendNotification);
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
    const float scale = juce::jmin(getWidth() / 1200.0f, getHeight() / 820.0f);
    const int ox = (getWidth() - juce::roundToInt(1200.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(820.0f * scale)) / 2;
    const float x = ((float)event.x - (float)ox) / scale;
    const float y = ((float)event.y - (float)oy) / scale;

    const int sampleIndex = juce::jlimit(0, 2047,
        juce::roundToInt((x - 352.0f) / 782.0f * 2047.0f));
    const float value = juce::jlimit(-1.0f, 1.0f, (296.0f - y) / 58.0f);
    proc.editImportedWavetableRange(drawingFrame, previousDrawIndex, sampleIndex,
                                    previousDrawValue, value);
    previousDrawIndex = sampleIndex;
    previousDrawValue = value;
    wavetableStatusLabel.setText("EDITING FRAME " + juce::String(drawingFrame + 1)
                                 + " / 64", juce::dontSendNotification);
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

    if (currentPage == 3) {
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
    currentPage = page;
    synthTab.setToggleState(page == 0, juce::dontSendNotification);
    wtTab.setToggleState(page == 1, juce::dontSendNotification);
    fxTab.setToggleState(page == 2, juce::dontSendNotification);
    modTab.setToggleState(page == 3, juce::dontSendNotification);
    seqTab.setToggleState(page == 4, juce::dontSendNotification);
    auto show = [page](std::initializer_list<juce::Component*> list, bool visible) {
        for (auto* component : list) component->setVisible(visible);
    };
    show({ &mix,&detune,&sub,&cutoff,&res,&attack,&decay,&sustain,&release,&master,&drive,&width,&wave1,&wave2,
           &oscModeBox,&filterTypeBox,&fmAmount,&syncRatio }, page == 0);
    show({ &wtTable,&wtPosition,&unison,&uniDetune,&uniSpread,&importWavetableButton,
           &clearWavetableButton,&wavetableStatusLabel }, page == 1);
    show({ &fxEq,&fxLow,&fxMid,&fxHigh,&fxDist,&fxChorus,&fxFlange,&fxDelay,&fxReverb,
            &fxCompThreshold,&fxCompRatio,&fxCompAttack,&fxCompRelease,&fxCompMakeup,&fxCompMix,
            &fxCompEnabledButton,&fxPhaserRate,&fxPingTime,&fxPingFeedback,&fxReverbRoom,
            &fxReverbDamping,&fxLimiterCeiling }, page == 2);
    for (auto& slot : fxSlots)
        slot.setVisible(page == 2);
    show({ &lfo1Rate,&lfo2Rate,&lfo3Rate,&filterAttack,&filterDecay,&filterSustain,&filterRelease,&filterEnvAmount,
           &lfo1Shape,&lfo2Shape,&lfo3Shape }, page == 3);
    for (int i = 0; i < 4; ++i) {
        modSources[(size_t)i].setVisible(page == 3);
        modTargets[(size_t)i].setVisible(page == 3);
        modAmounts[(size_t)i].setVisible(page == 3);
    }
    show({ &seqModeBox, &arpStyleBox, &seqRateBox, &seqDirectionBox, &seqLengthBox,
           &seqPageBox, &seqPatternSlotBox, &seqOctaves, &seqGate, &seqSwing,
           &stepPitch, &stepVelocity, &stepGate, &stepAccentButton, &savePatternButton,
           &loadPatternButton, &selectedStepLabel, &patternStatusLabel }, page == 4);
    for (auto& button : stepButtons)
        button.setVisible(page == 4);
    if (page == 4)
        refreshSelectedStepControls();
    startTimerHz(30);
    repaint();
    resized();
}


void AuroraEditor::parameterChanged(const juce::String& parameterID, float) {
    if (parameterID == "wave1" || parameterID == "wave2" ||
        parameterID == "wtTable" || parameterID == "wtPosition")
        repaint();
}

void AuroraEditor::paint(juce::Graphics& g) {
    juce::ColourGradient background(juce::Colour(0xff171026), 0.0f, 0.0f,
                                    bg, 1200.0f, 820.0f, false);
    g.setGradientFill(background);
    g.fillAll();

    const float scale = juce::jmin(getWidth() / 1200.0f, getHeight() / 820.0f);
    const int ox = (getWidth() - juce::roundToInt(1200.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(820.0f * scale)) / 2;
    g.addTransform(juce::AffineTransform::translation((float)ox, (float)oy).scaled(scale));

    g.setColour(surface);
    g.fillRoundedRectangle(12.0f, 12.0f, 1176.0f, 796.0f, 19.0f);
    g.setColour(border);
    g.drawRoundedRectangle(12.5f, 12.5f, 1175.0f, 795.0f, 19.0f, 1.0f);

    // Header / patch browser.
    g.setColour(violet);
    g.fillRoundedRectangle(34.0f, 26.0f, 4.0f, 40.0f, 2.0f);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
    g.drawText("NORTHSTAR AUDIO  /  VIRTUAL INSTRUMENT", 43, 65, 205, 12,
               juce::Justification::centredLeft);
    g.setColour(white);
    g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.drawText("PATCH BROWSER", 748, 17, 170, 12, juce::Justification::centredLeft);

    g.setColour(border);
    g.drawLine(32.0f, 108.0f, 1168.0f, 108.0f, 1.0f);
    g.setColour(violet.withAlpha(0.9f));
    g.fillRoundedRectangle(32.0f, 106.0f, 190.0f, 3.0f, 1.5f);

    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
    g.drawText("OUT", 982, 72, 30, 12, juce::Justification::centredLeft);
    g.setColour(cardHi);
    g.fillRoundedRectangle(1013.0f, 76.0f, 155.0f, 5.0f, 2.5f);
    g.setColour(violetHi.withAlpha(0.95f));
    g.fillRoundedRectangle(1013.0f, 76.0f,
        155.0f * juce::jlimit(0.0f, 1.0f, proc.getOutputPeak()), 5.0f, 2.5f);

    auto panel = [&](int x, int y, int w, int h, const juce::String& title,
                     const juce::String& subtitle) {
        g.setColour(card);
        g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 12.0f);
        g.setColour(border.withAlpha(0.9f));
        g.drawRoundedRectangle((float)x + 0.5f, (float)y + 0.5f,
                               (float)w - 1.0f, (float)h - 1.0f, 12.0f, 1.0f);
        g.setColour(violet);
        g.fillRoundedRectangle((float)x + 15.0f, (float)y + 15.0f, 3.0f, 17.0f, 1.5f);
        g.setColour(white);
        g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        g.drawText(title, x + 27, y + 11, w - 42, 20, juce::Justification::centredLeft);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.8f)));
        g.drawText(subtitle, x + 27, y + 33, w - 42, 14, juce::Justification::centredLeft);
        g.setColour(border.withAlpha(0.75f));
        g.drawLine((float)x + 14.0f, (float)y + 60.0f,
                   (float)(x + w - 14), (float)y + 60.0f, 1.0f);
    };

    auto label = [&](int x, int y, int w, const juce::String& text) {
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.8f, juce::Font::bold)));
        g.drawFittedText(text, x, y, w, 14, juce::Justification::centred, 1);
    };

    auto waveform = [&](int x, int y, int w, int h, int shape, const juce::String& title) {
        g.setColour(juce::Colour(0xff100e1a));
        g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 8.0f);
        g.setColour(border);
        g.drawRoundedRectangle((float)x + 0.5f, (float)y + 0.5f,
                               (float)w - 1.0f, (float)h - 1.0f, 8.0f, 1.0f);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::bold)));
        g.drawText(title, x + 10, y + 7, w - 20, 12, juce::Justification::centredLeft);
        const float mid = y + h * 0.62f;
        const float amp = h * 0.28f;
        g.setColour(border);
        g.drawLine((float)x + 9.0f, mid, (float)(x + w - 9), mid, 0.8f);
        juce::Path path;
        for (int i = 0; i <= 192; ++i) {
            const float t = (float)i / 192.0f;
            float value = 0.0f;
            switch (shape) {
                case 1: value = t < 0.5f ? 1.0f : -1.0f; break;
                case 2: value = 1.0f - 4.0f * std::abs(t - 0.5f); break;
                case 3: value = std::sin(t * juce::MathConstants<float>::twoPi); break;
                default: value = 2.0f * t - 1.0f; break;
            }
            const float px = x + 9.0f + t * (w - 18.0f);
            const float py = mid - value * amp;
            if (i == 0) path.startNewSubPath(px, py); else path.lineTo(px, py);
        }
        g.setColour(violet.withAlpha(0.15f));
        g.strokePath(path, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
        g.setColour(violetHi);
        g.strokePath(path, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    };

    if (currentPage == 0) {
        panel(32, 128, 250, 276, "OSCILLATOR 1", "PRIMARY WAVEFORM");
        panel(294, 128, 250, 276, "OSCILLATOR 2", "SECONDARY / MODULATOR");
        panel(556, 128, 280, 276, "VOICE MIX", "BLEND / DETUNE / SUB");
        panel(848, 128, 320, 276, "FILTER", "TONE SHAPING");
        panel(32, 410, 700, 316, "AMPLIFIER ENVELOPE", "ATTACK / DECAY / SUSTAIN / RELEASE");
        panel(744, 410, 424, 316, "OUTPUT", "LEVEL / DRIVE / STEREO WIDTH");

        const int w1 = (int)proc.params.getRawParameterValue("wave1")->load();
        const int w2 = (int)proc.params.getRawParameterValue("wave2")->load();
        waveform(52, 238, 210, 80, w1, "OSC 1 WAVE");
        waveform(314, 238, 210, 80, w2, "OSC 2 WAVE");
        label(52, 190, 210, "WAVEFORM");
        label(314, 190, 210, "WAVEFORM");
        label(572, 190, 248, "ENGINE MODE");
        label(314, 320, 210, "FM AMOUNT");
        label(314, 359, 210, "HARD SYNC RATIO");
        label(568, 230, 80, "OSC MIX");
        label(656, 230, 80, "DETUNE");
        label(744, 230, 80, "SUB LEVEL");
        label(867, 190, 280, "FILTER TYPE");
        label(874, 252, 128, "CUTOFF");
        label(1022, 252, 128, "RESONANCE");
        label(62, 480, 142, "ATTACK");
        label(224, 480, 142, "DECAY");
        label(386, 480, 142, "SUSTAIN");
        label(548, 480, 142, "RELEASE");
        label(768, 480, 112, "MASTER");
        label(900, 480, 112, "DRIVE");
        label(1032, 480, 112, "WIDTH");
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f)));
        g.drawText("SELECT A SHAPE TO CHANGE THE OSCILLATOR", 52, 355, 210, 14,
                   juce::Justification::centredLeft);
    }
    else if (currentPage == 1) {
        panel(32, 128, 270, 268, "TABLE SOURCE", "SELECT / IMPORT / MORPH");
        panel(318, 128, 850, 268, "WAVETABLE DISPLAY", "64-FRAME MORPH TABLE / DRAW TO EDIT");
        panel(32, 414, 1136, 312, "UNISON ENGINE", "VOICE STACK / DETUNE / STEREO SPREAD");

        label(54, 190, 226, "WAVETABLE BANK");
        label(82, 226, 166, "MORPH POSITION");
        const int table = (int)proc.params.getRawParameterValue("wtTable")->load();
        const float position = proc.params.getRawParameterValue("wtPosition")->load();
        auto tableWave = [this, table, position](float t) {
            if (table == 6)
                return proc.getImportedWavetableSample(t, position);
            const float saw = 2.0f * t - 1.0f;
            const float sine = std::sin(t * juce::MathConstants<float>::twoPi);
            const float square = t < 0.5f ? 1.0f : -1.0f;
            const float triangle = 1.0f - 4.0f * std::abs(t - 0.5f);
            float a = saw, b = sine;
            switch (table) {
                case 1: a = square; b = triangle; break;
                case 2: a = triangle; b = sine; break;
                case 3: a = saw; b = square; break;
                case 4: a = sine; b = std::sin(t * juce::MathConstants<float>::twoPi * 2.0f); break;
                case 5: a = saw; b = std::tanh(saw * 3.0f); break;
                default: a = saw; b = sine; break;
            }
            return juce::jmap(position, a, b);
        };
        const int gx = 342, gy = 210, gw = 802, gh = 156;
        g.setColour(juce::Colour(0xff100e1a));
        g.fillRoundedRectangle((float)gx, (float)gy, (float)gw, (float)gh, 9.0f);
        g.setColour(border);
        g.drawRoundedRectangle((float)gx + 0.5f, (float)gy + 0.5f,
                               (float)gw - 1.0f, (float)gh - 1.0f, 9.0f, 1.0f);
        const float mid = gy + gh * 0.55f;
        g.setColour(border);
        for (int i = 1; i < 8; ++i) {
            const float gxLine = gx + (float)i * gw / 8.0f;
            g.drawLine(gxLine, (float)gy + 8.0f, gxLine, (float)(gy + gh - 8), 0.5f);
        }
        g.drawLine((float)gx + 10.0f, mid, (float)(gx + gw - 10), mid, 0.8f);
        juce::Path preview;
        for (int i = 0; i <= 320; ++i) {
            const float t = (float)i / 320.0f;
            const float px = gx + 10.0f + t * (gw - 20.0f);
            const float py = mid - tableWave(t) * 58.0f;
            if (i == 0) preview.startNewSubPath(px, py); else preview.lineTo(px, py);
        }
        g.setColour(violet.withAlpha(0.14f));
        g.strokePath(preview, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
        g.setColour(violetHi);
        g.strokePath(preview, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::bold)));
        g.drawText("FRAME 01", gx + 12, gy + gh - 18, 90, 12, juce::Justification::centredLeft);
        g.drawText("FRAME 64", gx + gw - 102, gy + gh - 18, 90, 12, juce::Justification::centredRight);
        g.setColour(violetHi);
        g.drawText("DRAW DIRECTLY ON THE WAVEFORM TO EDIT THE SELECTED FRAME",
                   gx + 12, gy + gh + 8, gw - 24, 14, juce::Justification::centred);
        label(174, 490, 140, "UNISON VOICES");
        label(510, 490, 140, "DETUNE AMOUNT");
        label(846, 490, 140, "STEREO SPREAD");
        g.setColour(cardHi);
        g.fillRoundedRectangle(1016.0f, 510.0f, 120.0f, 142.0f, 9.0f);
        g.setColour(border);
        g.drawRoundedRectangle(1016.5f, 510.5f, 119.0f, 141.0f, 9.0f, 1.0f);
        g.setColour(violetHi);
        g.setFont(juce::Font(juce::FontOptions(9.0f, juce::Font::bold)));
        g.drawText("7 VOICE", 1026, 530, 100, 18, juce::Justification::centred);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f)));
        g.drawFittedText("Wide stacked voices with a focused centre.", 1028, 558, 96, 72,
                         juce::Justification::centred, 4);
    }
    else if (currentPage == 2) {
        panel(32, 128, 1136, 112, "FX SIGNAL CHAIN", "SIX INSERT SLOTS / LEFT TO RIGHT");
        for (int i = 0; i < 6; ++i) {
            const int sx = 42 + i * 186;
            g.setColour(muted);
            g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::bold)));
            g.drawText("SLOT " + juce::String(i + 1), sx + 6, 176, 164, 12,
                       juce::Justification::centredLeft);
            g.setColour(violet.withAlpha(0.24f));
            g.fillRoundedRectangle((float)sx, 193.0f, 178.0f, 32.0f, 7.0f);
            g.setColour(border);
            g.drawRoundedRectangle((float)sx + 0.5f, 193.5f, 177.0f, 31.0f, 7.0f, 1.0f);
        }

        panel(32, 256, 258, 470, "EQ / DRIVE", "TONE SHAPING / HARMONICS");
        panel(304, 256, 420, 470, "DYNAMICS", "COMPRESSOR / LEVEL CONTROL");
        panel(738, 256, 430, 470, "TIME / SPACE", "MODULATION / AMBIENCE / LIMITING");

        label(42, 338, 70, "EQ MIX");
        label(114, 338, 70, "LOW");
        label(186, 338, 70, "MID");
        label(78, 502, 70, "HIGH");
        label(150, 502, 70, "SATURATION");

        label(324, 362, 110, "THRESHOLD");
        label(450, 362, 110, "RATIO");
        label(576, 362, 110, "ATTACK");
        label(324, 526, 110, "RELEASE");
        label(450, 526, 110, "MAKEUP");
        label(576, 526, 110, "MIX");

        label(750, 326, 92, "CHORUS");
        label(844, 326, 92, "PHASER");
        label(938, 326, 92, "PING-PONG");
        label(1032, 326, 92, "REVERB");
        label(752, 492, 180, "PHASER RATE");
        label(950, 492, 190, "PING TIME");
        label(752, 555, 180, "PING FEEDBACK");
        label(950, 555, 190, "ROOM SIZE");
        label(752, 618, 180, "DAMPING");
        label(950, 618, 190, "LIMITER CEILING");
    }
    else if (currentPage == 3) {
        panel(32, 128, 360, 212, "LFO 1", "GLOBAL MODULATOR");
        panel(420, 128, 360, 212, "LFO 2", "GLOBAL MODULATOR");
        panel(808, 128, 360, 212, "LFO 3", "GLOBAL MODULATOR");
        panel(32, 360, 730, 366, "MODULATION MATRIX", "ROUTE A SOURCE TO A DESTINATION");
        panel(780, 360, 388, 366, "FILTER ENVELOPE", "PER-VOICE CUTOFF SHAPING");

        label(52, 190, 90, "RATE");
        label(170, 190, 190, "WAVE SHAPE");
        label(440, 190, 90, "RATE");
        label(558, 190, 190, "WAVE SHAPE");
        label(828, 190, 90, "RATE");
        label(946, 190, 190, "WAVE SHAPE");

        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
        g.drawText("SOURCE", 54, 428, 145, 13, juce::Justification::centredLeft);
        g.drawText("DESTINATION", 224, 428, 145, 13, juce::Justification::centredLeft);
        g.drawText("DEPTH / ACTIVITY", 400, 428, 315, 13, juce::Justification::centredLeft);
        for (int i = 0; i < 4; ++i) {
            const int rowY = 444 + i * 62;
            const float activity = proc.getModRouteActivity(i);
            g.setColour(cardHi);
            g.fillRoundedRectangle(400.0f, (float)(rowY + 37), 310.0f, 4.0f, 2.0f);
            g.setColour(violetHi.withAlpha(0.95f));
            g.fillRoundedRectangle(400.0f, (float)(rowY + 37),
                310.0f * juce::jlimit(0.0f, 1.0f, activity), 4.0f, 2.0f);
            g.setColour(border.withAlpha(0.7f));
            g.drawLine(52.0f, (float)(rowY + 52), 738.0f, (float)(rowY + 52), 0.8f);
        }
        label(800, 448, 82, "ATTACK");
        label(888, 448, 82, "DECAY");
        label(976, 448, 82, "SUSTAIN");
        label(1064, 448, 82, "RELEASE");
        label(824, 610, 300, "FILTER ENV AMOUNT");
    }
    else {
        panel(32, 128, 1136, 176, "PERFORMANCE", "HOST SYNC / ARPEGGIATOR / STEP SEQUENCER");
        panel(32, 328, 1136, 398, "STEP PATTERN", "32 STEPS / SELECT A STEP TO EDIT ITS PARAMETERS");
        label(54, 172, 180, "MODE");
        label(252, 172, 180, "ARP ORDER");
        label(450, 172, 180, "RATE");
        label(648, 172, 460, "OCTAVES");
        label(54, 248, 180, "DIRECTION");
        label(252, 248, 160, "LENGTH");
        label(430, 248, 300, "GLOBAL GATE");
        label(750, 248, 370, "SWING");

        panel(590, 386, 548, 318, "STEP DETAIL", "PITCH / VELOCITY / GATE / ACCENT");
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f, juce::Font::bold)));
        g.drawText("PITCH (SEMITONES)", 620, 456, 420, 12, juce::Justification::centredLeft);
        g.drawText("VELOCITY", 620, 530, 420, 12, juce::Justification::centredLeft);
        g.drawText("STEP GATE", 620, 604, 420, 12, juce::Justification::centredLeft);
        g.drawText("USER PATTERN BANK", 54, 654, 180, 12, juce::Justification::centredLeft);

        const int pageOffset = seqPageBox.getSelectedId() == 2 ? 16 : 0;
        if (lastActiveStep >= pageOffset && lastActiveStep < pageOffset + 16) {
            const int local = lastActiveStep - pageOffset;
            const int x = 54 + (local % 4) * 124;
            const int y = 414 + (local / 4) * 60;
            g.setColour(violetHi.withAlpha(0.85f));
            g.drawRoundedRectangle((float)x - 3.0f, (float)y - 3.0f, 118.0f, 48.0f, 7.0f, 2.0f);
        }
        const int selectedX = 54 + (selectedStep % 4) * 124;
        const int selectedY = 414 + (selectedStep / 4) * 60;
        g.setColour(violet.withAlpha(0.9f));
        g.drawRoundedRectangle((float)selectedX - 2.0f, (float)selectedY - 2.0f,
                               116.0f, 46.0f, 7.0f, 1.5f);
    }

    g.setColour(border);
    g.drawLine(32.0f, 776.0f, 1168.0f, 776.0f, 1.0f);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(8.5f)));
    g.drawText("2RICH  /  NORTHSTAR AUDIO", 34, 784, 280, 12, juce::Justification::centredLeft);
    g.drawText("SYNTH  /  WAVETABLE  /  FX  /  MOD  /  SEQ", 828, 784, 340, 12,
               juce::Justification::centredRight);
}

void AuroraEditor::resized() {
    const float scale = juce::jmin(getWidth() / 1200.0f, getHeight() / 820.0f);
    const int ox = (getWidth() - juce::roundToInt(1200.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(820.0f * scale)) / 2;
    auto place = [&](juce::Component& component, juce::Rectangle<int> bounds) {
        component.setBounds(scaledBounds(bounds, scale, ox, oy));
    };

    // Header: brand, navigation and patch browser.
    place(heading, {43, 22, 180, 40});
    place(synthTab, {258, 34, 86, 34});
    place(wtTab, {352, 34, 112, 34});
    place(fxTab, {472, 34, 72, 34});
    place(modTab, {552, 34, 78, 34});
    place(seqTab, {638, 34, 80, 34});
    place(presetSearch, {748, 34, 142, 30});
    place(presets, {898, 34, 270, 30});
    place(presetFavoriteButton, {748, 72, 66, 24});
    place(savePresetButton, {822, 72, 66, 24});
    place(loadPresetButton, {896, 72, 66, 24});

    // Main synth page.
    place(wave1, {52, 205, 210, 30});
    place(wave2, {314, 205, 210, 30});
    place(oscModeBox, {572, 205, 248, 30});
    place(fmAmount, {314, 337, 210, 22});
    place(syncRatio, {314, 375, 210, 22});
    place(mix, {568, 248, 80, 126});
    place(detune, {656, 248, 80, 126});
    place(sub, {744, 248, 80, 126});
    place(filterTypeBox, {868, 205, 280, 30});
    place(cutoff, {874, 270, 128, 126});
    place(res, {1022, 270, 128, 126});
    place(attack, {62, 504, 142, 188});
    place(decay, {224, 504, 142, 188});
    place(sustain, {386, 504, 142, 188});
    place(release, {548, 504, 142, 188});
    place(master, {768, 504, 112, 188});
    place(drive, {900, 504, 112, 188});
    place(width, {1032, 504, 112, 188});

    // Wavetable page.
    place(wtTable, {54, 205, 226, 30});
    place(wtPosition, {82, 238, 166, 96});
    place(importWavetableButton, {54, 344, 106, 27});
    place(clearWavetableButton, {174, 344, 106, 27});
    place(wavetableStatusLabel, {54, 374, 226, 18});
    place(unison, {174, 518, 140, 174});
    place(uniDetune, {510, 518, 140, 174});
    place(uniSpread, {846, 518, 140, 174});

    // FX page: six insert slots, EQ/drive, dynamics and time/space.
    for (int i = 0; i < 6; ++i)
        place(fxSlots[(size_t)i], {42 + i * 186, 193, 178, 32});
    place(fxEq, {42, 356, 70, 126});
    place(fxLow, {114, 356, 70, 126});
    place(fxMid, {186, 356, 70, 126});
    place(fxHigh, {78, 520, 70, 126});
    place(fxDist, {150, 520, 70, 126});
    place(fxCompEnabledButton, {326, 328, 376, 28});
    place(fxCompThreshold, {324, 382, 110, 132});
    place(fxCompRatio, {450, 382, 110, 132});
    place(fxCompAttack, {576, 382, 110, 132});
    place(fxCompRelease, {324, 548, 110, 132});
    place(fxCompMakeup, {450, 548, 110, 132});
    place(fxCompMix, {576, 548, 110, 132});
    place(fxChorus, {750, 344, 92, 132});
    place(fxFlange, {844, 344, 92, 132});
    place(fxDelay, {938, 344, 92, 132});
    place(fxReverb, {1032, 344, 92, 132});
    place(fxPhaserRate, {752, 512, 180, 28});
    place(fxPingTime, {950, 512, 190, 28});
    place(fxPingFeedback, {752, 575, 180, 28});
    place(fxReverbRoom, {950, 575, 190, 28});
    place(fxReverbDamping, {752, 638, 180, 28});
    place(fxLimiterCeiling, {950, 638, 190, 28});

    // Modulation matrix page.
    place(lfo1Rate, {52, 216, 90, 118});
    place(lfo1Shape, {170, 222, 190, 30});
    place(lfo2Rate, {440, 216, 90, 118});
    place(lfo2Shape, {558, 222, 190, 30});
    place(lfo3Rate, {828, 216, 90, 118});
    place(lfo3Shape, {946, 222, 190, 30});
    for (int i = 0; i < 4; ++i) {
        const int rowY = 444 + i * 62;
        place(modSources[(size_t)i], {54, rowY, 145, 30});
        place(modTargets[(size_t)i], {224, rowY, 145, 30});
        place(modAmounts[(size_t)i], {400, rowY + 4, 310, 26});
    }
    place(filterAttack, {800, 476, 82, 126});
    place(filterDecay, {888, 476, 82, 126});
    place(filterSustain, {976, 476, 82, 126});
    place(filterRelease, {1064, 476, 82, 126});
    place(filterEnvAmount, {824, 638, 300, 42});

    // Sequencer page: performance controls above the step grid and detail editor.
    place(seqModeBox, {54, 193, 180, 30});
    place(arpStyleBox, {252, 193, 180, 30});
    place(seqRateBox, {450, 193, 180, 30});
    place(seqOctaves, {648, 193, 460, 30});
    place(seqDirectionBox, {54, 267, 180, 30});
    place(seqLengthBox, {252, 267, 160, 30});
    place(seqGate, {430, 267, 300, 30});
    place(seqSwing, {750, 267, 370, 30});
    for (int i = 0; i < 16; ++i) {
        const int x = 54 + (i % 4) * 124;
        const int y = 414 + (i / 4) * 60;
        place(stepButtons[(size_t)i], {x, y, 112, 42});
    }
    place(selectedStepLabel, {1004, 394, 120, 24});
    place(stepPitch, {620, 472, 460, 32});
    place(stepVelocity, {620, 546, 460, 32});
    place(stepGate, {620, 620, 460, 32});
    place(stepAccentButton, {620, 666, 160, 28});
    place(seqPageBox, {54, 680, 160, 28});
    place(seqPatternSlotBox, {230, 680, 156, 28});
    place(savePatternButton, {402, 680, 76, 28});
    place(loadPatternButton, {488, 680, 76, 28});
    place(patternStatusLabel, {806, 680, 328, 24});
}

