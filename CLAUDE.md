<!-- GSD:project-start source:PROJECT.md -->
## Project

**Justifier**

A desktop application for exploring just intonation through direct manipulation of sound. You build soundscapes by layering voices tuned to JI ratios from a reference frequency, tweaking parameters, and hearing the result in real time. The core goal is discoverability through interaction — every parameter is visible and tweakable, so you learn what's possible by experimenting.

**Core Value:** Real-time audio feedback from manipulating JI ratios — hear the math immediately.

### Constraints

- **Audio engine**: Faust via libfaust — chosen for embeddability, functional DSP model, non-GPL license
- **Platform**: Desktop-only v1 (Linux primary, macOS/Windows tested)
- **UI framework**: Flutter with Material 3, heavily themed dark/dense
- **Architecture**: All actions must be invokable by name (command pattern) for future keybinding integration
<!-- GSD:project-end -->

<!-- GSD:stack-start source:research/STACK.md -->
## Technology Stack

## Recommended Stack
### Layer 1: DSP Engine — Faust / libfaust
| Component | Version | Purpose | Why |
|-----------|---------|---------|-----|
| libfaust (interpreter backend) | 2.75.x (latest) | Compile and run Faust DSP code at runtime | No LLVM dependency; ships as a single .so/.dylib/.dll; 3-10x slower than LLVM JIT but more than fast enough for <20 voice oscillator synthesis |
| C wrapper (hand-written) | n/a | Thin C layer over libfaust C++ API | Dart FFI cannot call C++ directly; C API headers already exist for interpreter backend |
### Layer 2: C Wrapper (custom, ~200 LOC)
- Wraps `createInterpreterDSPFactoryFromString` and instance lifecycle
- Exposes `compute(dsp, n_samples, float** outputs)` for the audio callback
- Owns the summing mix buffer: iterates all active instances and sums into a stereo output buffer
- Exposes `set_param(dsp, path, value)` for real-time parameter updates from Dart
### Layer 3: Audio Output — miniaudio (direct, not via a Flutter package)
| Component | Version | Purpose | Why |
|-----------|---------|---------|-----|
| miniaudio | 0.11.x (header-only) | Cross-platform audio output: ALSA (Linux), CoreAudio (macOS), WASAPI (Windows) | Single-header C library, MIT license, zero dependencies, proven in production, designed for embedding |
| Option | Assessment |
|--------|-----------|
| **miniaudio** | RECOMMENDED. Header-only, MIT, handles ALSA/CoreAudio/WASAPI automatically, used in production games and audio tools. The `ma_device` callback model integrates directly with the libfaust `compute()` call. |
| PortAudio | AVOID. LGPL license (requires dynamic linking or releasing source), significantly more complex build story than miniaudio, no meaningful benefit for this use case. |
| flutter_soloud | NOT RECOMMENDED for this use case. flutter_soloud wraps SoLoud (which uses miniaudio internally), but it is designed for playing pre-rendered audio samples and streaming PCM into it from Dart. The round-trip — compute in C, send to Dart, push back into SoLoud — adds latency and complexity. Better to own miniaudio directly and compute in the C callback. |
| ALSA/CoreAudio/WASAPI direct | AVOID. Platform-specific, enormous per-platform code burden. miniaudio already wraps these correctly. |
| RtAudio | Viable but overkill. C++ API, LGPL, heavier than miniaudio for this use case. |
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
### Layer 4: Dart FFI Binding
| Component | Version | Purpose | Why |
|-----------|---------|---------|-----|
| `dart:ffi` (stdlib) | bundled with Dart/Flutter | Call C functions from Dart | Standard, no additional packages |
| `package:ffi` | 2.1.x | Arena allocator, `Pointer<Utf8>`, struct helpers | Makes FFI ergonomic; official Dart team package |
| `ffigen` | 9.x | Generate Dart bindings from C headers | Avoids handwriting bindings for large APIs |
### Layer 5: Flutter Plugin Build System — CMake + Flutter Build Hooks
| Component | Purpose |
|-----------|---------|
| CMake (3.14+) | Build libfaust (if building from source) and faust_bridge C wrapper on all platforms |
| Flutter plugin_ffi template | Scaffold for the native code + CMakeLists.txt integration |
| Prebuilt libfaust binaries | Preferred over building LLVM toolchain for CI |
- Linux: `apt install faust` or download `.deb` from GitHub releases — provides `libfaust.so` and headers
- macOS: `brew install faust` — provides `libfaust.dylib` and headers
- Windows: Faust GitHub releases provide a Windows installer with `faust.dll`
# native/CMakeLists.txt
# Platform-specific libfaust location
### Layer 6: Dart/Flutter Application Stack
| Package | Version | Purpose | Why |
|---------|---------|---------|-----|
| `flutter_riverpod` | 2.6.x | State management | Already decided; no codegen version per spec |
| `package:ffi` | 2.1.x | FFI helpers (arena allocator, Utf8) | Essential for ergonomic C interop |
| `ffigen` (dev) | 9.x | Generate Dart bindings from C header | Eliminates handwritten binding boilerplate |
| `path_provider` | 2.1.x | Find app documents dir for preset storage | Cross-platform, standard |
| `file_picker` | 8.x | Optional: import/export presets | For file open/save dialogs on desktop |
- `just_audio` — file/stream player, not synthesis
- `audioplayers` — same issue, file player
- `flutter_soloud` — designed for sample playback, not real-time DSP computation
- `dart_vlc` — abandoned, GPL
- Any OSC package — the old architecture used OSC to scsynth; libfaust is in-process, no OSC needed
## Full Stack Diagram
## Alternatives Considered
| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| DSP engine | libfaust (interpreter) | libfaust (LLVM) | LLVM adds 50-200MB, complex cross-compile, no perf benefit for <30 voices |
| DSP engine | libfaust | Csound | Csound C API is viable (LGPL) but Faust's functional model maps more naturally to "define synth graph, manipulate parameters from UI" |
| DSP engine | libfaust | fundsp (Rust) | Ruled out in architecture spec: fun but risky for timeline |
| Audio output | miniaudio | PortAudio | LGPL license, more complex build, no benefit |
| Audio output | miniaudio | flutter_soloud | Adds latency via Dart round-trip; designed for sample playback not DSP compute callback |
| FFI binding | dart:ffi + ffigen | Method channels | Method channels are for platform UI (camera, permissions); FFI is correct for C libraries |
| Build | Prebuilt libfaust binaries | Build libfaust from source | Building libfaust requires LLVM toolchain, infeasible for CI without significant effort |
## Installation
# Dart/Flutter packages
# System libfaust for development (Linux)
# or grab prebuilt binary from https://github.com/grame-cncm/faust/releases
# macOS
# Windows: download installer from https://github.com/grame-cncm/faust/releases
## Open Questions / Confidence Flags
| Area | Confidence | Note |
|------|-----------|------|
| libfaust interpreter C API | HIGH | Official docs confirm API surface; header names verified |
| miniaudio cross-platform | HIGH | Well-established; powers SoLoud and dozens of production tools |
| Dart FFI loading .so/.dylib/.dll | HIGH | Flutter docs confirm pattern; plugin_ffi template exists |
| ffigen with hand-written C header | HIGH | Standard Flutter pattern |
| libfaust prebuilt binary availability on Windows | MEDIUM | Faust GitHub releases show Windows installer but DLL extraction path needs verification |
| Thread safety between Dart FFI and miniaudio callback | MEDIUM | Standard lock-free pattern well-understood; specific libfaust interpreter thread safety needs verification — is interpreter state per-instance or shared? |
| macOS notarization with bundled .dylib | LOW | macOS requires code-signing all bundled dylibs; this is a known Flutter desktop pain point but has known solutions (sign during build) |
## Sources
- [Faust Embedding Documentation](https://faustdoc.grame.fr/manual/embedding/) — official, covers interpreter/LLVM/Wasm backends and C API
- [Faust GitHub](https://github.com/grame-cncm/faust) — source, releases, prebuilt binaries
- [Dart C interop (dart:ffi)](https://dart.dev/interop/c-interop) — official
- [Flutter macOS C interop](https://docs.flutter.dev/platform-integration/macos/c-interop) — official
- [miniaudio](https://miniaud.io/) — header-only library, MIT license
- [flutter_soloud pub.dev](https://pub.dev/packages/flutter_soloud) — reviewed as alternative; not recommended for synthesis use case
- [cyfaust](https://github.com/shakfu/cyfaust) — reference implementation of Faust interpreter + audio in Python, useful for understanding the integration pattern
<!-- GSD:stack-end -->

<!-- GSD:conventions-start source:CONVENTIONS.md -->
## Conventions

Conventions not yet established. Will populate as patterns emerge during development.
<!-- GSD:conventions-end -->

<!-- GSD:architecture-start source:ARCHITECTURE.md -->
## Architecture

Architecture not yet mapped. Follow existing patterns found in the codebase.
<!-- GSD:architecture-end -->

<!-- GSD:workflow-start source:GSD defaults -->
## GSD Workflow Enforcement

Before using Edit, Write, or other file-changing tools, start work through a GSD command so planning artifacts and execution context stay in sync.

Use these entry points:
- `/gsd:quick` for small fixes, doc updates, and ad-hoc tasks
- `/gsd:debug` for investigation and bug fixing
- `/gsd:execute-phase` for planned phase work

Do not make direct repo edits outside a GSD workflow unless the user explicitly asks to bypass it.
<!-- GSD:workflow-end -->



<!-- GSD:profile-start -->
## Developer Profile

> Profile not yet configured. Run `/gsd:profile-user` to generate your developer profile.
> This section is managed by `generate-claude-profile` -- do not edit manually.
<!-- GSD:profile-end -->
