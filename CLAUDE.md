# Justifier

Flutter desktop app for exploring just intonation through direct sound manipulation.

## Tech stack

- **Audio engine**: Faust via libfaust (interpreter backend, no LLVM)
- **Audio output**: miniaudio (C, MIT, cross-platform)
- **FFI**: `dart:ffi` + `package:ffi` + `ffigen`
- **UI**: Flutter + `flutter_riverpod` 2.6.x
- **Build**: CMake 3.14+ for native code

## Architecture

- `native/` — C/C++ audio engine (miniaudio device, SPSC command queue, Faust DSP pool)
- `lib/` — Flutter app (Riverpod providers, voice cards, waveform selection)
- Faust DSP files compiled to C++ at build time via CMake `add_custom_command`

## Faust install

- Linux: `pacman -S faust` or build from source
- macOS: `brew install faust`

## Key technical details

- SPSC lock-free command queue (moodycamel::ReaderWriterQueue) for UI→audio thread
- Pre-allocated 32-voice pool with 20ms waveform crossfade
- Faust MapUI registers params with full paths (`/sine/freq`) — wrapper does suffix matching
- C11 `_Atomic` doesn't work in .cpp — use `std::atomic` throughout
- All actions invokable by name (command pattern) for keybinding integration
