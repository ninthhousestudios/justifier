# Phase 1: C Audio Engine - Research

**Researched:** 2026-04-01
**Domain:** Real-time C audio engine — miniaudio, Faust compile-time DSP, SPSC lock-free queue, pre-allocated voice pool
**Confidence:** HIGH

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

- **D-01:** 32 voice slots, pre-allocated at startup. Each slot holds one Faust DSP instance.
- **D-02:** 20ms crossfade when switching waveform type on a live voice. Run both DSP instances during the overlap, fade one out and the other in. Atomic pointer swap pattern in the C wrapper.
- **D-03:** Default buffer size: 256 samples. Default sample rate: 48000 Hz.
- **D-04:** Buffer size and sample rate are defaults, not hardcoded. PipeWire on Linux may override — handle gracefully.
- **D-05:** Per-voice parameters: `frequency` (Hz), `amplitude` (0.0–1.0), `gate` (on/off with configurable attack/release), `pan` (-1.0 to 1.0), `detune` (cents offset).
- **D-06:** FM waveform additionally exposes: `mod_ratio`, `mod_index`.
- **D-07:** Compile-time Faust for v1. The 7 `.dsp` files compiled to C++ via `faust -lang cpp` during the CMake build. No runtime libfaust. C wrapper interface abstracted for future runtime drop-in.
- **D-08:** Proven header-only lock-free SPSC queue (Cameron Desrochers' `readerwriterqueue`). No custom implementation.
- **D-09:** miniaudio for audio output. Abstracted behind `audio_engine.c` so it can be swapped.

### Claude's Discretion

- C test harness design (how to verify audio output without Flutter)
- CMake build structure for Faust compilation + miniaudio + SPSC queue
- Voice slot lifecycle implementation details (activation, deactivation, reclamation)
- Silence flag implementation (atomic bool vs atomic int)

### Deferred Ideas (OUT OF SCOPE)

None — discussion stayed within phase scope.

</user_constraints>

---

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| AUD-01 | App produces real-time audio output via miniaudio on Linux, macOS, and Windows | miniaudio `ma_device` callback model; ALSA/CoreAudio/WASAPI handled automatically; see Standard Stack and Code Examples |
| AUD-02 | 7 waveform types (sine, triangle, saw, square, pulse, noise, FM) compiled from Faust .dsp files at build time | Compile-time Faust via `faust -lang cpp`; CMake `add_custom_command` pattern; see Architecture Patterns |
| AUD-03 | UI parameter changes reach audio thread without glitches via lock-free message passing | SPSC queue (readerwriterqueue); parameters applied each buffer cycle; atomic `is_silent` flag for panic; see SPSC Queue Pattern |
| AUD-04 | Voice slots are pre-allocated at startup to avoid audio-thread allocation | Fixed pool of 32 `VoiceSlot` structs; voice add/remove = state flag change, no malloc; see Voice Pool Pattern |

</phase_requirements>

---

## Summary

Phase 1 delivers the complete C audio foundation in isolation from Flutter. The goal is to prove the entire real-time pipeline — miniaudio device, SPSC control queue, pre-allocated voice pool, Faust-compiled DSP — works correctly on Linux before Dart is introduced. The output artifact is `justifier_audio.h` (the public C API) plus a static/shared library that Phase 2 will bind to via Dart FFI.

The architecture is settled: compile-time Faust (no runtime libfaust), miniaudio for audio I/O, Cameron Desrochers' `readerwriterqueue` for lock-free parameter passing, and a fixed 32-slot voice pool. The audio callback is a pure C function that never allocates, never blocks, and never touches Dart. All parameter updates flow through the SPSC queue written by the caller and consumed by the audio thread each buffer cycle.

The most important design discipline for this phase is the real-time audio thread boundary. Every architecture decision — pre-allocated pool, SPSC queue, atomic panic flag — exists to enforce one rule: the audio callback must never block. This is not a bug fix category; it is a structural constraint that must be built in before any audio plays.

**Primary recommendation:** Build and validate in two sub-steps: (1) bare miniaudio sine oscillator with no Faust, to confirm the audio device, callback, and SPSC queue work; (2) integrate compiled Faust DSP classes into the working pipeline. Validate each sub-step with a simple C test binary before proceeding.

---

## Standard Stack

### Core

| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| miniaudio | 0.11.x | Cross-platform audio I/O (ALSA/CoreAudio/WASAPI) | Single-header, public domain, zero deps, production-proven in SoLoud and dozens of game engines |
| readerwriterqueue | 1.0.6 | Single-producer/single-consumer lock-free queue | Cameron Desrochers' implementation is the canonical C++ SPSC queue; header-only, well-tested |
| Faust compiler (`faust` CLI) | 2.75.x | Compile `.dsp` → C++ at build time | Required for AUD-02; available via `apt install faust` on Linux |
| C11 atomics (`stdatomic.h`) | C11 stdlib | `is_silent` flag, voice state flags | Zero-dependency; `_Atomic bool` is sufficient for the panic flag use case |

### Supporting

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| CMake | 3.14+ (4.3.1 installed) | Build system orchestrating Faust codegen + C++ compilation | Required for `add_custom_command` Faust → C++ step |
| miniaudio device enumeration | (bundled) | List available audio devices for the test harness | Useful during development/debugging |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| readerwriterqueue | hand-rolled SPSC ring buffer | Cameron's implementation handles C++ memory model edge cases correctly; no benefit to custom |
| miniaudio | PortAudio | LGPL, complex build, no benefit for this use case |
| `_Atomic bool` panic flag | SPSC queue message | Queue message can be delayed by backlog; atomic flag bypasses queue entirely — required for Pitfall 13 |
| Faust compile-time | libfaust interpreter backend | Interpreter: runtime libfaust dep, 3–10x slower DSP, bundling complexity; compile-time has none of these costs |

**Installation:**
```bash
# Linux — system Faust compiler (build-time only, not shipped)
sudo apt install faust

# miniaudio and readerwriterqueue — vendored as headers in native/include/
# Download:
#   https://raw.githubusercontent.com/mackron/miniaudio/master/miniaudio.h
#   https://raw.githubusercontent.com/cameron314/readerwriterqueue/master/readerwriterqueue.h
#   https://raw.githubusercontent.com/cameron314/readerwriterqueue/master/atomicops.h
```

**Version verification:** As of 2026-04-01:
- `faust` (system): not installed yet (confirmed by env check — `apt install faust` required)
- cmake: 4.3.1 (confirmed installed)
- gcc: 15.2.1 (confirmed installed)
- miniaudio: 0.11.21 (latest as of knowledge cutoff — verify at https://github.com/mackron/miniaudio/releases)
- readerwriterqueue: 1.0.6 (verify at https://github.com/cameron314/readerwriterqueue/releases)

---

## Architecture Patterns

### Recommended Project Structure

```
native/
├── include/
│   ├── miniaudio.h          # vendored, header-only
│   ├── readerwriterqueue.h  # vendored, header-only
│   └── atomicops.h          # readerwriterqueue dependency
├── dsp/
│   ├── sine.dsp             # Faust source
│   ├── triangle.dsp
│   ├── saw.dsp
│   ├── square.dsp
│   ├── pulse.dsp
│   ├── noise.dsp
│   └── fm.dsp
├── generated/               # CMake output — .cpp files from faust -lang cpp
│   ├── sine_dsp.cpp
│   └── ...                  # gitignore this directory
├── src/
│   ├── justifier_audio.h    # Public C API — the only file Phase 2 imports
│   ├── audio_engine.c       # miniaudio device, callback, SPSC queue, voice pool
│   ├── voice_slot.h         # VoiceSlot struct definition
│   └── faust_wrapper.cpp    # C++ shim: instantiates generated DSP classes
├── test/
│   └── test_audio.c         # C test harness — no Flutter dependency
└── CMakeLists.txt
```

### Pattern 1: Audio Callback — Lock-Free Control Queue

**What:** The `ma_device` callback reads the SPSC queue once per buffer cycle, applies pending parameter changes, then calls `compute()` on each active voice.

**When to use:** Every parameter change from outside the audio thread goes through this queue. No exceptions.

**Example:**
```c
// audio_engine.c
static void audio_callback(ma_device* device, void* output,
                            const void* input, ma_uint32 frame_count) {
    AudioEngine* eng = (AudioEngine*)device->pUserData;

    // 1. Check panic flag first — bypasses queue entirely
    if (atomic_load_explicit(&eng->is_silent, memory_order_relaxed)) {
        memset(output, 0, frame_count * 2 * sizeof(float));
        return;
    }

    // 2. Drain control queue — apply pending parameter updates
    ControlMessage msg;
    while (spsc_try_dequeue(&eng->control_queue, &msg)) {
        apply_control_message(eng, &msg);
    }

    // 3. Mix all active voices into output buffer
    float* out = (float*)output;
    memset(out, 0, frame_count * 2 * sizeof(float));

    float tmp_L[MAX_BUFFER_SIZE], tmp_R[MAX_BUFFER_SIZE];
    float ch_ptrs[2] = { tmp_L, tmp_R };  // Faust expects float**

    for (int i = 0; i < MAX_VOICES; i++) {
        VoiceSlot* slot = &eng->voices[i];
        if (!atomic_load_explicit(&slot->active, memory_order_relaxed)) continue;

        slot->dsp->compute(frame_count, NULL, (float**)ch_ptrs);

        // Accumulate into stereo output
        float amp_L = slot->amplitude * (1.0f - slot->pan) * 0.5f;
        float amp_R = slot->amplitude * (1.0f + slot->pan) * 0.5f;
        for (ma_uint32 s = 0; s < frame_count; s++) {
            out[s*2]   += tmp_L[s] * amp_L;
            out[s*2+1] += tmp_R[s] * amp_R;
        }
    }
}
```

### Pattern 2: Voice Pool — Pre-Allocated Fixed Array

**What:** All 32 `VoiceSlot` structs allocated at `justifier_init()`. Activating a voice = finding an inactive slot and setting its state. No `malloc` in the audio callback.

**When to use:** All voice creation and destruction.

**Example:**
```c
// voice_slot.h
typedef struct {
    _Atomic int   active;          // 0=free, 1=active, 2=fading_out
    FaustDSPBase* dsp;             // pointer to pre-allocated DSP instance
    FaustDSPBase* dsp_pending;     // non-null during 20ms waveform crossfade
    float         frequency;
    float         amplitude;
    float         pan;
    float         detune_cents;
    float         crossfade_gain;  // 0.0→1.0 during crossfade
    int           crossfade_samples_remaining;
} VoiceSlot;

// audio_engine.c — pre-allocated at init, never malloc'd in callback
typedef struct {
    VoiceSlot            voices[MAX_VOICES];     // 32 slots
    FaustDSPBase*        dsp_pool[MAX_VOICES];   // pre-allocated DSP instances per type
    // ... (one pool per waveform type, indexed by WaveformType enum)
    moodycamel_ReaderWriterQueue* control_queue;
    _Atomic bool         is_silent;
    ma_device            device;
} AudioEngine;
```

### Pattern 3: Waveform Crossfade via Atomic Pointer Swap

**What:** When waveform type changes on a live voice, a new DSP instance is activated alongside the old one. The audio callback runs both for 20ms (960 samples at 48kHz), crossfading between them, then deactivates the old one.

**When to use:** All waveform type changes on voices that are currently playing.

**Example:**
```c
// Control message that initiates crossfade
typedef struct {
    MessageType type;  // MSG_VOICE_CHANGE_WAVEFORM
    int         voice_id;
    int         new_waveform_type;
    float       initial_frequency;
    float       initial_amplitude;
} ControlMessage;

// In apply_control_message() — runs on audio thread, no alloc
case MSG_VOICE_CHANGE_WAVEFORM: {
    VoiceSlot* slot = &eng->voices[msg.voice_id];
    // Get pre-allocated DSP instance for new type from pool
    FaustDSPBase* new_dsp = acquire_dsp_from_pool(eng, msg.new_waveform_type);
    new_dsp->init(eng->sample_rate);
    new_dsp->setParamValue("frequency", msg.initial_frequency);
    slot->dsp_pending = new_dsp;
    slot->crossfade_samples_remaining = (int)(0.020f * eng->sample_rate); // 960 at 48kHz
    slot->crossfade_gain = 0.0f;
    break;
}
```

### Pattern 4: Faust Compile-Time DSP via CMake

**What:** CMake `add_custom_command` runs `faust -lang cpp` on each `.dsp` file, generating a C++ class. The generated `.cpp` files are compiled into the native library.

**When to use:** Build step for all 7 waveform types.

**Example:**
```cmake
# native/CMakeLists.txt

set(FAUST_DSP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dsp")
set(FAUST_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
file(MAKE_DIRECTORY ${FAUST_GEN_DIR})

# List all .dsp files
set(FAUST_DSP_FILES sine triangle saw square pulse noise fm)

foreach(DSP_NAME ${FAUST_DSP_FILES})
    add_custom_command(
        OUTPUT  "${FAUST_GEN_DIR}/${DSP_NAME}_dsp.cpp"
        COMMAND faust -lang cpp -a minimal.cpp
                "${FAUST_DSP_DIR}/${DSP_NAME}.dsp"
                -o "${FAUST_GEN_DIR}/${DSP_NAME}_dsp.cpp"
        DEPENDS "${FAUST_DSP_DIR}/${DSP_NAME}.dsp"
        COMMENT "Compiling Faust DSP: ${DSP_NAME}.dsp"
    )
    list(APPEND GENERATED_DSP_SOURCES "${FAUST_GEN_DIR}/${DSP_NAME}_dsp.cpp")
endforeach()

add_library(justifier_audio STATIC
    src/audio_engine.c
    src/faust_wrapper.cpp
    ${GENERATED_DSP_SOURCES}
)
target_include_directories(justifier_audio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${FAUST_GEN_DIR}
    /usr/include/faust   # Faust headers (base dsp class, UI interfaces)
)
```

**Critical:** The `-a minimal.cpp` flag specifies the Faust architecture file. Use `minimal.cpp` (ships with Faust) to get a plain C++ class with no JACK/ALSA boilerplate. The output is a class named `mydsp` — you will want to rename it or wrap it.

### Pattern 5: C Test Harness

**What:** A standalone `test_audio.c` that links against `justifier_audio` and plays a sine wave for 3 seconds. No Flutter, no Dart.

**When to use:** Phase 1 verification. Run after every significant change to the C layer.

**Example:**
```c
// native/test/test_audio.c
#include "justifier_audio.h"
#include <stdio.h>
#include <unistd.h>

int main(void) {
    int result = justifier_init(48000, 256);
    if (result != 0) { fprintf(stderr, "init failed: %d\n", result); return 1; }

    int voice_id = justifier_voice_add(WAVEFORM_SINE, 440.0f, 0.3f);
    printf("Playing sine at 440 Hz for 3 seconds...\n");
    sleep(3);

    justifier_voice_remove(voice_id);
    sleep(1);  // let release tail complete

    justifier_shutdown();
    printf("Done. Xruns: %d\n", justifier_get_xrun_count());
    return 0;
}
```

### Anti-Patterns to Avoid

- **Calling `malloc`/`free` in the audio callback:** All DSP instances and mix buffers must be pre-allocated at `justifier_init()`. Violation causes audio thread blocking and glitches.
- **Using a mutex instead of SPSC queue:** Mutexes can block. The audio thread must never wait for any lock held by the UI thread.
- **Calling back into Dart from the audio callback:** There is no Dart runtime on the audio thread. Any `NativeCallable` invocation from the audio callback crashes in release mode. Status reporting (xrun count) is polled by Dart on a timer.
- **Putting the panic command in the SPSC queue:** Queue messages can be delayed by backlog. The `is_silent` flag is read at the very top of the callback, before queue processing, for guaranteed one-cycle response.
- **Compiling miniaudio's `MINIAUDIO_IMPLEMENTATION` in multiple translation units:** This is a single-header library — `#define MINIAUDIO_IMPLEMENTATION` must appear in exactly one `.c` file. Put it in `audio_engine.c` only.
- **Using `hslider`/`vslider` in Faust DSP programs (without testing):** These UI primitives trigger Pitfall 3 (heap corruption on `deleteDSPFactory`) in certain compiler/LLVM combinations. Use `nentry` as a safer alternative, or confirm behavior is clean with the chosen Faust + GCC version.

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Lock-free SPSC queue | custom ring buffer with atomic head/tail | `readerwriterqueue` | Desrochers' implementation handles C++ memory model subtleties (acquire/release ordering, false sharing avoidance) correctly; subtle bugs in hand-rolled versions cause rare data corruption under load |
| Cross-platform audio I/O | ALSA/CoreAudio/WASAPI direct | `miniaudio` | Each platform API is ~500-1000 lines of boilerplate; miniaudio handles all three correctly including PipeWire compat on Linux |
| DSP parameter smoothing | manual lerp in callback | Faust `si.smooth` primitive | `si.smooth` is a one-pole lowpass filter that is mathematically correct and sample-accurate; hand-rolled lerp tends to produce zipper noise on discontinuities |
| Waveform oscillators | hand-coded C oscillators | Faust `.dsp` files | Required by D-07 and AUD-02; Faust handles phase accumulator precision and band-limiting correctly; FM especially is complex to implement correctly without zipper noise |

**Key insight:** The audio thread constraint means every data structure touching the callback must be wait-free. Any library that is not explicitly designed for wait-free access (e.g., `std::queue`, `std::mutex`) is wrong here regardless of how "simple" the use case looks.

---

## Common Pitfalls

### Pitfall 1: Audio Thread Allocation
**What goes wrong:** Any `malloc`/`new`/`free` call in the audio callback causes intermittent glitches that worsen under CPU load.
**Why it happens:** The system allocator uses an internal mutex; if the UI thread holds it during a GC or large allocation, the audio thread blocks.
**How to avoid:** Pre-allocate the entire voice pool (32 `VoiceSlot` + their DSP instances) at `justifier_init()`. Voice add = flip an atomic flag. Voice remove = schedule deactivation via queue message.
**Warning signs:** Glitches that only appear when creating/destroying voices or when the system is under load.

### Pitfall 2: DSP Hot-Swap Use-After-Free
**What goes wrong:** Swapping a voice's DSP instance for a new waveform type while the audio callback may be running produces use-after-free if not synchronized correctly.
**Why it happens:** Naive implementation: `slot->dsp = new_dsp; free(old_dsp)`. The callback may have read `slot->dsp` before the swap, now has a pointer to freed memory.
**How to avoid:** Use the crossfade pattern (Pattern 3 above). The queue message sets `dsp_pending`; the callback blends for 960 samples, then atomically promotes `dsp_pending` → `dsp` and returns the old one to the pool. Old DSP is only reclaimed after the callback confirms it has stopped using it.
**Warning signs:** Occasional crashes when switching waveform types, especially at low buffer sizes.

### Pitfall 3: Panic Flag as Queue Message
**What goes wrong:** Sending panic as a normal queue message means it is delayed behind backlog during rapid parameter automation. The user hears 10–100ms of audio before silence.
**Why it happens:** The SPSC queue is FIFO; if 50 parameter messages are enqueued ahead of the panic message, they all run first.
**How to avoid:** `justifier_panic()` sets an `_Atomic bool is_silent = true` directly. The callback reads this flag as its very first action, returns silence immediately. No queue involvement.
**Warning signs:** Panic button takes noticeably more than one buffer cycle (~5ms) to silence output.

### Pitfall 4: Faust Architecture File Mismatch
**What goes wrong:** `faust -lang cpp` without `-a architecture.cpp` generates a file that is missing the base `dsp` class definition. The generated code won't compile.
**Why it happens:** Faust requires an architecture file that provides the runtime scaffolding (`dsp` base class, `UI` interface). The default behavior generates only the DSP algorithm without the scaffolding.
**How to avoid:** Always use `-a minimal.cpp` (or equivalent) in the CMake command. The `minimal.cpp` architecture file ships with Faust and generates a self-contained compilable class.
**Warning signs:** Compile errors like `'dsp' does not name a type` or `undefined reference to UI::declare`.

### Pitfall 5: PipeWire Buffer Size Override
**What goes wrong:** PipeWire sets a system-wide quantum that all audio clients must use. If another app has requested 64 samples, Justifier is forced to run at 64 samples (1.33ms) instead of the requested 256 — increasing CPU pressure.
**Why it happens:** PipeWire's quantum is global, not per-application. The requested buffer size is a hint, not a guarantee.
**How to avoid:** D-04 explicitly requires the code to handle this gracefully. The audio callback receives the actual `frame_count` from miniaudio — never assume it equals the requested buffer size. Size all temporary buffers to `MAX_BUFFER_SIZE` (e.g., 4096), not the configured default.
**Warning signs:** Crashes or glitches only on Linux when other audio apps are running.

### Pitfall 6: `MINIAUDIO_IMPLEMENTATION` in Multiple Files
**What goes wrong:** Multiple `.c`/`.cpp` files that each `#define MINIAUDIO_IMPLEMENTATION` produce duplicate symbol linker errors.
**Why it happens:** miniaudio is a single-header library where the implementation is conditionally compiled. Multiple definitions violate the one-definition rule.
**How to avoid:** `#define MINIAUDIO_IMPLEMENTATION` in exactly one translation unit (`audio_engine.c`). All other files `#include "miniaudio.h"` without the define.
**Warning signs:** Linker errors like `multiple definition of ma_device_init`.

---

## Code Examples

Verified patterns from official sources and established C audio programming practice:

### Public C API (`justifier_audio.h`)
```c
// justifier_audio.h — the only file Phase 2's Dart FFI binds to
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WAVEFORM_SINE     = 0,
    WAVEFORM_TRIANGLE = 1,
    WAVEFORM_SAW      = 2,
    WAVEFORM_SQUARE   = 3,
    WAVEFORM_PULSE    = 4,
    WAVEFORM_NOISE    = 5,
    WAVEFORM_FM       = 6,
} WaveformType;

// Engine lifecycle
int  justifier_init(int sample_rate, int buffer_size);
void justifier_shutdown(void);

// Voice management — all writes go through the SPSC queue
int  justifier_voice_add(WaveformType type, float frequency, float amplitude);
void justifier_voice_remove(int voice_id);
void justifier_voice_set_frequency(int voice_id, float hz);
void justifier_voice_set_amplitude(int voice_id, float amplitude);
void justifier_voice_set_pan(int voice_id, float pan);
void justifier_voice_set_detune(int voice_id, float cents);
void justifier_voice_set_waveform(int voice_id, WaveformType type);

// FM-specific
void justifier_voice_set_mod_ratio(int voice_id, float ratio);
void justifier_voice_set_mod_index(int voice_id, float index);

// Gate (envelope on/off — used for attack on create, release on sleep-before-destroy)
void justifier_voice_set_gate(int voice_id, int gate_on);

// Global
void justifier_panic(void);     // atomic flag — one buffer cycle to silence
void justifier_set_master_volume(float volume);

// Status — safe to call from any thread (reads atomics, no locks)
int  justifier_is_running(void);
int  justifier_get_xrun_count(void);
int  justifier_get_active_voice_count(void);

#ifdef __cplusplus
}
#endif
```

### miniaudio Device Init
```c
// Source: https://miniaud.io/docs/manual/index.html
// audio_engine.c
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

int justifier_init(int sample_rate, int buffer_size) {
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format   = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate        = (ma_uint32)sample_rate;
    config.periodSizeInFrames = (ma_uint32)buffer_size;  // hint only — PipeWire may override
    config.dataCallback      = audio_callback;
    config.pUserData         = &g_engine;

    if (ma_device_init(NULL, &config, &g_engine.device) != MA_SUCCESS) {
        return -1;
    }
    return ma_device_start(&g_engine.device) == MA_SUCCESS ? 0 : -2;
}
```

### Faust Parameter Path Convention
```cpp
// Faust generates parameter paths from the DSP program structure.
// For a .dsp file with:
//   process = vgroup("justifier", hgroup("sine", ...));
// The path for a parameter named "freq" would be:
//   "/justifier/sine/freq"
//
// In faust_wrapper.cpp:
void set_voice_frequency(VoiceSlot* slot, float hz) {
    slot->dsp->setParamValue("/justifier/sine/freq", hz);
}
// Use JSONUI at init time to discover actual paths programmatically,
// rather than hardcoding path strings.
```

### SPSC Queue Usage (readerwriterqueue)
```cpp
// Source: https://github.com/cameron314/readerwriterqueue
// faust_wrapper.cpp
#include "readerwriterqueue.h"

// Single global queue: Dart/UI side writes, audio callback reads
static moodycamel::ReaderWriterQueue<ControlMessage> g_control_queue(1024);

// UI thread (safe to call from any non-audio thread)
void justifier_voice_set_frequency(int voice_id, float hz) {
    ControlMessage msg = { MSG_SET_FREQUENCY, voice_id, .value = hz };
    g_control_queue.enqueue(msg);  // non-blocking, wait-free
}

// Audio callback (audio thread only)
static void drain_control_queue(AudioEngine* eng) {
    ControlMessage msg;
    while (eng->control_queue.try_dequeue(msg)) {
        apply_message(eng, msg);
    }
}
```

---

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| libfaust runtime interpreter | Compile-time Faust (`faust -lang cpp`) | Decision: 2026-04-01 | Smaller binary, no runtime libfaust dep, native DSP speed, eliminates Pitfall 3 and Pitfall 7 |
| OSC to scsynth subprocess | Dart FFI → C wrapper → miniaudio | Architecture pivot | In-process audio, no IPC latency, no scsynth process management |
| LLVM JIT backend | Interpreter or compile-time | Research: 2026-04-01 | LLVM adds 50–200MB; no perf benefit for <32 oscillator voices |

**Deprecated/outdated patterns for this project:**
- `PortAudio`: LGPL license, no benefit over miniaudio for this use case
- `flutter_soloud`: designed for sample playback, wrong abstraction for synthesis
- `hslider`/`vslider` Faust UI primitives: implicated in Pitfall 3 (heap corruption); prefer `nentry` or test carefully

---

## Open Questions

1. **Faust architecture file for generated classes**
   - What we know: `faust -lang cpp -a minimal.cpp` produces a self-contained class; `minimal.cpp` ships with Faust
   - What's unclear: Exact path to `minimal.cpp` varies by OS install (`/usr/share/faust/` on Linux, varies on macOS/Windows)
   - Recommendation: In CMakeLists.txt, find the Faust share directory dynamically via `faust --print-dir` or hardcode per-platform and document

2. **Per-voice DSP pool layout for crossfade**
   - What we know: Crossfade requires two DSP instances per voice temporarily; 32 slots × 7 waveform types × 2 = 448 pre-allocated instances is excessive
   - What's unclear: Whether to pre-allocate per slot (2 instances per slot × 32 = 64 instances) or use a global pool of 64 instances shared across slots
   - Recommendation: Fixed 64-instance global pool (32 active + 32 pending). Voice slot holds two indices into the pool. Pool management is a simple free-list, managed only from the audio thread.

3. **Gate envelope time constants**
   - What we know: D-05 specifies `gate` with configurable attack/release; CONTEXT.md mentions 10s fade-out for sleep-before-destroy
   - What's unclear: Default attack time for new voice creation (design spec has `fadeTime: 0.5s` in JSON)
   - Recommendation: Default attack 50ms, release 10s for sleep-before-destroy. Both configurable via `justifier_voice_set_gate_times(voice_id, attack_ms, release_ms)`.

---

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake | Build system | Yes | 4.3.1 | — |
| GCC | C/C++ compilation | Yes | 15.2.1 | Clang (not checked) |
| `faust` CLI | AUD-02 (DSP compilation) | No | — | `apt install faust` |
| miniaudio | AUD-01 (audio I/O) | No (not vendored yet) | 0.11.x | Download single header |
| readerwriterqueue | AUD-03 (SPSC queue) | No (not vendored yet) | 1.0.6 | Download single header |
| ALSA headers | miniaudio on Linux | Unknown | — | `apt install libasound2-dev` |

**Missing dependencies with no fallback:**
- `faust` CLI — required to compile `.dsp` files; `apt install faust` before any build

**Missing dependencies with fallback / trivial install:**
- miniaudio, readerwriterqueue — single-header files downloaded into `native/include/`
- ALSA development headers — `apt install libasound2-dev` if miniaudio ALSA backend fails to link

---

## Sources

### Primary (HIGH confidence)
- [miniaudio official docs](https://miniaud.io/docs/manual/) — `ma_device_config`, callback model, format constants; confirmed current
- [readerwriterqueue GitHub](https://github.com/cameron314/readerwriterqueue) — API, thread-safety guarantees, usage pattern
- [Faust manual — embedding](https://faustdoc.grame.fr/manual/embedding/) — compile-time C++ generation, architecture files, parameter paths
- [Faust manual — architecture files](https://faustdoc.grame.fr/manual/architectures/) — `minimal.cpp` architecture, what it provides
- [Ross Bencina — Real-time audio programming 101](http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing) — canonical audio thread rules (no alloc, no lock, no syscall)
- CONTEXT.md (`01-CONTEXT.md`) — all locked decisions (D-01 through D-09)
- `.planning/research/PITFALLS.md` — 13 pitfalls, phases, mitigations

### Secondary (MEDIUM confidence)
- [cyfaust](https://github.com/shakfu/cyfaust) — reference integration of Faust interpreter + miniaudio in Python; confirms callback model
- [libfaust heap corruption issue #221](https://github.com/grame-cncm/faust/issues/221) — Pitfall 3 origin; confirmed fixed by using `nentry` or compile-time approach
- `.planning/research/ARCHITECTURE.md` — component diagram, thread safety model, voice pool design
- `.planning/research/STACK.md` — library rationale and alternatives

### Tertiary (LOW confidence)
- gcc 15.2.1 behavior with `_Atomic bool` in C11 mode — assumed standard; verify with `-std=c11` flag in CMake

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — miniaudio and readerwriterqueue are well-documented, production-proven; Faust compile-time path is confirmed in official docs
- Architecture: HIGH — patterns derived from first principles + canonical audio programming references; the real-time constraint is non-negotiable and well-understood
- Pitfalls: HIGH — audio thread pitfalls are canonical (Ross Bencina, etc.); Faust-specific pitfalls confirmed in official issue tracker

**Research date:** 2026-04-01
**Valid until:** 2026-07-01 (miniaudio and readerwriterqueue are stable; Faust 2.75.x is current; re-verify if upgrading any component)
