# Phase 4d: JI Lattice Explorer

A 2D interactive lattice for navigating just intonation intervals, spawning voices by clicking nodes, and visualizing active voices and their harmonic relationships.

## Core concept

The lattice serves two roles equally: **navigator** (browse and spawn voices) and **map** (see where existing voices sit relative to each other). It is not a separate mode — it lives alongside the voice cards in a resizable panel.

## UI layout

### Resizable side panel

The lattice replaces the current console placeholder. The main area becomes two panes:

- **Voice panel** (left) — existing wave list / empty state
- **Lattice panel** (right) — new lattice widget
- **Drag divider** between them — 4px visible bar, 8px hit target, vertical

Divider position stored as a fraction (0.0 = lattice hidden, 1.0 = lattice fullscreen). Double-click divider to toggle between last position and collapsed. This means the lattice can be fullscreen when desired.

### Controls overlay

Semi-transparent dark strip along the top of the lattice panel. Contains:

- View mode switcher (Nested / Projection + prime axis dropdowns)
- Octave spinner (sets octave for newly spawned voices, default 1)
- Home button (snap to 1/1, default zoom)
- Label toggle (persistent cents+Hz vs hover-only)

Uses `AppTheme.monoSmall` text style. Compact — should not compete with the lattice.

## Data model

### LatticeNode

Represents one position on the lattice:

| Field | Type | Description |
|---|---|---|
| ratio | (int, int) | Numerator/denominator in lowest terms |
| primeFactors | Map<int, int> | Prime exponent map, e.g. 15/8 = {2: -3, 3: 1, 5: 1} |
| position | Offset | Computed 2D coordinate based on current view mode |

### LatticeState (Riverpod)

| Field | Type | Default | Description |
|---|---|---|---|
| viewMode | LatticeViewMode | nested | Nested or projection(primeX, primeY) |
| panOffset | Offset | (0, 0) | Current pan position |
| zoom | double | 1.0 | Current zoom level (min 0.3, max 4.0) |
| spawnOctave | int | 1 | Octave for new voices (range 0-9, matching Voice.octave) |
| expandedNodes | Set<(int,int)> | {} | Ratios whose higher-prime neighbors are shown |
| previewRatio | (int,int)? | null | Currently auditioned ratio |
| hoveredRatio | (int,int)? | null | Currently hovered node |
| dividerFraction | double | 0.6 | Panel split position |

### LatticeViewMode

Sealed class:

- `LatticeViewMode.nested` — 5-limit base grid, higher primes expandable
- `LatticeViewMode.projection(int primeX, int primeY)` — any two primes as axes

### Provider relationship

The lattice provider reads `workspaceProvider` to know which ratios have active voices and which wave colors to use. Voice mutations (add/remove) still go through `workspaceProvider`. The lattice provider is not a voice authority — it's a view over voice state plus its own UI state (pan, zoom, expanded nodes, preview).

## Lattice geometry

### Coordinate system

Each prime gets a direction vector in 2D:

**Nested mode (5-limit base):**
- Prime 3: X axis (1, 0) — each step right = multiply by 3/2
- Prime 5: Y axis (0, 1) — each step up = multiply by 5/4
- Node at grid position (a, b) = ratio `(3/2)^a * (5/4)^b`, octave-reduced to [1, 2)
- Equivalently: `3^a * 5^b * 2^k` where k is chosen to place the result in [1, 2)

**Projection mode:**
- Selected primeX and primeY become the X and Y axes
- Node at (a, b) = `(primeX/2^floor(log2(primeX)))^a * (primeY/2^floor(log2(primeY)))^b`, octave-reduced
- Same structure, different primes

### Octave reduction

All ratios are octave-reduced: repeatedly multiply or divide by 2 until the ratio falls in [1, 2). So 3/1 and 3/2 map to the same lattice node. The `spawnOctave` selector determines which actual octave a spawned voice gets.

### Node generation

The lattice is infinite. Only nodes within the visible viewport are generated:

1. From `panOffset` and `zoom`, compute the visible rectangle in lattice coordinates
2. Enumerate integer grid positions (a, b) within that rectangle
3. For each, compute the octave-reduced ratio and create a `LatticeNode`
4. Recompute on every pan/zoom change — cheap for ~200 nodes

### Higher-prime expansion (nested mode)

When a node is in `expandedNodes`:

- Compute neighbors: multiply the node's ratio by p/1 and 1/p for primes 7, 11, 13 (and beyond if configured), octave-reduce each
- Position neighbors radially around the parent at fixed angular offsets — like petals around a flower
- Offsets are small relative to grid spacing so they cluster near their parent
- Expanded neighbors are themselves expandable (recursive), though rarely more than one level deep in practice
- Nodes with available higher-prime neighbors show a small expand indicator

## Rendering architecture

Hybrid: CustomPainter for lines/grid, Flutter widgets for interactive nodes.

### Layer stack (inside InteractiveViewer)

