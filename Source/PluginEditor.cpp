#include "PluginEditor.h"

namespace {
const juce::Colour bg       (0xff0b0e14);
const juce::Colour surface  (0xff121722);
const juce::Colour card     (0xff192130);
const juce::Colour cardHi   (0xff222d3d);
const juce::Colour border   (0xff303d51);
const juce::Colour violet   (0xff9d8cff);
const juce::Colour violetHi (0xffc7bdff);
const juce::Colour neon     (0xff6fe3d3);
const juce::Colour white    (0xfff2f5fb);
const juce::Colour muted    (0xff8996aa);

class RichLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override {
        const float size = (float)juce::jmin(w, h) - 27.0f;
        const float cx = x + w * 0.5f, cy = y + h * 0.43f;
        const float radius = size * 0.5f, thickness = 5.0f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        g.setColour(border);
        g.drawEllipse(cx - radius, cy - radius, size, size, thickness);
        juce::Path arc;
        arc.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, angle, true);
        g.setColour(violet);
        g.strokePath(arc, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
        g.setColour(cardHi);
        g.fillEllipse(cx - radius + 7.0f, cy - radius + 7.0f, size - 14.0f, size - 14.0f);
        juce::Path pointer;
        pointer.addRoundedRectangle(-1.5f, -radius + 11.0f, 3.0f, radius * 0.43f, 1.5f);
        g.setColour(violetHi);
        g.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(cx, cy));
        g.setColour(juce::Colour(0xffd8ccff));
        g.fillEllipse(cx - 2.0f, cy - 2.0f, 4.0f, 4.0f);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour&, bool highlighted, bool down) override {
        auto r = button.getLocalBounds().toFloat().reduced(0.5f);
        const bool active = button.getToggleState();
        g.setColour(active ? juce::Colour(0xff51458a) : (down ? cardHi : surface));
        g.fillRoundedRectangle(r, 8.0f);
        g.setColour(active ? violet : (highlighted ? violet.withAlpha(0.65f) : border));
        g.drawRoundedRectangle(r, 8.0f, active ? 1.4f : 1.0f);
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
        g.setColour(cardHi); g.fillRoundedRectangle(r, 7.0f);
        g.setColour(border); g.drawRoundedRectangle(r.reduced(0.5f), 7.0f, 1.0f);
        juce::Path arrow;
        const float cx = width - 16.0f, cy = height * 0.5f;
        arrow.startNewSubPath(cx - 3.5f, cy - 1.5f);
        arrow.lineTo(cx, cy + 2.0f); arrow.lineTo(cx + 3.5f, cy - 1.5f);
        g.setColour(violetHi); g.strokePath(arrow, juce::PathStrokeType(1.6f));
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
    setSize(960, 640);
    for (const auto* id : { "wave1", "wave2", "wtTable", "wtPosition" })
        proc.params.addParameterListener(id, this);
    setResizable(true, true);
    setResizeLimits(820, 560, 1440, 960);

    heading.setText("2rich", juce::dontSendNotification);
    heading.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold)));
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
    presets.onChange = [this] { if (presets.getSelectedId() > 0) applyPreset(presets.getSelectedId()); };

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
                    wtTable.setSelectedId(7, juce::sendNotification);
                    wavetableStatusLabel.setText("IMPORTED: " + file.getFileName(),
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
    addChoice(oscModeBox, { "Classic", "FM", "Ring Mod", "Hard Sync" }, "oscMode");
    addChoice(filterTypeBox, { "Low Pass", "Band Pass", "High Pass" }, "filterType");

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
    addExtra(fxDist,"fxDist","DISTORT"); addExtra(fxChorus,"fxChorus","CHORUS"); addExtra(fxFlange,"fxFlange","FLANGER"); addExtra(fxDelay,"fxDelay","DELAY"); addExtra(fxReverb,"fxReverb","REVERB");
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
                          &fxFlange, &fxDelay, &fxReverb, &lfo1Rate, &lfo2Rate, &lfo3Rate, &filterAttack,
                          &filterDecay, &filterSustain, &filterRelease, &filterEnvAmount,
                          &modAmounts[0], &modAmounts[1], &modAmounts[2], &modAmounts[3],
                          &seqOctaves, &seqGate, &seqSwing, &stepPitch, &stepVelocity, &stepGate })
        slider->setLookAndFeel(nullptr);
    wave1.setLookAndFeel(nullptr); wave2.setLookAndFeel(nullptr); presets.setLookAndFeel(nullptr); wtTable.setLookAndFeel(nullptr);
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

    const float scale = juce::jmin(getWidth() / 960.0f, getHeight() / 640.0f);
    const int ox = (getWidth() - juce::roundToInt(960.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(640.0f * scale)) / 2;
    const float x = ((float)event.x - (float)ox) / scale;
    const float y = ((float)event.y - (float)oy) / scale;
    if (x < 328.0f || x > 904.0f || y < 174.0f || y > 296.0f)
        return;

    drawingWavetable = true;
    drawingFrame = juce::jlimit(0, 63, juce::roundToInt((float)wtPosition.getValue() * 63.0f));
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
    const float scale = juce::jmin(getWidth() / 960.0f, getHeight() / 640.0f);
    const int ox = (getWidth() - juce::roundToInt(960.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(640.0f * scale)) / 2;
    const float x = ((float)event.x - (float)ox) / scale;
    const float y = ((float)event.y - (float)oy) / scale;

    const int sampleIndex = juce::jlimit(0, 2047,
        juce::roundToInt((x - 338.0f) / 556.0f * 2047.0f));
    const float value = juce::jlimit(-1.0f, 1.0f, (238.66f - y) / 43.0f);
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
    show({ &fxEq,&fxLow,&fxMid,&fxHigh,&fxDist,&fxChorus,&fxFlange,&fxDelay,&fxReverb }, page == 2);
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
    if (page == 3 || page == 4)
        startTimerHz(30);
    else
        stopTimer();
    repaint();
    resized();
}


void AuroraEditor::parameterChanged(const juce::String& parameterID, float) {
    if (parameterID == "wave1" || parameterID == "wave2" ||
        parameterID == "wtTable" || parameterID == "wtPosition")
        repaint();
}

void AuroraEditor::paint(juce::Graphics& g) {
    g.fillAll(bg);
    const float scale = juce::jmin(getWidth() / 960.0f, getHeight() / 640.0f);
    const int ox = (getWidth() - juce::roundToInt(960.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(640.0f * scale)) / 2;
    g.addTransform(juce::AffineTransform::translation((float)ox, (float)oy).scaled(scale));
    g.setColour(surface);
    g.fillRoundedRectangle(14, 14, 932, 612, 18.0f);
    g.setColour(border);
    g.drawRoundedRectangle(14.5f, 14.5f, 931.0f, 611.0f, 18.0f, 1.0f);

    g.setColour(white);
    g.setFont(juce::Font(juce::FontOptions(29.0f, juce::Font::bold)));
    g.setColour(neon);
    g.fillRoundedRectangle(33, 62, 34, 3, 1.5f);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
    g.drawText("SYNTH ENGINE", 145, 28, 150, 15, juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(9.0f)));
    g.drawText("NORTHSTAR AUDIO", 145, 45, 180, 13, juce::Justification::centredLeft);
    g.drawText("PRESET", 735, 20, 170, 12, juce::Justification::centredLeft);
    g.setColour(violet);
    g.fillRoundedRectangle(32, 84, 896, 1, 0.5f);

    auto panel = [&](int x, int y, int w, int h, const juce::String& title, const juce::String& sub) {
        g.setColour(card);
        g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 12.0f);
        g.setColour(border);
        g.drawRoundedRectangle((float)x + 0.5f, (float)y + 0.5f, (float)w - 1.0f,
                               (float)h - 1.0f, 12.0f, 1.0f);
        g.setColour(violet);
        g.fillRoundedRectangle((float)x + 14.0f, (float)y + 15.0f, 3.0f, 17.0f, 1.5f);
        g.setColour(white);
        g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        g.drawText(title, x + 25, y + 12, w - 39, 19, juce::Justification::centredLeft);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(9.0f)));
        g.drawText(sub, x + 25, y + 32, w - 39, 14, juce::Justification::centredLeft);
        g.setColour(border.withAlpha(0.65f));
        g.drawLine((float)x + 14.0f, (float)y + 54.0f, (float)(x + w - 14), (float)y + 54.0f, 1.0f);
    };
    auto knobLabel = [&](int x, int y, int w, const juce::String& label) {
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(9.2f, juce::Font::bold)));
        g.drawFittedText(label, x, y, w, 14, juce::Justification::centred, 1);
    };
    auto drawWave = [&](int x, int y, int w, int h, int choice, const juce::String& label,
                        juce::Colour waveColour) {
        g.setColour(juce::Colour(0xff101722));
        g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 7.0f);
        g.setColour(border);
        g.drawRoundedRectangle((float)x + 0.5f, (float)y + 0.5f, (float)w - 1.0f,
                               (float)h - 1.0f, 7.0f, 1.0f);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.8f, juce::Font::bold)));
        g.drawText(label, x + 9, y + 5, 90, 12, juce::Justification::centredLeft);
        const float mid = y + h * 0.60f, amp = h * 0.28f;
        g.setColour(border.withAlpha(0.9f));
        g.drawLine((float)x + 8.0f, mid, (float)(x + w - 8), mid, 0.8f);
        juce::Path path;
        for (int i = 0; i <= 192; ++i) {
            const float t = (float)i / 192.0f;
            float v = 0.0f;
            switch (choice) {
                case 1: v = t < 0.5f ? 1.0f : -1.0f; break;
                case 2: v = 1.0f - 4.0f * std::abs(t - 0.5f); break;
                case 3: v = std::sin(t * juce::MathConstants<float>::twoPi); break;
                default: v = 2.0f * t - 1.0f; break;
            }
            const float px = x + 8.0f + t * (w - 16.0f);
            const float py = mid - v * amp;
            if (i == 0) path.startNewSubPath(px, py); else path.lineTo(px, py);
        }
        g.setColour(waveColour.withAlpha(0.16f));
        g.strokePath(path, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
        g.setColour(waveColour);
        g.strokePath(path, juce::PathStrokeType(1.9f, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    };

    if (currentPage == 0) {
        panel(32,106,262,216,"OSCILLATOR 1","WAVEFORM / PRIMARY TONE");
        panel(304,106,262,216,"OSCILLATOR 2","WAVEFORM / SECONDARY TONE");
        panel(576,106,352,216,"VOICE MIX","BLEND / DETUNE / SUB LEVEL");
        panel(32,338,248,242,"FILTER","LOW PASS / RESONANCE");
        panel(290,338,404,242,"AMPLIFIER ENVELOPE","SHAPE THE AMP RESPONSE");
        panel(704,338,224,242,"OUTPUT","FINAL LEVEL / STEREO");

        const int w1 = (int)proc.params.getRawParameterValue("wave1")->load();
        const int w2 = (int)proc.params.getRawParameterValue("wave2")->load();
        drawWave(50,205,226,66,w1,"OSC 1 SHAPE",violetHi);
        drawWave(322,205,226,66,w2,"OSC 2 SHAPE",neon);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.0f,juce::Font::bold)));
        g.drawText("ENGINE MODE",50,277,226,11,juce::Justification::centredLeft);
        g.drawText("FM INDEX",322,268,226,11,juce::Justification::centredLeft);
        g.drawText("SYNC RATIO",322,294,226,11,juce::Justification::centredLeft);
        g.drawText("FILTER TYPE",49,348,214,12,juce::Justification::centredLeft);
        knobLabel(595,164,98,"OSC MIX");
        knobLabel(703,164,98,"DETUNE");
        knobLabel(811,164,98,"SUB LEVEL");
        knobLabel(49,399,98,"CUTOFF");
        knobLabel(165,399,98,"RESONANCE");
        knobLabel(301,400,88,"ATTACK");
        knobLabel(399,400,88,"DECAY");
        knobLabel(497,400,88,"SUSTAIN");
        knobLabel(595,400,88,"RELEASE");
        knobLabel(710,400,68,"MASTER");
        knobLabel(782,400,68,"DRIVE");
        knobLabel(854,400,68,"WIDTH");
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f)));
        g.drawText("Choose a waveform to update the live preview.",49,289,230,15,juce::Justification::centredLeft);
    }
    else if (currentPage == 1) {
        panel(32,106,260,218,"TABLE BANK","SOURCE / MORPH POSITION");
        panel(304,106,624,218,"WAVETABLE PREVIEW","LIVE SHAPE FROM SELECTED TABLE");
        panel(32,340,896,240,"UNISON ENGINE","VOICE COUNT / DETUNE / STEREO SPREAD");
        knobLabel(66,199,184,"TABLE POSITION");
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(7.5f,juce::Font::bold)));
        g.drawText("IMPORT AUDIO AS 64 MORPH FRAMES",52,274,210,10,juce::Justification::centredLeft);

        const int table = (int)proc.params.getRawParameterValue("wtTable")->load();
        const float position = proc.params.getRawParameterValue("wtPosition")->load();
        auto tableWave = [this, table, position](float t) {
            if (table == 6)
                return proc.getImportedWavetableSample(t, position);
            const float saw = 2.0f*t-1.0f;
            const float sine = std::sin(t*juce::MathConstants<float>::twoPi);
            const float square = t < 0.5f ? 1.0f : -1.0f;
            const float triangle = 1.0f-4.0f*std::abs(t-0.5f);
            float a=saw,b=sine;
            switch(table) {
                case 1: a=square; b=triangle; break;
                case 2: a=triangle; b=sine; break;
                case 3: a=saw; b=square; break;
                case 4: a=sine; b=std::sin(t*juce::MathConstants<float>::twoPi*2.0f); break;
                case 5: a=saw; b=std::tanh(saw*3.0f); break;
                default: a=saw; b=sine; break;
            }
            return juce::jmap(position,a,b);
        };
        const int gx=328, gy=174, gw=576, gh=122;
        g.setColour(juce::Colour(0xff101722));
        g.fillRoundedRectangle((float)gx,(float)gy,(float)gw,(float)gh,8.0f);
        g.setColour(border);
        g.drawRoundedRectangle((float)gx+0.5f,(float)gy+0.5f,(float)gw-1.0f,(float)gh-1.0f,8.0f,1.0f);
        const float mid=gy+gh*0.53f;
        g.setColour(border);
        g.drawLine((float)gx+10.0f,mid,(float)(gx+gw-10),mid,0.8f);
        juce::Path preview;
        for(int i=0;i<=256;++i) {
            const float t=(float)i/256.0f;
            const float px=gx+10.0f+t*(gw-20.0f);
            const float py=mid-tableWave(t)*43.0f;
            if(i==0) preview.startNewSubPath(px,py); else preview.lineTo(px,py);
        }
        g.setColour(violet.withAlpha(0.14f));
        g.strokePath(preview,juce::PathStrokeType(8.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        g.setColour(violetHi);
        g.strokePath(preview,juce::PathStrokeType(2.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f)));
        g.drawText("FRAME 01",gx+10,gy+gh-15,90,10,juce::Justification::centredLeft);
        g.drawText("FRAME 64",gx+gw-100,gy+gh-15,90,10,juce::Justification::centredRight);
        g.setColour(neon.withAlpha(0.9f));
        g.setFont(juce::Font(juce::FontOptions(8.0f,juce::Font::bold)));
        g.drawText("DRAW ON THE WAVEFORM TO EDIT THE SELECTED MORPH FRAME",
                   gx+10,gy+gh+4,gw-20,12,juce::Justification::centred);

        knobLabel(88,418,160,"UNISON VOICES");
        knobLabel(322,418,160,"DETUNE AMOUNT");
        knobLabel(556,418,160,"STEREO SPREAD");
        g.setColour(juce::Colour(0xff202b3b));
        g.fillRoundedRectangle(754.0f,424.0f,150.0f,128.0f,9.0f);
        g.setColour(border);
        g.drawRoundedRectangle(754.5f,424.5f,149.0f,127.0f,9.0f,1.0f);
        g.setColour(neon);
        g.setFont(juce::Font(juce::FontOptions(9.0f,juce::Font::bold)));
        g.drawText("UP TO 7 VOICES",768,440,122,16,juce::Justification::centred);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f)));
        g.drawFittedText("Add weight and width while keeping the centre focused.",
                         768,463,122,65,juce::Justification::centred,4);
    }
    else if (currentPage == 2) {
        panel(32,106,286,474,"TONE / EQ","FOUR CONTROLS / FREQUENCY BALANCE");
        panel(337,106,286,474,"SATURATION","HARMONICS / DRIVE AMOUNT");
        panel(642,106,286,474,"MODULATION / SPACE","MOVEMENT / DEPTH / AMBIENCE");
        knobLabel(51,239,104,"EQ MIX");
        knobLabel(175,239,104,"LOW");
        knobLabel(51,395,104,"MID");
        knobLabel(175,395,104,"HIGH");
        knobLabel(423,270,112,"DISTORTION");
        knobLabel(660,239,104,"CHORUS");
        knobLabel(784,239,104,"FLANGER");
        knobLabel(660,395,104,"DELAY");
        knobLabel(784,395,104,"REVERB");
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f)));
        g.drawFittedText("Blend the equalizer, then shape low, mid and high frequencies.",
                         52,178,244,44,juce::Justification::centredLeft,3);
        g.drawFittedText("Add controlled harmonic density. Keep the amount subtle for cleaner patches.",
                         357,178,244,58,juce::Justification::centredLeft,4);
        g.drawFittedText("Use modulation for motion and time effects for depth.",
                         662,178,244,44,juce::Justification::centredLeft,3);
        g.setColour(neon.withAlpha(0.8f));
        g.fillRoundedRectangle(357,532,246,2,1.0f);
    }
    else if (currentPage == 3) {
        panel(32,106,286,174,"LFO 1","GLOBAL MODULATOR / RATE AND SHAPE");
        panel(337,106,286,174,"LFO 2","GLOBAL MODULATOR / RATE AND SHAPE");
        panel(642,106,286,174,"LFO 3","GLOBAL MODULATOR / RATE AND SHAPE");
        panel(32,296,610,284,"MODULATION MATRIX","FOUR ROUTES / SOURCE TO TARGET");
        panel(660,296,268,284,"FILTER ENVELOPE","PER-VOICE CUTOFF SHAPING");

        knobLabel(45,164,76,"RATE");
        knobLabel(128,164,145,"SHAPE");
        knobLabel(350,164,76,"RATE");
        knobLabel(433,164,145,"SHAPE");
        knobLabel(655,164,76,"RATE");
        knobLabel(738,164,145,"SHAPE");

        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.0f,juce::Font::bold)));
        g.drawText("SOURCE",52,356,140,14,juce::Justification::centredLeft);
        g.drawText("DESTINATION",214,356,150,14,juce::Justification::centredLeft);
        g.drawText("DEPTH / ACTIVITY",394,356,205,14,juce::Justification::centredLeft);
        for (int i = 0; i < 4; ++i) {
            const int rowY = 365 + i * 48;
            g.setColour(border.withAlpha(0.7f));
            const float activity = proc.getModRouteActivity(i);
            g.setColour(juce::Colour(0xff263244));
            g.fillRoundedRectangle(394.0f,(float)(rowY+31),205.0f,4.0f,2.0f);
            g.setColour(neon.withAlpha(0.9f));
            g.fillRoundedRectangle(394.0f,(float)(rowY+31),205.0f * juce::jlimit(0.0f,1.0f,activity),4.0f,2.0f);
            g.setColour(border.withAlpha(0.7f));
            g.drawLine(50.0f,(float)(rowY+40),622.0f,(float)(rowY+40),0.8f);
            g.setColour(muted);
            g.setFont(juce::Font(juce::FontOptions(8.0f,juce::Font::bold)));

        }

        knobLabel(668,382,52,"ATK");
        knobLabel(720,382,52,"DEC");
        knobLabel(772,382,52,"SUS");
        knobLabel(824,382,52,"REL");
        knobLabel(744,478,100,"FILTER ENV AMOUNT");
    }
    else {
        panel(32,106,896,158,"PERFORMANCE","HOST-TEMPO SYNC / ARPEGGIATOR / PATTERN PLAYBACK");
        panel(32,280,896,300,"32 STEP PATTERN","SELECT A STEP TO EDIT PITCH, VELOCITY, GATE AND ACCENT");
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.0f,juce::Font::bold)));
        g.drawText("MODE",52,164,150,12,juce::Justification::centredLeft);
        g.drawText("ARP ORDER",222,164,150,12,juce::Justification::centredLeft);
        g.drawText("RATE",392,164,150,12,juce::Justification::centredLeft);
        g.drawText("OCTAVES",566,164,150,12,juce::Justification::centredLeft);
        g.drawText("DIRECTION",52,210,170,12,juce::Justification::centredLeft);
        g.drawText("LENGTH",242,210,150,12,juce::Justification::centredLeft);
        g.drawText("GLOBAL GATE",422,210,205,12,juce::Justification::centredLeft);
        g.drawText("SWING",657,210,250,12,juce::Justification::centredLeft);
        g.setColour(cardHi);
        g.fillRoundedRectangle(526.0f,348.0f,380.0f,183.0f,9.0f);
        g.setColour(border);
        g.drawRoundedRectangle(526.5f,348.5f,379.0f,182.0f,9.0f,1.0f);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f,juce::Font::bold)));
        g.drawText("PITCH",548,383,58,12,juce::Justification::centredLeft);
        g.drawText("VELOCITY",548,425,75,12,juce::Justification::centredLeft);
        g.drawText("STEP GATE",548,467,75,12,juce::Justification::centredLeft);
        g.drawText("USER PATTERN BANK",52,534,200,12,juce::Justification::centredLeft);
        const int pageOffset = seqPageBox.getSelectedId() == 2 ? 16 : 0;
        if (lastActiveStep >= pageOffset && lastActiveStep < pageOffset + 16) {
            const int local = lastActiveStep - pageOffset;
            const int x = 52 + (local % 4) * 116;
            const int y = 354 + (local / 4) * 44;
            g.setColour(neon.withAlpha(0.95f));
            g.drawRoundedRectangle((float)x - 3.0f,(float)y - 3.0f,110.0f,35.0f,7.0f,2.0f);
        }
        const int selectedX = 52 + (selectedStep % 4) * 116;
        const int selectedY = 354 + (selectedStep / 4) * 44;
        g.setColour(violetHi.withAlpha(0.85f));
        g.drawRoundedRectangle((float)selectedX - 2.0f,(float)selectedY - 2.0f,108.0f,33.0f,7.0f,1.3f);
    }

    g.setColour(border);
    g.drawLine(32.0f,604.0f,928.0f,604.0f,1.0f);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(8.5f)));
    g.drawText("2RICH  /  NORTHSTAR AUDIO",34,609,280,12,juce::Justification::centredLeft);
    g.drawText("SYNTH  /  WAVETABLE  /  FX  /  MOD  /  SEQ",620,609,306,12,juce::Justification::centredRight);
}

