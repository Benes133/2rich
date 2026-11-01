# 2rich 0.4.0 — Modulation Update

2rich is an original JUCE-based polyphonic synthesizer for Windows VST3.

## New in 0.4.0

- Dedicated **MOD** workspace.
- Two independent global LFOs with Sine, Triangle, Saw, Square and Sample & Hold shapes.
- Four-slot modulation matrix with source, target and bipolar depth controls.
- Modulation destinations: filter cutoff, oscillator mix, wavetable position, pitch, pan and resonance.
- Per-voice Filter Envelope with Attack, Decay, Sustain, Release and Amount controls.
- Mod Wheel as a modulation source.
- Voice-level filter processing so each held note has its own filter envelope response.

## Interface

- **SYNTH** — two oscillators, live waveform previews, oscillator blend, detune, sub level, filter, ADSR and output controls.
- **WAVETABLE** — built-in wavetable bank, morph-position preview and unison controls.
- **FX** — separate tone/EQ, saturation, and modulation/space modules.
- **MOD** — dual LFOs, four modulation routes and a dedicated filter envelope.

## Sound engine

- 16-voice polyphonic MIDI instrument
- Saw, square, triangle and sine oscillator shapes
- Oscillator blend, detune and sub oscillator
- ADSR amplitude envelope
- Per-voice state-variable low-pass filter
- Built-in wavetable morphing
- Up to 7 unison voices with detune and stereo spread
- EQ, distortion, chorus, flanger, delay and reverb controls
- Parameter state persistence
- 45 factory presets across leads, pads, keys, basses, plucks, atmospheres and FX

## Build and download

GitHub Actions builds the Windows VST3 on pushes to `main` or manually from Actions.

Download the **`2rich-Windows-VST3`** artifact from a successful workflow run. The artifact contains `2rich.vst3`.

## Project

- Framework: JUCE 8.0.6
- Build system: CMake
- Plugin format: VST3
- Current version: 0.4.0