1. **Background canvas** (CustomPainter):
   - Subtle grid lines for orientation
   - Connection lines between active voices (thin, #6b00ff at 60% opacity, straight)
   - 1/1 crosshair lines extending to viewport edges (violet, low opacity)

2. **Node widget layer** (Stack + Positioned):
   - Each visible LatticeNode is a small widget (~24px circle)
   - Positioned based on lattice coordinate * zoom + panOffset
   - Only viewport-visible nodes get widgets

3. **Controls overlay** (not transformed by pan/zoom):
   - View mode, octave, home button, label toggle

### Pan/zoom

`InteractiveViewer` with `TransformationController`, `constrained: false` for infinite panning.

- Scroll wheel = zoom (centered on cursor)
- Click-drag on empty space = pan
- Pinch = zoom (trackpad)
- Home button or H key = snap to 1/1, default zoom

## Interaction model

### Node click behaviors

| Action | Effect |
|---|---|
| Click empty node | Spawn voice into focused wave (spawn octave, sine waveform) |
| Click active node | Select/scroll to that voice card in the voice panel |
| Alt-click empty node | Start preview (toggle audition on) |
| Alt-click previewing node | Stop preview |
| Alt-click different node | Move preview to new node |
| Right-click node | Expand/collapse higher-prime neighbors (nested mode) |
| Hover any node | Show tooltip (ratio, cents from 1/1, Hz at current reference) |

### Wave focus

Voices spawn into the currently focused wave:

- Add `focusedWaveId` (`String?`) to `WorkspaceState`
- Clicking a wave column header or voice card sets focus
- Focused wave gets a subtle visual indicator (brighter header border)
- If no wave exists, clicking a lattice node creates one first
- If the focused wave is deleted, `focusedWaveId` resets to the first remaining wave (or null if none)
- On app launch with no waves, `focusedWaveId` is null

### Preview (audition)

Alt-click toggles a preview tone:

- `LatticeNotifier` manages preview directly through `AudioEngine` — calls addVoice/removeVoice without touching `WorkspaceState`
- Preview voice: sine waveform, spawn octave, fixed amplitude
- Counts toward 32-voice engine cap but doesn't appear in voice cards
- If voice pool is full (32 active), preview silently does nothing — no error dialog
- Toggling off or committing (regular click) kills the preview

### Active node ambiguity

Multiple voices can share a ratio (different octaves, waveforms). The node shows as active if any voice has that ratio. Clicking an active node selects the first matching voice by wave list order, then voice list order within the wave. Not worth a picker for this edge case.

### Voice pool full

When all 32 engine voice slots are occupied, clicking a lattice node to spawn silently does nothing. Same for preview. No error dialog — the voice card count in the top bar already shows capacity.

## Visual design

Prometheus theme: black background, #00ff00 green, #6b00ff violet.

### Node states

| State | Appearance |
|---|---|
| Empty | 24px circle, #1a1a1a fill, thin green border (30% opacity) |
| Hovered | Border brightens to full green, tooltip appears |
| Active voice | Filled with wave color, 28px, subtle glow |
| Multiple voices | Active style + small count badge |
| Preview/audition | Green pulsing ring animation |
| Higher-prime child | 18px, same styling, short line to parent |
| 1/1 node | 30px, double ring — outer green, inner violet, green outer. Violet crosshair lines to viewport edges |

### Connection lines

Lines connect active voice nodes that are **grid-adjacent** (one step apart on any axis in the current view). Not all-pairs — only direct lattice neighbors. #6b00ff at 60% opacity, straight. No curves, no labels (the geometry is the point).

### Expand indicator

Small triangle or `+` glyph at the node edge for nodes with available higher-prime neighbors. Green at low opacity. Disappears when expanded.

## Integration

### App shell changes

- Replace `_ConsolePanel` with resizable two-pane layout
- `_VoicePanel` (left) + `_LatticePanel` (right) + drag divider
- Console info (engine stats) moves to top bar tooltip or lattice panel footer

### Voice-lattice sync

Bidirectional read, unidirectional write:

- Lattice reads `workspaceProvider` to highlight active nodes and draw connections
- Voice cards remain the authority for editing voice parameters
- Removing a voice from voice card un-highlights the lattice node
- Lattice only writes to `workspaceProvider` for addVoice (spawn) and indirectly for removeVoice (if we add that later)

### No new FFI needed

All engine functions required (addVoice, removeVoice, setFrequency, setGate, etc.) already exist and are wired through the FFI bridge.

## Deferred

- Consonance shading overlay
- Beating rate indicators
- Interval labels on connection lines
- Animated transitions when switching projection axes
- Lattice-driven voice removal (right-click active node to remove)
- Persistence of lattice state (pan position, expanded nodes)
- Auto-scroll to voice card when clicking active lattice node
- Keyboard shortcuts beyond H (zoom, expand, toggle labels, etc.)
- Console/engine stats display (relocate to top bar tooltip or lattice footer)
- Configurable default waveform for lattice-spawned voices
