# Architecture Review: justifier @ checkpoint

**Date:** 2026-05-20
**Scope:** all of HEAD (commit 57576a9, pre-big-change)

---

## Observations

The codebase is small and tidily layered at this checkpoint. The native audio engine (`native/`) is the most substantial piece: `audio_engine.cpp` (999 lines), `faust_wrapper.cpp` (325 lines), two headers, and 20 Faust DSP source files that faust_wrapper.cpp `#include`s directly as a single large translation unit. On the Dart side there are three audio files (`audio.dart` is just a barrel re-export), four stub screens, a main shell, and an orphaned theme class. The desktop-ui archived code is excluded from this review.

The audio engine is genuinely deep in the sense that matters most here: the public C interface in `justifier_audio.h` (29 functions) sits above a substantial amount of real machinery — SPSC queue, PENDING/ACTIVE/FADING/RELEASING voice state machine, Faust DSP pool with per-type pre-allocation, 20ms crossfade, send/return effect buses, and an atomic panic flag. Delete `audio_engine.cpp` and all of that complexity vanishes from the caller's view. The FFI bridge (`justifier_audio_bindings.dart`) and Dart wrapper (`audio_engine.dart`) are a different story: together they mirror the C API almost perfectly, adding nothing except type conversion for `WaveformType` and a thin multi-path library loader. They are currently pass-throughs.

The mobile reorientation introduces two new domains that don't exist yet in any form: a JI theory layer (ratios, intervals, temperament math) and a pitch detection layer (microphone capture, YIN/McLeod algorithm, ratio identification). The current codebase has no seams for either. The four stub screens are structurally correct for the navigation shape but contain zero implementation. There is also no Riverpod state at the root package level — all providers were in desktop-ui and are now gone. The app has a ProviderScope wrapper in main.dart but nothing inside it.

One concrete technical debt item is called out in `docs/app-design.md` and confirmed by reading the source: `native/dsp/reverb.dsp` hardcodes `fsmax = 48000`, which is a parameter to `re.zita_rev1_stereo`. On a mobile device running at a different sample rate (e.g. 44100 Hz on some iOS hardware), the reverb's internal delay-line sizes will be computed incorrectly, causing artifacts or crashes. This is the kind of bug that won't show up on desktop Linux (PipeWire defaults to 48 kHz) but will bite immediately on device.

---

## Deepening Candidates

### 1. `AudioEngine` is a pass-through over `JustifierAudioBindings`

**Files:** `lib/audio/audio_engine.dart`, `lib/audio/justifier_audio_bindings.dart`

**Problem:** Delete `AudioEngine` and replace it with direct calls to `JustifierAudioBindings` — the caller loses almost nothing. The only real work `AudioEngine` does is: (a) convert `WaveformType` to `NativeWaveformType` in two methods (`addVoice`, `setWaveform`), (b) negate the return value comparison (`== 0`) for `init`, (c) negate the return value for `isRunning`, and (d) load the native library. Every other method is a one-line delegation. The interface of `AudioEngine` (36 methods, all public) is nearly as wide as its implementation. This is a shallow module by the deletion test.

The interface is also raw at the wrong level for its callers. A drone consists of one or more voices with a root frequency and optionally detuned intervals. The Tuner needs a reference pitch. A lesson needs to start a pre-configured drone and kill it when the lesson ends. None of those concepts exist at the `AudioEngine` interface; callers must assemble them from individual `addVoice`/`setFrequency`/`setDetune` calls, tracking voice IDs themselves.

**Proposed change:** Keep `AudioEngine` as the low-level FFI wrapper (its current role). Above it, introduce a `DroneController` module with a richer interface: `startDrone(root, intervals, waveform)`, `stopDrone()`, `detuneInterval(interval, cents)`. `DroneController` owns voice IDs, handles lifecycle, and is the thing lessons and the drone screen actually talk to. `AudioEngine` becomes an internal implementation detail that `DroneController` depends on, not something the rest of the app calls directly.

**Benefits:**
- *Locality*: voice ID management, "which voice is which interval", and drone teardown all live in one place instead of being reconstructed at each call site.
- *Leverage*: callers get drone-level semantics from a small interface rather than raw voice primitives.
- *Testability*: `DroneController` can be tested (or faked) without a native library — its interface is in terms of musical intent, not C function calls. `AudioEngine` remains untestable in isolation (requires audio hardware) but is now narrow enough that it doesn't need unit tests.

