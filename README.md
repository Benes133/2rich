# 2rich 1.3.0 — TIMAUDIO

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
- Dedicated Sampler and Granular workspaces with separate, task-focused controls.
- Full-screen 500-patch Preset Browser with category filtering, live search, favourite filtering and a selected-patch view.
- Sampler: WAV/AIFF import, source-key and source-tempo analysis, adjustable transient slicing, MIDI slice mapping from C2, trim start/end, sensitivity, minimum slice gap and slice-count controls.
- Sampler: project-tempo sync for Auto, bar and sub-bar divisions, pitch-preserving overlapping-window stretch, varispeed mode, loop, reverse, root note, transpose and fine tune.
- Granular: independent tempo-locked grain duration, 17 dedicated granular patches, window shape, density, source position, spray, timing jitter, stereo spread, transpose, fine tune and pitch.
- Automatic target-key matching can use the key inferred from the incoming MIDI part; target key can also be manually selected.
- Sample source paths are stored with DAW state; the original audio file must remain accessible at the saved path.
- The MIDI-derived project-key estimate is a track-local estimate, not a DAW-wide global key signature. Standard JUCE VST3 playhead position information exposes tempo and time signature, but not a global key signature.
- Sampler pitch-preserving stretch uses overlapping windows and is not intended to replace specialist algorithms such as Serato Sample's transient-aware time-stretching.

## Build

- Framework: JUCE 8.0.6.
- Build system: CMake.
- Plugin format: Windows VST3.
- Current version: 1.3.0.
- Company/manufacturer display name: TIMAUDIO.

GitHub Actions builds the Windows VST3 when changes are pushed to main. Download the 2rich-Windows-VST3 artifact from a successful workflow run.

## Identity compatibility

The displayed company name is now TIMAUDIO. The existing four-character VST manufacturer and plugin codes remain unchanged so existing DAW sessions can continue to resolve the same plug-in identity.
