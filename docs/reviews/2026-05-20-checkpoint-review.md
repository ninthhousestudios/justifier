# Code Review: justifier @ checkpoint

**Date:** 2026-05-20
**Scope:** all of HEAD (commit 57576a9, pre-mobile-reorientation)
**Verdict:** continue with adjustments

## Verification

- Build: NOT_RUN (requires Faust compiler + CMake; pre-built artifacts present)
- Tests: NOT_RUN (C test binary requires audio device; no Flutter tests exist)
- Lint: PASS — `flutter analyze` reports no issues
- Format: DRIFT — 3 files out of format: `lib/audio/audio_engine.dart`, `lib/audio/justifier_audio_bindings.dart`, `lib/main.dart`

---

## Design

The native audio engine is the strongest part of this codebase. The SPSC queue + pre-allocated pool pattern is correct and well-executed: UI thread enqueues, audio thread dequeues, no heap allocation in the callback, no locks. The VOICE_PENDING state to prevent a TOCTOU race on voice slot reservation is a nice touch. The Faust layer — compiling DSPs to C++ at build time, including them into a single TU, using MapUI for suffix-matched parameter lookup — is solid and portable. The layering from `waveform_type.dart` → `justifier_audio_bindings.dart` → `audio_engine.dart` is clean. Nothing in the audio engine architecture will resist the mobile reorientation.

The Flutter side is a skeleton, which is fine and expected at this checkpoint. The concern is that the skeleton has two concrete problems that will surface immediately when real work begins: the `AudioEngine` is never instantiated anywhere in the root package (no provider, no init call, no lifecycle management), and the `_loadLibrary()` function covers macOS, Linux, and Windows but has no branch for Android or iOS — calling it on either mobile platform will either crash at `DynamicLibrary.open()` (iOS, where dynamic loading of arbitrary paths is not permitted) or silently fall through to a `StateError` (Android, where the `.so` is bundled differently). These are not hypothetical: they will be the first failures when someone does `flutter run -d android`.

The feature surface encoded in the API is considerably wider than what the education app needs. Five effect buses beyond reverb (delay, chorus, phaser, flanger, EQ, saturation) are exposed through the full Dart layer and are all initialized unconditionally in `justifier_init`. These were built for the studio/sound design use case that is now explicitly deferred to desktop. They are not harmful to keep, but they cost initialization time, consume 7 singleton Faust DSPs, and add 224 KB of stack pressure in the audio callback (see finding `process-effect-stack`). The mobile drone/tuner use case needs sine, a couple of waveforms, and reverb. Deciding now whether to strip the unused effects or just leave them dormant will save cleanup work later.

---

## Findings

```yaml
- id: mobile-library-loading
  severity: critical
  category: correctness
  title: _loadLibrary() has no Android or iOS branch — will crash on first mobile run
  location: lib/audio/audio_engine.dart:172-213
  evidence: |
    Platform.isMacOS -> candidate paths
    Platform.isLinux, Platform.isWindows -> candidates list
    No Platform.isAndroid, no Platform.isIOS branch.
    Fallback: DynamicLibrary.open(bareName) then StateError.
  why: |
    On iOS, DynamicLibrary.open() with an arbitrary path is rejected by the OS —
    static linking or framework embedding is required, accessed via
    DynamicLibrary.executable() or DynamicLibrary.process(). On Android, the .so
    is bundled by Gradle and must be loaded via DynamicLibrary.open("libjustifier_audio.so")
    (bare name, no path), which the current fallback would attempt but only after
    an unnecessary path-based search that will always fail. The iOS path will throw
    or silently use a wrong library; the Android path may accidentally work in
    debug but will fail under release with minification.
  recommendation: |
    Add Platform.isAndroid and Platform.isIOS branches before the existing
    platform checks. Android: return DynamicLibrary.open('libjustifier_audio.so').
    iOS: return DynamicLibrary.process() (static linking is assumed; the CMakeLists.txt
    will need a corresponding STATIC build type for iOS). This is the first thing
    to wire before any other mobile work.
  confidence: high
```

The iOS and Android build plumbing (NDK/gradle CMake wiring, static vs. shared, AVAudioSession) is called out in `app-design.md` as known future work, but the Dart-side stub must be in place before that work can proceed.

---