---

### 2. The library loader is an unnamed, untested seam inside `AudioEngine`

**Files:** `lib/audio/audio_engine.dart` (`_loadLibrary`, lines 172–213)

**Problem:** The `_loadLibrary` static method contains the entire platform dispatch logic: macOS framework paths, Linux `.so` search, Windows DLL search, a bare-name fallback, and a hard `throw` on failure. It's not behind any interface — it's `static` and called directly in the constructor. There is one adapter (the real native library). That makes it a hypothetical seam: you can imagine a mock or stub, but the code provides no way to swap it.

For mobile, this method must grow Android and iOS cases. The Android case requires `DynamicLibrary.open('libjustifier_audio.so')` (the NDK bundles it differently from desktop). The iOS case is more complex: iOS prohibits `DynamicLibrary.open` on non-system libraries; you must use `DynamicLibrary.process()` with static linking. Both branches need to be added. They will be added inline here, making an already-long `if/else` ladder longer.

The deeper problem is testability: because `AudioEngine` constructs `JustifierAudioBindings` via `_loadLibrary()` in its constructor, there is no way to instantiate `AudioEngine` in a Dart test without a built native library present. The binding is baked in.

**Proposed change:** Extract `LibraryLoader` as an explicit interface (abstract class in Dart) with a single method `DynamicLibrary load()`. Provide a `PlatformLibraryLoader` that contains the current path logic, extended for Android and iOS. Pass the loader into `AudioEngine`'s constructor (or into a factory). In tests, pass a `FakeLibraryLoader` that returns `DynamicLibrary.process()` or throws a controlled error.

This is now a real seam: `PlatformLibraryLoader` (desktop paths) and a future `MobileLibraryLoader` (or platform-dispatched variants) are two adapters, making the seam real.

**Benefits:**
- *Testability*: Dart-layer tests can construct `AudioEngine` with a fake loader without requiring a built `.so`.
- *Locality*: all platform library-path knowledge lives in one place rather than being a large private static method buried inside a class it has nothing conceptually to do with.
- *Leverage*: the loader is composable — you can log, cache, or retry without touching `AudioEngine`.

---

### 3. `WaveformType` spans two layers without a clear owner

**Files:** `lib/audio/waveform_type.dart`, `lib/audio/justifier_audio_bindings.dart` (`NativeWaveformType`, lines 449–481)

**Problem:** There are two waveform type enumerations in the codebase. `WaveformType` is the Dart-domain enum with semantic properties (`isPitched`, `label`). `NativeWaveformType` is the ffigen-generated copy of the C enum, used only as a translation artifact in `addVoice` and `setWaveform`. They have the same 12 values in the same order. The conversion `NativeWaveformType.fromValue(type.index)` is fragile: it relies on `WaveformType.index` (positional, implicit) matching `NativeWaveformType` values (explicit integers). If someone inserts a new waveform into `WaveformType` at a position other than the end, the mapping silently breaks.

