cmake_minimum_required(VERSION 3.22)
project(AuroraSynth VERSION 0.3.0 LANGUAGES CXX)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(AuroraSynth
    COMPANY_NAME "Northstar Audio"
    BUNDLE_ID "audio.northstar.aurora"
    PLUGIN_MANUFACTURER_CODE NSta
    PLUGIN_CODE Auro
    FORMATS VST3
    PRODUCT_NAME "2rich"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(AuroraSynth)

target_sources(AuroraSynth PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)
target_compile_features(AuroraSynth PRIVATE cxx_std_17)
target_compile_definitions(AuroraSynth PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(AuroraSynth PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
