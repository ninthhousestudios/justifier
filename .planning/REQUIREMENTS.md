# Requirements: Justifier

**Defined:** 2026-04-01
**Core Value:** Real-time audio feedback from manipulating JI ratios — hear the math immediately.

## v1 Requirements

### Audio Engine

- [ ] **AUD-01**: App produces real-time audio output via miniaudio on Linux, macOS, and Windows
- [ ] **AUD-02**: 7 waveform types available (sine, triangle, saw, square, pulse, noise, FM), compiled from Faust .dsp files at build time
- [ ] **AUD-03**: UI parameter changes reach the audio thread without glitches via lock-free message passing
- [ ] **AUD-04**: Voice slots are pre-allocated at startup to avoid audio-thread allocation

### Voice Control

- [ ] **VOC-01**: User can create a voice tuned to a JI ratio from a reference frequency (default 172.8 Hz)
- [ ] **VOC-02**: Voice card displays derived Hz and cents values, updated live
- [ ] **VOC-03**: User can adjust per-voice amplitude via slider
- [ ] **VOC-04**: User can mute/solo individual voices
- [ ] **VOC-05**: User can select waveform type per voice from dropdown
- [ ] **VOC-06**: Deleting a voice triggers sleep-before-destroy (10s undo window, fade out, then destroy)

### Workspace

- [ ] **WRK-01**: User can create named wave groups to organize voices
- [ ] **WRK-02**: User can collapse/expand wave groups (configure > play > collapse workflow)
- [ ] **WRK-03**: Panic button instantly silences all audio with one click, no confirmation
- [ ] **WRK-04**: User can save workspace state as JSON preset
- [ ] **WRK-05**: User can load a previously saved workspace preset

### UI Foundation

- [ ] **UI-01**: Material 3 dark theme, dense layout, desktop-optimized
- [ ] **UI-02**: 6 custom components: VoiceCard, WaveColumn, RatioInput, WaveformSelector, ConnectionStatusBadge, WorkspaceHeader
- [ ] **UI-03**: ServerConsole panel showing audio engine state (right side on desktop)
- [ ] **UI-04**: All actions invokable by name via command pattern (for future keybinding integration)

## v2 Requirements

### Extended Synthesis

- **EXT-01**: Runtime Faust compilation — user-defined .dsp programs loaded at runtime via libfaust interpreter
- **EXT-02**: Effects routing (buses, sends, effect chains)
- **EXT-03**: Pattern sequencing / automation

### Visualization

- **VIS-01**: JI lattice visualization showing ratio relationships
- **VIS-02**: Waveform display per voice (oscilloscope view)

### Platform

- **PLT-01**: Tablet/mobile responsive UI
- **PLT-02**: Community preset sharing / library

## Out of Scope

| Feature | Reason |
|---------|--------|
| Node-based visual wiring | Building-blocks UI is the chosen paradigm — simpler, more guided |
| DAW integration / VST export | Standalone app, not a plugin |
| MIDI input | Not needed for JI ratio exploration; future consideration |
| Equal temperament mode | JI is the organizing principle; ET defeats the purpose |
| User accounts / cloud sync | Local-first, JSON presets are sufficient |
| Keybinding system | Separate shared Flutter package (Quill), not in Justifier scope |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| AUD-01 | - | Pending |
| AUD-02 | - | Pending |
| AUD-03 | - | Pending |
| AUD-04 | - | Pending |
| VOC-01 | - | Pending |
| VOC-02 | - | Pending |
| VOC-03 | - | Pending |
| VOC-04 | - | Pending |
| VOC-05 | - | Pending |
| VOC-06 | - | Pending |
| WRK-01 | - | Pending |
| WRK-02 | - | Pending |
| WRK-03 | - | Pending |
| WRK-04 | - | Pending |
| WRK-05 | - | Pending |
| UI-01 | - | Pending |
| UI-02 | - | Pending |
| UI-03 | - | Pending |
| UI-04 | - | Pending |

**Coverage:**
- v1 requirements: 19 total
- Mapped to phases: 0
- Unmapped: 19

---
*Requirements defined: 2026-04-01*
*Last updated: 2026-04-01 after initial definition*