```yaml
- id: process-effect-stack
  severity: high
  category: performance
  title: PROCESS_EFFECT macro allocates 224 KB on the audio callback stack at every call
  location: native/src/audio_engine.cpp:564-583
  evidence: |
    #define PROCESS_EFFECT(...) \
        if (active && ...) { \
            float ret_L[JUSTIFIER_MAX_BUFFER_SIZE]; \  // 4096 * 4 = 16 KB
            float ret_R[JUSTIFIER_MAX_BUFFER_SIZE]; \  // 16 KB
            ...
        }
    PROCESS_EFFECT(...) // x7 = 7 * 32 KB = 224 KB stack per callback invocation
  why: |
    The comment at line 395 explicitly notes "static to avoid ~240KB stack
    pressure" for the send buffers — but then allocates an equivalent amount
    as automatic variables inside PROCESS_EFFECT. Mobile audio threads typically
    run with 64 KB or 128 KB of stack (Android's audio thread stack is 64 KB by
    default). 224 KB of stack locals from a single callback will overflow on
    Android, producing a silent crash or stack corruption. The code is correct on
    desktop (generous stack sizes) but will fail on mobile.
  recommendation: |
    Make ret_L and ret_R static inside the macro, the same way the send buffers
    are static. Since the audio callback is single-threaded, static is safe and
    matches the existing pattern. Alternatively, promote them to the AudioEngine
    struct to make the intent explicit.
  confidence: high
```

This is the kind of latent bug that works perfectly on Linux, passes the test harness, and then produces a hard-to-diagnose crash on the first Android device.

---

```yaml
- id: engine-not-wired
  severity: high
  category: design
  title: AudioEngine is never instantiated or initialized in the root package
  location: lib/main.dart, lib/screens/*.dart
  evidence: |
    main.dart: no AudioEngine(), no engine.init(), no ProviderScope provider
    that exposes it. flutter_riverpod is imported but no providers are defined
    in lib/. The audio FFI layer is complete code with no caller in the app.
  why: |
    The mobile reorientation commit wired up the navigation shell but did not
    carry over any audio lifecycle management. When DroneScreen or TunerScreen
    are built out, each will need an initialized audio engine accessible via
    a provider. Without a provider holding the engine singleton, the first
    implementation attempt will either create multiple engine instances
    (double-init the C layer, which memsets g_engine and leaks the previous
    queue), or re-discover the architecture question of where engine lifecycle
    lives. Better to stub this now than to have it discovered mid-feature.
  recommendation: |
    Add a single Riverpod provider (StateProvider or Provider with manual
    dispose) in lib/audio/ that creates and inits AudioEngine on first access
    and shuts it down on dispose. Wire it into the ProviderScope already in
    main.dart. The screens can then ref it. This is three lines of provider
    code and prevents a class of bugs when drone and tuner screens go live.
  confidence: high
```

---

```yaml
- id: faust-required-at-mobile-build
  severity: high
  category: design
  title: CMakeLists.txt hard-requires the Faust compiler — blocks all mobile builds
  location: native/CMakeLists.txt:9-18
  evidence: |
    find_program(FAUST_EXECUTABLE faust)
    if(NOT FAUST_EXECUTABLE)
        message(FATAL_ERROR "Faust compiler not found...")
    endif()
    add_custom_command(... COMMAND ${FAUST_EXECUTABLE} ...)
  why: |
    Android NDK builds and iOS Xcode builds invoke CMake in restricted
    environments where the Faust compiler is not available. The FATAL_ERROR
    means any `flutter build apk` or `flutter build ios` will fail at CMake
    configure time. app-design.md correctly identifies pre-generating the
    Faust C++ files as a requirement; this finding is flagging that it is
    blocking work, not a future nice-to-have. The generated _dsp.cpp files
    are also not currently committed to the repo (they are in the build tree).
  recommendation: |
    Pre-generate the Faust C++ files (they are deterministic given the .dsp
    source) and commit them to native/src/generated/ or similar. Then guard
    the faust find_program/add_custom_command with an option:
    option(JUSTIFIER_REGEN_FAUST "Re-run Faust compiler" OFF). When OFF
    (the default for mobile), use the committed generated files directly.
    This unblocks mobile builds without removing the ability to regenerate
    when DSP files change.
  confidence: high
```

---

```yaml
- id: reverb-sample-rate-hardcode
  severity: medium
  category: correctness
  title: reverb.dsp hardcodes fsmax=48000 — produces wrong reverb on non-48kHz devices
  location: native/dsp/reverb.dsp:8
  evidence: |
    fsmax = 48000;
    process = re.zita_rev1_stereo(rdel, f1, f2, t60dc, t60m, fsmax);
  why: |
    zita_rev1_stereo uses fsmax to size internal delay lines. If the device
    runs at 44100 Hz (common on iOS) or 96000 Hz (some Android devices),
    the reverb tail lengths will be wrong — either clipped (44.1kHz) or
    producing metallic artifacts (96kHz). This is a known issue called out
    in app-design.md and is worth fixing before mobile rather than after,
    because it requires a Faust DSP change and a rebuild.
  recommendation: |
    Replace the hardcoded literal with the Faust fSamplingFreq variable:
    fsmax = fSamplingFreq;. This is the correct Faust idiom for sample-rate-
    aware reverb sizing. Requires re-running faust on reverb.dsp and committing
    the regenerated reverb_dsp.cpp.
  confidence: high
```

