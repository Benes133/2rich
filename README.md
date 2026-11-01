# 2rich 1.1.0 — TIMAUDIO

2rich is a JUCE-based polyphonic virtual analog synthesizer for Windows VST3.

## 1.1.0 — Interface redesign

- Rebuilt the interface as a brushed-metal hardware-style instrument panel with recessed displays, machined encoder caps, illuminated violet position rings and tactile buttons.
- Consolidated five separate workspaces into three focused screens: VOICE, MOTION and FX.
- Moved wavetable import, morphing and drawing into the main VOICE screen.
- Combined LFOs, filter envelope, modulation matrix, arpeggiator and 32-step sequencer on MOTION.
- Consolidated oscillator, filter, amplitude envelope, unison, voice mix and output controls on VOICE.
- Reworked component bounds and label placement to remove crowded sections and prevent overlaps.

## Sound engine

- 16-voice polyphonic MIDI instrument.
- Oscillator modes: Classic, FM, Ring Mod, Hard Sync and Super Saw.
- Saw, square, triangle and sine oscillator shapes.
- Low-pass, band-pass and high-pass state-variable filter modes.
- ADSR amplitude envelope and per-voice filter envelope.
- Up to 7 unison voices with detune and stereo spread.
- WAV/AIFF wavetable import, 64 frames × 2048 samples, with morphing and mouse-draw editing.
- Imported and hand-drawn wavetable data is embedded in DAW project state.
- Six ordered FX insert slots with 14 selectable modules: Compressor, Saturation, Phaser, Ping-Pong Delay, Reverb, Chorus, Multiband Distortion, Limiter, Bitcrusher, Stereo Width, Tremolo, Ring Modulator, resonant Filter and Flanger.
- Dedicated module controls for drive/tone, stereo width and balance, modulation depth/rate, delay tone/feedback, filter cutoff/resonance and limiter release/input/mix. FX insert order is editable from the rack.
- Four modulation routes with LFO, velocity, envelope and MIDI expression sources.
- Host-synchronised arpeggiator and 32-step sequencer with per-step pitch, velocity, gate and accent.
- 25 factory presets, search, favourites and external .2richpreset save/load.

## Build

- Framework: JUCE 8.0.6.
- Build system: CMake.
- Plugin format: Windows VST3.
- Current version: 1.1.0.
- Company/manufacturer display name: TIMAUDIO.

GitHub Actions builds the Windows VST3 when changes are pushed to main. Download the 2rich-Windows-VST3 artifact from a successful workflow run.

## Identity compatibility

The displayed company name is now TIMAUDIO. The existing four-character VST manufacturer and plugin codes remain unchanged so existing DAW sessions can continue to resolve the same plug-in identity.
