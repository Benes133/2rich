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
    presets.addSectionHeading("EXPANSION - MOTION & COLOR");
    presets.addItem("Prism Lead",26); presets.addItem("Analog Edge",27);
    presets.addItem("Vocal Formant",28); presets.addItem("Pulse Driver",29);
    presets.addItem("Velvet Orbit",30); presets.addItem("Amber Motion",31);
    presets.addItem("Northern Lights",32); presets.addItem("Lunar Glass",33);
    presets.addItem("FM Bell",34); presets.addItem("Rhodes Circuit",35);
    presets.addItem("Lo-Fi Keys",36); presets.addItem("Deep Mono",37);
    presets.addItem("Rubber Sub",38); presets.addItem("Acid Reactor",39);
    presets.addItem("Neon Droplet",40); presets.addItem("Carbon Pluck",41);
    presets.addItem("Marble Mallet",42); presets.addItem("Polar Bloom",43);
    presets.addItem("Static Rain",44); presets.addItem("Hyperdrive",45);
    presets.setTextWhenNothingSelected("SELECT PRESET");
    presets.onChange = [this] { if (presets.getSelectedId() > 0) applyPreset(presets.getSelectedId()); };

    wtTable.setLookAndFeel(&richLook); wtTable.setColour(juce::ComboBox::textColourId,white);
    wtTable.addItemList({"Basic","Harmonics","Triangle Morph","Digital","Octave","Fold"},1);
    addAndMakeVisible(wtTable); aWtTable=std::make_unique<ChoiceAttach>(ap,"wtTable",wtTable);
    auto addExtra=[&](juce::Slider& s,const juce::String& id,const juce::String& label){addKnob(s,label,id);attachments.push_back(std::make_unique<Attach>(ap,id,s));};
    addExtra(wtPosition,"wtPosition","POSITION"); addExtra(unison,"unison","VOICES"); addExtra(uniDetune,"uniDetune","DETUNE"); addExtra(uniSpread,"uniSpread","SPREAD");
    addExtra(fxEq,"fxEq","EQ MIX"); addExtra(fxLow,"fxLow","LOW"); addExtra(fxMid,"fxMid","MID"); addExtra(fxHigh,"fxHigh","HIGH");
    addExtra(fxDist,"fxDist","DISTORT"); addExtra(fxChorus,"fxChorus","CHORUS"); addExtra(fxFlange,"fxFlange","FLANGER"); addExtra(fxDelay,"fxDelay","DELAY"); addExtra(fxReverb,"fxReverb","REVERB");
    for(auto* tab:{&synthTab,&wtTab,&fxTab}){tab->setLookAndFeel(&richLook);tab->setColour(juce::TextButton::textColourOffId,white);tab->setColour(juce::TextButton::textColourOnId,violetHi);tab->setClickingTogglesState(false);addAndMakeVisible(*tab);}
    synthTab.onClick=[this]{setPage(0);};wtTab.onClick=[this]{setPage(1);};fxTab.onClick=[this]{setPage(2);};
    unison.setRange(1,7,1); unison.setNumDecimalPlacesToDisplay(0);
    add(mix, "mix", "MIX"); add(detune, "detune", "DETUNE"); add(sub, "sub", "SUB");
    add(cutoff, "cutoff", "CUTOFF"); add(res, "res", "RES");
    add(attack, "attack", "ATTACK"); add(decay, "decay", "DECAY");
    add(sustain, "sustain", "SUSTAIN"); add(release, "release", "RELEASE");
    add(master, "master", "MASTER"); add(drive, "drive", "DRIVE"); add(width, "width", "WIDTH");
}

