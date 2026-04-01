# Phase 1: C Audio Engine - Context

**Gathered:** 2026-04-01
**Status:** Ready for planning

<domain>
## Phase Boundary

A standalone C audio layer that produces all 7 waveforms with correct real-time threading, proven independently of Flutter. This phase delivers a C test harness that can play any waveform type with parameter control — no Dart, no UI. The output is a static library + header (`justifier_audio.h`) that Phase 2 will bind to via Dart FFI.

</domain>

<decisions>
## Implementation Decisions

### Voice Pool Sizing
- **D-01:** Maximum 32 simultaneous voice slots, pre-allocated at startup. Each slot holds one Faust DSP instance. Can be bumped with a recompile if needed later.

### Waveform Crossfade
- **D-02:** 20ms crossfade when switching waveform type on a live voice. Run both old and new DSP instances during the overlap, fade one out and the other in. Requires atomic pointer swap pattern in the C wrapper.

### Audio Buffer Configuration
- **D-03:** Default buffer size: 256 samples. Default sample rate: 48000 Hz. ~5.3ms latency — responsive enough for parameter tweaking, stable on most systems.
- **D-04:** These are defaults, not hardcoded. PipeWire on Linux may override the buffer size via its quantum setting — the code should handle this gracefully.

### DSP Voice Parameters
- **D-05:** Each voice exposes these parameters to the C wrapper:
  - `frequency` (Hz, derived from JI ratio + reference freq)
  - `amplitude` (0.0-1.0)
  - `gate` (on/off with configurable attack/release envelope — attack for fade-in on create, release for fade-out on sleep-before-destroy)
  - `pan` (stereo position, -1.0 to 1.0)
  - `detune` (cents offset from the JI-derived frequency — for subtle beating effects)
- **D-06:** FM waveform type additionally exposes:
  - `mod_ratio` (modulator frequency as ratio of carrier frequency)
  - `mod_index` (modulation depth)

### Faust Build Approach
- **D-07:** Compile-time Faust for v1. The 7 `.dsp` files are compiled to C++ via `faust -lang cpp` during the CMake build. No runtime libfaust dependency. The C wrapper interface is abstracted so runtime libfaust can slot in later as a drop-in.

### SPSC Queue
- **D-08:** Use a proven header-only lock-free SPSC queue (e.g., Cameron Desrochers' readerwriterqueue) for UI-to-audio parameter messages. No custom implementation.

### Audio Output
- **D-09:** miniaudio for cross-platform audio output. Single-header, MIT license, handles ALSA/PipeWire/CoreAudio/WASAPI. Abstracted behind `audio_engine.c` so it can be swapped without touching upper layers.

### Claude's Discretion
- C test harness design (how to verify audio output without Flutter)
- CMake build structure for Faust compilation + miniaudio + SPSC queue
- Voice slot lifecycle implementation details (activation, deactivation, reclamation)
- Silence flag implementation (atomic bool vs atomic int)

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Design Spec
- `docs/superpowers/specs/2026-03-26-justifier-design.md` -- Full UI/UX design spec. Phase 1 doesn't implement UI, but the voice model and parameter set must align with what the design spec expects.

### Architecture Research
- `.planning/research/ARCHITECTURE.md` -- Component diagram, data flow, thread safety model. Critical for the C wrapper design.
- `.planning/research/STACK.md` -- Stack recommendations including libfaust interpreter vs compile-time tradeoffs, miniaudio details.
- `.planning/research/PITFALLS.md` -- 13 numbered pitfalls. Audio thread rules, Dart FFI threading constraints, libfaust memory bugs, cross-platform gotchas.
- `.planning/research/SUMMARY.md` -- Synthesis with build order recommendation and key tension resolution.

### Architecture Options
- `docs/superpowers/specs/2026-03-31-architecture-options.md` -- Background on why Faust was chosen over SC and Csound.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- None — this is a blank Flutter project. Phase 1 builds the C layer from scratch.

### Established Patterns
- None yet. Phase 1 establishes the foundational patterns (threading model, memory lifecycle, parameter passing) that all subsequent phases build on.

### Integration Points
- The output of this phase (`justifier_audio.h` + static library) is the integration point for Phase 2's Dart FFI bindings.

</code_context>

<specifics>
## Specific Ideas

- Josh's SC code at `~/w/astro/soft/sc/base.sc` has sine ocean and noise ocean soundscapes — the synthesis output should produce comparable drone/ambient sound quality.
- The gate envelope replaces SC's `EnvGen.kr` concept — attack time for voice creation, release time for the sleep-before-destroy 10s fade-out.
- Pan per voice enables spatial distribution of dense soundscapes — important for making many simultaneous voices distinguishable.
- Detune (cents offset) enables subtle beating effects between nearly-unison voices — a common technique in drone music.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---

*Phase: 01-c-audio-engine*
*Context gathered: 2026-04-01*
