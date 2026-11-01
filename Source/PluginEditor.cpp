#include "PluginEditor.h"

namespace {
const juce::Colour bg       (0xff0d0c13);
const juce::Colour surface  (0xff15131e);
const juce::Colour card     (0xff1c1928);
const juce::Colour cardHi   (0xff242033);
const juce::Colour border   (0xff302943);
const juce::Colour violet   (0xffa78bfa);
const juce::Colour violetHi (0xffc4b5fd);
const juce::Colour white    (0xfff4f0ff);
const juce::Colour muted    (0xff8f899f);

class RichLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override {
        const float size = (float)juce::jmin(w, h) - 27.0f;
        const float cx = x + w * 0.5f, cy = y + h * 0.43f;
        const float radius = size * 0.5f, thickness = 5.0f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);
        g.setColour(juce::Colour(0xff302b3d));
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
    aWave1 = std::make_unique<ChoiceAttach>(ap, "wave1", wave1);
    aWave2 = std::make_unique<ChoiceAttach>(ap, "wave2", wave2);

    presets.addSectionHeading("LEADS");
    presets.addItem("Violet Lead", 1);
    presets.addItem("Neon Saw", 2);
    presets.addItem("Hollow Lead", 3);
    presets.addSectionHeading("PADS");
    presets.addItem("Cloud Pad", 4);
    presets.addItem(" glass cathedral", 5);
    presets.addItem("Slow Motion", 6);
    presets.addSectionHeading("KEYS");
    presets.addItem("Digital Keys", 7);
    presets.addItem("Soft Piano-ish", 8);
    presets.addItem("Pluck Keys", 9);
    presets.setTextWhenNothingSelected("SELECT PRESET");
    presets.onChange = [this] { if (presets.getSelectedId() > 0) applyPreset(presets.getSelectedId()); };

    add(mix, "mix", "MIX"); add(detune, "detune", "DETUNE"); add(sub, "sub", "SUB");
    add(cutoff, "cutoff", "CUTOFF"); add(res, "res", "RES");
    add(attack, "attack", "ATTACK"); add(decay, "decay", "DECAY");
    add(sustain, "sustain", "SUSTAIN"); add(release, "release", "RELEASE");
    add(master, "master", "MASTER"); add(drive, "drive", "DRIVE"); add(width, "width", "WIDTH");
}

AuroraEditor::~AuroraEditor() {
    for (auto* slider : { &mix, &detune, &sub, &cutoff, &res, &attack, &decay, &sustain,
                          &release, &master, &drive, &width }) slider->setLookAndFeel(nullptr);
    wave1.setLookAndFeel(nullptr); wave2.setLookAndFeel(nullptr); presets.setLookAndFeel(nullptr);
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
    auto set = [this](const char* key, float value) {
        if (auto* parameter = proc.params.getParameter(key))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(value));
    };
    // Values are plain parameter units; the preset changes the actual synth parameters.
    struct Patch { int w1, w2; float mix, detune, sub, cutoff, res, a, d, s, r, drive, width; };
    static const Patch patches[] = {
        {0,2,.42f,11.f,.18f,10500.f,.22f,.004f,.22f,.72f,.18f,.12f,.72f},
        {0,1,.58f,18.f,.10f,8500.f,.35f,.002f,.16f,.62f,.12f,.24f,.85f},
        {3,0,.25f,5.f,.05f,6200.f,.18f,.015f,.35f,.48f,.32f,.05f,.55f},
        {2,3,.62f,7.f,.28f,5200.f,.28f,1.8f,3.2f,.82f,4.0f,.08f,.95f},
        {3,2,.50f,3.f,.12f,4300.f,.42f,2.6f,3.8f,.76f,4.5f,.04f,.9f},
        {0,2,.38f,9.f,.20f,6800.f,.25f,1.2f,2.4f,.68f,3.8f,.10f,.8f},
        {2,3,.32f,2.f,.10f,9200.f,.12f,.002f,.18f,.35f,.55f,.02f,.35f},
        {3,2,.45f,4.f,.08f,7400.f,.20f,.008f,.42f,.48f,.8f,.02f,.45f},
        {1,3,.22f,12.f,.05f,5800.f,.16f,.001f,.24f,.20f,.38f,.16f,.3f}
    };
    if (id < 1 || id > 9) return;
    const auto& p = patches[id - 1];
    set("wave1", (float)p.w1); set("wave2", (float)p.w2);
    set("mix",p.mix); set("detune",p.detune); set("sub",p.sub);
    set("cutoff",p.cutoff); set("res",p.res);
    set("attack",p.a); set("decay",p.d); set("sustain",p.s); set("release",p.r);
    set("drive",p.drive); set("width",p.width); set("master",.8f);
    repaint();
}

