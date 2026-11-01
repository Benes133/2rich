#include "PluginEditor.h"

namespace {
const juce::Colour bg        (0xff0d0c13);
const juce::Colour surface   (0xff15131e);
const juce::Colour card      (0xff1c1928);
const juce::Colour cardHi    (0xff242033);
const juce::Colour border    (0xff302943);
const juce::Colour violet    (0xffa78bfa);
const juce::Colour violetHi  (0xffc4b5fd);
const juce::Colour white     (0xfff4f0ff);
const juce::Colour muted     (0xff8f899f);

class RichLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                          float sliderPos, float startAngle, float endAngle,
                          juce::Slider&) override {
        const float size = (float)juce::jmin(w, h) - 27.0f;
        const float cx = x + w * 0.5f;
        const float cy = y + h * 0.43f;
        const float radius = size * 0.5f;
        const float thickness = 5.0f;
        const float angle = startAngle + sliderPos * (endAngle - startAngle);

        g.setColour(juce::Colour(0xff302b3d));
        g.drawEllipse(cx - radius, cy - radius, size, size, thickness);

        juce::Path arc;
        arc.addCentredArc(cx, cy, radius, radius, 0.0f, startAngle, angle, true);
        g.setColour(violet);
        g.strokePath(arc, juce::PathStrokeType(thickness, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

        g.setColour(cardHi);
        g.fillEllipse(cx - radius + 7.0f, cy - radius + 7.0f,
                      size - 14.0f, size - 14.0f);

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
        g.strokePath(arrow, juce::PathStrokeType(1.6f));
        juce::ignoreUnused(box);
    }
};
RichLookAndFeel richLook;
}

AuroraEditor::AuroraEditor(AuroraProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setSize(960, 640);
    setResizable(true, true);
    setResizeLimits(820, 560, 1200, 800);

    heading.setText("2rich", juce::dontSendNotification);
    heading.setFont(juce::Font(juce::FontOptions(30.0f, juce::Font::bold)));
    heading.setColour(juce::Label::textColourId, white);
    addAndMakeVisible(heading);

    auto& ap = proc.params;
    auto add = [&](juce::Slider& s, const juce::String& id, const juce::String& label) {
        addKnob(s, label, id);
        attachments.push_back(std::make_unique<Attach>(ap, id, s));
    };

    wave1.addItemList({"Saw", "Square", "Triangle", "Sine"}, 1);
    wave2.addItemList({"Saw", "Square", "Triangle", "Sine"}, 1);
    for (auto* box : { &wave1, &wave2 }) {
        box->setLookAndFeel(&richLook);
        box->setColour(juce::ComboBox::textColourId, white);
        box->setColour(juce::ComboBox::arrowColourId, violetHi);
        addAndMakeVisible(*box);
    }
    aWave1 = std::make_unique<ChoiceAttach>(ap, "wave1", wave1);
    aWave2 = std::make_unique<ChoiceAttach>(ap, "wave2", wave2);

    add(mix, "mix", "MIX");
    add(detune, "detune", "DETUNE");
    add(sub, "sub", "SUB");
    add(cutoff, "cutoff", "CUTOFF");
    add(res, "res", "RES");
    add(attack, "attack", "ATTACK");
    add(decay, "decay", "DECAY");
    add(sustain, "sustain", "SUSTAIN");
    add(release, "release", "RELEASE");
}

AuroraEditor::~AuroraEditor()
{
    for (auto* slider : { &mix, &detune, &sub, &cutoff, &res, &attack, &decay, &sustain, &release })
        slider->setLookAndFeel(nullptr);
    wave1.setLookAndFeel(nullptr);
    wave2.setLookAndFeel(nullptr);
}

void AuroraEditor::addKnob(juce::Slider& s, const juce::String&, const juce::String&)
{
    s.setLookAndFeel(&richLook);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 66, 19);
    s.setColour(juce::Slider::textBoxTextColourId, white);
    s.setColour(juce::Slider::textBoxBackgroundColourId, cardHi);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    s.setColour(juce::Slider::textBoxHighlightColourId, violet);
    addAndMakeVisible(s);
}

void AuroraEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    auto bounds = getLocalBounds().reduced(14);
    g.setColour(surface);
    g.fillRoundedRectangle(bounds.toFloat(), 18.0f);
    g.setColour(border);
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 18.0f, 1.0f);

    // Header
    g.setColour(violet);
    g.fillRoundedRectangle(32.0f, 79.0f, 38.0f, 3.0f, 1.5f);
    g.setColour(muted);
    g.setFont(11.0f);
    g.drawText("WAVETABLE  /  POLYPHONIC SYNTH", 145, 37, 300, 18,
               juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff29243a));
    g.fillRoundedRectangle((float)getWidth() - 142.0f, 34.0f, 104.0f, 27.0f, 8.0f);
    g.setColour(violetHi);
    g.setFont(10.5f);
    g.drawText("16 VOICES", getWidth() - 135, 39, 90, 17, juce::Justification::centred);

    auto panel = [&](int x, int y, int w, int h, const juce::String& title,
                     const juce::String& subtitle) {
        g.setColour(card);
        g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 13.0f);
        g.setColour(border);
        g.drawRoundedRectangle((float)x + 0.5f, (float)y + 0.5f,
                               (float)w - 1.0f, (float)h - 1.0f, 13.0f, 1.0f);
        g.setColour(white);
        g.setFont(13.0f);
        g.drawText(title, x + 16, y + 13, w - 32, 19, juce::Justification::centredLeft);
        g.setColour(muted);
        g.setFont(10.0f);
        g.drawText(subtitle, x + 16, y + 34, w - 32, 15, juce::Justification::centredLeft);
    };

    panel(30, 96, 550, 236, "OSCILLATORS", "SHAPE  /  BLEND  /  TUNING");
    panel(594, 96, 336, 236, "FILTER", "LOW PASS  /  RESONANCE");
    panel(30, 348, 900, 250, "AMPLIFIER ENVELOPE", "ADSR  /  AMP MODULATION");

    g.setColour(muted);
    g.setFont(10.0f);
    g.drawText("OSC 1", 49, 143, 100, 16, juce::Justification::centredLeft);
    g.drawText("OSC 2", 220, 143, 100, 16, juce::Justification::centredLeft);

    const juce::String labels[] = { "MIX", "DETUNE", "SUB", "CUTOFF", "RESONANCE",
                                    "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    const juce::Rectangle<int> b[] = {
        mix.getBounds(), detune.getBounds(), sub.getBounds(), cutoff.getBounds(), res.getBounds(),
        attack.getBounds(), decay.getBounds(), sustain.getBounds(), release.getBounds()
    };
    for (int i = 0; i < 9; ++i) {
        g.setColour(muted);
        g.setFont(10.0f);
        g.drawText(labels[i], b[i].getX(), b[i].getY() - 15, b[i].getWidth(), 14,
                   juce::Justification::centred);
    }

    g.setColour(juce::Colour(0xff393149));
    g.drawLine(31.0f, (float)getHeight() - 29.0f, (float)getWidth() - 31.0f,
               (float)getHeight() - 29.0f, 1.0f);
    g.setColour(muted);
    g.setFont(9.5f);
    g.drawText("2RICH  •  SOUND WITHOUT LIMITS", 34, getHeight() - 24, 300, 14,
               juce::Justification::centredLeft);
}

void AuroraEditor::resized()
{
    const int w = getWidth();
    heading.setBounds(34, 24, 100, 45);

    wave1.setBounds(49, 170, 132, 29);
    wave2.setBounds(220, 170, 132, 29);

    mix.setBounds(385, 151, 82, 145);
    detune.setBounds(477, 151, 82, 145);
    sub.setBounds(610, 157, 91, 142);
    cutoff.setBounds(711, 157, 91, 142);
    res.setBounds(812, 157, 91, 142);

    const int gap = 20;
    const int knobW = (w - 120 - gap * 3) / 4;
    const int startX = 60;
    attack.setBounds(startX, 411, knobW, 155);
    decay.setBounds(startX + knobW + gap, 411, knobW, 155);
    sustain.setBounds(startX + (knobW + gap) * 2, 411, knobW, 155);
    release.setBounds(startX + (knobW + gap) * 3, 411, knobW, 155);
}
