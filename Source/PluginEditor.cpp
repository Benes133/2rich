#include "PluginEditor.h"

namespace {
const juce::Colour bg       (0xff0d0c13);
const juce::Colour surface  (0xff15131e);
const juce::Colour card     (0xff1c1928);
const juce::Colour cardHi   (0xff242033);
const juce::Colour border   (0xff302943);
const juce::Colour violet   (0xffa78bfa);
const juce::Colour violetHi (0xffc4b5fd);
const juce::Colour neon (0xff8b5cf6);
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
    for (auto* slider : { &mix, &detune, &sub, &cutoff, &res, &attack, &decay, &sustain,
                          &release, &master, &drive, &width }) slider->setLookAndFeel(nullptr);
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
        {0,2,0,.42f,11,.18f,10500,.22f,.004f,.22f,.72f,.18f,.80f,.12f,.72f,1,12,.65f,.20f,.20f,0,0,.12f,.15f,0,.08f,.12f},
        {0,1,3,.58f,18,.10f,8500,.35f,.002f,.16f,.62f,.12f,.78f,.24f,.85f,3,18,.85f,.28f,.35f,1,2,.24f,.10f,.18f,.16f,.10f},
        {3,0,2,.25f,5,.05f,6200,.18f,.015f,.35f,.48f,.32f,.82f,.05f,.55f,1,8,.40f,.18f,.10f,0,1,.05f,.05f,.0f,.10f,.08f},
        {2,3,2,.62f,7,.28f,5200,.28f,1.8f,3.2f,.82f,4.0f,.76f,.08f,.95f,3,18,.9f,.65f,.25f,2,3,.04f,.48f,.0f,.22f,.48f},
        {3,2,4,.50f,3,.12f,4300,.42f,2.6f,3.8f,.76f,4.5f,.78f,.04f,.9f,5,24,1.0f,.72f,.35f,3,4,.02f,.62f,.0f,.28f,.62f},
        {0,2,1,.38f,9,.20f,6800,.25f,1.2f,2.4f,.68f,3.8f,.76f,.10f,.8f,4,20,.9f,.58f,.22f,1,2,.10f,.55f,.0f,.25f,.52f},
        {2,3,3,.32f,2,.10f,9200,.12f,.002f,.18f,.35f,.55f,.82f,.02f,.35f,1,5,.25f,.12f,.10f,0,1,.02f,.08f,.0f,.08f,.08f},
        {3,2,4,.45f,4,.08f,7400,.20f,.008f,.42f,.48f,.8f,.82f,.02f,.45f,1,6,.35f,.16f,.12f,0,2,.02f,.10f,.0f,.12f,.12f},
        {1,3,1,.22f,12,.05f,5800,.16f,.001f,.24f,.20f,.38f,.84f,.16f,.3f,1,10,.30f,.14f,.12f,0,1,.16f,.04f,.0f,.10f,.08f},
        {0,1,5,.48f,24,.35f,2600,.62f,.01f,.42f,.50f,.8f,.78f,.38f,.72f,2,22,.75f,.35f,.32f,2,1,.38f,.12f,.15f,.22f,.18f},
        {0,3,5,.55f,31,.18f,1800,.72f,.02f,.55f,.42f,.9f,.76f,.62f,.68f,4,30,.95f,.42f,.38f,3,2,.62f,.18f,.28f,.26f,.12f},
        {1,0,3,.20f,8,.28f,1400,.82f,.003f,.28f,.58f,.35f,.82f,.72f,.42f,1,12,.40f,.12f,.20f,1,0,.72f,.0f,.10f,.05f,.08f},
        {0,2,4,.42f,16,.32f,2200,.48f,.006f,.38f,.52f,.55f,.80f,.44f,.75f,3,25,.88f,.38f,.30f,1,2,.44f,.22f,.12f,.22f,.16f},
        {3,2,2,.28f,6,.04f,7600,.16f,.001f,.20f,.18f,.24f,.84f,.08f,.38f,1,7,.35f,.18f,.12f,0,2,.08f,.08f,.0f,.14f,.10f},
        {2,3,3,.36f,14,.06f,6400,.28f,.001f,.32f,.22f,.32f,.82f,.18f,.52f,2,14,.65f,.28f,.18f,1,3,.18f,.18f,.18f,.20f,.14f},
        {3,1,4,.30f,9,.08f,8200,.22f,.002f,.26f,.24f,.40f,.82f,.12f,.58f,2,11,.72f,.30f,.22f,1,4,.12f,.24f,.08f,.18f,.18f},
        {3,2,4,.52f,5,.12f,5400,.24f,1.1f,2.8f,.78f,4.2f,.76f,.04f,.95f,5,18,1.0f,.68f,.25f,1,3,.04f,.55f,.0f,.35f,.62f},
        {0,3,5,.44f,20,.22f,3100,.38f,.8f,2.2f,.62f,3.6f,.78f,.20f,.88f,6,28,1.0f,.62f,.32f,2,4,.20f,.62f,.22f,.38f,.58f},
        {3,2,4,.60f,4,.10f,4800,.30f,1.5f,3.4f,.90f,4.8f,.74f,.02f,.98f,5,16,1.0f,.74f,.28f,2,3,.02f,.72f,.0f,.34f,.72f},
        {2,3,2,.48f,10,.18f,3900,.34f,2.2f,3.0f,.84f,4.5f,.76f,.08f,.92f,7,24,1.0f,.68f,.34f,2,3,.08f,.68f,.12f,.32f,.68f},
        {1,0,3,.32f,22,.02f,9800,.12f,.001f,.12f,.05f,.45f,.82f,.48f,.72f,3,32,.95f,.22f,.15f,0,3,.48f,.04f,.55f,.25f,.10f},
        {1,3,1,.40f,15,.10f,7200,.40f,.001f,.24f,.16f,.55f,.80f,.30f,.80f,4,26,.92f,.34f,.24f,2,2,.30f,.18f,.45f,.28f,.18f},
        {0,2,4,.50f,8,.12f,6100,.22f,.006f,.45f,.72f,2.8f,.78f,.10f,.96f,7,28,1.0f,.72f,.30f,1,3,.10f,.62f,.0f,.35f,.62f},
        {0,1,5,.58f,38,.28f,1700,.68f,.003f,.38f,.48f,.72f,.76f,.72f,.86f,7,42,1.0f,.48f,.42f,3,4,.72f,.25f,.48f,.30f,.24f},
        {3,0,4,.46f,13,.16f,4600,.26f,.45f,2.6f,.86f,4.6f,.76f,.14f,.94f,6,30,1.0f,.70f,.30f,2,3,.14f,.58f,.30f,.48f,.70f}
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
    auto show=[page](std::initializer_list<juce::Component*> list,bool visible){for(auto* c:list)c->setVisible(visible);};
    show({&mix,&detune,&sub,&cutoff,&res,&attack,&decay,&sustain,&release,&master,&drive,&width,&wave1,&wave2},page==0);
    show({&wtTable,&wtPosition,&unison,&uniDetune,&uniSpread},page==1);
    show({&fxEq,&fxLow,&fxMid,&fxHigh,&fxDist,&fxChorus,&fxFlange,&fxDelay,&fxReverb},page==2);
    repaint();resized();
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
    panel(30,96,900,502,currentPage==0?"OSCILLATORS":(currentPage==1?"WAVETABLE LAB":"FX RACK"),currentPage==0?"SHAPE  /  BLEND  /  TUNING":(currentPage==1?"TABLE MORPHING  /  UNISON ENGINE":"SHAPE THE SOUND  /  INSERT EFFECTS"));
    g.setColour(juce::Colour(0x228b5cf6));g.drawRoundedRectangle(30.5f,96.5f,899,501,13,2.0f);
    if(currentPage==0){
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

    } else if(currentPage==1) {
        g.setColour(muted);g.setFont(12);g.drawText("MORPH BETWEEN WAVETABLE FRAMES • BUILT-IN TABLE BANK",55,160,820,24,juce::Justification::centredLeft);
        juce::Path wave;for(int i=0;i<=256;++i){float t=i/256.0f;float v=std::sin(t*juce::MathConstants<float>::twoPi)+.35f*std::sin(t*juce::MathConstants<float>::twoPi*3);float x=55+t*840,y=300-v*75;if(i==0)wave.startNewSubPath(x,y);else wave.lineTo(x,y);}g.setColour(juce::Colour(0x338b5cf6));g.strokePath(wave,juce::PathStrokeType(10));g.setColour(violetHi);g.strokePath(wave,juce::PathStrokeType(2.2f));
        g.setColour(muted);g.setFont(10);g.drawText("OSCILLATOR UNISON • UP TO 7 VOICES",55,420,400,18,juce::Justification::centredLeft);
    } else {
        g.setColour(muted);g.setFont(12);g.drawText("EFFECTS ARE PROCESSED IN REAL TIME",55,160,800,20,juce::Justification::centredLeft);
        g.setColour(violet);g.drawRoundedRectangle(55,198,840,320,12,1.5f);
        g.setColour(juce::Colour(0x338b5cf6));g.drawRoundedRectangle(58,201,834,314,10,5);
    }
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
    place(wtTable,{65,230,220,34});place(wtPosition,{70,340,160,155});place(unison,{300,340,145,155});place(uniDetune,{485,340,145,155});place(uniSpread,{670,340,145,155});
    juce::Slider* fx[]={&fxEq,&fxLow,&fxMid,&fxHigh,&fxDist,&fxChorus,&fxFlange,&fxDelay,&fxReverb};
    for(int i=0;i<9;++i)place(*fx[i],{55+(i%5)*165,220+(i/5)*175,145,150});
    if(currentPage!=0){
        juce::Component* synthControls[] = {&mix,&detune,&sub,&cutoff,&res,&attack,&decay,&sustain,&release,&master,&drive,&width,&wave1,&wave2};
        for(auto* c : synthControls) c->setVisible(false);
    }
    if(currentPage!=1){
        juce::Component* wtControls[] = {&wtTable,&wtPosition,&unison,&uniDetune,&uniSpread};
        for(auto* c : wtControls) c->setVisible(false);
    }
    if(currentPage!=2){
        juce::Component* fxControls[] = {&fxEq,&fxLow,&fxMid,&fxHigh,&fxDist,&fxChorus,&fxFlange,&fxDelay,&fxReverb};
        for(auto* c : fxControls) c->setVisible(false);
    }
}
