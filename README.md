# 2rich 0.3.0

2rich is an original JUCE-based polyphonic synthesizer for Windows VST3.

## Interface

The editor is organized into three dedicated workspaces:

- **SYNTH** — two oscillators, live waveform previews, oscillator blend, detune, sub level, filter, ADSR and output controls.
- **WAVETABLE** — built-in wavetable bank, morph-position preview and unison controls.
- **FX** — separate tone/EQ, saturation, and modulation/space modules.

The UI uses a consistent panel grid, aligned controls and readable parameter labels.

## Sound engine

- 16-voice polyphonic MIDI instrument
- Saw, square, triangle and sine oscillator shapes
- Oscillator blend, detune and sub oscillator
- ADSR amplitude envelope
- Low-pass filter with cutoff and resonance
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
- Current version: 0.3.0
