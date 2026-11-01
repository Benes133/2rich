#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>

class AuroraEditor : public juce::AudioProcessorEditor, private juce::AudioProcessorValueTreeState::Listener, private juce::Timer {
public:
    explicit AuroraEditor(AuroraProcessor&);
    ~AuroraEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AuroraProcessor& proc;
    juce::Label heading;
    juce::Slider mix, detune, sub, cutoff, res, attack, decay, sustain, release, master, drive, width;
    juce::ComboBox wave1, wave2, presets, wtTable, oscModeBox, filterTypeBox;
    juce::Slider wtPosition, unison, uniDetune, uniSpread, fmAmount, syncRatio;
    juce::TextButton importWavetableButton{"IMPORT WAV"}, clearWavetableButton{"CLEAR"};
    juce::Label wavetableStatusLabel;
    std::unique_ptr<juce::FileChooser> wavetableChooser;
    juce::Slider fxEq, fxLow, fxMid, fxHigh, fxDist, fxChorus, fxFlange, fxDelay, fxReverb;
    juce::TextButton synthTab{"SYNTH"}, wtTab{"WAVETABLE"}, fxTab{"FX"}, modTab{"MOD"}, seqTab{"SEQ"};
    juce::ComboBox seqModeBox, arpStyleBox, seqRateBox, seqDirectionBox, seqLengthBox;
    juce::ComboBox seqPageBox, seqPatternSlotBox;
    juce::Slider seqOctaves, seqGate, seqSwing;
    std::array<juce::TextButton, 16> stepButtons;
    juce::Slider stepPitch, stepVelocity, stepGate;
    juce::TextButton stepAccentButton{"ACCENT"}, savePatternButton{"SAVE"}, loadPatternButton{"LOAD"};
    juce::Label selectedStepLabel, patternStatusLabel;
    int selectedStep = 0;
    int lastActiveStep = -1;
    std::array<float, 4> lastModRouteActivity{};
    juce::Slider lfo1Rate, lfo2Rate, lfo3Rate, filterAttack, filterDecay, filterSustain, filterRelease, filterEnvAmount;
    juce::ComboBox lfo1Shape, lfo2Shape, lfo3Shape;
    std::array<juce::ComboBox, 4> modSources, modTargets;
    std::array<juce::Slider, 4> modAmounts;
    int currentPage = 0;

    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ChoiceAttach = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::vector<std::unique_ptr<Attach>> attachments;
    std::vector<std::unique_ptr<ChoiceAttach>> choiceAttachments;
    std::unique_ptr<ChoiceAttach> aWave1, aWave2, aWtTable;

    void addKnob(juce::Slider&, const juce::String&, const juce::String&);
    void applyPreset(int presetId);
    void setPage(int page);
    void refreshSelectedStepControls();
    void writeSelectedStepParameter(const juce::String& baseId, float value);
    void timerCallback() override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AuroraEditor)
};
