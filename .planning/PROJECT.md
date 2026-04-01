# Justifier

## What This Is

A desktop application for exploring just intonation through direct manipulation of sound. You build soundscapes by layering voices tuned to JI ratios from a reference frequency, tweaking parameters, and hearing the result in real time. The core goal is discoverability through interaction — every parameter is visible and tweakable, so you learn what's possible by experimenting.

## Core Value

Real-time audio feedback from manipulating JI ratios — hear the math immediately.

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] Create and destroy voices tuned to JI ratios from a reference frequency (default 172.8 Hz)
- [ ] Select waveform type per voice (sine, triangle, saw, square, pulse, noise, FM)
- [ ] Group voices into waves for organization and collective control
- [ ] Real-time audio synthesis via embedded libfaust (Dart FFI to C wrapper)
- [ ] Display derived values (Hz, cents) on every voice card
- [ ] Collapse/expand voice cards (configure > play > collapse workflow)
- [ ] Sleep-before-destroy pattern for deletions (10s undo window)
- [ ] Panic button — instant silence, one click, no confirmation
- [ ] Save/load workspaces as JSON presets

### Out of Scope

- Effects routing (buses, sends, chains) — v2, after core voice model is solid
- Pattern sequencing / automation — v2
- Lattice visualization of JI relationships — v2, explore Entonal Studio first
- Mobile/tablet UI — desktop-only v1, responsive shell for future
- Keybinding system — separate shared Flutter package (Quill), not in Justifier scope
- Node-based / visual wiring interface — building blocks UI instead

## Context

- Flutter desktop app, building on Linux, testing on Linux/macOS/Windows
- Audio engine: Faust via libfaust, embedded through Dart FFI. Integration approach needs research — no existing Flutter/Faust binding exists
- Design spec completed: `docs/superpowers/specs/2026-03-26-justifier-design.md` — M3 dark theme, 6 custom components, WCAG AA
- Architecture exploration: `docs/superpowers/specs/2026-03-31-architecture-options.md` — considered SuperCollider (abandoned, GPL, wrong paradigm), Csound, Rust self-synthesis. Chose Faust for embeddability and DSP-focused design
- Josh's SC code at `~/w/astro/soft/sc/base.sc` — sine ocean and noise ocean soundscapes, reference for what the synthesis should produce
- Existing tools in the space: Entonal Studio (JI lattice), Hayward Tuning Vine (JI exploration), AudioNodes (visual modular)

## Constraints

- **Audio engine**: Faust via libfaust — chosen for embeddability, functional DSP model, non-GPL license
- **Platform**: Desktop-only v1 (Linux primary, macOS/Windows tested)
- **UI framework**: Flutter with Material 3, heavily themed dark/dense
- **Architecture**: All actions must be invokable by name (command pattern) for future keybinding integration

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Faust over SuperCollider | SC is designed to be the app, not be wrapped; GPL license; Faust embeds natively via C++ | -- Pending |
| Faust over Csound | Faust's functional DSP model is a closer fit for "define synth graphs and manipulate from GUI" | -- Pending |
| Faust over Rust self-synthesis | Faust is a proven DSP language; self-synthesis is fun but risky for timeline | -- Pending |
| Flutter + Dart FFI for integration | Flutter is the UI framework; FFI is the standard way to call C/C++ from Dart | -- Pending |
| Desktop-only v1 | Focus on the core experience without mobile constraints | -- Pending |
| Command pattern for all actions | Enables future keybinding package (Quill) to plug in without refactoring | -- Pending |

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd:transition`):
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to Key Decisions
5. "What This Is" still accurate? → Update if drifted

**After each milestone** (via `/gsd:complete-milestone`):
1. Full review of all sections
2. Core Value check — still the right priority?
3. Audit Out of Scope — reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-04-01 after initial project definition*
