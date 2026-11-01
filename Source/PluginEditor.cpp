#include "PluginEditor.h"

namespace {
const juce::Colour bg       (0xff101018);
const juce::Colour panel    (0xff191925);
const juce::Colour panel2   (0xff20202e);
const juce::Colour purple   (0xffa78bfa);
const juce::Colour textMain (0xfff1effa);
const juce::Colour textMute (0xff9692a8);
}

AuroraEditor::AuroraEditor(AuroraProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setSize(900, 600);
    heading.setText("2rich", juce::dontSendNotification);
    heading.setFont(juce::Font(juce::FontOptions(29.0f, juce::Font::bold)));
    heading.setColour(juce::Label::textColourId, textMain);
    addAndMakeVisible(heading);

    auto& ap = proc.params;
    auto add = [&](juce::Slider& s, const juce::String& id, const juce::String& label) {
        addKnob(s, label, id);
        attachments.push_back(std::make_unique<Attach>(ap, id, s));
    };

    wave1.addItemList({"Saw", "Square", "Triangle", "Sine"}, 1);
    wave2.addItemList({"Saw", "Square", "Triangle", "Sine"}, 1);
    for (auto* box : { &wave1, &wave2 }) {
        box->setColour(juce::ComboBox::backgroundColourId, panel2);
        box->setColour(juce::ComboBox::textColourId, textMain);
        box->setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
        addAndMakeVisible(*box);
    }
    aWave1 = std::make_unique<ChoiceAttach>(ap, "wave1", wave1);
    aWave2 = std::make_unique<ChoiceAttach>(ap, "wave2", wave2);

    add(mix, "mix", "OSC MIX");
    add(detune, "detune", "DETUNE");
    add(sub, "sub", "SUB LEVEL");
    add(cutoff, "cutoff", "CUTOFF");
    add(res, "res", "RESONANCE");
    add(attack, "attack", "ATTACK");
    add(decay, "decay", "DECAY");
    add(sustain, "sustain", "SUSTAIN");
    add(release, "release", "RELEASE");
}

void AuroraEditor::addKnob(juce::Slider& s, const juce::String&, const juce::String&)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
    s.setColour(juce::Slider::rotarySliderFillColourId, purple);
    s.setColour(juce::Slider::rotarySliderOutlineColourId, panel2);
    s.setColour(juce::Slider::thumbColourId, textMain);
    s.setColour(juce::Slider::textBoxTextColourId, textMain);
    s.setColour(juce::Slider::textBoxBackgroundColourId, panel2);
    s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(s);
}

void AuroraEditor::paint(juce::Graphics& g)
{
    g.fillAll(bg);
    auto area = getLocalBounds().reduced(16);
    g.setColour(panel);
    g.fillRoundedRectangle(area.toFloat(), 16.0f);

    g.setColour(juce::Colour(0xff292638));
    g.drawRoundedRectangle(area.toFloat(), 16.0f, 1.0f);

    g.setColour(purple);
    g.fillRoundedRectangle(34.0f, 76.0f, 34.0f, 3.0f, 1.5f);

    auto section = [&](const juce::String& name, int x, int y, int w, int h) {
        g.setColour(panel2);
        g.fillRoundedRectangle((float)x, (float)y, (float)w, (float)h, 11.0f);
        g.setColour(textMute);
        g.setFont(11.0f);
        g.drawText(name, x + 16, y + 10, w - 32, 18, juce::Justification::centredLeft);
    };

    section("OSCILLATORS", 30, 96, 510, 190);
    section("FILTER", 554, 96, 316, 190);
    section("AMP ENVELOPE", 30, 302, 840, 260);

    g.setColour(textMute);
    g.setFont(11.0f);
    g.drawText("OSC 1", 48, 132, 100, 18, juce::Justification::centredLeft);
    g.drawText("OSC 2", 218, 132, 100, 18, juce::Justification::centredLeft);

    const juce::String labels[] = { "OSC MIX", "DETUNE", "SUB LEVEL", "CUTOFF", "RESONANCE",
                                    "ATTACK", "DECAY", "SUSTAIN", "RELEASE" };
    const juce::Rectangle<int> bounds[] = {
        mix.getBounds(), detune.getBounds(), sub.getBounds(), cutoff.getBounds(), res.getBounds(),
        attack.getBounds(), decay.getBounds(), sustain.getBounds(), release.getBounds()
    };
    for (int i = 0; i < 9; ++i)
        g.drawText(labels[i], bounds[i].getX(), bounds[i].getY() - 17, bounds[i].getWidth(), 16,
                   juce::Justification::centred);
}

void AuroraEditor::resized()
{
    heading.setBounds(34, 25, 300, 44);
    wave1.setBounds(48, 158, 130, 28);
    wave2.setBounds(218, 158, 130, 28);

    mix.setBounds(370, 142, 78, 126);
    detune.setBounds(452, 142, 78, 126);
    sub.setBounds(566, 142, 110, 126);
    cutoff.setBounds(680, 142, 110, 126);
    res.setBounds(790, 142, 70, 126);

    attack.setBounds(55, 370, 175, 150);
    decay.setBounds(260, 370, 175, 150);
    sustain.setBounds(465, 370, 175, 150);
    release.setBounds(670, 370, 175, 150);
}
