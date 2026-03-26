# Justifier — Just Intonation Sound Design Tool

**Date:** 2026-03-26
**Status:** Approved

## Overview

Justifier is a Flutter desktop application for exploring just intonation through direct manipulation of sound. It provides a card-based workspace for creating, tweaking, and layering voices — each defined by JI ratios from a reference frequency — with SuperCollider's `scsynth` as the audio engine.

The primary goal is **discoverability through interaction**: every parameter is a visible knob, so the user learns what's possible by turning things and hearing the result.

## Architecture

```
Flutter (Justifier UI)  ──OSC/UDP──▶  scsynth (audio engine)
       │                                    │
   Riverpod state                    SC synth nodes
   Voice/Wave models                 Groups, buses
   Preset JSON files                 SynthDef definitions
```

- **Flutter** is the control surface. All UI, state management, persistence.
- **scsynth** is the audio engine. All synthesis, mixing, output.
- **OSC over UDP** is the wire protocol. Dart `osc` package.
- **Riverpod** (no codegen) for state management.

### Deployment

- **Desktop (v1):** Flutter app launches scsynth as a child process, connects to `localhost:57110`. Zero configuration.
- **Remote (future):** Same app connects to scsynth on another machine via host/port settings. Mobile becomes a wireless control surface.

## Voice Model

A **voice** is a single synth node on scsynth. Its parameters are defined by its SynthDef descriptor (see Extensibility section), but the v1 built-in voices share this common parameter set:

| Parameter | Type | Range | Notes |
|-----------|------|-------|-------|
| Ratio | fraction | any JI ratio | Numerator + denominator inputs |
| Octave | int | 0–8 | Multiplier: freq = reference × ratio × 2^octave |
| Mod offset | float | 0–10 Hz | Beating / detuning |
| Amplitude | float | 0.0–1.0 | Per-voice volume |
| Waveform | enum | sine, sine-mod, triangle, saw, pulse, LFNoise0, filtered-noise | Selects SynthDef; switching = crossfade swap |
| Pan | float | -1.0 to 1.0 | Stereo position |
| Fade time | float | 0.01–5.0 sec | On/off transition duration |
| Filter cutoff | float | 20–20000 Hz | Active only for filtered-noise |
| Filter rq | float | 0.01–1.0 | Reciprocal of Q (rq = 1/Q). Active only for filtered-noise. Display as "Resonance" in UI. |
| Enabled | bool | on/off | Toggles without destroying node (fades amp) |

**Derived display values** (read-only):
- Actual frequency in Hz: `reference × ratio × 2^octave`
- Cents from reference

All frequencies are expressed as ratios from a global reference frequency. No 12-TET. The reference defaults to 172.8 Hz and is user-configurable.

## Groups ("Waves")

A **wave** is a named, user-created group of voices. Maps to an SC group node.

- No fixed types — a wave named "sine ocean" happens to contain sine voices, but nothing enforces it
- **Master controls per wave:** volume, mute, solo, pan offset (additive with per-voice pan, clamped to -1..1)
- **Solo behavior:** soloing a wave mutes all other waves. Multiple waves can be soloed simultaneously (only unsoloed waves are muted). Implemented by setting `\gate 0` on voices in muted waves and `\gate 1` on voices in soloed/unmuted waves.
- **Add/remove voices** via "+ Add Voice" card within the wave
- **Collapse/expand** waves to save screen space
- Reorder waves by drag (stretch goal for v1)

## Workspace Layout

Single-screen application. No tabs, no navigation.

### Top bar
- App name
- Reference frequency input (Hz)
- Master volume
- Connection status indicator (connected/disconnected to scsynth)
- Preset selector dropdown
- Panic button (kills all nodes)

### Workspace area
- Horizontal scroll of wave columns
- Each wave: header (name, color, master controls) → vertical stack of voice cards → "+ Add Voice" card
- "Add Wave" button at the right edge

### Responsive breakpoints
- Desktop (>1200px): 3–4 waves visible
- Tablet (600–1200px): 2 waves visible, horizontal scroll
- Mobile (<600px): 1 wave at a time, swipe

### Theme
Dark theme only for v1.

### Key interactions
- **Add Wave:** button at right edge, creates empty wave
- **Add Voice:** dashed card at bottom of wave, adds voice with defaults
- **Remove Voice:** long-press or swipe on card, with confirmation
- **Remove Wave:** menu on wave header, confirms if voices exist
- **Edit wave name/color:** tap wave header
- **Drag reorder:** voices within wave, waves within workspace (stretch goal)

## SynthDef Descriptors (Extensibility)

The voice model is **data-driven**. Each waveform type is described by a SynthDef descriptor JSON file that tells the UI what controls to render.

```json
{
  "id": "sine-mod",
  "name": "Sine (beating)",
  "category": "tonal",
  "synthDef": "interval-mod",
  "controls": [
    {"name": "base", "type": "float", "min": 20, "max": 20000, "default": 172.8, "hidden": true, "source": "reference"},
    {"name": "interval", "type": "ratio", "default": [1, 1]},
    {"name": "octave", "type": "int", "min": 0, "max": 8, "default": 4},
    {"name": "mod", "type": "float", "min": 0, "max": 10, "default": 0, "unit": "Hz", "label": "Beat offset"},
    {"name": "mul", "type": "float", "min": 0, "max": 1, "default": 0.05, "label": "Amplitude"},
    {"name": "pan", "type": "float", "min": -1, "max": 1, "default": 0}
  ]
}
```

**Behavior:**
- `hidden: true` + `source: "reference"` — auto-set from global reference, no knob rendered
- `type: "ratio"` — renders numerator/denominator input
- Voice card reads the descriptor and renders one knob per visible control
- Adding a new SynthDef + descriptor = new waveform in the UI, zero Dart code changes

