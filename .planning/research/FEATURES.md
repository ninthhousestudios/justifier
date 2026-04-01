# Feature Landscape — JI Exploration / Microtonal Synthesis Tools

**Domain:** Just intonation exploration and audio synthesis desktop tool
**Researched:** 2026-04-01
**Confidence:** MEDIUM — primary tools surveyed via web search; direct tool use limited

## Ecosystem Survey

Tools examined:
- **Entonal Studio** (Node Audio) — MIDI plugin/host, JI lattice visualization, scale editing, radial graph UI, tuning export
- **Wilsonic** (iOS) — Erv Wilson scale systems, visualization, Scala export
- **Hayward Tuning Vine** — JI ratio exploration, visual tuning tree
- **Scala** — text-based tuning format tool, reference standard for scale interchange
- **AudioNodes** — node-based web audio graph, visual wiring, modular approach

Justifier's position: not a tuning plugin (like Entonal), not a scale editor (like Scala/Wilsonic). It is a **direct synthesis tool** — you hear JI ratios as layered continuous tones, not as retuned keyboard notes. The reference use case is drone/soundscape construction, not melody composition.

---

## Table Stakes

Features users expect from any JI synthesis/exploration tool. Missing = product feels incomplete or untrustworthy.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Ratio input (numerator/denominator) | JI is defined by ratios — any other input method is an abstraction that loses the point | Low | Already specced: RatioInput component |
| Derived Hz display | Users need to know what frequency they're actually producing — trust the math | Low | Already specced: computed from reference × ratio × 2^octave |
| Cents display | Standard unit for comparing intervals — users from music theory backgrounds expect it | Low | Derived value alongside Hz |
| Reference frequency control | JI is relative — the reference pitch must be explicit and user-controlled | Low | Already specced: default 172.8 Hz |
| Multiple simultaneous voices | Single tone is not exploration; layering is where JI relationships emerge | Medium | Already specced: WaveColumn + VoiceCard model |
| Per-voice amplitude control | Balancing layers is immediate need — without it, everything fights | Low | Already specced: mul parameter |
| Per-voice on/off toggle | Users need to A/B compare individual voices against the whole | Low | Already specced: enabled bool |
| Waveform selection | Sine is a starting point; timbre affects how intervals sound — triangle/saw/pulse matter | Low | Already specced: 7 waveforms |
| Preset save/load | Sessions are valuable — losing work kills the tool | Medium | Already specced: JSON workspace |
| Panic/silence button | Audio tools must have an emergency stop — no exceptions | Low | Already specced: one-click, no confirmation |
| Real-time parameter feedback | Sound must respond immediately to changes — any latency breaks the exploration loop | High | Core architectural requirement; Faust via FFI |

---

## Differentiators

Features that make this tool stand out. Not expected from the category, but valued when present.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Sleep-before-destroy pattern | Exploration tools encourage experimentation — accidental deletion of a tuned voice is costly; graceful undo without a full undo stack | Medium | Already specced; architecturally elegant, unique to Justifier |
| Wave grouping with master controls | Organise layers into named semantic groups (sine ocean, noise texture) with collective mute/solo/volume | Medium | Already specced: WaveColumn |
| OSC console / node tree visibility | Exposes the synthesis engine's actual state — educational and debuggable; most tools hide this entirely | Medium | Already specced: ServerConsole |
| Descriptor-driven voice UI | New SynthDef = new card layout, zero UI code changes — extensibility without complexity | High | Already specced; this is a real differentiator for a tool meant to grow |
| Waveform crossfade on switch | Switching waveform mid-session without a click/pop or silence is a quality-of-life detail most tools skip | Medium | Already specced; requires two overlapping nodes |
| Octave control per voice | Placing a ratio in different octaves changes the harmonic context dramatically — this is core JI exploration | Low | Already specced: octave int 0–8 |
| Beat offset (mod) per voice | Micro-detuning between two voices creates beating — central to JI soundscapes and drone music | Low | Already specced: mod float 0–10 Hz |
| Pan per voice + wave pan offset | Spatial placement of ratios in stereo field; wave-level offset composited with per-voice pan | Low-Medium | Already specced; additive pan model is thoughtful |
| Collapse/expand waves | Configure → play → collapse → move on is a real workflow, not just space-saving | Low | Already specced; correctly identified as Phase 1 not Polish |
| Reference frequency as first-class parameter | Most tools treat root pitch as setup; making it a live knob invites questions like "what if the drone is 128 Hz instead?" | Low | Already specced |

---

## Anti-Features

Features that make these tools worse. Deliberate non-choices.

