# Release Review Synthesis: justifier @ checkpoint

**Date:** 2026-05-20
**Scope:** all of HEAD (commit 57576a9)
**Type:** Checkpoint — pre-big-change audit
**Verdict:** continue with adjustments
**Sources:**
- [Architecture pass](2026-05-20-checkpoint-architecture.md)
- [Code review pass](2026-05-20-checkpoint-review.md)

---

## Convergent root causes

Both passes independently identified the same three high-leverage problems. When two passes hit the same disease from different angles, that's the signal.

### 1. The native audio callback has desktop stack assumptions

**Architecture** flagged `reverb.dsp` hardcoding `fsmax = 48000`. **Code review** found the same, plus the `PROCESS_EFFECT` macro allocating 224 KB of automatic variables per callback invocation — which will stack-overflow on Android's 64 KB audio thread.

Both are one-line fixes that apply the pattern already established elsewhere in the code (static locals for send buffers, parameterized sample rate). They share a fix window because both require a Faust regeneration pass.

### 2. The mobile reorientation was structural, not functional

**Architecture** identified two missing seams: no audio lifecycle coordinator (nobody calls `init()`/`shutdown()`) and no library loading for Android/iOS. **Code review** found the same as three concrete findings (`engine-not-wired`, `mobile-library-loading`, `faust-required-at-mobile-build`) — adding the CMake hard-requirement on the Faust compiler, which blocks `flutter build apk/ios` unconditionally.

Same disease: the reorientation commit was a navigation skeleton with no audio wiring. Fixing any one in isolation still leaves the others blocking.

### 3. No JI domain model exists

**Architecture** identified this as the highest-priority deepening candidate: the entire app is about ratios, but there is no `Ratio` type, no interval math, no cents conversion. Every screen and lesson will independently reinvent this arithmetic.

**Code review** didn't flag this (it reviews what exists, not what's missing), which is exactly why the two-pass approach works — architecture catches missing seams, review catches bugs in existing code.

---

## Fix order (waves)

### Wave A — Native correctness (blocks device testing)

1. Make `PROCESS_EFFECT` ret_L/ret_R static (matches existing send-buffer pattern)
2. Fix `reverb.dsp` fsmax to use runtime sample rate
3. Pre-generate Faust C++ files, commit them, and make faust find_program optional in CMake
4. Run `dart format` on the 3 drifted files

**Why first:** Until these are fixed, the app cannot run on a mobile device without stack corruption and broken reverb, and can't even build for mobile without Faust installed.

### Wave B — Audio lifecycle + mobile loading (blocks any audio feature work)

5. Add `Platform.isAndroid` and `Platform.isIOS` branches to `_loadLibrary()`
6. Create a Riverpod provider for AudioEngine (singleton, init on first access, shutdown on dispose)
7. Wire the provider into the existing ProviderScope in main.dart

**Why second:** Every audio-consuming screen (drone, tuner, lessons) needs an initialized engine accessible via a provider. This is the entry condition for all feature work.

### Wave C — JI theory module (blocks lesson/tuner/drone implementation)

8. Create `lib/theory/ratio.dart` with a `Ratio` value type: numerator, denominator, `toHz(rootHz)`, `toCents()`, label, nearest-ratio lookup
9. Write tests for it — this is where the project gets its first unit tests

**Why third:** The drone screen, tuner, and every lesson need ratio math. Building it first prevents N independent implementations and establishes the testing pattern.

### Wave D — Cleanup (non-blocking, batch when convenient)

10. Rewrite README to match the mobile-first education app vision
11. Update AppTheme to support three themes when Settings screen work begins
12. Narrow the Dart-side audio interface for mobile (architecture candidate #6 — defer until DroneController is designed)
13. Extract LibraryLoader seam (architecture candidate #2 — defer until first Dart-layer test needs to mock audio)
14. Make WaveformType→NativeWaveformType mapping explicit (architecture candidate #3 — defer until a new waveform is added)

---

## What both passes agreed is fine

The native audio engine — lock-free SPSC queue, pre-allocated voice pool, Faust DSP management, crossfade state machine, parameter dispatch — is correct, well-designed, and deep in the architectural sense. Neither pass found correctness issues in the audio engine logic itself. It will port to mobile without structural changes once the stack and sample-rate issues are fixed.
