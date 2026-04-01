# Phase 1: C Audio Engine - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-04-01
**Phase:** 01-c-audio-engine
**Areas discussed:** Voice pool sizing, Waveform crossfade, Audio buffer config, DSP file authoring

---

## Voice Pool Sizing

| Option | Description | Selected |
|--------|-------------|----------|
| 32 | Comfortable for layered soundscapes, modest CPU/memory. Can bump later with a recompile. | * |
| 64 | Room for very complex arrangements. Still reasonable for modern CPUs. | |
| 128 | Overkill for JI exploration but future-proof. ~1MB memory, CPU is the real limit. | |

**User's choice:** 32 (Recommended)
**Notes:** None — straightforward pick.

---

## Waveform Crossfade

| Option | Description | Selected |
|--------|-------------|----------|
| 20ms crossfade | Short enough to feel instant, long enough to avoid clicks. Standard for real-time audio. | * |
| 50ms crossfade | Smoother transition, slightly noticeable as a blend. | |
| No crossfade, just declick | Hard swap at zero-crossing point. Simpler implementation, may still click on complex waveforms. | |

**User's choice:** 20ms crossfade (Recommended)
**Notes:** None.

---

## Audio Buffer Config

### Buffer Size

| Option | Description | Selected |
|--------|-------------|----------|
| 256 samples | ~5ms latency at 48kHz. Responsive enough for parameter tweaking, stable on most systems. | * |
| 128 samples | ~2.7ms. Snappier feel but more prone to xruns on busy systems. | |
| You decide | Claude picks based on research and implementation. | |

**User's choice:** 256 samples (Recommended)

### Sample Rate

| Option | Description | Selected |
|--------|-------------|----------|
| 48000 Hz | Modern standard. Good quality, wide hardware support. | * |
| 44100 Hz | CD standard. Slightly less CPU, universal support. | |
| You decide | Claude picks based on platform defaults. | |

**User's choice:** 48000 Hz (Recommended)

---

## DSP File Authoring

### Voice Parameters

| Option | Description | Selected |
|--------|-------------|----------|
| Gate with fade envelope | on/off with configurable attack/release times (for sleep-before-destroy fade-out) | * |
| Pan (stereo position) | Left/right placement per voice. Makes dense soundscapes more spatial. | * |
| Detune (cents offset) | Fine-tune offset from the JI ratio. Useful for subtle beating effects. | * |

**User's choice:** All three selected (multiselect)
**Notes:** These are in addition to the base frequency and amplitude parameters.

### FM Parameters

| Option | Description | Selected |
|--------|-------------|----------|
| Mod ratio + mod index | Standard FM: modulator frequency as ratio of carrier, and modulation depth. Simple, expressive. | * |
| You decide | Claude designs the FM DSP based on standard Faust FM synthesis patterns. | |

**User's choice:** Mod ratio + mod index

---

## Claude's Discretion

- C test harness design
- CMake build structure
- Voice slot lifecycle implementation details
- Silence flag implementation

## Deferred Ideas

None — discussion stayed within phase scope.
