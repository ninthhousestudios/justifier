# Project Research Summary

**Project:** Justifier — Just Intonation Sound Design Tool
**Domain:** Flutter desktop audio application with embedded real-time DSP engine
**Researched:** 2026-04-01
**Confidence:** MEDIUM

---

## Executive Summary

Justifier is a desktop drone/soundscape construction tool: the user layers voices tuned to JI ratios from a reference pitch, hears them in real time, and explores harmonic relationships through direct manipulation. This positions it as a synthesis tool, not a tuning plugin or scale editor — the closest analogues (Entonal Studio, Wilsonic) are JI vocabulary tools, not JI audio tools. The research confirms the technical approach is sound: Flutter + Dart FFI + libfaust + miniaudio is a viable, proven pattern for embedding a DSP engine in a desktop application.

The dominant engineering challenge is the real-time audio thread boundary. The audio callback runs on a C/OS thread at 1–5ms intervals; any blocking, allocation, or Dart call in the callback causes audible glitches. The architecture must be designed around this constraint from day one — a lock-free SPSC queue between Dart and the C audio thread is not optional. All four research dimensions agree on this: ARCHITECTURE, PITFALLS, and STACK all converge on the same pattern (SPSC queue, pre-allocated voice pool, no Dart code on audio thread).

The key unresolved tension is **how libfaust should be integrated**: the STACK researcher recommends the libfaust interpreter backend (runtime DSP compilation, ~3–10x slower than native but adequate for <30 voices); the ARCHITECTURE researcher recommends compile-time Faust (generate C++ from `.dsp` files at build time, link statically, ship no libfaust runtime at all). This is a fundamental architectural choice that affects binary size, build complexity, startup behavior, and future extensibility. It must be resolved before Phase 2. See the tension analysis below.

---

## Key Findings

### Recommended Stack

The stack is four layers: Flutter/Dart UI → Dart FFI → C wrapper (`faust_bridge`) → libfaust + miniaudio. The Dart side uses `dart:ffi` + `package:ffi` for C interop, `ffigen` to generate bindings from a hand-written C header, and Riverpod for state management. The C side uses miniaudio (header-only, MIT, cross-platform audio I/O) and libfaust for DSP. The C wrapper is ~200–400 LOC and owns the audio thread, voice pool, and SPSC parameter queue.

**Core technologies:**

- **libfaust** (GRAME): DSP compiler/runtime — only proven embeddable Faust integration; see tension below on interpreter vs. compile-time
- **miniaudio** (0.11.x, header-only, public domain): cross-platform audio I/O — single header, handles ALSA/CoreAudio/WASAPI automatically, no license complications, production-proven
- **dart:ffi + package:ffi**: C interop from Dart — standard Flutter pattern, no alternatives appropriate for C library binding
- **ffigen** (dev dependency): generate Dart bindings from `faust_bridge.h` — eliminates hand-written binding boilerplate
- **flutter_riverpod** (2.6.x): state management — already decided in design spec
- **CMake + Flutter plugin_ffi template**: native build system — standard Flutter native integration path

**What to avoid:** PortAudio (LGPL, complex build), flutter_soloud (designed for sample playback, wrong abstraction), LLVM backend for libfaust (50–200MB, complex cross-compile, no perf benefit for <30 voices), flutter_rust_bridge (adds Rust layer explicitly rejected in architecture exploration).

### Expected Features

The design spec is already well-calibrated. Research confirms the feature set is correctly scoped and the priority ordering matches what the domain requires.

**Must have (table stakes):**
- Ratio input (numerator/denominator) — JI is defined by ratios; any abstraction loses the point
- Derived Hz + cents display — trust and verification; users need to see the math
- Multiple simultaneous voices — single tone is not exploration
- Per-voice amplitude, enable/disable, waveform selection
- Preset save/load — losing session state kills the tool
- Panic button — non-negotiable for any audio tool

**Should have (differentiators):**
- Sleep-before-destroy — covers the real deletion use case without full undo stack complexity
- Wave grouping with master controls — compositional metaphor (sine ocean, noise texture) not found in comparable tools
- Descriptor-driven voice UI — new waveform type requires zero Dart UI changes
- Waveform crossfade on switch — quality-of-life detail most tools skip
- Beat offset (mod) per voice — micro-detuning for drone beating is central to the use case
- Octave control per voice — placing a ratio in different octaves is core JI exploration

