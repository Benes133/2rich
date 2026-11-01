#pragma once
#include <JuceHeader.h>
class AuroraProcessor : public juce::AudioProcessor {
public:
 using APVTS=juce::AudioProcessorValueTreeState;
 AuroraProcessor();
 void prepareToPlay(double,int) override;
 void releaseResources() override {}
 bool isBusesLayoutSupported(const BusesLayout&) const override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override;
 bool hasEditor() const override { return true; }
 const juce::String getName() const override { return "AURORA"; }
 bool acceptsMidi() const override { return true; }
 bool producesMidi() const override { return false; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 2.0; }
 int getNumPrograms() override { return 1; }
 int getCurrentProgram() override { return 0; }
 void setCurrentProgram(int) override {}
 const juce::String getProgramName(int) override { return {}; }
 void changeProgramName(int,const juce::String&) override {}
 void getStateInformation(juce::MemoryBlock&) override;
 void setStateInformation(const void*,int) override;
 APVTS params;
 static APVTS::ParameterLayout createLayout();
private:
 class Voice;
 juce::Synthesiser synth;
 juce::dsp::StateVariableTPTFilter<float> filter;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuroraProcessor)
};