| Anti-Feature | Why It Hurts | What to Do Instead |
|--------------|--------------|-------------------|
| 12-TET keyboard as primary interface | Forces microtonal tuning into a tempered mental model — the interface contradicts the content; users think in semitones not ratios | Ratio input + Hz/cents readout as the primary representation |
| Complex routing UI before any sound | Node-graph tools (AudioNodes) require the user to wire a signal path before hearing anything — friction kills exploration | Opinionated defaults: voice created with sensible defaults, immediately audible |
| Modal dialogs for parameter entry | Interrupts the "tweak and hear" loop — any modal kills flow | Inline editing everywhere; commit on blur/Enter |
| Undo/redo history stack | Full undo in a real-time audio tool is architecturally complex and in practice users don't trust it for audio state; false safety | Sleep-before-destroy covers the actual use case (accidental deletion) without the complexity |
| Scale/tuning library as the entry point | Entonal Studio / Scala lead with scale management — right tool for composition, wrong tool for exploration | Start with a blank canvas + reference pitch; tuning emerges from ratio manipulation |
| Auto-save / project management | File management UI is not why users open this tool; creates friction and complexity | Explicit save/load with named presets; user controls persistence |
| Effects chains (reverb, delay, etc.) | Expands scope before the core voice model is proven; effects routing is a separate design problem | Defer to v2; pure synthesis first |
| MIDI input in v1 | Changes the paradigm from drone/soundscape to performance instrument; different user, different design | Desktop mouse/keyboard interaction only for v1 |
| Mobile-first layout | Touch targets and screen density conflict with the dense-controls JI exploration UX; compromises the desktop tool to serve a future use case | Desktop-first; build responsive shell but ship desktop only |
| Real-time FFT / spectral analysis | Interesting but not what makes JI exploration work; complexity cost is high, payoff is unclear for target use case | Defer; the ears + Hz readout is sufficient for v1 |
| Per-voice effects (filter as default) | Filter should be a waveform type (filtered-noise), not a generic insert on every voice — scope creep that muddies the voice model | Filter params only on filtered-noise descriptor |

---

## Feature Dependencies

```
Reference frequency
  └── Ratio input
        └── Hz/cents display
              └── Voice card (all per-voice controls)
                    └── Wave grouping (master controls, mute/solo)
                          └── Preset save/load

Real-time audio engine (Faust/libfaust)
  └── All parameter changes (every voice parameter feeds this)
  └── Panic button
  └── Waveform crossfade

Sleep-before-destroy
  └── Voice deletion UX
  └── Wave deletion UX
  (depends on: audio engine gate 0 / doneAction: 0 pattern)

Descriptor-driven UI
  └── VoiceCard rendering
  └── Waveform selector (7 descriptors shipped)
  └── Extensibility path (future SynthDefs require no Dart changes)

OSC console / node tree
  └── ServerConsole component
  └── Depends on: scsynth connection, /g_queryTree polling
```

---

## MVP Recommendation

The specced v1 feature set is already well-calibrated. Priorities in order:

1. Real-time audio engine + ratio → Hz (the core loop; everything else is UI chrome without this)
2. Voice card with ratio, octave, amplitude, enable/disable (minimum viable exploration)
3. Wave grouping with collapse/expand (immediately needed once you have more than 3 voices)
4. Panic button (required before any demo or sharing)
5. Preset save/load (required before sustained sessions)

Defer (correctly already deferred):
- JI lattice visualization — valuable but v2; Entonal Studio already owns this; don't compete before core loop is solid
- MIDI input — different paradigm, different user
- Effects routing — v2
- Undo/redo — sleep-before-destroy is the right solution for the actual problem

The one v1 feature that could be descoped without regret if time pressure hits: **ServerConsole**. It is differentiating and educational, but the app works without it. It's Phase 2 in the design spec — that ordering is correct.

---

## What Existing Tools Don't Provide

Based on survey of Entonal Studio, Wilsonic, Hayward Tuning Vine, AudioNodes:

1. **Continuous drone synthesis with JI ratios** — existing tools are mostly tuning tools (retune a keyboard) or scale explorers (find a scale). None provide "add a voice at ratio 5/4, hear it immediately, layer 8 of them, adjust and listen."

2. **Soundscape-as-composition model** — the wave grouping idea (sine ocean, noise texture) is a compositional metaphor that none of the surveyed tools express. Entonal is about harmony/melody; Justifier is about texture/drone.

3. **Transparency about what the engine is doing** — the ServerConsole showing OSC messages and the live node tree is genuinely unusual. Most synthesis tools hide their internals completely.

4. **Sleep-before-destroy** — the specific UX pattern of "fade out but keep alive, give 10s window" is not seen in comparable tools. Standard is either immediate delete or a full undo stack.

5. **Exploration-first defaults** — no routing setup, no scale import required, no preset required. Open the app, reference Hz is set, add a voice, it makes sound. Entonal Studio and AudioNodes both require significant setup before sound.

---

## Sources

- [Entonal Studio — Node Audio](https://node.audio/products/entonal-studio) — product page, feature list (MEDIUM confidence)
- [Entonal Studio — KVR Audio](https://www.kvraudio.com/product/entonal-studio-by-node-audio) — user reviews and feature discussion (MEDIUM confidence)
- [Wilsonic — App Store](https://apps.apple.com/us/app/wilsonic/id848852071) — feature list (MEDIUM confidence)
- [Synthtopia — Wilsonic review](https://www.synthtopia.com/content/2015/01/05/new-app-wilsonic-lets-you-explore-profound-microtonal-harmonies/) — feature overview (LOW confidence — 2015, may be outdated)
- [Xenharmonic Wiki — List of music software](https://en.xen.wiki/w/List_of_music_software) — ecosystem survey (MEDIUM confidence)
- Project context: `.planning/PROJECT.md`, `docs/superpowers/specs/2026-03-26-justifier-design.md` (HIGH confidence — primary source)