---

```yaml
- id: app-theme-dead-code
  severity: low
  category: slop
  title: AppTheme class is fully dead — imported but its only consumer is desktop-ui
  location: lib/theme/app_theme.dart
  evidence: |
    lib/main.dart imports 'theme/app_theme.dart' and calls AppTheme.justifier().
    desktop-ui/ (archived) also used it. The root main.dart actually does call
    AppTheme.justifier() — sutra's dead-code report is a false positive here.
    However, AppTheme defines only one theme and the app-design calls for three
    (light, dark, justifier). The class structure doesn't support theme switching.
  why: |
    Not dead in the strict sense — main.dart calls it. But it will need to be
    refactored immediately to support the three-theme requirement from app-design.md,
    and theme selection will need to be a runtime state (Riverpod provider or
    inherited widget). The current static-method structure (`AppTheme.justifier()`)
    cannot accommodate this without a rewrite.
  recommendation: |
    Leave the file as-is for now. When implementing Settings screen, replace the
    static method with a ThemeMode-aware provider and implement the three themes.
    No cleanup needed before starting the mobile work.
  confidence: high
```

---

```yaml
- id: readme-stale-platform
  severity: low
  category: slop
  title: README describes a desktop application; project is now a mobile-first education app
  location: README.md:5
  evidence: |
    "A desktop application for exploring just intonation through direct manipulation of sound."
    Features section lists FM synthesis, 32-voice polyphony, wave grouping,
    voice cards, ADSR sliders — none of which are in the mobile app scope.
  why: |
    Minor, but if this repo ever gets a collaborator or external reader,
    the README will be actively misleading about the app's purpose and audience.
  recommendation: |
    Rewrite the opening paragraph and features list to match app-design.md's
    vision. The technical architecture section can stay; it describes the engine
    accurately.
  confidence: high
```

---

## Synthesis

There are two root causes, not six findings.

**Root cause 1: the mobile reorientation commit (57576a9) was a navigation skeleton, not a functional port.** It correctly established the tab structure and placeholder screens, but did not carry forward audio engine lifecycle, did not add mobile platform branches to library loading, and did not address the build-time Faust dependency that blocks mobile compilation. Findings `mobile-library-loading`, `engine-not-wired`, and `faust-required-at-mobile-build` are all symptoms of this single commit being a structure-only change. They need to be addressed together at the start of the mobile build effort — not independently, because each one will surface the moment someone tries to run the app on a real device.

**Root cause 2: the audio callback was written for desktop stack sizes.** The `PROCESS_EFFECT` stack allocation (`process-effect-stack`) and the `reverb.dsp` sample rate hardcode (`reverb-sample-rate-hardcode`) are both desktop assumptions that compile and run correctly on Linux but will produce crashes or audible artifacts on mobile. They are independent of each other but share the same fix pattern: apply the existing pattern (static locals, parameterized sample rate) and regenerate.

**Fix order:**
1. Fix `process-effect-stack` and `reverb-sample-rate-hardcode` in the same CMake/DSP pass (they both require regenerating Faust output).
2. Pre-commit generated DSP files and add the opt-in regen flag (`faust-required-at-mobile-build`).
3. Add Android and iOS library loading branches (`mobile-library-loading`) and stub the audio engine provider (`engine-not-wired`) — these are the entry conditions for any real screen work.
4. `readme-stale-platform` and `app-theme-dead-code` can wait until Settings screen work begins.

The audio engine itself — the lock-free queue, the voice pool, the crossfade state machine, the parameter dispatch — is correct and needs no changes before the mobile port.

---

## Slop list

1. `README.md:6` — "A desktop application" — stale platform claim
2. `lib/theme/app_theme.dart` — entire file: structurally wired but will be rewritten for three-theme support; the static-method API is already obsolete against the design
3. `android/app/build.gradle.kts:35-37` — TODO comment for signing config (`// TODO: Add your own signing config`)
4. `android/app/build.gradle.kts:23` — TODO comment for application ID confirmation
5. `lib/audio/audio_engine.dart`, `lib/audio/justifier_audio_bindings.dart`, `lib/main.dart` — format drift (run `dart format`)
6. `native/dsp/reverb.dsp:8` — `fsmax = 48000` hardcode; stale relative to the mobile target