AuroraEditor::~AuroraEditor() {
    for (const auto* id : { "wave1", "wave2", "wtTable", "wtPosition" })
        proc.params.removeParameterListener(id, this);
    for (auto* slider : { &mix, &detune, &sub, &cutoff, &res, &attack, &decay, &sustain,
                          &release, &master, &drive, &width, &wtPosition, &unison, &uniDetune,
                          &uniSpread, &fxEq, &fxLow, &fxMid, &fxHigh, &fxDist, &fxChorus,
                          &fxFlange, &fxDelay, &fxReverb }) slider->setLookAndFeel(nullptr);
    wave1.setLookAndFeel(nullptr); wave2.setLookAndFeel(nullptr); presets.setLookAndFeel(nullptr); wtTable.setLookAndFeel(nullptr);
    synthTab.setLookAndFeel(nullptr);wtTab.setLookAndFeel(nullptr);fxTab.setLookAndFeel(nullptr);
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
        {3,0,4,.46f,13,.16f,4600,.26f,.45f,2.6f,.86f,4.6f,.76f,.14f,.94f,6,30,1.0f,.70f,.30f,2,3,.14f,.58f,.30f,.48f,.70f,0.0f},
        {0,1,1,.55f,12f,.05f,9600f,.24f,.003f,.22f,.55f,.18f,.82f,.12f,.86f,3f,16f,.78f,.82f,.22f,.10f,-.4f,.2f,.10f,.22f,.06f,.10f,.10f},
        {1,0,0,.35f,24f,.12f,6800f,.50f,.002f,.18f,.40f,.26f,.78f,.32f,.70f,2f,25f,.64f,.35f,.14f,.45f,.5f,-.2f,.32f,.08f,.12f,.06f,.08f},
        {2,3,2,.42f,6f,.04f,5200f,.31f,.08f,.50f,.68f,.45f,.80f,.08f,.95f,4f,12f,.90f,.62f,.42f,-.6f,.8f,1.1f,.08f,.28f,.12f,.24f,.42f},
        {1,0,3,.30f,18f,.02f,7500f,.40f,.001f,.22f,.32f,.20f,.84f,.20f,.68f,2f,22f,.72f,.32f,.20f,.6f,-.4f,.8f,.25f,.08f,.18f,.08f,.12f},
        {3,2,4,.58f,4f,.14f,4100f,.26f,2.4f,3.6f,.80f,4.2f,.78f,.04f,.96f,5f,18f,.92f,1.0f,.68f,-.5f,.4f,.9f,.05f,.12f,.08f,.38f,.72f},
        {2,3,2,.48f,9f,.10f,3700f,.33f,1.2f,2.8f,.75f,3.8f,.78f,.10f,.92f,4f,15f,.85f,.86f,.55f,.2f,.0f,.8f,.10f,.18f,.20f,.30f,.55f},
        {3,2,4,.62f,5f,.12f,4600f,.30f,1.8f,3.4f,.86f,4.8f,.75f,.02f,1.0f,6f,20f,1.0f,.95f,.75f,.5f,.3f,1.1f,.02f,.12f,.08f,.56f,.82f},
        {3,2,2,.54f,2f,.06f,6900f,.18f,1.4f,2.2f,.70f,4.4f,.80f,.02f,.95f,4f,10f,.92f,.72f,.70f,.0f,.3f,.6f,.02f,.06f,.0f,.45f,.65f},
        {3,2,4,.66f,3f,.02f,11200f,.20f,.001f,.9f,.08f,1.4f,.82f,.03f,.72f,2f,8f,.72f,.40f,.22f,.3f,.2f,.5f,.02f,.12f,.0f,.22f,.38f},
        {2,3,4,.42f,5f,.03f,5800f,.40f,.003f,.8f,.45f,1.3f,.80f,.10f,.82f,2f,9f,.78f,.66f,.32f,.5f,.2f,.2f,.06f,.15f,.08f,.12f,.32f},
        {0,2,5,.36f,12f,.12f,3900f,.58f,.002f,.65f,.34f,1.0f,.75f,.34f,.64f,1f,4f,.35f,.55f,.45f,.2f,.1f,-.2f,.30f,.10f,.18f,.10f,.30f},
        {0,1,5,.45f,28f,.42f,950f,.82f,.003f,.55f,.46f,.32f,.83f,.52f,.42f,1f,0f,.0f,.76f,.12f,1.2f,-.4f,.0f,.46f,.0f,.0f,.10f,.08f},
        {2,0,0,.62f,14f,.62f,520f,.66f,.006f,.32f,.58f,.35f,.86f,.22f,.45f,1f,5f,.15f,.20f,.06f,.2f,.0f,.0f,.18f,.02f,.0f,.04f,.06f},
        {0,1,3,.30f,22f,.02f,2700f,.88f,.001f,.28f,.40f,.18f,.82f,.76f,.55f,1f,3f,.15f,.48f,.34f,.8f,-.2f,.7f,.72f,.04f,.12f,.06f,.12f},
        {2,3,2,.48f,8f,.04f,7600f,.20f,.001f,.34f,.12f,.55f,.81f,.18f,.88f,3f,12f,.86f,.76f,.25f,.2f,.0f,.3f,.10f,.30f,.18f,.16f,.24f},
        {1,3,5,.40f,11f,.08f,3400f,.52f,.001f,.24f,.12f,.42f,.79f,.55f,.76f,2f,14f,.78f,.72f,.30f,.5f,.3f,.0f,.46f,.08f,.22f,.12f,.14f},
        {3,2,4,.58f,4f,.02f,8800f,.17f,.001f,.55f,.16f,.85f,.82f,.02f,.92f,3f,6f,.86f,.58f,.42f,.1f,.0f,.7f,.02f,.12f,.10f,.30f,.50f},
        {3,2,4,.62f,6f,.08f,4400f,.24f,2.4f,3.8f,.85f,4.8f,.76f,.02f,1.0f,7f,18f,1.0f,.98f,.74f,.4f,.4f,.9f,.02f,.24f,.12f,.56f,.80f},
        {1,0,5,.45f,17f,.18f,7200f,.34f,.7f,2.3f,.45f,3.7f,.75f,.50f,.84f,5f,28f,.92f,.82f,.42f,.1f,1.0f,-.5f,.50f,.25f,.42f,.38f,.50f},
        {0,1,3,.58f,35f,.14f,3900f,.92f,.002f,.28f,.52f,.30f,.78f,.86f,.92f,7f,42f,1.0f,.92f,.48f,1.5f,-.5f,.8f,.86f,.42f,.35f,.42f,.40f}
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
    repaint();
}