**Shipped descriptors for v1:** sine, sine-mod, triangle, saw, pulse, LFNoise0, filtered-noise (7 total).

## OSC Layer

Thin service class that owns the UDP socket and translates Riverpod state changes into OSC messages.

### Responsibilities
- Connect to scsynth (host/port configurable, default `localhost:57110`)
- On desktop, launch scsynth as child process via `dart:io` `Process.start`. Detect readiness by polling `/status` until a `/status.reply` is received (timeout after 10 seconds → show error).
- Allocate and track node IDs. Start at 1000, increment per allocation. No recycling — at 32-bit range this is effectively unlimited for interactive use. On preset load (full workspace reset), counter resets to 1000.
- On startup, create an app group: `/g_new appGroupId 0 0`. All wave groups are children of this app group. Panic targets the app group, not root.
- Load SynthDef bytes at startup via `/d_recv`

### State-to-OSC mapping

| State change | OSC message |
|-------------|-------------|
| Voice created | `/s_new synthDefName nodeId 0 groupId \base ... \mul ... \pan ... \fadeTime ... \gate 1` |
| Parameter changed | `/n_set nodeId \paramName value` |
| Voice disabled | `/n_set nodeId \gate 0` — EnvGen release fades out over `fadeTime`, but node stays alive (gate can be re-opened) |
| Voice enabled | `/n_set nodeId \gate 1` — EnvGen attack fades back in |
| Voice removed | `/n_set nodeId \gate 0`, then after `fadeTime` delay: `/n_free nodeId` |
| Waveform switched | 1) `/s_new newDef newNodeId 0 groupId ...params... \fadeTime crossfadeTime \gate 1` 2) `/n_set oldNodeId \gate 0` 3) After `crossfadeTime` delay: `/n_free oldNodeId`. Both nodes overlap during crossfade. |
| Wave created | `/g_new groupId 0 appGroupId` (child of app group) |
| Wave removed | `/g_freeAll groupId` then `/g_free groupId` |
| Wave pan offset changed | `/n_set voiceNodeId \pan effectivePan` for each voice in wave (effectivePan = voicePan + wavePanOffset, clamped to -1..1) |
| Panic | `/g_freeAll appGroupId` (frees all voices/groups under the app group, leaves scsynth root intact) |

### Throttling
If a parameter changes more than ~40 times/sec (fast slider drag), drop intermediate messages, send latest value only.

### Error handling
scsynth doesn't ACK most commands (fire and forget). Connection loss detected by periodic `/status` ping — no response triggers "disconnected" banner in UI.

### Not in scope for v1
No audio analysis, no FFT readback, no recording. Strictly one-way: Flutter → scsynth.

## Presets

A preset is a JSON snapshot of the entire workspace state.

```json
{
  "name": "deep drone",
  "referenceHz": 172.8,
  "waves": [
    {
      "name": "sine ocean",
      "color": "#7b9",
      "masterVol": 0.8,
      "mute": false,
      "voices": [
        {
          "synthDescriptor": "sine-mod",
          "params": {
            "interval": [1, 1],
            "octave": 4,
            "mod": 0.0,
            "mul": 0.12,
            "pan": 0.0,
            "fadeTime": 0.5
          },
          "enabled": true
        }
      ]
    }
  ]
}
```

**Operations:** Save, Load, Save As, Delete, Quick-switch (dropdown in top bar).

Stored in app documents directory under `presets/`.

No auto-save, no undo history for v1.

## Dependencies

| Package | Purpose |
|---------|---------|
| `flutter_riverpod` | State management |
| `osc` | Dart OSC/UDP client |

Process management (launching scsynth) uses `dart:io` `Process` — no additional package needed.

## Out of Scope for v1

- Automation / plugin system (future)
- Audio analysis / FFT visualization
- Recording / export audio
- MIDI input
- User-created SynthDef descriptors (shipped set only)
- Undo/redo
- Multiple themes
- Mobile deployment (desktop only)

## SynthDefs Required

The following SuperCollider SynthDefs need to exist (some already do in base.sc, others need creation):

| SynthDef | Controls | Status |
|----------|----------|--------|
| `interval` | base, interval, octave, mul, pan, fadeTime, gate | Exists (needs pan, fadeTime, gate) |
| `interval-mod` | base, interval, octave, mod, mul, pan, fadeTime, gate | Exists (needs pan, fadeTime, gate) |

Note: The `base` and `base-mod` SynthDefs from base.sc are subsumed by `interval` and `interval-mod` with `interval = [1,1]`. No need for separate SynthDefs.
| `tri` | base, interval, octave, mul, pan, fadeTime, gate | New |
| `saw` | base, interval, octave, mul, pan, fadeTime, gate | New |
| `pulse` | base, interval, octave, mul, pan, fadeTime, gate | New |
| `lfnoise` | base, interval, octave, mul, pan, fadeTime, gate | New |
| `filtered-noise` | base, interval, octave, mul, pan, fadeTime, gate, cutoff, rq | New (rq = reciprocal of Q, SC convention) |

All SynthDefs must follow this envelope pattern:

```supercollider
// Standard voice envelope — all SynthDefs use this
var env = EnvGen.kr(
    Env.asr(fadeTime, 1, fadeTime),
    gate,
    doneAction: 0  // Do NOT self-free. Flutter manages node lifecycle.
);
Out.ar(out, Pan2.ar(sig * mul * env, pan));
```

- `\gate` (default 1): controls EnvGen. Set to 0 to fade out, 1 to fade in.
- `\fadeTime` (default 0.5): attack and release time for the envelope.
- `doneAction: 0`: node stays alive after release so gate can be re-opened (for enable/disable toggle). Flutter sends `/n_free` explicitly when truly removing a voice.
