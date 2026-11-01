#include "PluginEditor.h"
AuroraEditor::AuroraEditor(AuroraProcessor& p):AudioProcessorEditor(&p),proc(p){
 setSize(850,570);heading.setText("A U R O R A   //   HYBRID SYNTH",juce::dontSendNotification);heading.setFont(juce::Font(25.f,juce::Font::bold));heading.setColour(juce::Label::textColourId,juce::Colour(0xffeaf2ff));addAndMakeVisible(heading);
 auto& ap=proc.params;
 auto add=[&](juce::Slider& s,const juce::String& id,const juce::String& label){addKnob(s,label,id);attachments.push_back(std::make_unique<Attach>(ap,id,s));};
 wave1.addItemList({"Saw","Square","Triangle","Sine"},1);wave2.addItemList({"Saw","Square","Triangle","Sine"},1);addAndMakeVisible(wave1);addAndMakeVisible(wave2);
 aWave1=std::make_unique<ChoiceAttach>(ap,"wave1",wave1);aWave2=std::make_unique<ChoiceAttach>(ap,"wave2",wave2);
 add(mix,"mix","OSC MIX");add(detune,"detune","DETUNE");add(sub,"sub","SUB");add(cutoff,"cutoff","CUTOFF");add(res,"res","RESONANCE");
 add(attack,"attack","ATTACK");add(decay,"decay","DECAY");add(sustain,"sustain","SUSTAIN");add(release,"release","RELEASE");
}
void AuroraEditor::addKnob(juce::Slider& s,const juce::String& label,const juce::String&){
 s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);s.setTextBoxStyle(juce::Slider::TextBoxBelow,false,65,18);
 s.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xff58e0c1));s.setColour(juce::Slider::thumbColourId,juce::Colour(0xfff4f7ff));s.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffd9e4f5));s.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff152033));addAndMakeVisible(s);
}
void AuroraEditor::paint(juce::Graphics& g){
 g.fillAll(juce::Colour(0xff0b1220));auto r=getLocalBounds().toFloat().reduced(12);g.setColour(juce::Colour(0xff14243a));g.fillRoundedRectangle(r,18);
 g.setColour(juce::Colour(0xff58e0c1));g.drawRoundedRectangle(r,18,1.5f);g.setColour(juce::Colour(0xff29415e));g.drawLine(28,78,getWidth()-28,78,1);
 g.setColour(juce::Colour(0xff9fb2cc));g.setFont(13.f);g.drawText("OSCILLATORS",30,95,300,20,juce::Justification::left);g.drawText("FILTER",30,285,300,20,juce::Justification::left);g.drawText("AMP ENVELOPE",30,420,300,20,juce::Justification::left);
}
void AuroraEditor::resized(){
 heading.setBounds(30,25,700,42);wave1.setBounds(35,125,150,28);wave2.setBounds(205,125,150,28);
 const int xs[]={35,190,345,500,655};juce::Slider* top[]={&mix,&detune,&sub,&cutoff,&res};
 for(int i=0;i<5;++i)top[i]->setBounds(xs[i],160,135,112);
 juce::Slider* env[]={&attack,&decay,&sustain,&release};for(int i=0;i<4;++i)env[i]->setBounds(35+i*190,450,150,105);
}