void AuroraEditor::paint(juce::Graphics& g) {
    g.fillAll(bg);
    const float scale = juce::jmin(getWidth() / 960.0f, getHeight() / 640.0f);
    const int ox = (getWidth() - juce::roundToInt(960.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(640.0f * scale)) / 2;
    g.addTransform(juce::AffineTransform::translation((float)ox, (float)oy).scaled(scale));

    g.setColour(surface); g.fillRoundedRectangle(14, 14, 932, 612, 18.0f);
    g.setColour(border); g.drawRoundedRectangle(14.5f, 14.5f, 931, 611, 18.0f, 1.0f);
    g.setColour(violet); g.fillRoundedRectangle(32, 79, 38, 3, 1.5f);
    g.setColour(muted); g.setFont(11.0f);
    g.drawText("DUAL OSCILLATOR  /  POLYPHONIC SYNTH", 145, 37, 350, 18, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff29243a)); g.fillRoundedRectangle(830, 34, 96, 27, 8);
    g.setColour(violetHi); g.setFont(10.5f); g.drawText("16 VOICES", 835, 39, 86, 17, juce::Justification::centred);

    auto panel = [&](int x, int y, int w, int h, const juce::String& title, const juce::String& subtitle) {
        g.setColour(card); g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 13);
        g.setColour(border); g.drawRoundedRectangle((float)x+.5f, (float)y+.5f, (float)w-1, (float)h-1, 13, 1);
        g.setColour(white); g.setFont(13); g.drawText(title,x+16,y+13,w-32,19,juce::Justification::centredLeft);
        g.setColour(muted); g.setFont(10); g.drawText(subtitle,x+16,y+34,w-32,15,juce::Justification::centredLeft);
    };
    panel(30,96,550,236,"OSCILLATORS","SHAPE  /  BLEND  /  TUNING");
    panel(594,96,336,236,"FILTER","LOW PASS  /  RESONANCE");
    panel(30,348,620,250,"AMPLIFIER ENVELOPE","ADSR  /  AMP MODULATION");
    panel(664,348,266,250,"OUTPUT","LEVEL  /  SATURATION  /  STEREO");

    auto drawWave = [&](int x, int y, int w, int h, int choice, const juce::String& label) {
        g.setColour(juce::Colour(0xff11101a)); g.fillRoundedRectangle((float)x,(float)y,(float)w,(float)h,6);
        g.setColour(border); g.drawRoundedRectangle((float)x+.5f,(float)y+.5f,(float)w-1,(float)h-1,6,1);
        g.setColour(muted); g.setFont(9); g.drawText(label,x+7,y+4,50,12,juce::Justification::centredLeft);
        juce::Path path;
        const float mid = y + h * .59f, amp = h * .27f;
        for (int i=0;i<=160;++i) {
            const float t = i/160.0f;
            float v = 0;
            switch(choice) {
                case 1: v = t < .5f ? 1.0f : -1.0f; break;
                case 2: v = 1.0f - 4.0f * std::abs(t-.5f); break;
                case 3: v = std::sin(t * juce::MathConstants<float>::twoPi); break;
                default: v = 2.0f*t-1.0f; break;
            }
            const float px = x + 7 + t*(w-14), py = mid - v*amp;
            if (i==0) path.startNewSubPath(px,py); else path.lineTo(px,py);
        }
        g.setColour(juce::Colour(0xff65528f)); g.drawLine((float)x+7,mid,(float)(x+w-7),mid,.6f);
        g.setColour(violet); g.strokePath(path,juce::PathStrokeType(2.0f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    };
    drawWave(49,204,132,72,wave1.getSelectedId()-1,"OSC 1");
    drawWave(220,204,132,72,wave2.getSelectedId()-1,"OSC 2");

    const juce::String footer = "2RICH  •  SOUND WITHOUT LIMITS";
    g.setColour(juce::Colour(0xff393149)); g.drawLine(31,611,929,611,1);
    g.setColour(muted); g.setFont(9.5f); g.drawText(footer,34,614,300,12,juce::Justification::centredLeft);
}

void AuroraEditor::resized() {
    const float scale = juce::jmin(getWidth() / 960.0f, getHeight() / 640.0f);
    const int ox = (getWidth() - juce::roundToInt(960.0f * scale)) / 2;
    const int oy = (getHeight() - juce::roundToInt(640.0f * scale)) / 2;
    auto place = [&](juce::Component& c, juce::Rectangle<int> r) { c.setBounds(scaledBounds(r,scale,ox,oy)); };

    place(heading,{34,24,100,45});
    place(presets,{610,33,205,29});
    place(wave1,{49,170,132,27}); place(wave2,{220,170,132,27});
    place(mix,{385,151,82,145}); place(detune,{477,151,82,145});
    place(sub,{610,157,91,142}); place(cutoff,{711,157,91,142}); place(res,{812,157,91,142});

    const int gap=12, knobW=132;
    place(attack,{43,411,knobW,155}); place(decay,{43+knobW+gap,411,knobW,155});
    place(sustain,{43+(knobW+gap)*2,411,knobW,155}); place(release,{43+(knobW+gap)*3,411,knobW,155});
    place(master,{676,405,78,154}); place(drive,{756,405,78,154}); place(width,{836,405,78,154});
}
