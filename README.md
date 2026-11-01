# 2rich 0.5.0 — Sequencer Update

2rich is an original JUCE-based polyphonic synthesizer for Windows VST3.

## New in 0.5.0

- Tempo-synced arpeggiator with Up, Down, Up/Down and Random order.
- Arpeggiator octave range from one to four octaves.
- 16-step sequencer with per-step enable and semitone pitch offset.
- Step rates: 1/4, 1/8, 1/16, 1/32 and 1/8 triplet.
- Gate and Swing controls.
- Host BPM synchronization with a 120 BPM fallback if the host does not provide tempo.
- Dedicated SEQ workspace with aligned control and step grids.
- Restored the factory list to the original 25 presets; the additional 20-preset expansion bank has been removed. Preset categories and expansion support remain part of the browser concept.

## Previous modulation features

- Two global LFOs with Sine, Triangle, Saw, Square and Sample & Hold shapes.
- Four-slot modulation matrix with source, target and bipolar depth controls.
- Modulation destinations: filter cutoff, oscillator mix, wavetable position, pitch, pan and resonance.
- Per-voice Filter Envelope and Mod Wheel source.

## Interface

- **SYNTH** — two oscillators, live waveform previews, oscillator blend, detune, sub level, filter, ADSR and output.
- **WAVETABLE** — built-in wavetable bank, morph-position preview and unison.
- **FX** — tone/EQ, saturation, modulation and space.
- **MOD** — dual LFOs, modulation matrix and filter envelope.
- **SEQ** — arpeggiator and 16-step sequencer.

## Sound engine

- 16-voice polyphonic MIDI instrument
- Saw, square, triangle and sine oscillator shapes
- Oscillator blend, detune and sub oscillator
- ADSR amplitude envelope and per-voice filter envelope
- Per-voice state-variable low-pass filter
- Built-in wavetable morphing
- Up to 7 unison voices with detune and stereo spread
- EQ, distortion, chorus, flanger, delay and reverb controls
- Parameter state persistence
- 25 original factory presets

## Build and download

GitHub Actions builds the Windows VST3 on pushes to `main` or manually from Actions.

Download the **`2rich-Windows-VST3`** artifact from a successful workflow run. The artifact contains `2rich.vst3`.

## Project

- Framework: JUCE 8.0.6
- Build system: CMake
- Plugin format: VST3
- Current version: 0.5.0
