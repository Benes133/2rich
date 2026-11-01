#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <atomic>
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
 void saveUserPattern(int slot);
 bool loadUserPattern(int slot);
 int getSequencerCurrentStep() const noexcept { return sequenceCurrentStep.load(std::memory_order_relaxed); }
private:
 struct StepData { bool enabled = true; int pitch = 0; float velocity = 127.0f; float gate = 100.0f; bool accent = false; };
 struct UserPattern {
  std::array<StepData, 32> steps{};
  int direction = 0, length = 2, rate = 2;
  float globalGate = 82.0f, swing = 0.0f;
 };
 std::array<UserPattern, 8> userPatterns{};
 std::array<bool, 8> userPatternValid{};
 class Voice;
 juce::Synthesiser synth;
 std::vector<float> lfo1Block, lfo2Block;
 double lfo1Phase=0.0, lfo2Phase=0.0;
 float lfo1Hold=0.0f, lfo2Hold=0.0f;
 uint32_t randomSeed=0x6d2b79f5u;
 std::array<float, 128> heldNoteVelocities{};
 double sequenceSamplesUntilStep=0.0, sequenceSamplesUntilOff=0.0;
 int sequenceStepIndex=0, sequenceCurrentNote=-1, arpDirection=1;
 bool sequenceNoteActive=false, sequencerWasRunning=false;
 std::atomic<int> sequenceCurrentStep{-1};
 juce::dsp::Reverb reverb;
 std::array<std::vector<float>,2> delayBuffer;
 int delayWrite=0;
 double sampleRate=44100.0;
 float fxLfoPhase=0.0f;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuroraProcessor)
};