**Defer to v2+:**
- JI lattice visualization (Entonal Studio already owns this; don't compete before core loop is solid)
- Effects routing / filter chains
- MIDI input (different paradigm, different user)
- Undo/redo stack (sleep-before-destroy is the right solution for the actual problem)
- Real-time FFT / spectral analysis
- ServerConsole — differentiating but the app works without it; descope if time pressure hits

### Architecture Approach

Four strict layers: Flutter UI (reads Riverpod state, dispatches named Commands) → State/Command Layer (Riverpod notifiers, single authoritative state, preset serialization) → AudioBridge (Dart FFI calls, voice handle tracking, SPSC queue writes) → C wrapper + libfaust + miniaudio (owns audio thread, DSP instances, voice pool). State flows one direction: UI dispatches Command → Notifier updates state + calls AudioBridge → AudioBridge writes to SPSC queue → C audio callback reads queue next buffer cycle. AudioBridge is never called directly from widget callbacks; Commands are pure (just state transitions); audio sync happens inside Notifiers.

**Major components:**

1. **C audio engine** (`justifier_audio.h/cpp`) — owns audio thread, miniaudio device, SPSC queue, pre-allocated DSP voice pool; the only code that touches audio hardware
2. **AudioBridge** (Dart) — owns FFI bindings, maps voice IDs to C handles, translates Dart state changes to C control messages; runs on main isolate, writes to SPSC queue
3. **State layer** (Riverpod Notifiers + CommandRegistry) — single source of truth for all app state; routes Commands from UI to AudioBridge; owns preset serialization
4. **Flutter UI layer** — renders state, captures input, dispatches named Commands; no business logic, no direct audio calls
5. **Faust DSP programs** (7 `.dsp` files) — one per waveform type; compiled to voice instances in the audio engine

### Critical Pitfalls

1. **Audio thread blocking** (Pitfall 1) — any allocation, mutex, syscall, or Dart FFI call in the audio callback causes audible glitches. Prevention is architectural: SPSC queue for all parameter updates, pre-allocated voice pool, no Dart code on the audio path. This is not a bug fix — design it in before the first audio plays.

2. **Dart FFI callbacks from non-Dart threads** (Pitfall 2) — calling back into Dart from the audio thread crashes in release mode. Data flow is strictly one-way at audio time: Dart writes to C queue; C reads it. Status reporting (xrun count, engine health) is polled by Dart on a timer, never pushed from the audio thread.

3. **deleteDSPFactory heap corruption** (Pitfall 3) — confirmed libfaust bug (issue #221) where destructor corrupts heap on certain compiler/LLVM combinations. Affects voice deletion, which is a core feature (sleep-before-destroy). Prevention: pin Faust + LLVM versions; wrap destructor calls; test deletion path on all three platforms before shipping.

4. **DSP hot-swap crash** (Pitfall 4) — swapping a live voice's waveform type naively causes use-after-free if the audio callback fires during the swap. Prevention: atomic pointer swap via SPSC queue; UI thread prepares the new DSP instance fully before posting a SWAP message; old instance freed only after at least one callback cycle confirms the swap.

5. **Panic button queue delay** (Pitfall 13) — posting panic as a normal queue message may be delayed behind a backlog. Prevention: dedicated atomic `is_silent` flag read at the top of the audio callback, before any queue processing. One buffer cycle to silence, maximum.

---

## Key Tension: Runtime Interpreter vs. Compile-Time Faust

This is the one point where the STACK and ARCHITECTURE researchers give fundamentally different recommendations. It must be resolved before Phase 2.

### STACK researcher recommendation: libfaust interpreter backend

Compile Faust `.dsp` source at runtime using the interpreter backend (`interpreter-dsp.h`). Ship `libfaust.so/.dylib/.dll` with the app. The interpreter backend runs Faust bytecode in a Faust VM — no LLVM at runtime, simpler than the LLVM backend, but still requires libfaust at runtime.

**Arguments for:**
- One build path for all 7 waveform types: add a `.dsp` file, it works at runtime without rebuilding the native layer
- If future waveform types are added post-release, users get them without a new binary
- Factory caching (compile once per waveform type at startup) keeps voice creation fast
- C API surface is clean and well-documented (`createInterpreterDSPFactoryFromString`, etc.)

**Arguments against:**
- Ships libfaust at runtime (~20–50MB depending on build): larger binary, bundling/signing complexity (especially macOS notarization)
- Interpreter is 3–10x slower than native (acceptable for <30 voices, but a ceiling exists)
- Pitfall 7 applies: factory compilation at startup must happen off the UI thread
- Pitfall 3 (deleteDSPFactory corruption) is a confirmed bug in this path

### ARCHITECTURE researcher recommendation: compile-time Faust (AOT)

Run `faust -lang cpp` on each `.dsp` file as a build step, generating C++ classes (`sine_dsp.cpp`, etc.). Compile those directly into the native library. No libfaust at runtime — only the Faust compiler toolchain is needed at build time.

**Arguments for:**
- Zero runtime libfaust dependency: smaller binary, no bundling/notarization issues for libfaust
- Native-speed DSP: the generated C++ is as fast as hand-written DSP
- Pitfall 3 (deleteDSPFactory) does not apply — no factory creation/deletion at runtime
- Pitfall 7 (compilation blocking UI) does not apply — there is no runtime compilation
- The 7 waveform types are known at build time; this is not a constraint for v1

**Arguments against:**
- Adding a new waveform type requires a rebuild + redeploy (not a concern for v1 desktop)
- Faust compiler toolchain must be present on the build machine (available via brew/apt/winget)
- Build system complexity: CMake must invoke `faust` as a code-generation step before compiling C++

### Synthesis / Recommendation

**The compile-time approach is the stronger choice for v1, with a clear migration path if runtime compilation is later needed.**

The rationale: the 7 waveform types are fixed in the v1 spec. AOT compilation eliminates two of the three critical pitfalls specific to libfaust (Pitfall 3 and Pitfall 7), simplifies the binary bundling story, and produces faster DSP. The runtime interpreter's only material advantage — "add a waveform without a rebuild" — is not a v1 requirement and is arguably not a good fit for a desktop app that will have proper releases anyway.

**The build complexity concern is real but manageable:** CMake's `add_custom_command` can invoke `faust -lang cpp -a minimal.cpp` as a pre-build step. The Faust compiler is a ~10MB install with no deep dependencies. A CI runner that has Flutter also has Faust via `apt install faust` or `brew install faust`.

**The migration path is clean:** if v2 needs runtime DSP (user-defined synthesis, plugin DSP loading), the C wrapper interface (`justifier_audio.h`) doesn't change — only the internal implementation switches from pre-compiled DSP classes to interpreter-created instances. The Dart layer never knows which approach is in use.

**Decision needed from Josh:** confirm compile-time approach before Phase 2 begins. This affects how Phase 2's build system is structured.

---

## Implications for Roadmap

Suggested phase structure based on component dependencies and pitfall sequencing:

### Phase 1: C Audio Foundation

**Rationale:** The audio thread constraint is the hardest architectural constraint. Build the C layer first, prove it works in isolation with a C test harness, before writing any Flutter code. This surfaces audio threading and SPSC queue issues before Dart is in the picture.

**Delivers:** Working audio output (sine wave in C), audio thread + SPSC queue, `justifier_audio.h` C API, miniaudio integration, panic atomic flag.

**Avoids:** Pitfall 1 (audio thread blocking), Pitfall 2 (Dart callbacks from audio thread), Pitfall 13 (panic queue delay) — all resolved by design before Dart is involved.

**Note:** No Faust yet. Prove the pipeline with a hardcoded sine oscillator in C. This lets you validate the audio stack independently of the libfaust integration.

### Phase 2: Faust DSP Integration

**Rationale:** Once the audio pipeline works in C, integrate Faust. This is where the interpreter vs. compile-time decision lands. Using compile-time: add `.dsp` files, generate C++ via build step, implement voice pool in C wrapper.

**Delivers:** All 7 waveform types producing sound, voice creation/destruction, parameter setting, sleep-before-destroy audio mechanics (gain fade, slot reclaim).

**Addresses:** Pitfall 3 (deleteDSPFactory) — if using compile-time, this is moot; if using interpreter, test deletion on all platforms here.

**Avoids:** Pitfall 4 (hot-swap crash) — implement atomic DSP swap from the start.

**Research flag:** Build system integration (CMake invoking Faust compiler, or CMake linking prebuilt libfaust) needs a spike before planning. The cross-platform details are non-trivial.

### Phase 3: Dart FFI Bindings + AudioBridge

**Rationale:** With a working C API, generate Dart bindings and build the AudioBridge class. This is the integration layer — nothing in Dart touches audio until this phase.

**Delivers:** `ffigen`-generated bindings for `justifier_audio.h`, `AudioBridge` Dart class, `VoiceHandle` type, all control message types, engine lifecycle (init/shutdown).

**Avoids:** Pitfall 5 (FFI memory leaks) — `NativeFinalizer` on all handle types; Pitfall 11 (struct ABI differences) — use only primitive types at FFI boundary; Pitfall 12 (malloc/free mismatch) — no cross-boundary ownership.

**Research flag:** Standard pattern, well-documented. No deep research needed.

### Phase 4: State Layer

**Rationale:** AudioBridge exists but nothing drives it. Build Riverpod providers, Notifiers, and CommandRegistry. This is the wiring layer — state changes drive AudioBridge calls. Preset serialization also lives here.

**Delivers:** Full AppState/WaveState/VoiceState model, WorkspaceNotifier/WaveNotifier/VoiceNotifier, CommandRegistry, SynthDescriptor loading from assets, preset JSON save/load.

**Avoids:** Pitfall 9 (UI/audio state drift) — single authoritative state store; every audio sync happens inside Notifiers, not Commands.

**Research flag:** Standard Riverpod patterns. No deep research needed.

### Phase 5: Core UI — End to End

**Rationale:** First full vertical slice. Build the minimum UI that exercises the entire stack: add a voice, hear it, change its ratio, hear the change.

**Delivers:** WorkspaceHeader, WaveColumn (basic), VoiceCard (ratio + octave + amplitude only), RatioInput, Hz/cents display, voice enable/disable, panic button in UI.

**Avoids:** Pitfall 10 (zipper noise) — validate `si.smooth` in Faust DSP programs here; any ratio change must be click-free before proceeding.

### Phase 6: Full Voice Control

**Rationale:** Expand VoiceCard to the full descriptor-driven model. Implement waveform switching with crossfade. Add all per-voice controls and wave-level controls.

**Delivers:** Descriptor-driven VoiceCard rendering, WaveformSelector with crossfade (DSP hot-swap), full slider set (pan, mod offset, fade time, filter params), mute/solo, sleep-before-destroy UX, wave master volume + pan offset.

**Avoids:** Pitfall 4 (hot-swap crash) — waveform crossfade depends on correct atomic DSP swap from Phase 2.

### Phase 7: Persistence

**Rationale:** Preset save/load is required before the tool is usable for sustained sessions. Kept separate from UI to keep Phase 5 focused on the audio loop.

**Delivers:** Preset JSON serialization (full AppState → file, file → AppState), path_provider integration, preset list UI (save/load/switch/delete).

### Phase 8: Platform Integration + Polish

**Rationale:** Cross-platform behavior needs dedicated validation, not assumed.

**Delivers:** macOS notarization pipeline (code-sign bundled dylib/so), Windows DLL bundling validation, PipeWire quantum configuration on Linux, ConnectionStatusBadge equivalent (audio engine health polling), CI smoke test (build → run → verify audio), keyboard accessibility pass.

**Avoids:** Pitfall 6 (PipeWire quantum interference), Pitfall 8 (library bundling failure).

**Research flag:** macOS notarization with bundled dylib is a known Flutter desktop pain point — needs platform-specific research or spike before this phase.

### Phase Ordering Rationale

- Phases 1–2 build and verify the entire C audio stack independently of Flutter. This is deliberate — audio thread issues discovered in a C test harness are much easier to debug than the same issues inside Flutter.
- Phase 3 (FFI) requires Phase 1 (the C API must exist to bind to).
- Phase 4 (State) requires Phase 3 (AudioBridge must exist to call).
- Phase 5 (UI) requires Phase 4 (providers must exist to watch).
- Phase 7 (Presets) requires Phase 4 (AppState must exist to serialize).
- Phase 6 (Full voice control) requires Phase 5 (basic VoiceCard) and Phase 2 (DSP hot-swap).
- Phase 8 can be parallelized with Phase 7; both depend on Phase 6 being complete.

### Research Flags

Phases needing deeper research or a spike before planning:

- **Phase 2 (Faust DSP Integration):** CMake + Faust code generation build system is non-trivial cross-platform. Needs a spike: confirm `faust` CLI availability in CI, confirm CMake `add_custom_command` pattern for Faust → C++ generation, confirm generated C++ compiles cleanly with the Flutter native build system.
- **Phase 8 (Platform + Polish):** macOS notarization with bundled `.dylib` is a known Flutter desktop pain point with sparse documentation. Needs platform research before planning.

Phases with standard, well-documented patterns (skip research-phase):

- **Phase 3 (Dart FFI Bindings):** `dart:ffi` + `ffigen` is a standard Flutter native integration; official docs cover the exact pattern.
- **Phase 4 (State Layer):** Riverpod + Command pattern is well-documented Flutter architecture.
- **Phase 7 (Persistence):** JSON serialization + `path_provider` is standard Flutter; no novel patterns.

---

## Open Questions Requiring Human Decision

These are not research gaps — they are design decisions that need Josh's call before the roadmap is locked.

1. **Runtime interpreter vs. compile-time Faust (CRITICAL — must decide before Phase 2).** Research recommends compile-time. Confirm or override.

2. **ServerConsole: Phase 1 or Phase 2 UI?** The design spec places it in Phase 2. FEATURES research agrees it can be deferred. But if the debugging value is high during development, it may be worth building earlier. Josh's call.

3. **DSP voice pool size.** The architecture requires pre-allocation. What is the maximum supported simultaneous voice count? The design spec implies ~20–30 voices; this caps the pre-allocation. Needs a number before C wrapper implementation.

4. **Waveform crossfade time.** The design spec mentions crossfade during waveform switching. What is the target crossfade duration (ms)? Affects DSP implementation in Phase 2.

5. **Preset storage location.** `path_provider` returns the OS-standard app documents directory. Is that the right location, or should presets be stored somewhere more accessible (e.g., `~/Documents/Justifier/`)?

---

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | MEDIUM | libfaust C API surface confirmed via official docs. Windows DLL path and macOS notarization are medium-confidence. |
| Features | MEDIUM | Ecosystem survey via web search; direct tool use limited. Design spec is high-confidence primary source. |
| Architecture | HIGH | Component boundaries from first principles; real-time audio threading rules are canonical (Ross Bencina, etc.). |
| Pitfalls | MEDIUM-HIGH | Audio thread rules are well-established. Dart FFI + libfaust specifics are thinner; some pitfalls inferred from patterns. |

**Overall confidence:** MEDIUM — sufficient to plan a roadmap and begin Phase 1. The compile-time vs. interpreter decision is the main open risk.

### Gaps to Address

- **libfaust prebuilt binary on Windows:** Faust GitHub releases show a Windows installer but DLL extraction into a Flutter bundle needs hands-on verification. Spike in Phase 2.
- **macOS notarization with bundled `.dylib`:** Known Flutter desktop pain point. Solutions exist but need research specific to this setup. Address in Phase 8 research.
- **libfaust interpreter thread safety:** Is the interpreter DSP state per-instance or is there shared global state that requires locking? Matters only if taking the interpreter path. Verify in Phase 2 if runtime compilation is chosen.
- **Faust compiler toolchain on CI:** Confirm `apt install faust` / `brew install faust` provides the correct version and CLI tool for code generation. Spike in Phase 2.

---

## Sources

### Primary (HIGH confidence)

- [Faust Embedding Documentation](https://faustdoc.grame.fr/manual/embedding/) — interpreter/LLVM/compile-time backends, C API surface
- [Faust GitHub](https://github.com/grame-cncm/faust) — source, releases, issue #221 (deleteDSPFactory heap corruption)
- [Dart C interop (dart:ffi)](https://dart.dev/interop/c-interop) — official FFI patterns
- [Flutter macOS C interop](https://docs.flutter.dev/platform-integration/macos/c-interop) — official
- [Ross Bencina — Real-time audio programming 101](http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing) — canonical audio thread rules
- `docs/superpowers/specs/2026-03-26-justifier-design.md` — primary design spec
- `.planning/PROJECT.md` — project definition and constraints

### Secondary (MEDIUM confidence)

- [miniaudio](https://miniaud.io/) — library docs and API
- [Dart FFI NativeCallable threading](https://github.com/dart-lang/sdk/issues/54276) — sync vs async callback rules
- [cyfaust](https://github.com/shakfu/cyfaust) — reference: Faust interpreter + audio in Python
- [Entonal Studio](https://node.audio/products/entonal-studio), [Wilsonic](https://apps.apple.com/us/app/wilsonic/id848852071), [Xenharmonic Wiki](https://en.xen.wiki/w/List_of_music_software) — ecosystem survey
- [PipeWire quantum — ArchWiki](https://wiki.archlinux.org/title/PipeWire) — Linux audio configuration
- `docs/superpowers/specs/2026-03-31-architecture-options.md` — architecture options considered

### Tertiary (LOW confidence)

- [Wilsonic Synthtopia review](https://www.synthtopia.com/content/2015/01/05/new-app-wilsonic-lets-you-explore-profound-microtonal-harmonies/) — 2015, likely outdated feature overview

---

*Research completed: 2026-04-01*
*Ready for roadmap: yes — pending resolution of interpreter vs. compile-time Faust decision*
