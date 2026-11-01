# AURORA Synth
An original JUCE-based subtractive synthesizer prototype for Windows VST3.

## Current features
- 16-voice polyphonic MIDI instrument
- Two oscillators (saw, square, triangle, sine)
- Oscillator mix, detune and sub oscillator
- ADSR amplitude envelope
- Low-pass filter with cutoff and resonance
- Dark custom JUCE interface
- Parameter state persistence

## Build
GitHub Actions builds the Windows VST3 automatically on pushes to main or via Actions > Build AURORA VST3 (Windows) > Run workflow.
Download the `AURORA-Windows-VST3` artifact from a successful run.

This is an early original synth prototype, not a Serum clone. Wavetable editing, FM, unison, modulation matrix and effects are planned, not yet implemented.
