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

## Component Strategy

### Design System: Material 3 (heavily themed)

Material 3 as invisible foundation — dark, dense, compact. Provides battle-tested interaction infrastructure (focus traversal, scroll physics, gesture handling, accessibility) without imposing visual identity. Justifier's identity comes from the custom components.

**Theme approach:** Custom `ThemeData` — dark backgrounds, muted accent colors, compact density, monospace fonts for numeric values.

### Material 3 Foundation Components

| Component | Used For | Customization |
|-----------|----------|---------------|
| `Slider` | Amplitude, pan, mod offset, filter cutoff, filter rq, fade time | Compact, no labels — value shown in card |
| `TextField` | Reference frequency, wave name, ratio numerator/denominator | Monospace, tight padding, number-only |
| `DropdownButton` | Preset selector, waveform selector | Dark, minimal chrome |
| `IconButton` | Mute, solo, enable/disable, panic, delete | Small, icon-only, color-coded states |
| `Card` | Voice card shell | Custom painted, not stock Material card |
| `AlertDialog` | Delete confirmations | Dark themed |
| `Tooltip` | Parameter hints, Hz readout on hover | |

### Custom Components

#### VoiceCard

The core interactive unit. One card = one synth node.

- **Content:** Data-driven from SynthDef descriptor — renders one control per visible parameter
- **States:** enabled (normal), disabled (dimmed, still visible), creating (fade in)
- **Interactions:** Toggle enable, edit all parameters inline, long-press to delete
- **Derived values:** Actual Hz and cents from reference — always visible on card
- **Key detail:** The descriptor drives what controls appear — adding a SynthDef descriptor automatically produces a new card layout with zero Dart changes

#### WaveColumn

A named group of voices with master controls.

- **Anatomy:** Header (name, color swatch, volume slider, mute/solo buttons, menu) → vertical scroll of VoiceCards → "Add Voice" button at bottom
- **States:** expanded (default), collapsed (header only), soloed (visually highlighted), muted (dimmed)
- **Interactions:** Edit name/color, adjust master controls, collapse/expand, add voice, delete wave from menu

#### RatioInput

Enter a JI ratio as numerator/denominator.

- **Anatomy:** Two small integer TextFields with a `/` divider between them
- **Display:** Shows derived Hz and cents below as read-only text
- **Validation:** Positive integers only, denominator cannot be zero

#### WaveformSelector

Choose which SynthDef a voice uses.

- **Visual:** Dropdown — 7 waveforms in v1, too many for segmented buttons
- **Behavior:** Switching waveform triggers a crossfade swap on scsynth (old node fades out, new node fades in, both overlap briefly)
- **States:** Current selection highlighted, transition state during crossfade

#### ConnectionStatusBadge

Show scsynth connection state.

- **States:** connected (green dot + "connected"), disconnected (red + "disconnected" banner), connecting (pulsing)
- **Location:** Top bar, always visible
- **Behavior:** Periodic `/status` ping drives state

#### WorkspaceHeader

Top bar — global controls.

- **Anatomy:** App name | Reference Hz input | Master volume slider | ConnectionStatusBadge | Preset dropdown | Panic button

#### ServerConsole

Live window into scsynth communication and state.

- **Anatomy:** Two sections — **Message Log** (scrolling OSC messages sent to scsynth) and **Node Tree** (live hierarchy: app group → wave groups → voice nodes)
- **Desktop:** Always visible as a bottom panel or right-side panel — plenty of screen space
- **Tablet/Mobile:** Toggleable via tab/switch, not always visible
- **Node tree:** Polled via `/g_queryTree` on scsynth, shows real-time server state
- **Purpose:** Discoverability — users see the cause-and-effect between UI action and server state. Invaluable for learning and debugging.

### Component Implementation Roadmap

**Phase 1 — Core (must have before anything makes sound):**
- WorkspaceHeader (reference freq, panic, connection status)
- WaveColumn (basic: add/remove, name, master volume, collapse/expand)
- VoiceCard (basic: ratio, octave, amplitude, enable/disable)
- RatioInput
- ConnectionStatusBadge

**Phase 2 — Full voice control:**
- VoiceCard with full descriptor-driven rendering
- WaveformSelector with crossfade
- All slider controls (pan, mod, fade time, filter params)
- Mute/solo on waves
- ServerConsole (message log + node tree)

**Phase 3 — Polish:**
- Preset save/load/switch
- Drag reorder (stretch goal)
- Confirmation dialogs

## UX Consistency Patterns

### Parameter Editing

All parameter controls follow the same interaction contract:

