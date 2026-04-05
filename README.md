# Justifier

A desktop application for exploring just intonation through direct manipulation of sound.

Build soundscapes by layering voices tuned to JI ratios from a reference frequency, tweaking parameters, and hearing the result in real time. Every parameter is visible and tweakable — you learn what's possible by experimenting.

## Features

- **12 waveform types**: sine, triangle, saw, square, pulse, white/pink/brown noise, lfnoise0/1/2, FM synthesis
- **Just intonation ratios**: set numerator/denominator per voice, octave selector, detune in cents
- **Per-voice controls**: amplitude, pan, waveform crossfade (20ms), enable/mute toggle
- **Per-voice multi-mode filter**: lowpass, highpass, bandpass, notch with cutoff and resonance
- **Per-voice reverb send**: stereo send/return bus with Faust Zita Rev1 reverb, global return level
- **ADSR envelope**: per-voice attack, decay, sustain, release with proper voice release lifecycle
- **FM synthesis controls**: mod ratio and mod index (FM waveform only)
- **Sleep-before-destroy**: voices fade out over their release time with undo option
- **Wave grouping**: organize voices into named, colored wave columns
- **32-voice polyphony**: pre-allocated pool, zero allocation on the audio thread
- **Lock-free audio**: SPSC command queue, atomic panic flag, stack-allocated buffers
- **Cross-platform**: Linux, macOS, Windows (miniaudio backend)

## Architecture

- **Audio engine** (`native/`): C/C++ library using Faust DSP and miniaudio
  - 12 voice DSPs compiled from Faust at build time via CMake
  - Reverb effect as singleton Faust DSP (Zita Rev1 stereo), separate from voice pool
  - Pre-allocated 32-voice pool, lock-free SPSC command queue (moodycamel::ReaderWriterQueue)
  - 20ms waveform crossfade, stereo send bus for reverb, atomic panic flag
  - Zero allocation on the audio thread
- **FFI bridge** (`lib/audio/`): Dart FFI bindings to the native audio engine, platform-specific library loading
- **UI** (`lib/`): Flutter with Riverpod state management, Material 3 dark theme
  - Voice cards with inline sliders for all parameters
  - Waveform selector, ratio input, octave controls
  - Filter type buttons (LP/HP/BP/NT), cutoff/resonance sliders
  - Reverb send slider, ADSR envelope sliders

## Building

### Prerequisites

- Faust compiler (`pacman -S faust` / `brew install faust`)
- CMake 3.14+
- ALSA dev headers (Linux only)
- Flutter SDK

### Native audio engine

```bash
cd native
mkdir -p build && cd build
cmake ..
make
```

### Flutter app

```bash
flutter run -d linux   # or -d macos, -d windows
```

### Tests

```bash
# C audio engine tests
cd native/build
./test_audio           # full mode (~30s, audible verification)
./test_audio --quick   # quick mode (~10s)

# Flutter widget tests
flutter test
```

## Status

- Phase 1 (C audio engine): complete
- Phase 2 (Dart FFI bridge): complete
- Phase 3 (Flutter UI): complete — voice cards, wave columns, all controls
- Phase 4 (Audio features):
  - 4a Per-voice filters: complete
  - 4b1 ADSR envelope: complete
  - 4c Effects bus (reverb): complete
  - 4b2 Filter envelope: planned
  - 4d JI lattice explorer: planned
