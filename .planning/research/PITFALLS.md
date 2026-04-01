# Domain Pitfalls: Flutter + libfaust Desktop Audio App

**Project:** Justifier
**Domain:** GUI application embedding a DSP engine via FFI
**Researched:** 2026-04-01
**Confidence:** MEDIUM-HIGH (audio thread rules are well-established; Dart FFI + libfaust specifics are thinner)

---

## Critical Pitfalls

Mistakes that cause rewrites, permanent audio glitches, or crashes in the field.

---

### Pitfall 1: Calling Any Blocking Operation on the Audio Thread

**What goes wrong:** The audio callback fires on a dedicated real-time thread managed by the audio backend (ALSA/PipeWire/CoreAudio/WASAPI). If that callback blocks — even for a few milliseconds — the buffer runs dry and you get clicks, pops, or silence. Blocking causes include: any memory allocator (`malloc`/`free`, which may take a lock), any mutex, any syscall, any Dart FFI call that crosses an isolate boundary.

**Why it happens:** The C++ DSP wrapper calls back into Dart to query parameter values, or Dart allocates memory on the audio path to pass buffers, or the DSP graph is updated in-place while the callback is running.

**Consequences:** Audible glitches at all sample rates. Impossible to debug deterministically — it appears intermittently based on OS scheduler pressure.

**Prevention:**
- The audio thread must touch only pre-allocated, lock-free data structures. No Dart calls whatsoever from the audio callback.
- All parameter changes from UI → audio use a lock-free single-producer/single-consumer ring buffer (SPSC queue). The C wrapper reads from this ring buffer at the top of each audio callback.
- All Dart FFI calls that touch audio state must be synchronous calls *into* the C layer from the Dart/UI thread — never callbacks *out* of C into Dart on the audio thread. Dart FFI `NativeCallable` async callbacks are the only safe direction for audio-thread-originated events, and even those should be minimized.
- Pre-allocate all DSP resources at voice creation time, not at audio callback time.

**Warning signs:** Intermittent clicks that worsen under CPU load. Glitches that disappear on a fast machine.

**Phase:** Address in the C wrapper design before any audio plays. This is architecture, not a bug fix.

---

### Pitfall 2: Dart FFI Callbacks from Non-Dart Threads

**What goes wrong:** The Faust audio callback runs on a thread that was not created by Dart. Calling back into Dart from that thread via a `NativeCallable` that was created as a synchronous callback will crash with an unrecoverable error.

**Why it happens:** libfaust's `compute()` method is called by the audio backend's thread, not by Dart. If the C wrapper invokes a Dart callback from inside `compute()`, the Dart VM has no isolate context on that thread.

**Consequences:** Hard crash, no recovery. Particularly insidious because it may work in debug mode (different threading) but crash in release.

**Prevention:**
- Never invoke a Dart callback from within the audio callback. The data flow is strictly one-way at audio time: Dart writes parameters to a C-side ring buffer; C reads them. Audio output goes directly to the hardware.
- Any status reporting back to Dart (level meters, playback state) must be polled by Dart via FFI calls from the UI thread on a timer, not pushed from the audio thread.
- If you need async notifications (e.g., voice finished), use `NativeCallable.listener()` (async), never `NativeCallable.isolateLocal()` from a non-Dart thread.

**Warning signs:** Crashes with `SIGABRT` or `Unhandled exception: Invalid argument` in release mode that don't appear in debug. Stack traces pointing into Dart VM internals.

**Phase:** C wrapper design. Write a threading contract document for the wrapper before implementation.

---

### Pitfall 3: libfaust `deleteDSPFactory` / `delete` Memory Corruption

