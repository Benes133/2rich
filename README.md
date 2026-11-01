# 2rich 1.4.0 — TIMAUDIO

2rich is a JUCE-based polyphonic virtual analog synthesizer for Windows VST3.

## Interface

- Four focused workspaces: VOICE, MOTION, FX and PRESETS.
- The PRESETS workspace uses a three-column browser inspired by the directory workflow in u-he Diva: sound families on the left, an alphabetically sorted patch list in the centre, and information about the selected sound on the right.
- The browser includes all 500 factory patches, instant name search, a dedicated favourites filter, per-patch favourite toggles, and external .2richpreset save/load.
- Selecting a patch loads it immediately. The list supports sequential keyboard navigation.
- The Sampler and Granular workspaces and their audio playback engines have been removed.
- Wavetable import and drawing remain part of VOICE because they edit the oscillator wavetable rather than play back audio clips.

## Sound engine

- 16-voice polyphonic MIDI instrument.
- Oscillator modes: Classic, FM, Ring Mod, Hard Sync and Super Saw.
- Saw, square, triangle and sine oscillator shapes.
- Low-pass, band-pass and high-pass state-variable filter modes.
- ADSR amplitude envelope and per-voice filter envelope.
- Up to 7 unison voices with detune and stereo spread.
- WAV/AIFF wavetable import, 64 frames × 2048 samples, with morphing and mouse-draw editing.
- Imported and hand-drawn wavetable data is embedded in DAW project state.
- Six ordered FX insert slots: Compressor, Saturation, Phaser, Ping-Pong Delay, Reverb, Chorus, Multiband Distortion and Limiter, plus Bitcrusher, Stereo Width, Tremolo, Ring Mod, Filter and Flanger.
- Four modulation routes with LFO, velocity, envelope and MIDI expression sources.
- Host-synchronised arpeggiator and 32-step sequencer with per-step pitch, velocity, gate and accent.

## Build

- Framework: JUCE 8.0.6.
- Build system: CMake.
- Plugin format: Windows VST3.
- Current version: 1.4.0.
- Company/manufacturer display name: TIMAUDIO.

GitHub Actions builds the Windows VST3 when changes are pushed to main. Download the 2rich-Windows-VST3 artifact from a successful workflow run.

## Identity compatibility

The displayed company name is TIMAUDIO. The existing four-character VST manufacturer and plugin codes remain unchanged so existing DAW sessions can continue to resolve the same plug-in identity.
