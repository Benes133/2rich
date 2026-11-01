#pragma once
#include <JuceHeader.h>
#include <array>
#include <vector>
#include <atomic>
#include <memory>
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
 void setPresetFavorite(int presetId, bool favorite) noexcept;
 bool isPresetFavorite(int presetId) const noexcept;
 bool loadWavetableFromFile(const juce::File& file, juce::String& error);
 void clearImportedWavetable();
 void editImportedWavetableRange(int frame, int firstSample, int lastSample,
                                 float firstValue, float lastValue);
 bool hasImportedWavetable() const noexcept {
  return (bool)std::atomic_load_explicit(&importedWavetable, std::memory_order_acquire);
 }
 std::shared_ptr<const std::vector<float>> getImportedWavetable() const noexcept {
  return std::atomic_load_explicit(&importedWavetable, std::memory_order_acquire);
 }
 float getImportedWavetableSample(float phase, float position) const noexcept;
 const juce::String& getImportedWavetableName() const noexcept { return importedWavetableName; }
 int getSequencerCurrentStep() const noexcept { return sequenceCurrentStep.load(std::memory_order_relaxed); }
 float getModRouteActivity(int route) const noexcept {
  return route >= 0 && route < 4 ? modRouteActivity[(size_t)route].load(std::memory_order_relaxed) : 0.0f;
 }
 float getOutputPeak() const noexcept { return outputPeak.load(std::memory_order_relaxed); }
private:
 struct StepData { bool enabled = true; int pitch = 0; float velocity = 127.0f; float gate = 100.0f; bool accent = false; };
 struct UserPattern {
  std::array<StepData, 32> steps{};
  int direction = 0, length = 2, rate = 2;
  float globalGate = 82.0f, swing = 0.0f;
 };
 std::array<UserPattern, 8> userPatterns{};
 std::array<bool, 8> userPatternValid{};
 std::array<std::atomic<bool>, 25> presetFavorites{};
 std::shared_ptr<const std::vector<float>> importedWavetable;
 juce::String importedWavetableName;
 class Voice;
 juce::Synthesiser synth;
 std::vector<float> lfo1Block, lfo2Block, lfo3Block;
 std::vector<float> pitchBendBlock, aftertouchBlock;
 std::vector<float> fxChorusModBlock, fxDelayModBlock, fxReverbModBlock, fxModWeightBlock;
 double lfo1Phase=0.0, lfo2Phase=0.0, lfo3Phase=0.0;
 float lfo1Hold=0.0f, lfo2Hold=0.0f, lfo3Hold=0.0f;
 float pitchBendValue=0.0f, aftertouchValue=0.0f;
 std::array<std::atomic<float>, 4> modRouteActivity{};
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
 float compressorEnvelope=0.0f;
 std::array<std::array<std::vector<float>, 2>, 6> fxDelayBuffers;
 std::array<int, 6> fxDelayWrites{};
 std::array<std::array<std::array<float, 4>, 2>, 6> phaserStates{};
 std::array<std::array<float, 2>, 6> multibandLowStates{}, multibandMidStates{};
 std::array<float, 6> fxPhases{};
 float limiterGain=1.0f;
 std::atomic<float> outputPeak{0.0f};
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuroraProcessor)
};