# Technology Stack: Justifier Audio Synthesis

**Project:** Justifier — Just Intonation Sound Design Tool
**Researched:** 2026-04-01
**Overall confidence:** MEDIUM (libfaust C API well-documented; Dart FFI path proven; audio output layer has one clear choice with caveats)

---

## Recommended Stack

### Layer 1: DSP Engine — Faust / libfaust

| Component | Version | Purpose | Why |
|-----------|---------|---------|-----|
| libfaust (interpreter backend) | 2.75.x (latest) | Compile and run Faust DSP code at runtime | No LLVM dependency; ships as a single .so/.dylib/.dll; 3-10x slower than LLVM JIT but more than fast enough for <20 voice oscillator synthesis |
| C wrapper (hand-written) | n/a | Thin C layer over libfaust C++ API | Dart FFI cannot call C++ directly; C API headers already exist for interpreter backend |

**Backend choice: Interpreter over LLVM**

The LLVM backend (`faust/dsp/llvm-dsp.h`) compiles Faust source to native machine code via LLVM JIT — maximum performance, but requires bundling LLVM, which is large (50–200 MB depending on targets) and complex to cross-compile. The interpreter backend (`faust/dsp/interpreter-dsp.h`) compiles Faust source to bytecode and executes it in a Faust virtual machine. It is 3–10x slower than LLVM-JIT but produces a much smaller binary footprint and simpler build story.

For Justifier's synthesis requirements — at most 20–30 simultaneous oscillator voices, no convolution, no FFT — the interpreter backend is more than adequate. A sine oscillator in Faust bytecode will run at well under 1% CPU per voice on any modern desktop. The performance concern only becomes relevant for algorithmic patches with heavy feedback networks or spectral processing.

**The libfaust C API surface (interpreter backend):**

```c
// faust/dsp/interpreter-dsp.h (C API via interpreter-dsp-c.h wrapper)

// Factory: compile Faust source to bytecode
interpreter_dsp_factory* createInterpreterDSPFactoryFromString(
    const char* name_app,
    const char* dsp_content,
    int argc, const char* argv[],
    char* error_msg
);
void deleteInterpreterDSPFactory(interpreter_dsp_factory* factory);

// Instance: one voice = one instance
interpreter_dsp* createInterpreterDSPInstance(interpreter_dsp_factory* factory);
void deleteInterpreterDSPInstance(interpreter_dsp* dsp);

// DSP lifecycle
void initInterpreterDSPInstance(interpreter_dsp* dsp, int sample_rate);
int  getNumInputsInterpreterDSPInstance(interpreter_dsp* dsp);
int  getNumOutputsInterpreterDSPInstance(interpreter_dsp* dsp);

// Parameter control (use JSONUI or MapUI to get param addresses)
void setParamValueInterpreterDSPInstance(interpreter_dsp* dsp,
    const char* path, FAUSTFLOAT value);
FAUSTFLOAT getParamValueInterpreterDSPInstance(interpreter_dsp* dsp,
    const char* path);

// Audio processing — called on audio thread every buffer period
void computeInterpreterDSPInstance(interpreter_dsp* dsp,
    int count,
    FAUSTFLOAT** inputs,
    FAUSTFLOAT** outputs);
```

The factory is created once per waveform type (sine, saw, etc.) and shared across all instances of that waveform. Each voice in Justifier maps to one `interpreter_dsp*` instance. Parameter control (ratio, amplitude, pan) maps to `setParamValue` calls using the parameter path from the JSON UI descriptor.

**What NOT to use:** Do not use the LLVM backend for v1. The LLVM shared library adds significant build complexity and binary size for no meaningful benefit given the synthesis requirements. If you later need per-voice FM synthesis with dozens of operators, revisit.

---

### Layer 2: C Wrapper (custom, ~200 LOC)

Dart FFI requires a C linkage layer. libfaust's interpreter API is technically C++ (the objects are opaque pointers, but the functions have C++ linkage). You need a thin C wrapper.

**What it does:**
- Wraps `createInterpreterDSPFactoryFromString` and instance lifecycle
- Exposes `compute(dsp, n_samples, float** outputs)` for the audio callback
- Owns the summing mix buffer: iterates all active instances and sums into a stereo output buffer
- Exposes `set_param(dsp, path, value)` for real-time parameter updates from Dart

**Where it lives:** `native/faust_bridge/faust_bridge.c` (or `.cpp` compiled as C-compatible). Built via CMake as part of the Flutter plugin build.

**Threading model:** The C wrapper owns a dedicated audio thread. Dart calls `set_param` from the UI thread; the C layer must handle thread-safe parameter updates (use atomic float or a lock-free ring buffer for parameter changes, not a mutex in the audio callback).

---

### Layer 3: Audio Output — miniaudio (direct, not via a Flutter package)