**What goes wrong:** Calling `deleteDSPFactory()` on certain compiler/LLVM version combinations triggers heap corruption: `corrupted size vs. prev_size`. This is a known libfaust bug (GitHub issue #221) affecting g++ 5.5, g++7, clang 5-6 with LLVM 3.8-6.0. It is linked to how UI slider metadata is handled in the generated DSP class destructor.

**Why it happens:** The Faust LLVM backend generates code that, in certain configurations, produces a destructor with a bad free() sequence when sliders are present in the DSP program.

**Consequences:** Crash on voice destruction. In Justifier, voice deletion (including the sleep-before-destroy pattern) is a core feature — this would make deletion unreliable.

**Prevention:**
- Pin libfaust and LLVM versions in the build; don't update casually. Document the exact versions that have been tested.
- Wrap `deleteDSPFactory` in the C wrapper with a try/catch and log failures — crashes from this function should never propagate into Flutter.
- Test the sleep-before-destroy path on all three platforms in CI before shipping.
- If the crash appears, the workaround is replacing hslider/vslider UI elements with `nentry` (number entry) in the Faust DSP code — confirmed to eliminate the issue in the bug report.

**Warning signs:** Crash on voice deletion, especially when multiple voices are deleted in rapid succession. Crash only on Linux (where g++ is most commonly used).

**Phase:** C wrapper implementation and the voice deletion feature (sleep-before-destroy milestone).

---

### Pitfall 4: DSP Graph Hot-Swap Causing Audio Gap or Crash

**What goes wrong:** Changing waveform type on a live voice requires swapping the Faust DSP instance. If this happens on the audio thread (or while the audio callback holds a reference to the old DSP), the swap causes either a use-after-free crash or a gap in audio output.

**Why it happens:** The natural implementation is "create new DSP, delete old DSP, point the slot at the new one." If the audio callback fires between "delete old" and "point at new," it reads freed memory.

**Consequences:** Crash or silent period during waveform change.

**Prevention:**
- Use a double-buffering or atomic-pointer approach in the C wrapper. Prepare the new DSP instance completely (including initial parameter state) before making it visible to the audio callback.
- Only the audio callback reads the active DSP pointer. The UI thread prepares a "pending" DSP and signals via the SPSC queue. The audio callback checks the queue, swaps the pointer atomically, and the UI thread then frees the old one *after* at least one callback cycle has passed.
- This is the same pattern used by FaustLive and faustgen~ for hot-reload without audio dropout.

**Warning signs:** Occasional crashes or clicks when switching waveform type, especially at low latency settings.

**Phase:** C wrapper design, before waveform-switching is implemented in the UI.

---

## Moderate Pitfalls

---

### Pitfall 5: Dart FFI Memory Not Freed (Finalizer Gap)

**What goes wrong:** Memory allocated in C and returned to Dart as a pointer is not freed when the Dart object wrapping it is garbage collected — Dart's GC does not know about native memory. Under normal usage this is slow-drip; under voice-creation-heavy usage (users experimenting rapidly) it becomes a leak.

**Prevention:**
- Attach a `NativeFinalizer` to every Dart object that owns a native resource (DSP instance, compiled factory, parameter block). The finalizer calls the C-side free function when the Dart object is collected.
- Alternatively, use explicit lifecycle management: Dart calls `voice.dispose()` which calls the C destructor. Make disposal mandatory, not optional. The sleep-before-destroy pattern gives a natural disposal point.
- Audit the C wrapper API: every `create*` function must have a paired `destroy*` function. No orphaned allocations.

**Warning signs:** Increasing RSS over a session where voices are created and deleted. Memory does not return to baseline after voice deletion.

**Phase:** C wrapper API design. Add a memory lifecycle test to CI early.

---

### Pitfall 6: PipeWire Quantum Interference on Linux

**What goes wrong:** PipeWire uses a single quantum (buffer size) for all clients. Any other running application that requests a lower quantum forces all applications — including Justifier — to operate at that lower quantum. This can cause unexpected latency spikes or buffer underruns that appear random and platform-specific.

**Prevention:**
- Request a specific quantum in the JACK/PipeWire client configuration (via `pw_properties` on the stream). Don't rely on system defaults.
- Document the minimum supported quantum in user-facing notes.
- Test with pavucontrol and other audio apps running simultaneously, not just in isolation.
- On ALSA (non-PipeWire), avoid reading ALSA timestamps in scheduling — some drivers produce bad timestamps (known ALSA issue). Use a monotonic clock instead.

**Warning signs:** Latency and glitches that only appear on Linux, and only when other audio apps are running.

**Phase:** Platform integration testing. Not a design issue, but must be validated before shipping Linux builds.

---

### Pitfall 7: Faust Compilation Overhead Blocking the UI

**What goes wrong:** `createDSPFactoryFromString()` compiles Faust DSP source via LLVM JIT. On first call this can take 100ms–2s depending on DSP complexity and machine. If called on the main Dart isolate (UI thread), the app freezes.

**Prevention:**
- Always call Faust compilation inside a `compute:` isolate or off-thread via `Isolate.run()`. Never on the UI thread.
- Cache compiled factories by DSP source string hash. The seven waveform types in Justifier are fixed — compile them all at startup in parallel and cache. Voice creation then only instantiates from the cached factory (`createDSPInstance()`), which is fast.
- Show a loading state during initial DSP factory compilation at app startup. Don't wait until first voice creation.

**Warning signs:** UI jank or freeze when creating the first voice of each waveform type.

**Phase:** Application startup and voice creation phases.

---

### Pitfall 8: Flutter Desktop Plugin Not Found at Runtime (Library Bundling)

**What goes wrong:** The compiled native library (`libfaust.so`, `libfaust.dylib`, `faust.dll`) is not included in the application bundle, or is placed in a location the dynamic linker does not search. App crashes at startup with a missing library error, or silently falls back to null.

**Prevention:**
- On Linux: the `.so` must be in the bundle's `lib/` directory. Use `flutter build linux` and inspect the bundle structure. Add a post-build check that verifies the library is present.
- On macOS: the `.dylib` must be inside the `.app` bundle's `Frameworks/` directory and must pass notarization (`@rpath` linking, signed). Unsigned `.dylib` files are rejected silently on newer macOS.
- On Windows: the `.dll` must be alongside the `.exe` in the same directory. Watch for Visual C++ Runtime dependencies — the LLVM build of libfaust may require a specific CRT version.
- Add a smoke test in CI: build, then run the binary and check it loads and calls a basic DSP method without crashing.

**Warning signs:** "Symbol not found" or "image not found" crash at startup on clean machines that don't have libfaust installed system-wide.

**Phase:** Build system setup, before any platform-specific features are built.

---

### Pitfall 9: UI ↔ Audio State Drift

**What goes wrong:** The UI displays parameter values (frequency, amplitude, ratio) that are out of sync with what the audio engine is actually computing. This is especially likely for the derived display values (Hz, cents) which must be computed from the reference frequency and the ratio — if the reference frequency changes, all derived values must update simultaneously.

**Why it happens:** The UI mutates its own state optimistically (before confirming the audio engine accepted it), or the audio engine applies changes with a sample-accurate delay (parameter smoothing) while the UI updates instantly.

**Prevention:**
- Define a single authoritative state store for all voice parameters. The UI reads from this store; commands write to it; the store dispatches to both the UI renderer and the audio engine's SPSC queue.
- Parameter smoothing (to avoid clicks on frequency/amplitude changes) must be done in the DSP code, not by delaying the UI update. The UI should show the *target* value, not the smoothed value.
- For the reference frequency change case: update all derived display values atomically — compute the new Hz/cents values in a single pass before triggering any re-render.

**Warning signs:** Displayed Hz doesn't match the audible pitch after a reference frequency change. Knob position and audible parameter value diverge after rapid automation.

**Phase:** State management architecture, before voice card UI is built.

---

### Pitfall 10: Parameter Smoothing Causing Zipper Noise

**What goes wrong:** Frequency or amplitude changes applied as step changes in the DSP produce audible zipper noise (rapid-fire clicks). This is a classic DSP mistake for JI applications where the user directly manipulates ratios — ratio changes mean large, sudden frequency jumps.

**Prevention:**
- All continuously variable parameters (frequency, amplitude, FM ratio) in the Faust DSP must use `si.smooth` (Faust's smoothing primitive, which implements a one-pole lowpass filter) or an equivalent ramp generator.
- Choose smoothing time constants carefully for JI: frequency changes between harmonically related ratios can be made relatively fast (5–20ms) without sounding wrong, but amplitude changes on attack/release need longer constants (50–100ms).
- Add an explicit test: change a voice's ratio while audio is running and listen for clicks. Do this test on each waveform type.

**Warning signs:** Clicking or "stepping" sound when dragging a ratio knob, even at low CPU.

**Phase:** DSP implementation, first voice synthesis milestone.

---

## Minor Pitfalls

---

### Pitfall 11: Dart FFI Struct ABI Differences Across Platforms

**What goes wrong:** C structs passed between Dart and the C wrapper have different alignment/padding on different platforms (e.g., arm64 macOS vs. x86-64 Linux). Dart's `@Packed` annotations or missing alignment attributes cause fields to be read at wrong offsets.

**Prevention:** Don't pass complex structs across the FFI boundary. Pass only primitive types (int, float, double, pointer). Keep the FFI surface minimal: integers for voice IDs, floats for parameter values, opaque pointers for DSP instances. All struct manipulation happens in C.

**Warning signs:** Parameters have wrong values on macOS/Windows but work on Linux (or vice versa).

**Phase:** C wrapper API design.

---

### Pitfall 12: `malloc`/`free` Mismatch Between Dart and C Allocators

**What goes wrong:** Memory allocated with Dart's `calloc` (which uses the C runtime's `malloc`) is freed with a C-side `free` from a different CRT on Windows. On Windows, each DLL can have its own CRT heap; freeing across CRT boundaries corrupts the heap.

**Prevention:** All memory that will be freed by C code must be allocated by C code. All memory that will be freed by Dart must be allocated with Dart's `calloc`/`malloc` via `package:ffi`. Never pass ownership across the boundary unless you have a documented and matched allocator pair.

**Warning signs:** Heap corruption crashes on Windows only, usually in cleanup paths.

**Phase:** C wrapper design, Windows build.

---

### Pitfall 13: Panic Button Race Condition

**What goes wrong:** The panic button is supposed to produce instant silence. If it posts a "set all amplitudes to 0" message through the same SPSC queue as normal parameter updates, it may be delayed behind a backlog of pending updates, producing a perceptible delay before silence.

**Prevention:** The panic command must bypass the normal parameter queue. The C wrapper should expose a dedicated `panic()` function that atomically sets a global `is_silent` flag read at the very top of the audio callback, before any parameter processing. No queue, no delay.

**Warning signs:** Panic button takes more than one audio buffer cycle to silence output.

**Phase:** Audio engine integration, panic button feature.

---

## Phase-Specific Warning Table

| Phase Topic | Most Likely Pitfall | Mitigation |
|---|---|---|
| C wrapper design | Audio thread callbacks into Dart (Pitfall 2) | Threading contract doc before any code |
| C wrapper design | Memory ownership (Pitfalls 5, 12) | Every `create*` has `destroy*`; no cross-boundary frees |
| First audio output | Audio thread blocking (Pitfall 1) | Lock-free SPSC queue from day one |
| Voice waveform types | Faust compilation blocking UI (Pitfall 7) | Pre-compile all 7 types at startup |
| Voice deletion (sleep-before-destroy) | deleteDSPFactory crash (Pitfall 3) | Pin LLVM/Faust versions; test on all platforms |
| Waveform switching | Hot-swap crash (Pitfall 4) | Atomic pointer swap in C wrapper |
| Ratio/frequency UI | Zipper noise (Pitfall 10) | `si.smooth` in all Faust DSP programs |
| Reference freq change | State drift (Pitfall 9) | Single authoritative state store |
| Linux platform build | PipeWire quantum interference (Pitfall 6) | Request specific quantum; test with other audio apps running |
| macOS/Windows build | Library bundling failure (Pitfall 8) | Smoke test on clean CI runner for each platform |
| Panic button | Queue delay under load (Pitfall 13) | Dedicated atomic flag, bypass queue |

---

## Sources

- [Ross Bencina — Real-time audio programming 101](http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing) — canonical reference on audio thread rules. HIGH confidence.
- [Four common mistakes in audio development — atastypixel](https://atastypixel.com/four-common-mistakes-in-audio-development/) — practical catalogue. HIGH confidence.
- [Faust — Embedding the Compiler](https://faustdoc.grame.fr/manual/embedding/) — official libfaust embedding guide. HIGH confidence.
- [libfaust heap corruption issue #221](https://github.com/grame-cncm/faust/issues/221) — deleteDSPFactory crash, confirmed bug. HIGH confidence.
- [Dart FFI — NativeCallable threading](https://github.com/dart-lang/sdk/issues/54276) — sync vs async callback rules. HIGH confidence.
- [Memory Management in Dart FFI — Medium](https://medium.com/@andycall/memory-management-in-dart-ffi-24577067ba43) — practical Dart FFI memory patterns. MEDIUM confidence.
- [Dart weak references and finalizers — QuickBird Studios](https://quickbirdstudios.com/blog/dart-weak-references-finalizers/) — NativeFinalizer usage. MEDIUM confidence.
- [Using locks in real-time audio processing safely — timur.audio](https://timur.audio/using-locks-in-real-time-audio-processing-safely) — nuanced lock guidance. HIGH confidence.
- [ADC 2024 — SeqLock wait-free synchronisation](https://conference.audio.dev/session/2024/wait-free-thread-synchronisation-with-the-seqlock/) — current best practices. MEDIUM confidence.
- [Low-latency ALSA audio — nyanpasu64](https://nyanpasu64.gitlab.io/blog/low-latency-audio-output-duplex-alsa/) — ALSA period/timestamp pitfalls. MEDIUM confidence.
- [PipeWire quantum behaviour — ArchWiki](https://wiki.archlinux.org/title/PipeWire) — global quantum side-effects. MEDIUM confidence.
