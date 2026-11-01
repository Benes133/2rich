#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
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
 const juce::String getName() const override { return "2rich"; }
 bool acceptsMidi() const override { return true; }
 bool producesMidi() const override { return false; }
 bool isMidiEffect() const override { return false; }
 double getTailLengthSeconds() const override { return 4.0; }
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
 std::vector<float> lfo1Block, lfo2Block;
 double lfo1Phase=0.0, lfo2Phase=0.0;
 float lfo1Hold=0.0f, lfo2Hold=0.0f;
 uint32_t randomSeed=0x6d2b79f5u;
 juce::dsp::Reverb reverb;
 std::array<std::vector<float>,2> delayBuffer;
 int delayWrite=0;
 double sampleRate=44100.0;
 float fxLfoPhase=0.0f;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuroraProcessor)
};