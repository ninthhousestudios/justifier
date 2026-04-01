# Architecture Patterns

**Project:** Justifier
**Researched:** 2026-04-01
**Domain:** Flutter desktop audio app with embedded Faust DSP

---

## Important Context: The Pivot

The design spec (`2026-03-26-justifier-design.md`) was written for a SuperCollider backend — Flutter sends OSC to scsynth running as a separate process. **PROJECT.md has since superseded this**: the audio engine is now Faust via libfaust embedded directly in the Flutter app via Dart FFI. The OSC layer, node IDs, SynthDef concepts, and scsynth process management in the design spec are artifacts of the SC approach and do not apply to the Faust architecture.

The UI design, voice model, wave/group model, preset JSON format, and interaction patterns from the spec remain valid and carry forward.

---

## Recommended Architecture

The system has four distinct layers with strict rules about what can call what:

```
┌─────────────────────────────────────────────────────────────┐
│                        Flutter UI Layer                      │
│  WorkspaceHeader  WaveColumn  VoiceCard  RatioInput          │
│  WaveformSelector  ConnectionStatusBadge  ServerConsole      │
│                         (Main isolate)                       │
└──────────────────────┬──────────────────────────────────────┘
                       │ reads providers
                       │ dispatches Commands
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                   State / Command Layer                      │
│  Riverpod providers  AppState  CommandRegistry               │
│  WorkspaceNotifier   VoiceNotifier  WaveNotifier             │
│                         (Main isolate)                       │
└──────────────────────┬──────────────────────────────────────┘
                       │ sends control messages (lock-free queue)
                       │ receives metering/status (lock-free queue)
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                  Audio Bridge Layer                          │
│  AudioBridge (Dart)  — owns the FFI calls                    │
│  FaustDSPGraph       — manages Faust box/signal graph        │
│  Isolate boundary or direct FFI on audio thread             │
└──────────────────────┬──────────────────────────────────────┘
                       │ C FFI calls
                       ▼
┌─────────────────────────────────────────────────────────────┐
│                    C Wrapper + libfaust                      │
│  justifier_audio.h/c  — thin C wrapper around libfaust C++  │
│  libfaust             — Faust compiler + runtime             │
│  RtAudio / miniaudio  — audio I/O callback                  │
└─────────────────────────────────────────────────────────────┘
```

---

## Component Boundaries

### 1. Flutter UI Layer

**Responsibility:** Render state, capture user input, dispatch named commands. No business logic, no audio calls.

**Communicates with:** State/Command Layer only — via Riverpod `ref.watch` / `ref.read`, never directly to AudioBridge.

**Key constraint:** UI widgets must never hold audio state. The voice card renders what Riverpod tells it; it dispatches a Command when the user moves a slider.

### 2. State / Command Layer

**Responsibility:** Single source of truth for all application state. Routes commands from UI to AudioBridge. Owns preset serialization/deserialization.

**Components:**

| Component | Role |
|-----------|------|
| `WorkspaceNotifier` | Owns wave list, reference Hz, master volume, preset name |
| `WaveNotifier` | Owns one wave: voices list, mute, solo, pan offset, master vol |
| `VoiceNotifier` | Owns one voice: all params, enabled state, sleep state |
| `CommandRegistry` | Map of `String → Command`. All user-triggerable actions registered here |
| `AppState` | Top-level immutable snapshot — what gets serialized to preset JSON |

**Communicates with:** AudioBridge (sends parameter updates after state changes), UI (exposes providers).

**Key constraint:** State changes always flow State → AudioBridge, never AudioBridge → State (except status/health callbacks like "audio thread running").

### 3. Audio Bridge Layer

**Responsibility:** Translate Dart-land voice/parameter model into Faust DSP graph operations. Owns the mapping between voice IDs and Faust node handles. Manages voice lifecycle in the DSP graph.

**Components:**

| Component | Role |
|-----------|------|
| `AudioBridge` | Dart class that owns FFI bindings, exposes typed API to State layer |
| `VoiceHandle` | Opaque reference to a running Faust DSP instance |
| `FaustGraphManager` | Manages the composite signal graph: `voice1 + voice2 + ... + voiceN → output` |
| `ControlMessage` | Immutable value sent from State layer to AudioBridge |

**Communicates with:** C Wrapper (via dart:ffi), State Layer (receives commands, sends status).

**Key constraint:** AudioBridge must never be called synchronously from a slider drag callback — all communication is via a lock-free control queue that the audio thread polls. AudioBridge itself runs on the main isolate but writes to shared memory structures the audio callback reads.