- **Real-time feedback:** Every parameter change updates scsynth immediately (throttled at ~40 msg/sec). The sound IS the confirmation — no toasts, no checkmarks.
- **Visual feedback:** The control itself reflects current value (slider position, text field content). Derived values (Hz, cents) update live as parameters change.
- **Server feedback:** The ServerConsole message log shows each OSC message sent, so users can see the wire-level effect of every tweak. This is the visual confirmation layer.
- **Sliders:** Drag to change. Value displayed on the card near the slider. No separate labels — the card layout makes the parameter name clear.
- **Text fields (ratio, reference Hz):** Edit inline. Commit on Enter or focus loss. Invalid input reverts to previous value — no error dialogs for parameter entry.
- **Dropdowns (waveform, preset):** Single tap to open, single tap to select. Waveform change triggers crossfade (visual transition state on the card during overlap).
- **Toggles (enable/disable, mute, solo):** Single tap. Immediate. Icon changes state and color. Sound fades per `fadeTime`.

### Destructive Actions & Sleep/Undo

Justifier uses a **sleep-before-destroy** pattern for all deletions. Nothing is immediately destroyed.

**Voice deletion:**
1. User triggers delete (long-press or swipe on card)
2. Voice enters "sleeping" state: `\gate 0` sent to scsynth (node fades out but stays alive), card visually ghosted with strikethrough on the name
3. 10-second window: user can tap the sleeping card to wake it (`\gate 1`, restore visual state)
4. After 10 seconds: `/n_free` sent, card removed from UI, state cleaned up

**Wave deletion:**
1. User triggers delete from wave header menu
2. All voices in the wave enter sleeping state, wave column ghosted with strikethrough on wave name
3. 10-second window: user can undo, all voices wake
4. After 10 seconds: `/g_freeAll`, `/g_free`, column removed

**Panic button:**
- One click, instant, no confirmation. Sends `/g_freeAll appGroupId`.
- This is the emergency stop — speed matters more than caution.
- All voice cards reset to disabled state. Waves remain in UI (structure preserved, nodes gone). User can re-enable voices to recreate nodes.

**Why sleep-before-destroy:** scsynth's `gate 0` / `doneAction: 0` architecture already keeps nodes alive after release. The sleep pattern exposes this as a UX concept. It generalizes well — any future destructible entity can follow the same sleep → timeout → destroy pipeline.

### State Indication

Consistent visual language for all states across the app:

| State | Visual Treatment | Where |
|-------|-----------------|-------|
| Enabled | Normal appearance, full opacity | Voice card |
| Disabled | Dimmed (reduced opacity), controls still visible | Voice card |
| Sleeping (pending delete) | Ghosted + strikethrough on name, "wake" affordance | Voice card, wave column |
| Muted | Dimmed, mute icon highlighted | Wave column |
| Soloed | Visually highlighted (subtle border or glow), solo icon active | Wave column |
| Connected | Green dot | ConnectionStatusBadge |
| Disconnected | Red dot + "disconnected" text, banner across top | ConnectionStatusBadge |
| Connecting | Pulsing dot | ConnectionStatusBadge |

**Opacity scale:**
- Full: 1.0 (enabled, active)
- Dimmed: 0.5 (disabled, muted)
- Ghosted: 0.3 (sleeping / pending delete)

### Navigation Patterns

Minimal — Justifier is a single-screen app.

- **Horizontal scroll:** Waves arranged as columns, scroll left/right to see more
- **Vertical scroll:** Voice cards within a wave scroll vertically
- **ServerConsole:** On desktop, always visible as a panel. On tablet/mobile, toggle via tab/switch.
- **No page navigation, no routing, no back button.** Everything is the workspace.

### Feedback Patterns

Justifier's primary feedback channel is **sound**. Visual feedback is secondary and should never compete with or distract from the audio.

- **Parameter change:** Sound updates in real-time. No visual toast/snackbar.
- **Voice created:** New card appears with fade-in animation. Node starts playing (if enabled).
- **Voice deleted:** Card ghosts (sleep pattern). Sound fades out per fadeTime.
- **Waveform switched:** Brief crossfade — both old and new cards visible during overlap, then old disappears.
- **Connection lost:** Red banner — this is the one case where visual feedback is loud, because the user needs to know sound control is gone.
- **Preset loaded:** Full workspace rebuilds. Brief transition (fade or dissolve) to signal the reset.

### Empty States

- **No waves:** Workspace shows centered "Add Wave" button with brief hint text
- **Wave with no voices:** Wave column shows "Add Voice" button as the only content
- **No presets:** Preset dropdown shows "No saved presets" disabled item
- **Server not connected:** All controls disabled/greyed. ConnectionStatusBadge shows red. ServerConsole shows connection attempts.

## Responsive Design & Accessibility

### Responsive Strategy

Desktop-first. v1 is desktop-only. Tablet/mobile are future targets (remote control surface use case).

**Desktop (>1200px) — v1 primary target:**
- Single-screen workspace: wave columns left, ServerConsole panel on right
- 3–4 wave columns visible depending on window width and collapsed/expanded state
- Dense controls are fine — mouse precision and hover states available
- Core workflow: **configure → play → collapse → move on**. Collapse/expand is how users manage space, not tiny fonts.
- Readable font sizes throughout. No need to shrink to fit — collapsed waves handle density.