void AuroraEditor::setPage(int page) {
    currentPage=page;
    synthTab.setToggleState(page==0,juce::dontSendNotification);
    wtTab.setToggleState(page==1,juce::dontSendNotification);
    fxTab.setToggleState(page==2,juce::dontSendNotification);
    auto show=[page](std::initializer_list<juce::Component*> list,bool visible){for(auto* c:list)c->setVisible(visible);};
    show({&mix,&detune,&sub,&cutoff,&res,&attack,&decay,&sustain,&release,&master,&drive,&width,&wave1,&wave2},page==0);
    show({&wtTable,&wtPosition,&unison,&uniDetune,&uniSpread},page==1);
    show({&fxEq,&fxLow,&fxMid,&fxHigh,&fxDist,&fxChorus,&fxFlange,&fxDelay,&fxReverb},page==2);
    repaint();resized();
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
    g.drawText("2rich", 32, 23, 98, 37, juce::Justification::centredLeft);
    g.setColour(neon);
    g.fillRoundedRectangle(33, 62, 34, 3, 1.5f);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(9.5f, juce::Font::bold)));
    g.drawText("POLYPHONIC SYNTHESIZER", 145, 28, 250, 15, juce::Justification::centredLeft);
    g.setFont(juce::Font(juce::FontOptions(9.0f)));
    g.drawText("NORTHSTAR AUDIO", 145, 45, 180, 13, juce::Justification::centredLeft);
    g.drawText("PRESET", 690, 20, 170, 12, juce::Justification::centredLeft);
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
        g.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
        g.drawText(title, x + 25, y + 12, w - 39, 19, juce::Justification::centredLeft);
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.5f)));
        g.drawText(sub, x + 25, y + 32, w - 39, 14, juce::Justification::centredLeft);
        g.setColour(border.withAlpha(0.65f));
        g.drawLine((float)x + 14.0f, (float)y + 54.0f, (float)(x + w - 14), (float)y + 54.0f, 1.0f);
    };
    auto knobLabel = [&](int x, int y, int w, const juce::String& label) {
        g.setColour(muted);
        g.setFont(juce::Font(juce::FontOptions(8.8f, juce::Font::bold)));
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
        g.setFont(juce::Font(juce::FontOptions(8.0f, juce::Font::bold)));
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
        drawWave(50,205,226,78,w1,"OSC 1 SHAPE",violetHi);
        drawWave(322,205,226,78,w2,"OSC 2 SHAPE",neon);
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
        g.drawText("Four-stage envelope",306,552,210,13,juce::Justification::centredLeft);
        g.drawText("Clean gain staging",720,552,170,13,juce::Justification::centredLeft);
    }
    else if (currentPage == 1) {
        panel(32,106,260,218,"TABLE BANK","SOURCE / MORPH POSITION");
        panel(304,106,624,218,"WAVETABLE PREVIEW","LIVE SHAPE FROM SELECTED TABLE");
        panel(32,340,896,240,"UNISON ENGINE","VOICE COUNT / DETUNE / STEREO SPREAD");
        knobLabel(66,199,184,"TABLE POSITION");

        const int table = (int)proc.params.getRawParameterValue("wtTable")->load();
        const float position = proc.params.getRawParameterValue("wtPosition")->load();
        auto tableWave = [table, position](float t) {
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
    else {
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

    g.setColour(border);
    g.drawLine(32.0f,604.0f,928.0f,604.0f,1.0f);
    g.setColour(muted);
    g.setFont(juce::Font(juce::FontOptions(8.5f)));
    g.drawText("2RICH  /  NORTHSTAR AUDIO",34,609,280,12,juce::Justification::centredLeft);
    g.drawText("SYNTH  /  WAVETABLE  /  FX",700,609,226,12,juce::Justification::centredRight);
}

void AuroraEditor::resized() {
    const float scale = juce::jmin(getWidth() / 960.0f, getHeight() / 640.0f);
    const int ox = (getWidth() - juce::roundToInt(960.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(640.0f * scale)) / 2;
    auto place = [&](juce::Component& component, juce::Rectangle<int> bounds) {
        component.setBounds(scaledBounds(bounds,scale,ox,oy));
    };
    place(heading,{32,23,100,42});
    place(synthTab,{352,31,92,30});
    place(wtTab,{450,31,128,30});
    place(fxTab,{584,31,70,30});
    place(presets,{690,34,233,30});

    place(wave1,{50,160,226,30});
    place(wave2,{322,160,226,30});
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

    place(wtTable,{54,160,216,31});
    place(wtPosition,{66,211,184,112});
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
}
