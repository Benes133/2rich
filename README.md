# 2rich 0.7.0 — Modulation Pro

2rich is an original JUCE-based polyphonic synthesizer for Windows VST3.

## New in 0.7.0

- Third global LFO with independent rate and waveform.
- Modulation sources include LFO 3, Pitch Bend and Aftertouch; legacy source indices remain stable.
- Four modulation routes can share the same destination.
- New destinations: voice volume, FX Chorus, FX Delay and FX Reverb.
- Live activity meters for all four modulation routes.

## Sequencer (0.6.0)

- 32-step MIDI sequencer with two editable 16-step pages.
- Per-step pitch, velocity, gate length and accent.
- Pattern lengths of 4, 8, 16 or 32 steps.
- Forward, Reverse, Ping-Pong and Random playback directions.
- Eight project-stored user pattern slots.

## Previous modulation features

- Two global LFOs with Sine, Triangle, Saw, Square and Sample & Hold shapes.
- Four-slot modulation matrix with source, target and bipolar depth controls.
- Modulation destinations: filter cutoff, oscillator mix, wavetable position, pitch, pan and resonance.
- Per-voice Filter Envelope and Mod Wheel source.

## Interface

- **SYNTH** — two oscillators, live waveform previews, oscillator blend, detune, sub level, filter, ADSR and output.
- **WAVETABLE** — built-in wavetable bank, morph-position preview and unison.
- **FX** — tone/EQ, saturation, modulation and space.
- **MOD** — three LFOs, modulation matrix, MIDI expression and filter envelope.
- **SEQ** — host-synced arpeggiator and 32-step sequencer with per-step sound shaping.

## Sound engine

- 16-voice polyphonic MIDI instrument.
- Saw, square, triangle and sine oscillator shapes.
- Oscillator blend, detune and sub oscillator.
- ADSR amplitude envelope and per-voice filter envelope.
- Per-voice state-variable low-pass filter.
- Built-in wavetable morphing.
- Up to 7 unison voices with detune and stereo spread.
- EQ, distortion, chorus, flanger, delay and reverb controls.
- Parameter and user-pattern state persistence.
- 25 original factory presets.

## Build and download

GitHub Actions builds the Windows VST3 on pushes to `main` or manually from Actions.

Download the **`2rich-Windows-VST3`** artifact from a successful workflow run. The artifact contains `2rich.vst3`.

## Project

- Framework: JUCE 8.0.6.
- Build system: CMake.
- Plugin format: VST3.
- Current version: 0.7.0.
