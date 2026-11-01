# 2rich 0.8.0 — Sound Design

2rich is an original JUCE-based polyphonic synthesizer for Windows VST3.

## New in 0.8.0

- Oscillator modes: Classic, FM, Ring Mod and Hard Sync.
- FM amount and Hard Sync ratio controls.
- Filter types: Low Pass, Band Pass and High Pass.
- WAV/AIFF wavetable import: audio is resampled into a 64-frame, 2048-sample wavetable.
- Imported tables can be scanned with the existing wavetable position control.
- Draw directly on the waveform preview to edit the selected frame or start a table from a sine wave.
- Imported and hand-drawn wavetable data is embedded in DAW project state and restored when the project is reopened.
- Oscillator phase lookup uses normalized cycle position derived from oscillator radians.

## Modulation (0.7.0)

- Third global LFO with independent rate and waveform.
- Modulation sources include LFO 3, Pitch Bend and Aftertouch.
- Four routes with live activity indicators.
- Destinations include voice volume, FX Chorus, FX Delay and FX Reverb.

## Sequencer (0.6.0)

- 32-step MIDI sequencer with two editable 16-step pages.
- Per-step pitch, velocity, gate length and accent.
- Pattern lengths of 4, 8, 16 or 32 steps.
- Forward, Reverse, Ping-Pong and Random playback directions.
- Eight user pattern slots stored with the DAW project.

## Interface

- **SYNTH** — dual oscillators, synthesis mode, FM and sync controls, filter type, ADSR and output.
- **WAVETABLE** — built-in tables, draw-to-edit waveform preview, WAV/AIFF import, 64-frame morphing and unison.
- **FX** — tone/EQ, saturation, modulation and space.
- **MOD** — three LFOs, modulation matrix, MIDI expression and filter envelope.
- **SEQ** — host-synced arpeggiator and 32-step sequencer.

## Sound engine

- 16-voice polyphonic MIDI instrument.
- Saw, square, triangle and sine oscillator shapes.
- Classic oscillator blend, phase-modulation FM, ring modulation and hard sync.
- Low-pass, band-pass and high-pass state-variable filter modes.
- ADSR amplitude envelope and per-voice filter envelope.
- Up to 7 unison voices with detune and stereo spread.
- EQ, distortion, chorus, flanger, delay and reverb.
- Parameter, pattern-bank, imported wavetable and DAW project state persistence.
- 25 original factory presets.

## Build and download

GitHub Actions builds the Windows VST3 on pushes to `main` or manually from Actions.

Download the **`2rich-Windows-VST3`** artifact from a successful workflow run. The artifact contains `2rich.vst3`.

## Project

- Framework: JUCE 8.0.6.
- Build system: CMake.
- Plugin format: VST3.
- Current version: 0.8.0.