Beyond the brittleness, `NativeWaveformType` has no semantic properties (`isPitched` doesn't exist on it), so all callers that need to know whether a waveform responds to pitch must use `WaveformType`. But `AudioEngine`'s interface accepts `WaveformType` and converts internally, meaning the conversion is hidden from callers who might want to guard a `setFrequency` call. The concept "is this waveform pitched?" is effectively invisible at the audio layer interface.

**Proposed change:** Make the C-to-Dart mapping explicit and centralized. Either: (a) replace the positional `type.index` mapping with an explicit `switch` that names each case (so a misalignment is a compile error or at worst a named mismatch, not a silent wrong value), or (b) collapse `NativeWaveformType` out of the public surface entirely — `AudioEngine` converts internally using an explicit lookup table. Either way, `WaveformType` should be the sole public-facing type; `NativeWaveformType` should be package-private.

The `isPitched` property on `WaveformType` is good and should stay. Consider adding it to the conversion point so callers composing a drone can introspect without importing from two places.

**Benefits:**
- *Safety*: explicit mapping catches enum extension bugs at compile time rather than producing silent audio pitch errors.
- *Locality*: all knowledge about the C↔Dart waveform mapping lives in one `switch`, not spread across two enum definitions and an implicit `index` comparison.

---

### 4. Missing seam: no JI math module

**Files:** (none — this concept does not exist yet)

**Problem:** The app-design.md describes an app whose entire UX is structured around JI ratios: intervals are expressed as ratios (3/2, 5/4, etc.), the tuner identifies ratios, lessons explain ratios, the drone plays intervals defined by ratios relative to a root. None of this arithmetic exists in the codebase. There is no `Ratio` type, no `Interval` type, no function to convert a ratio to Hz given a root, no function to identify the nearest JI ratio from a detected pitch, no temperament comparison logic.

This is a missing module with a clear, stable interface need: `Ratio(int numerator, int denominator)` with `toHz(double rootHz)`, `toCents()`, `label` (e.g. "just perfect fifth"), and `distanceFrom(double hz, double rootHz)` for tuner snap. The math is well-understood and purely functional — it has no dependencies on audio hardware, Flutter, or FFI.

**Proposed change:** Create `lib/theory/ratio.dart` with a `Ratio` value type and associated helpers. This becomes the shared vocabulary type used by `DroneController` (to configure intervals), the tuner display (to show what ratio the user is singing), the lesson model (to reference named intervals), and the JI lattice explorer if that feature survives into v1. Because the math is pure, it is trivially testable.

**Benefits:**
- *Leverage*: callers express musical intent in domain terms ("play a 3/2 above this root") rather than computing Hz from ratios themselves at each call site.
- *Testability*: pure functions, no hardware dependency, straightforward to test — this is where the project's first meaningful unit tests should live.
- *Locality*: the JI arithmetic doesn't spread across screens, lessons, and controllers.

---

### 5. Missing seam: no audio service / lifecycle coordinator

**Files:** `lib/main.dart`, `lib/audio/audio_engine.dart`

**Problem:** `AudioEngine.init()` and `AudioEngine.shutdown()` are the lifecycle entry points for the native audio device. Nobody calls them in the current mobile skeleton. `main.dart` creates a `ProviderScope` but no provider initializes the audio engine. The desktop-ui's providers presumably handled this; they're now gone. The pattern "who initializes the engine, when, and what happens if init fails" is unresolved.

On mobile, this matters more than on desktop. Mobile audio sessions require platform-specific setup (AVAudioSession on iOS, AudioManager on Android). Audio can be interrupted (phone calls, other apps), and the app needs to respond — silencing voices on interruption and resuming on return. The current `AudioEngine` interface has `panic()`/`unpanic()` which is the right primitive, but there is no coordinator that listens to platform audio focus events and calls them.

There is also no Riverpod provider for `AudioEngine` at the root level. Each UI component that needs audio will either instantiate its own engine (wrong — there's one miniaudio device) or pass it down manually (fragile). A singleton audio service, exposed via a Riverpod `Provider`, is the natural solution and creates a real seam: in tests, you provide a fake; in production, you provide the real one.

**Proposed change:** Create `lib/audio/audio_service.dart` — a class that wraps `AudioEngine`, owns the singleton, handles `init`/`shutdown` in `initState`/`dispose` of a root widget (or via a `ref.onDispose` in a provider), and subscribes to platform audio interruption events (via the `audio_session` package or Flutter's `AppLifecycleListener`). Expose it as a Riverpod `Provider<AudioService>`. This is the boundary between Flutter lifecycle and the native engine.

**Benefits:**
- *Locality*: audio lifecycle management (init, shutdown, interruption handling) lives in one file.
- *Testability*: tests provide a `MockAudioService` that doesn't touch native code.
- *Leverage*: all audio features — drone player, tuner, lesson audio — get audio access through one interface that handles interruptions correctly.

---

### 6. The send-bus effect system is desktop-era scope creep

**Files:** `native/src/audio_engine.cpp`, `native/src/justifier_audio.h`, `lib/audio/audio_engine.dart`, `lib/audio/justifier_audio_bindings.dart`

**Problem:** The C API exposes 14 of its 29 functions for effect send/return (reverb, delay, chorus, phaser, flanger, EQ, saturation — each with a per-voice send and a global return level). This is the full studio mixer architecture from the desktop "sound design playground". The app-design.md explicitly defers studio/sound design to a future desktop version. For v1 mobile, the drone player needs at most one effect (a touch of reverb for a pleasing drone sound), and the tuner needs none.

These 14 functions add surface area to every layer: 14 extra entries in `ControlMessageType`, 14 extra `case` branches in `apply_control_message`, 7 singleton effect DSP instances allocated at init time, 14 send-bus static buffers in the audio callback (adding ~240 KB of static memory pressure per the callback's comment), and corresponding Dart wrapper methods. They are not dead code — they're correctly implemented and correct — but they are out-of-scope features surfaced at every interface layer.

This is not a "delete it" candidate — the code is good and will be needed for desktop. It is a "hidden it" candidate: the Dart-side `AudioEngine` / `AudioService` could expose only the subset that v1 mobile actually uses, leaving the rest accessible but not promoted. The C API can stay complete; it's the Dart interface surface that should be narrowed.

**Proposed change:** Introduce a feature-scoped Dart interface: `DroneAudio` (the mobile v1 interface — add/remove voices, set frequency/detune/waveform, reverb send/return only) backed by `AudioEngine`. Desktop or future power-user code gets `AudioEngine` directly. This narrows the interface callers must understand for the common case, without deleting any native capability.

**Benefits:**
- *Leverage*: v1 mobile callers get a 6-method interface instead of 36, with the guarantee that everything behind it is appropriate to their use case.
- *Locality*: the distinction between "mobile drone audio" and "desktop sound design" becomes explicit at the code boundary.

---

### 7. `reverb.dsp` hardcodes `fsmax = 48000`

**Files:** `native/dsp/reverb.dsp`

**Problem:** This is a concrete bug, not a structural issue, but it's architectural in implication. The `re.zita_rev1_stereo` Faust function takes `fsmax` as a parameter that governs internal delay-line buffer allocation. Hardcoding `48000` means the reverb is allocated for 48 kHz. If the device runs at 44100 Hz (common on iOS devices), `justifier_init` passes the actual sample rate to Faust's `instance->init(sample_rate)`, but the reverb's internal buffers were sized for 48000. The resulting behavior is undefined — likely garbled reverb or a buffer overread.

The fix at the Faust level is to make `fsmax` a compile-time parameter using the Faust `ma.SR` constant (which reflects the actual sample rate), or to use the `ma.MAX_SR` constant. At the CMake level, the generated C++ should be verified to use the runtime sample rate correctly. This is a one-line Faust change but needs to be verified against Faust's `zita_rev1_stereo` documentation.

**Benefits:**
- *Correctness*: reverb works on iOS and Android devices that don't run at 48 kHz.
- *Locality*: the sample-rate concern lives in the DSP file that owns it, not as a silent assumption.

---

## Recommendations for the upcoming change

Given the mobile reorientation described in `docs/app-design.md`, the candidates divide into two groups: **must-haves before meaningful mobile development begins** and **valuable-but-deferrable cleanup**.

**Must-haves:**

Candidate **4 (JI math module)** is the highest priority. The app is fundamentally about ratios. Without a `Ratio` type, every screen — tuner display, drone interval selector, lesson content — will independently invent its own Hz-from-ratio arithmetic. This is where fragmentation starts. Build `lib/theory/ratio.dart` before writing any lesson or tuner UI. It's also where the project gets its first real tests, which matters given the current zero-test state.

Candidate **5 (audio service / lifecycle coordinator)** is required for any mobile audio to work correctly. The mobile skeleton currently has no code path that calls `AudioEngine.init()`. Before any audio screen can be built, there must be an answer to "who owns the engine, and what happens on iOS audio session interruption." The Riverpod provider structure here also unblocks all other audio consumers.

Candidate **7 (reverb.dsp fsmax)** is a one-line fix that should be done before mobile testing starts. It's the kind of bug that will be invisible in CI (no audio hardware) and mysterious on device ("reverb sounds wrong, is it the driver?").

**Valuable but deferrable:**

Candidate **1 (DroneController)** is the right next step after the audio service exists, because the drone screen and lesson embed both need it. But you can write a first-pass drone screen against raw `AudioEngine` and extract `DroneController` when the second consumer (the embedded lesson widget) appears. That's when the seam becomes real rather than hypothetical.

Candidate **2 (LibraryLoader seam)** matters most when you start mobile builds: Android and iOS require new library-loading branches, and doing that inside `_loadLibrary` is workable in the short term. The interface extraction is worth doing when you write your first Dart-layer test that needs to mock the audio layer.

Candidate **3 (WaveformType mapping)** is a correctness fix worth making before adding any new waveforms. The implicit `index`-based mapping is a latent bug waiting for the first enum extension.

Candidate **6 (effect bus scope)** is a genuine architectural mismatch but not urgent — the effects code is correct and unused in mobile v1 will simply be dormant. Address it when you're designing the `DroneController` interface; that's the natural moment to decide what the mobile audio contract looks like.
