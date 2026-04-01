## Project

**Justifier**

A desktop application for exploring just intonation through direct manipulation of sound. You build soundscapes by layering voices tuned to JI ratios from a reference frequency, tweaking parameters, and hearing the result in real time. The core goal is discoverability through interaction — every parameter is visible and tweakable, so you learn what's possible by experimenting.

**Core Value:** Real-time audio feedback from manipulating JI ratios — hear the math immediately.

### Constraints

- **Audio engine**: Faust via libfaust — chosen for embeddability, functional DSP model, non-GPL license
- **Platform**: Desktop-only v1 (Linux primary, macOS/Windows tested)
- **UI framework**: Flutter with Material 3, heavily themed dark/dense
- **Architecture**: All actions must be invokable by name (command pattern) for future keybinding integration

## Technology Stack

### DSP Engine — Faust / libfaust
- libfaust interpreter backend (2.75.x+) — no LLVM dependency, single .so/.dylib/.dll
- Hand-written C wrapper (~200 LOC) over libfaust C++ API
- Faust DSP files compiled to C++ at build time

### Audio Output — miniaudio
- Header-only C library, MIT license, zero dependencies
- Cross-platform: ALSA (Linux), CoreAudio (macOS), WASAPI (Windows)
- `ma_device` callback model integrates directly with Faust `compute()`

### Dart FFI Binding
- `dart:ffi` + `package:ffi` (arena allocator, Pointer<Utf8>)
- `ffigen` for generating bindings from C headers

### Flutter Application
- `flutter_riverpod` 2.6.x for state management
- `path_provider` for preset storage
- `file_picker` for import/export

### Build System
- CMake 3.14+ for native code
- Prebuilt libfaust binaries preferred over building from source
- Linux: `pacman -S faust` or Faust GitHub releases
- macOS: `brew install faust`
- Windows: Faust GitHub releases installer

## Conventions

Conventions not yet established. Will populate as patterns emerge during development.
