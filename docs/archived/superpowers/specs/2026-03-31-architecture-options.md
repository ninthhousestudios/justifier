# Justifier — Architecture Options for Discussion

**Date:** 2026-03-31
**Status:** Draft — seeking input

## What This Is

Justifier is a desktop app for exploring just intonation through direct manipulation of sound. You build soundscapes by layering voices tuned to JI ratios from a reference frequency, tweaking parameters, and hearing the result in real time.

The core goal is **discoverability through interaction** — every parameter is visible and tweakable, so you learn what's possible by experimenting.

## Background

The original design had a Flutter GUI talking directly to SuperCollider's audio server (scsynth) via OSC, with 7 fixed waveform types defined as JSON descriptors. Each voice was a card with knobs. Waves grouped voices. Presets saved everything as JSON.

This worked for the narrow case of "sine layers at JI ratios with beating," but it hit a ceiling fast. Every new capability (effects, bus routing, patterns, filtering) would require new Dart code. The GUI was reimplementing a limited subset of what SuperCollider's language (sclang) already does.

The next idea was to put sclang in between (Flutter → sclang → scsynth), which opens up SC's full power. But that means managing two separate processes via IPC, inventing a Flutter↔sclang bridge that doesn't exist, and working within SC's GPL license. SuperCollider is designed to *be* the application — wrapping it inside another app is swimming upstream.

## Where Things Stand Now

The JI exploration interface is the unique and interesting part. The synthesis engine underneath is interchangeable. So rather than commit to SuperCollider, I'm exploring three options for the audio engine:

### Option 1: Faust

[Faust](https://faust.grame.fr/) is a functional language specifically for DSP. You write signal processing as mathematical functions, and it compiles to C++, WebAssembly, VST plugins, etc. The key advantage: it can be **embedded** directly in the app via its C++ output and `libfaust` — no separate process. It's lower-level than sclang (no built-in patterns or live-coding), but for "define synth graphs and manipulate them from a GUI" it may be a more natural fit.

### Option 2: Csound

[Csound](https://csound.com/) has the largest library of unit generators of any audio environment, a very permissive license (LGPL), and crucially: a **C API designed for embedding**. You can run Csound inside your app as a library. A Dart FFI binding would be straightforward. The language is more verbose than sclang but extremely mature and well-documented.

### Option 3: Self-Synthesis (Rust/Dart)

Build the synthesis from scratch using Rust audio crates (cpal, fundsp) called from Dart via FFI. Total control, zero external dependencies, and honestly it just sounds fun. [fundsp](https://github.com/SamiPerttu/fundsp) has a combinator syntax close to Faust. The tradeoff is building everything yourself — but for JI soundscapes (which are fundamentally "oscillators at specific frequency ratios"), the synthesis requirements may be simple enough that this is viable.

## Existing Tools Worth Knowing About

- **[Entonal Studio](https://entonal.studio/)** — Microtonal plugin/standalone with a lattice view showing JI ratio relationships, color-coded by prime. Retunes any VST/AU instrument. Solves "explore JI visually" well — worth trying before building that part from scratch.
- **[Hayward Tuning Vine](https://www.tuningvine.com/)** — A JI exploration interface with color-coded prime axes and pitch mapped to vertical position. Interesting UI concepts for making ratios visual and navigable.
- **[AudioNodes](https://www.audionodes.com/)** — Browser-based modular synth with node-based visual wiring. Not JI-specific, but the closest existing example of "visual audio programming without code."

## Questions I'd Love Your Input On

1. **Which audio engine approach makes sense to you?** Faust (embedded DSP compiler), Csound (embedded mature engine), or building from scratch in Rust? What would you pick and why?

2. **What does SuperCollider / Csound / Faust do that I'm probably not thinking about?** I've been working with basic oscillator layers, buses, and reverb. What capabilities would change how I think about this if I understood them? (Granular synthesis? Spectral processing? Physical modeling? Something else?)

3. **How important is effects routing?** I experimented with buses and reverb in SC but mostly spent time on voices. Is effects processing (chains, sends, parallel routing) a core need for soundscape work, or a nice-to-have?

4. **Is the "building blocks" UI approach fundamentally limiting?** The idea is curated audio blocks (voices, effects, modulators) that compose visually — not node-based wiring, but higher-level than code. Would that frustrate you, or is it what you'd actually want?

5. **Is just intonation as the organizing principle interesting, or too niche?** Should the tool be broader — general microtonal, or even general synthesis — with JI as one mode?

6. **Do you know tools that do something similar?** Not DAWs, but specifically: a GUI for exploratory sound design aimed at discovery rather than production.

7. **Anything else that comes to mind.**
