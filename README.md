# 2rich
An original JUCE-based synthesizer plugin for Windows VST3.

## Current engine
- 16-voice polyphonic MIDI instrument
- Two basic oscillators: saw, square, triangle and sine
- Oscillator blend, detune and sub oscillator
- ADSR amplitude envelope
- Low-pass filter with cutoff and resonance
- Parameter state persistence

## Build
GitHub Actions builds the Windows VST3 on pushes to main or manually from Actions.
Download the `AURORA-Windows-VST3` artifact from a successful run. The internal artifact name will be renamed in a later cleanup; the plugin itself is now branded 2rich.

## Roadmap
Wavetable import and editing, unison, FM, modulation matrix, LFOs, presets and an effects rack are planned. This is an early original synth, not a Serum clone.
