#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>

class AuroraEditor : public juce::AudioProcessorEditor, private juce::AudioProcessorValueTreeState::Listener {
public:
    explicit AuroraEditor(AuroraProcessor&);
    ~AuroraEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AuroraProcessor& proc;
    juce::Label heading;
    juce::Slider mix, detune, sub, cutoff, res, attack, decay, sustain, release, master, drive, width;
    juce::ComboBox wave1, wave2, presets, wtTable;
    juce::Slider wtPosition, unison, uniDetune, uniSpread;
    juce::Slider fxEq, fxLow, fxMid, fxHigh, fxDist, fxChorus, fxFlange, fxDelay, fxReverb;
    juce::TextButton synthTab{"SYNTH"}, wtTab{"WAVETABLE"}, fxTab{"FX"}, modTab{"MOD"};
    juce::Slider lfo1Rate, lfo2Rate, filterAttack, filterDecay, filterSustain, filterRelease, filterEnvAmount;
    juce::ComboBox lfo1Shape, lfo2Shape;
    std::array<juce::ComboBox, 4> modSources, modTargets;
    std::array<juce::Slider, 4> modAmounts;
    int currentPage = 0;

    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ChoiceAttach = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::vector<std::unique_ptr<Attach>> attachments;
    std::vector<std::unique_ptr<ChoiceAttach>> choiceAttachments;
    std::unique_ptr<ChoiceAttach> aWave1, aWave2, aWtTable;

    void addKnob(juce::Slider&, const juce::String&, const juce::String&);
    void applyPreset(int presetId);
    void setPage(int page);
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuroraEditor)
};