void AuroraEditor::resized() {
    const float scale = juce::jmin(getWidth() / 960.0f, getHeight() / 640.0f);
    const int ox = (getWidth() - juce::roundToInt(960.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(640.0f * scale)) / 2;
    auto place = [&](juce::Component& component, juce::Rectangle<int> bounds) {
        component.setBounds(scaledBounds(bounds,scale,ox,oy));
    };
    place(heading,{32,23,100,42});
    place(synthTab,{310,31,78,30});
    place(wtTab,{394,31,118,30});
    place(fxTab,{518,31,58,30});
    place(modTab,{582,31,80,30});
    place(seqTab,{668,31,56,30});
    place(presets,{735,34,188,30});

    place(wave1,{50,164,226,29});
    place(wave2,{322,164,226,29});
    place(oscModeBox,{50,289,226,26});
    place(fmAmount,{322,280,226,22});
    place(syncRatio,{322,306,226,22});
    place(filterTypeBox,{49,365,214,26});
    place(mix,{595,176,98,132});
    place(detune,{703,176,98,132});
    place(sub,{811,176,98,132});
    place(cutoff,{49,411,98,145});
    place(res,{165,411,98,145});
    place(attack,{301,412,88,145});
    place(decay,{399,412,88,145});
    place(sustain,{497,412,88,145});
    place(release,{595,412,88,145});
    place(master,{710,412,68,145});
    place(drive,{782,412,68,145});
    place(width,{854,412,68,145});

    place(wtTable,{54,164,216,29});
    place(wtPosition,{66,211,184,76});
    place(importWavetableButton,{52,290,104,24});
    place(clearWavetableButton,{164,290,102,24});
    place(wavetableStatusLabel,{52,313,212,10});
    place(unison,{88,430,160,136});
    place(uniDetune,{322,430,160,136});
    place(uniSpread,{556,430,160,136});

    place(fxEq,{51,253,104,132});
    place(fxLow,{175,253,104,132});
    place(fxMid,{51,409,104,132});
    place(fxHigh,{175,409,104,132});
    place(fxDist,{423,299,112,150});
    place(fxChorus,{660,253,104,132});
    place(fxFlange,{784,253,104,132});
    place(fxDelay,{660,409,104,132});
    place(fxReverb,{784,409,104,132});

    place(lfo1Rate,{45,180,76,86});
    place(lfo1Shape,{128,188,145,34});
    place(lfo2Rate,{350,180,76,86});
    place(lfo2Shape,{433,188,145,34});
    place(lfo3Rate,{655,180,76,86});
    place(lfo3Shape,{738,188,145,34});
    place(filterAttack,{666,398,53,90});
    place(filterDecay,{719,398,53,90});
    place(filterSustain,{772,398,53,90});
    place(filterRelease,{825,398,53,90});
    place(filterEnvAmount,{739,494,104,76});
    for (int i = 0; i < 4; ++i) {
        const int rowY = 376 + i * 48;
        place(modSources[(size_t)i],{52,rowY,150,29});
        place(modTargets[(size_t)i],{214,rowY,150,29});
        place(modAmounts[(size_t)i],{394,rowY+2,205,25});
    }

    // Sequencer Pro workspace: transport, selectable 4x4 step grid and detail editor.
    place(seqModeBox,{52,180,150,26});
    place(arpStyleBox,{222,180,150,26});
    place(seqRateBox,{392,180,150,26});
    place(seqOctaves,{566,180,330,26});
    place(seqDirectionBox,{52,226,170,26});
    place(seqLengthBox,{242,226,150,26});
    place(seqGate,{422,226,205,26});
    place(seqSwing,{657,226,250,26});
    for (int i = 0; i < 16; ++i) {
        const int x = 52 + (i % 4) * 116;
        const int y = 354 + (i / 4) * 44;
        place(stepButtons[(size_t)i],{x,y,104,29});
    }
    place(selectedStepLabel,{548,354,180,24});
    place(stepPitch,{548,396,340,28});
    place(stepVelocity,{548,438,340,28});
    place(stepGate,{548,480,340,24});
    place(stepAccentButton,{548,507,112,24});
    place(seqPageBox,{52,548,160,24});
    place(seqPatternSlotBox,{222,548,150,24});
    place(savePatternButton,{388,546,76,27});
    place(loadPatternButton,{472,546,76,27});
    place(patternStatusLabel,{564,538,340,22});

    if(currentPage!=0) {
        juce::Component* controls[]={&mix,&detune,&sub,&cutoff,&res,&attack,&decay,&sustain,
            &release,&master,&drive,&width,&wave1,&wave2};
        for(auto* control:controls) control->setVisible(false);
    }
    if(currentPage!=1) {
        juce::Component* controls[]={&wtTable,&wtPosition,&unison,&uniDetune,&uniSpread};
        for(auto* control:controls) control->setVisible(false);
    }
    if(currentPage!=2) {
        juce::Component* controls[]={&fxEq,&fxLow,&fxMid,&fxHigh,&fxDist,&fxChorus,&fxFlange,&fxDelay,&fxReverb};
        for(auto* control:controls) control->setVisible(false);
    }
    if(currentPage!=3) {
        juce::Component* controls[]={&lfo1Rate,&lfo2Rate,&lfo3Rate,&filterAttack,&filterDecay,&filterSustain,
            &filterRelease,&filterEnvAmount,&lfo1Shape,&lfo2Shape,&lfo3Shape};
        for(auto* control:controls) control->setVisible(false);
        for(int i=0;i<4;++i) {
            modSources[(size_t)i].setVisible(false);
            modTargets[(size_t)i].setVisible(false);
            modAmounts[(size_t)i].setVisible(false);
        }
    }
    if(currentPage!=4) {
        juce::Component* controls[]={&seqModeBox,&arpStyleBox,&seqRateBox,&seqDirectionBox,&seqLengthBox,
            &seqPageBox,&seqPatternSlotBox,&seqOctaves,&seqGate,&seqSwing,&stepPitch,&stepVelocity,
            &stepGate,&stepAccentButton,&savePatternButton,&loadPatternButton,&selectedStepLabel,&patternStatusLabel};
        for(auto* control:controls) control->setVisible(false);
        for(auto& button:stepButtons) button.setVisible(false);
    }
}