### 4. C Wrapper + libfaust

**Responsibility:** Own the audio thread, manage Faust compiler instances, run the DSP callback.

**Components:**

| Component | Role |
|-----------|------|
| `justifier_audio.h` | Public C API — the only interface Dart sees |
| `faust_voice.cpp` | Wraps a single compiled Faust DSP instance |
| `audio_engine.cpp` | Owns RtAudio/miniaudio setup, callback, lock-free queue |
| `param_queue` | SPSC (single-producer, single-consumer) ring buffer for control messages |

**Communicates with:** Audio hardware (via RtAudio or miniaudio), AudioBridge Dart layer (via FFI).

---

## The Real-Time Audio Thread Problem

This is the hardest architectural constraint. The audio callback runs at ~1–5ms intervals on a dedicated OS thread with elevated priority. **Breaking real-time rules causes audible glitches** (dropouts, crackling). The rules are non-negotiable:

**Forbidden on the audio thread:**
- Memory allocation (`malloc`, `new`, Dart GC)
- Mutex locks (can block)
- File I/O
- System calls
- Any Dart code (Dart runtime is not real-time safe)

**Required pattern: Lock-free control queue**

```
Dart (main isolate)          C audio thread
        │                           │
        │  writes ControlMessage    │
        │ ──────────────────────▶  │ reads ControlMessage
        │  (SPSC ring buffer)       │  (wait-free read)
        │                           │
        │  reads StatusMessage      │
        │ ◀──────────────────────  │ writes StatusMessage
        │  (SPSC ring buffer)       │  (wait-free write)
```

The Dart side writes control messages (parameter changes, voice add/remove, panic) into a pre-allocated ring buffer. The C audio callback polls this queue each buffer cycle and applies changes. All memory for voices must be pre-allocated — voice add means allocating a slot in a fixed pool, not calling malloc in the callback.

**Faust DSP instances:**

Faust compiles a DSP program to a C++ class with a `compute()` method. The audio callback calls `compute()` on each active voice's DSP instance. Voice instances must be pre-allocated (fixed pool). Adding a voice = activating a slot. Removing a voice = marking a slot inactive (fade out, then reclaim).

---

## Faust Integration Layer

### What libfaust provides

libfaust has two modes:

1. **Compile-time:** Generate C++ from a `.dsp` file, compile it into your binary. Zero runtime dependency on the compiler. DSP programs are fixed at build time.
2. **Runtime interpreter:** `libfaust` with the `interp` backend — compile Faust DSP at runtime, run in an interpreter. Slower (~10–20% overhead) but allows dynamic DSP programs.

**Recommendation: Compile-time approach for v1.**

The 7 waveform types are known at build time. Generate C++ from each `.dsp` file as a build step. Link the generated classes directly. No runtime compiler dependency, no interpreter overhead, fully AOT.

The DSP class interface Faust generates:

```cpp
class sine_dsp : public dsp {
public:
    void init(int sample_rate);
    void compute(int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs);
    void setParamValue(const char* path, FAUSTFLOAT value);
    FAUSTFLOAT getParamValue(const char* path);
};
```

Each voice is one `sine_dsp` (or `saw_dsp`, etc.) instance. The C wrapper allocates these from a fixed pool.

### C API surface (what Dart calls via FFI)

```c
// Engine lifecycle
int  justifier_init(int sample_rate, int buffer_size);
void justifier_shutdown(void);

// Voice management (called from Dart, writes to control queue)
int  justifier_voice_add(int dsp_type, float* initial_params, int param_count);
void justifier_voice_remove(int voice_id);
void justifier_voice_set_param(int voice_id, int param_id, float value);
void justifier_voice_set_gain(int voice_id, float gain);  // for mute/sleep/fade

// Global
void justifier_set_master_volume(float volume);
void justifier_panic(void);  // atomic flag, audio thread checks each buffer

// Status (polled by Dart)
int  justifier_is_running(void);
int  justifier_get_xrun_count(void);
```

All `voice_add` / `voice_remove` / `set_param` calls write to the SPSC queue — they do not touch audio thread state directly. This means they are safe to call from the Dart main isolate without locks.

---

## Dart FFI Isolation Strategy

Dart FFI calls that write to the control queue are synchronous but very fast (queue write = a few atomic ops). They can run on the main isolate without blocking the UI.

**No audio isolate is needed.** The C layer owns the audio thread. Dart does not need a separate isolate for audio. The architecture is:

```
Dart main isolate ──FFI──▶ C control queue ──▶ C audio thread (OS thread)
```

A separate Dart isolate would only add complexity without benefit — the audio thread is already a C/OS thread, not a Dart isolate.

The only case where a Dart isolate helps is Faust compile-time DSP generation if using the runtime compiler. Since we use AOT-compiled DSP, this is moot.

---

## State Management: Riverpod + Command Pattern

### Riverpod provider structure

```
appStateProvider (StateNotifier)
  ├── referenceHzProvider
  ├── masterVolumeProvider
  ├── wavesProvider (List<WaveState>)
  │     └── waveProvider(waveId) (per-wave)
  │           ├── voicesProvider(waveId) (List<VoiceState>)
  │           │     └── voiceProvider(voiceId) (per-voice)
  │           ├── muteProvider(waveId)
  │           ├── soloProvider(waveId)
  │           └── panOffsetProvider(waveId)
  └── presetsProvider (List<PresetMeta>)
```

### Command pattern

Every user action goes through a named command:

```dart
abstract class Command {
  String get name;
  void execute(WidgetRef ref);
  // Optional: void undo() — for future undo/redo
}

class SetVoiceAmplitudeCommand implements Command {
  final String voiceId;
  final double value;

  @override
  String get name => 'voice.set_amplitude';

  @override
  void execute(WidgetRef ref) {
    ref.read(voiceProvider(voiceId).notifier).setAmplitude(value);
    // AudioBridge call happens inside the notifier, not here
  }
}
```

`CommandRegistry` maps `String → Command factory`. The keybinding system (future Quill package) calls `registry.execute('voice.set_amplitude', args)` — it never imports widget code.

**AudioBridge is called inside Notifiers, not Commands.** This keeps Commands pure (just state transitions), and the Notifier is responsible for keeping the audio engine in sync.

### State → Audio sync flow

```
UI widget
  │  dispatches Command
  ▼
CommandRegistry.execute(name, args)
  │  calls Command.execute(ref)
  ▼
VoiceNotifier.setAmplitude(value)
  │  1. updates immutable VoiceState
  │  2. notifies Riverpod listeners (UI rebuilds)
  │  3. calls AudioBridge.setVoiceParam(voiceId, PARAM_AMPLITUDE, value)
  ▼
AudioBridge.setVoiceParam(...)
  │  writes ControlMessage to SPSC queue
  ▼
C audio callback (next buffer cycle)
  │  reads ControlMessage
  │  calls dsp_instance->setParamValue(...)
  ▼
Audio output
```

The notifier owns both state update and audio sync. If state update fails (validation), audio sync never happens. This ensures state and audio stay consistent.

---

## Voice / Wave Model

### Dart-side model

```dart
// Immutable value objects (used in Riverpod state)

@freezed
class VoiceState {
  final String id;           // UUID, stable across sessions
  final String waveId;
  final String synthType;    // "sine", "sine_mod", "saw", etc.
  final JiRatio ratio;       // numerator + denominator
  final int octave;
  final double modOffset;    // Hz
  final double amplitude;
  final double pan;
  final double fadeTime;
  final double filterCutoff; // only relevant for filtered_noise
  final double filterRq;     // only relevant for filtered_noise
  final bool enabled;
  final VoiceLifecycle lifecycle; // active | sleeping | destroying
}

@freezed
class WaveState {
  final String id;
  final String name;
  final Color color;
  final double masterVolume;
  final double panOffset;
  final bool mute;
  final bool solo;
  final List<String> voiceIds; // ordered
  final WaveLifecycle lifecycle;
}

@freezed
class AppState {
  final double referenceHz;
  final double masterVolume;
  final List<WaveState> waves;
}
```

### Frequency computation

```dart
double computeHz(double referenceHz, JiRatio ratio, int octave) {
  return referenceHz * (ratio.numerator / ratio.denominator) * pow(2, octave);
}

double computeCents(double referenceHz, double hz) {
  return 1200 * log(hz / referenceHz) / log(2);
}
```

These are pure functions, computed in the UI layer for display. The audio engine receives the computed Hz value, not the ratio representation.

### Mapping to Faust parameters

Each Faust DSP type has a known parameter set. The descriptor JSON (from the design spec) maps UI control names to Faust path strings:

```dart
// SynthDescriptor loaded from assets at startup
class SynthDescriptor {
  final String id;           // "sine_mod"
  final String faustPath;    // "/justifier/sine_mod"
  final List<ControlSpec> controls;
}

class ControlSpec {
  final String name;         // "mod"
  final String faustParam;   // "/justifier/sine_mod/mod"
  final ControlType type;
  final double min, max, defaultValue;
  final bool hidden;
  final String? source;      // "reference" → auto-set from global ref Hz
}
```