| Component | Version | Purpose | Why |
|-----------|---------|---------|-----|
| miniaudio | 0.11.x (header-only) | Cross-platform audio output: ALSA (Linux), CoreAudio (macOS), WASAPI (Windows) | Single-header C library, MIT license, zero dependencies, proven in production, designed for embedding |

**Why miniaudio over alternatives:**

| Option | Assessment |
|--------|-----------|
| **miniaudio** | RECOMMENDED. Header-only, MIT, handles ALSA/CoreAudio/WASAPI automatically, used in production games and audio tools. The `ma_device` callback model integrates directly with the libfaust `compute()` call. |
| PortAudio | AVOID. LGPL license (requires dynamic linking or releasing source), significantly more complex build story than miniaudio, no meaningful benefit for this use case. |
| flutter_soloud | NOT RECOMMENDED for this use case. flutter_soloud wraps SoLoud (which uses miniaudio internally), but it is designed for playing pre-rendered audio samples and streaming PCM into it from Dart. The round-trip — compute in C, send to Dart, push back into SoLoud — adds latency and complexity. Better to own miniaudio directly and compute in the C callback. |
| ALSA/CoreAudio/WASAPI direct | AVOID. Platform-specific, enormous per-platform code burden. miniaudio already wraps these correctly. |
| RtAudio | Viable but overkill. C++ API, LGPL, heavier than miniaudio for this use case. |

**miniaudio integration pattern:**

```c
// In faust_bridge.c
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

static void audio_callback(ma_device* device, void* output,
                           const void* input, ma_uint32 frame_count) {
    float* out = (float*)output;
    // Zero the output buffer
    memset(out, 0, frame_count * 2 * sizeof(float));
    // Iterate active faust DSP instances, compute and accumulate
    for (int i = 0; i < active_voice_count; i++) {
        float* ch[2] = { tmp_L, tmp_R };
        computeInterpreterDSPInstance(voices[i], frame_count, NULL, ch);
        for (int s = 0; s < frame_count; s++) {
            out[s*2]   += tmp_L[s];
            out[s*2+1] += tmp_R[s];
        }
    }
}
```

The audio callback runs on miniaudio's audio thread. Dart FFI calls on the UI thread. Synchronization between them is the main design constraint (see Pitfalls).

---

### Layer 4: Dart FFI Binding

| Component | Version | Purpose | Why |
|-----------|---------|---------|-----|
| `dart:ffi` (stdlib) | bundled with Dart/Flutter | Call C functions from Dart | Standard, no additional packages |
| `package:ffi` | 2.1.x | Arena allocator, `Pointer<Utf8>`, struct helpers | Makes FFI ergonomic; official Dart team package |
| `ffigen` | 9.x | Generate Dart bindings from C headers | Avoids handwriting bindings for large APIs |

**Integration path:**

```
Flutter UI (Dart)
    │  calls via dart:ffi
    ▼
faust_bridge.h  (C API surface you control)
    │  calls
    ├──▶ interpreter-dsp-c.h  (libfaust interpreter C API)
    │        compiled into libfaust.so / libfaust.dylib / faust.dll
    └──▶ miniaudio.h  (header-only, compiled inline)

Audio thread (owned by miniaudio ma_device)
    └──▶ computeInterpreterDSPInstance() for each voice
    └──▶ writes to device output buffer → speakers
```

**Binding generation:** Use `ffigen` to generate Dart bindings from `faust_bridge.h`. Run it once and commit the generated file. The generated bindings cover: `create_voice`, `destroy_voice`, `set_voice_param`, `start_audio`, `stop_audio`, `panic`.

**Loading the library:**

```dart
// lib/src/audio/faust_bridge.dart
final DynamicLibrary _lib = () {
  if (Platform.isLinux)   return DynamicLibrary.open('libfaust_bridge.so');
  if (Platform.isMacOS)   return DynamicLibrary.open('libfaust_bridge.dylib');
  if (Platform.isWindows) return DynamicLibrary.open('faust_bridge.dll');
  throw UnsupportedError('Unsupported platform');
}();
```

**Isolate consideration:** For v1 with <30 voices and no audio analysis, calling `set_param` from the main Dart isolate is fine. If parameter-set volume becomes a bottleneck, move the bridge calls to a background isolate — but profile first, don't prematurely optimize.

---

### Layer 5: Flutter Plugin Build System — CMake + Flutter Build Hooks

| Component | Purpose |
|-----------|---------|
| CMake (3.14+) | Build libfaust (if building from source) and faust_bridge C wrapper on all platforms |
| Flutter plugin_ffi template | Scaffold for the native code + CMakeLists.txt integration |
| Prebuilt libfaust binaries | Preferred over building LLVM toolchain for CI |

**Build strategy — prebuilt libfaust:**

Building libfaust from source requires LLVM (even for the interpreter backend, LLVM is a build dependency though not a runtime dependency). This is a significant CI burden. The Faust project ships prebuilt packages:

- Linux: `apt install faust` or download `.deb` from GitHub releases — provides `libfaust.so` and headers
- macOS: `brew install faust` — provides `libfaust.dylib` and headers
- Windows: Faust GitHub releases provide a Windows installer with `faust.dll`

**Recommended approach:** Check prebuilt binaries into `native/lib/{linux,macos,windows}/` and vendor them with the app. This is the same approach used by most native Flutter audio plugins. Version-pin them (e.g., faust 2.75.7).

**CMakeLists.txt structure:**

```cmake
# native/CMakeLists.txt
cmake_minimum_required(VERSION 3.14)
project(faust_bridge)

# Platform-specific libfaust location
if(UNIX AND NOT APPLE)
  set(FAUST_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/linux/libfaust.so)
elseif(APPLE)
  set(FAUST_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/macos/libfaust.dylib)
elseif(WIN32)
  set(FAUST_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/windows/faust.dll)
endif()

add_library(faust_bridge SHARED
    faust_bridge.cpp
)
target_include_directories(faust_bridge PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include  # faust headers + miniaudio.h
)
target_link_libraries(faust_bridge ${FAUST_LIB})
```

**What NOT to do:** Do not use `flutter_rust_bridge` or pull in a Rust layer for audio. The project spec explicitly evaluated and declined self-synthesis in Rust. The C path (Dart FFI -> C wrapper -> libfaust + miniaudio) is simpler, better documented, and has fewer moving parts.

---

### Layer 6: Dart/Flutter Application Stack

| Package | Version | Purpose | Why |
|---------|---------|---------|-----|
| `flutter_riverpod` | 2.6.x | State management | Already decided; no codegen version per spec |
| `package:ffi` | 2.1.x | FFI helpers (arena allocator, Utf8) | Essential for ergonomic C interop |
| `ffigen` (dev) | 9.x | Generate Dart bindings from C header | Eliminates handwritten binding boilerplate |
| `path_provider` | 2.1.x | Find app documents dir for preset storage | Cross-platform, standard |
| `file_picker` | 8.x | Optional: import/export presets | For file open/save dialogs on desktop |

**What to NOT include:**
- `just_audio` — file/stream player, not synthesis
- `audioplayers` — same issue, file player
- `flutter_soloud` — designed for sample playback, not real-time DSP computation
- `dart_vlc` — abandoned, GPL
- Any OSC package — the old architecture used OSC to scsynth; libfaust is in-process, no OSC needed

---

## Full Stack Diagram

```
┌─────────────────────────────────────────────────────────┐
│  Flutter UI (Dart)                                       │
│  Riverpod state → FaustBridgeService → dart:ffi calls   │
└──────────────────────┬──────────────────────────────────┘
                       │ Dart FFI (DynamicLibrary.open)
┌──────────────────────▼──────────────────────────────────┐
│  faust_bridge.{so,dylib,dll}  (C/C++, your code)        │
│  - Voice lifecycle: create/destroy/set_param            │
│  - Owns voice array, mix buffer                         │
│  - Thread-safe param updates (lock-free)                │
└────────────┬──────────────────────┬─────────────────────┘
             │ libfaust C API        │ miniaudio
┌────────────▼────────┐  ┌──────────▼──────────────────── ┐
│  libfaust           │  │  miniaudio (header-only)         │
│  interpreter backend│  │  ALSA (Linux)                   │
│  DSP instances      │  │  CoreAudio (macOS)              │
│  compute() per buf  │  │  WASAPI (Windows)               │
└─────────────────────┘  └─────────────────────────────────┘
```

---

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

---

## Installation

```bash
# Dart/Flutter packages
flutter pub add flutter_riverpod
flutter pub add ffi
flutter pub add path_provider
flutter pub add dev:ffigen

# System libfaust for development (Linux)
sudo apt install faust
# or grab prebuilt binary from https://github.com/grame-cncm/faust/releases

# macOS
brew install faust

# Windows: download installer from https://github.com/grame-cncm/faust/releases
```

---

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

---

## Sources

- [Faust Embedding Documentation](https://faustdoc.grame.fr/manual/embedding/) — official, covers interpreter/LLVM/Wasm backends and C API
- [Faust GitHub](https://github.com/grame-cncm/faust) — source, releases, prebuilt binaries
- [Dart C interop (dart:ffi)](https://dart.dev/interop/c-interop) — official
- [Flutter macOS C interop](https://docs.flutter.dev/platform-integration/macos/c-interop) — official
- [miniaudio](https://miniaud.io/) — header-only library, MIT license
- [flutter_soloud pub.dev](https://pub.dev/packages/flutter_soloud) — reviewed as alternative; not recommended for synthesis use case
- [cyfaust](https://github.com/shakfu/cyfaust) — reference implementation of Faust interpreter + audio in Python, useful for understanding the integration pattern
