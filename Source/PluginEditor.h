#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class AuroraEditor:public juce::AudioProcessorEditor{
public:
 explicit AuroraEditor(AuroraProcessor&);
 ~AuroraEditor() override;
 void paint(juce::Graphics&)override; void resized()override;
private:
 AuroraProcessor& proc; juce::Label heading;
 juce::Slider mix,detune,sub,cutoff,res,attack,decay,sustain,release;
 juce::ComboBox wave1,wave2;
 using Attach=juce::AudioProcessorValueTreeState::SliderAttachment;
 using ChoiceAttach=juce::AudioProcessorValueTreeState::ComboBoxAttachment;
 std::vector<std::unique_ptr<Attach>> attachments;
 std::unique_ptr<ChoiceAttach> aWave1,aWave2;
 void addKnob(juce::Slider&,const juce::String&,const juce::String&);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuroraEditor)
};