When `AudioBridge.setVoiceParam` is called, it looks up the Faust path from the descriptor and passes it to `justifier_voice_set_param(voiceId, paramIndex, value)`.

---

## Preset / Workspace Serialization

The design spec's JSON format is clean and carries forward:

```json
{
  "version": 1,
  "name": "deep drone",
  "referenceHz": 172.8,
  "masterVolume": 0.8,
  "waves": [
    {
      "id": "wave-uuid-1",
      "name": "sine ocean",
      "color": "#7b9",
      "masterVolume": 0.8,
      "panOffset": 0.0,
      "mute": false,
      "solo": false,
      "voices": [
        {
          "id": "voice-uuid-1",
          "synthType": "sine_mod",
          "ratio": [1, 1],
          "octave": 4,
          "modOffset": 0.0,
          "amplitude": 0.12,
          "pan": 0.0,
          "fadeTime": 0.5,
          "filterCutoff": 1000.0,
          "filterRq": 0.5,
          "enabled": true
        }
      ]
    }
  ]
}
```

**Serialization rules:**
- `version` field for forward compatibility — unknown versions show a warning, don't crash
- IDs (UUIDs) are preserved across save/load — voice identity survives round-trips
- `lifecycle` state is NOT serialized — sleeping/destroying voices are not saved (they don't survive reload)
- On load: full workspace rebuild — clear audio engine state, instantiate all voices from JSON, set all parameters

**Storage:** `dart:io` path from `path_provider` package, under app documents directory at `presets/`. No database — files are the persistence layer. Simple `List<PresetMeta>` (name + filename) cached in memory.

---

## Sleep-Before-Destroy in the Audio Layer

The sleep pattern has audio implications:

```
User deletes voice
  │
  ▼
VoiceNotifier.startSleeping(voiceId)
  │  state: lifecycle = sleeping
  │  AudioBridge.setVoiceGain(voiceId, 0.0, fadeMs: fadeTime * 1000)
  │  schedules timer: 10 seconds
  ▼
(10 seconds pass, no undo)
  │
VoiceNotifier.destroy(voiceId)
  │  state: removes voice from WaveState.voiceIds
  │  AudioBridge.removeVoice(voiceId)
  │  (AudioBridge writes REMOVE message to control queue)
  ▼
C audio thread
  │  reads REMOVE message
  │  marks DSP slot as inactive
  │  slot returns to pool
```

The DSP instance slot is not freed until after the fade completes. The audio thread must handle the "gain is zero but slot is still active" state gracefully — the DSP still runs but outputs silence. This is intentional: it matches the design spec's `doneAction: 0` concept from SC (keep node alive, just silent).

---

## Component Build Order (Dependencies)

This ordering minimizes blocked work — each phase only depends on what's already built:

```
Phase 1: C foundation
  justifier_audio.h (C API)
  audio_engine.cpp (RtAudio/miniaudio, audio thread, SPSC queue)
  [No Faust yet — sine wave in C to prove the pipeline]

Phase 2: Faust DSP integration
  Faust .dsp files for 7 voice types
  Build system: faust → C++ → compiled into native lib
  C wrapper: voice pool, param routing
  [Now libfaust is linked and voices produce sound]

Phase 3: Dart FFI bindings
  dart:ffi bindings for justifier_audio.h
  AudioBridge Dart class
  [Dart can now add/remove/control voices]

Phase 4: State layer
  Riverpod providers (AppState, WaveState, VoiceState)
  CommandRegistry
  Notifiers that call AudioBridge
  SynthDescriptor loading from assets
  [State ↔ audio sync works, no UI yet]

Phase 5: UI — core
  WorkspaceHeader
  WaveColumn (basic)
  VoiceCard (basic: ratio, octave, amplitude)
  RatioInput
  [First full end-to-end: UI → state → audio]

Phase 6: UI — full voice control
  VoiceCard descriptor-driven rendering
  WaveformSelector with crossfade
  Full slider set (pan, mod, fade time, filter)
  Mute/solo on waves
  Sleep-before-destroy UX

Phase 7: Persistence
  Preset save/load/switch
  Preset JSON serialization
  path_provider integration

Phase 8: Polish
  ConnectionStatusBadge equivalent (audio engine health)
  Panic button
  Confirmation dialogs
  Keyboard accessibility pass
```

Dependencies that force ordering:
- Phase 3 (FFI) requires Phase 1 (C API to bind to)
- Phase 4 (State) requires Phase 3 (AudioBridge to call)
- Phase 5 (UI) requires Phase 4 (providers to watch)
- Phase 7 (Presets) requires Phase 4 (AppState to serialize)

Phases 1 and 2 can be built and tested independently of Dart — generate audio output and verify with a C test harness before writing any Flutter code.

---

## Thread Safety Summary

| Operation | Thread | Safety Mechanism |
|-----------|--------|-----------------|
| UI render | Main (Dart) | Riverpod immutable state — no mutation in widgets |
| State mutation (Notifier) | Main (Dart) | Riverpod serializes within provider |
| AudioBridge FFI calls | Main (Dart) | Writes to SPSC queue — non-blocking, wait-free |
| DSP compute | Audio (C OS thread) | Reads from SPSC queue — wait-free. Never touches Dart heap |
| Voice pool alloc | Audio (C OS thread) | Pre-allocated fixed pool — no malloc in callback |
| Preset save/load | Main (Dart) | File I/O, synchronous or async, never on audio thread |

The SPSC (single-producer, single-consumer) queue is the only shared data structure between Dart and the audio thread. One producer (Dart main), one consumer (C audio callback). No locks required.

---

## Architecture Risks and Mitigations

### Risk 1: Faust compilation toolchain complexity
**What:** Getting libfaust to compile for Linux/macOS/Windows and link with Flutter's native build system is non-trivial. CMake, dynamic vs static linking, platform-specific toolchains.
**Mitigation:** Use compile-time DSP generation (faust → C++ → compiled in). This reduces the runtime libfaust dependency to zero — only the Faust compiler toolchain is needed at build time. The shipped binary has no libfaust dependency.

### Risk 2: Audio I/O library selection
**What:** RtAudio and miniaudio both work but have different tradeoffs. RtAudio has better ASIO support (Windows); miniaudio is header-only and simpler to integrate.
**Mitigation:** Abstract behind `audio_engine.cpp`. The Dart layer never knows which library is used. Decision can be deferred or made per-platform.
**Recommendation:** miniaudio for v1 — single-header, permissive license (public domain), cross-platform, well-maintained. Switch to RtAudio only if ASIO/low-latency Windows support becomes a requirement.

### Risk 3: Flutter FFI and native library bundling
**What:** Flutter's CMake-based native build system needs to find and link the audio library. Asset bundling of the native `.so`/`.dylib`/`.dll` must work on all three platforms.
**Mitigation:** Use `flutter_rust_bridge` patterns for native plugin structure even though we're not using Rust — the plugin architecture is the same. Build the native lib as a Flutter plugin package (`justifier_audio_plugin`) with platform-specific CMake.

### Risk 4: Parameter update rate causing queue overflow
**What:** Fast slider drag could produce 200+ messages/second. SPSC queue must not overflow.
**Mitigation:** Two-layer throttle: (1) Dart-side: coalesce rapid updates, only write to queue ~60/sec per parameter. (2) Per-parameter "latest value wins" slot in shared memory — the audio thread reads the latest value, not a queue of intermediate values. For continuously-polled params (all voice params), latest-value wins is correct and eliminates the overflow concern entirely.

---

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Component boundaries | HIGH | Drawn from first principles, standard audio app patterns |
| Dart FFI approach | HIGH | dart:ffi is mature, this is standard Flutter native integration |
| Faust compile-time approach | MEDIUM | Correct pattern, but toolchain integration details need validation |
| Lock-free queue | HIGH | SPSC ring buffer is industry standard for audio control |
| miniaudio recommendation | MEDIUM | Well-regarded library, but cross-platform behavior needs testing |
| Riverpod + Command pattern | HIGH | Standard Flutter architecture, Command pattern is straightforward |

---

## Sources

- Faust documentation: https://faust.grame.fr/doc/manual/ (architecture, C++ output, embedding)
- miniaudio: https://miniaud.io (single-header audio I/O)
- dart:ffi documentation: https://dart.dev/guides/libraries/c-interop
- Flutter native plugins: https://docs.flutter.dev/platform-integration/android/c-interop
- Design spec: `docs/superpowers/specs/2026-03-26-justifier-design.md`
- Architecture options: `docs/superpowers/specs/2026-03-31-architecture-options.md`
- Project definition: `.planning/PROJECT.md`
- Audio programming patterns: Real-Time Audio Programming 101 (Ross Bencina) — lock-free audio thread rules