**Tablet (600–1200px) — future:**
- 2 wave columns visible, horizontal scroll
- ServerConsole as toggleable tab/switch (not permanent panel)
- All touch targets minimum 44px
- Sliders, buttons, ratio inputs all need touch-friendly sizing

**Mobile (<600px) — future:**
- 1 wave at a time, swipe between waves
- ServerConsole toggleable (may not be useful at this size — revisit when implementing)
- Maximum simplification of controls for touch

### Layout Architecture

```
┌─────────────────────────────────────────────┬──────────────┐
│                 Top Bar                      │              │
│  Name │ Ref Hz │ Master Vol │ Status │ Panic │              │
├──────────────────────────────────────────────┤  Server      │
│                                              │  Console     │
│  ┌─Wave 1──┐  ┌─Wave 2──┐  ┌─Wave 3──┐     │              │
│  │ Header  │  │ Header  │  │ Collapsed│     │  ┌─Log──┐   │
│  │ Voice 1 │  │ Voice 1 │  │ name,vol │     │  │ ...  │   │
│  │ Voice 2 │  │ Voice 2 │  │ mute,solo│     │  │ ...  │   │
│  │ Voice 3 │  │         │  │ 4 voices │     │  ├─Tree─┤   │
│  │ + Add   │  │ + Add   │  └─────────┘     │  │ ...  │   │
│  └─────────┘  └─────────┘                   │  │ ...  │   │
│                              ← scroll →      │  └──────┘   │
└──────────────────────────────────────────────┴──────────────┘
```

### Collapsed Wave Design

Collapsed waves show only what's needed to mix without expanding:

- **Visible:** Wave name, color indicator, voice count badge, master volume slider, mute/solo buttons, expand button
- **Hidden:** All voice cards, "Add Voice" button
- **Height:** Single row — as compact as a header bar
- **Interaction:** Click expand button or double-click header to expand. All master controls remain fully functional while collapsed.

This is a **core workflow pattern**, not polish. Configure voices → play → collapse → free up space for the next wave. Collapse/expand moves to Phase 1.

### Accessibility

**Target: WCAG AA compliance.**

Material 3 provides most of this for free if we don't strip it out during theming.

**Color & Contrast:**
- Minimum 4.5:1 contrast ratio for all text on dark backgrounds
- Dark theme must be carefully checked — low-contrast dark-on-dark is the #1 accessibility failure in dark UIs
- State colors (enabled/disabled/sleeping/muted) must be distinguishable by more than color alone (opacity + strikethrough + icon changes)

**Keyboard Navigation:**
- Tab traversal through all controls (Material provides this)
- Enter/Space to activate buttons and toggles
- Arrow keys for slider adjustment
- Visible focus rings on all interactive elements — must survive custom theming

**Screen Reader Support:**
- Semantic labels on all controls: "Amplitude slider, voice 3 in wave Sine Ocean, value 0.5" — not just "slider"
- Wave and voice structure communicated via ARIA/semantics so screen readers convey hierarchy
- State changes announced: "Voice 3 sleeping", "Wave muted", "Server disconnected"

**Audio Tool Reality:**
- The audio output itself cannot be made accessible to deaf users — that's the nature of the tool
- All *controls* are fully operable without hearing
- Visual state indication (opacity scale, strikethrough, color-coded badges) means a user who can't hear can still see structural state

### Testing Strategy

**Responsive (future, when targeting tablet/mobile):**
- Test on actual devices, not just browser emulation
- Verify touch targets meet 44px minimum
- Test horizontal/vertical scroll interactions with touch

**Accessibility (v1):**
- Automated: Flutter accessibility checker, contrast ratio validation in theme
- Manual: Tab through entire workspace with keyboard only
- Screen reader: Test with platform screen reader (VoiceOver on macOS, Orca on Linux, Narrator on Windows)
- Focus: Verify focus rings visible on every interactive element after theming

### Implementation Notes

- Use `MediaQuery` breakpoints but don't over-engineer for v1 — desktop layout is the only shipped layout
- Build the responsive shell (breakpoint-aware layout) so tablet/mobile can be added without restructuring
- Keep all accessibility labels in a centralized place for easy i18n later
- ServerConsole panel width should be adjustable (draggable divider) on desktop

## Keybinding System (Future)

Justifier will consume an external Flutter keybinding package (to be built as a separate shared library). Not in v1 scope — all interactions are mouse/touch only initially.

**Architectural requirement:** All user-triggerable actions must be invokable by name (command pattern), not buried in widget callbacks. This allows a keybinding layer to trigger any action without UI refactoring.

**Target features (from the shared package):**
- Vim-style modal keybindings (normal mode, insert mode, etc.)
- Leader key / chord sequences
- Hint overlay showing available bindings
- User-configurable binding maps
- Shared across multiple Flutter apps
