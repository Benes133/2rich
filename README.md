# 2rich 1.2.0 — TIMAUDIO

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
- Six ordered FX insert slots: Compressor, Saturation, Phaser, Ping-Pong Delay, Reverb, Chorus, Multiband Distortion and Limiter.
- Four modulation routes with LFO, velocity, envelope and MIDI expression sources.
- Host-synchronised arpeggiator and 32-step sequencer with per-step pitch, velocity, gate and accent.
- 500 factory presets across leads, pads, atmospheres, keys, pianos, plucks, basses, synths, sound effects and textures.
- Preset search and favourites across the complete factory bank, plus external .2richpreset save/load.
- Integrated audio Sample Lab with WAV/AIFF import, estimated musical-key detection, transient-based slicing and MIDI slice mapping from C2.
- Polyphonic sample playback with automatic root-note pitch tracking.
- Granular engine with adjustable grain size, density, position, spray and pitch.
- Sample source paths are stored with DAW state; the original audio file must remain accessible at the saved path.
- Sample playback uses varispeed pitch tracking; it does not yet provide Serato-style transient-preserving time-stretch or beat-grid warping.

## Build

- Framework: JUCE 8.0.6.
- Build system: CMake.
- Plugin format: Windows VST3.
- Current version: 1.2.0.
- Company/manufacturer display name: TIMAUDIO.

GitHub Actions builds the Windows VST3 when changes are pushed to main. Download the 2rich-Windows-VST3 artifact from a successful workflow run.

## Identity compatibility

The displayed company name is now TIMAUDIO. The existing four-character VST manufacturer and plugin codes remain unchanged so existing DAW sessions can continue to resolve the same plug-in identity.
