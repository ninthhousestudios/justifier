# Justifier

A desktop application for exploring just intonation through direct manipulation of sound.

Build soundscapes by layering voices tuned to JI ratios from a reference frequency, tweaking parameters, and hearing the result in real time. Every parameter is visible and tweakable — you learn what's possible by experimenting.

## Architecture

- **Audio engine** (`native/`): C/C++ library using Faust DSP and miniaudio
  - 7 waveform types: sine, triangle, saw, square, pulse, noise, FM
  - Pre-allocated 32-voice pool, lock-free SPSC command queue
  - 20ms waveform crossfade, atomic panic flag, stereo panning
  - Zero allocation on the audio thread
- **UI**: Flutter with Material 3 (desktop-only v1)
- **FFI bridge**: Dart FFI binding to the native audio engine

## Building the native audio engine

Prerequisites: Faust compiler, CMake 3.14+, ALSA dev headers (Linux).

```bash
cd native/build
cmake ..
make
```

## Running the test harness

```bash
cd native/build
./test_audio           # full mode (~30s, audible verification)
./test_audio --quick   # quick mode (~10s)
```

## Status

- Phase 1 (C audio engine): complete — all 7 waveforms, parameter control, voice pool, crossfade, panic
- Phase 2 (Dart FFI bridge): not started
- Phase 3 (Flutter UI): not started
- Phase 4 (Polish): not started
