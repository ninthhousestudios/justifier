# Roadmap: Justifier

## Overview

Four phases, bottom-up. The audio thread is the hardest constraint, so the C engine is proven in isolation before any Dart code exists. Once C is solid, the Dart integration layer (FFI + state) is built on top. Only then does the UI go up — first the voice interaction loop, then workspace organization and persistence. At the end, the full stack is wired: manipulate a JI ratio, hear the change, save the session.

## Phases

- [ ] **Phase 1: C Audio Engine** - C audio foundation with Faust compile-time DSP, proven before any Flutter code
- [ ] **Phase 2: Dart Integration** - FFI bindings, AudioBridge, Riverpod state layer, and command pattern
- [ ] **Phase 3: Voice UI** - Full voice interaction loop: create, tune, shape, and hear voices end-to-end
- [ ] **Phase 4: Workspace** - Wave group organization, panic button, and preset save/load

## Phase Details

### Phase 1: C Audio Engine
**Goal**: A working C audio layer produces all 7 waveforms with correct threading guarantees — independently of Flutter
**Depends on**: Nothing (first phase)
**Requirements**: AUD-01, AUD-02, AUD-03, AUD-04
**Success Criteria** (what must be TRUE):
  1. A C test harness can produce audible output (sine wave) on Linux without Flutter present
  2. All 7 waveform types (sine, triangle, saw, square, pulse, noise, FM) play correctly — compiled from Faust .dsp files at build time via CMake
  3. Parameter changes (frequency, amplitude) applied via SPSC queue reach the audio thread within one buffer cycle, with no audible glitches
  4. Voice slots are pre-allocated at startup; creating and destroying voices does not allocate on the audio thread
  5. A dedicated atomic `is_silent` flag silences all output within one buffer cycle when set
**Plans**: TBD

### Phase 2: Dart Integration
**Goal**: The C audio engine is fully callable from Dart — with a command-driven state layer that is the single source of truth for all app state
**Depends on**: Phase 1
**Requirements**: UI-04
**Success Criteria** (what must be TRUE):
  1. `ffigen`-generated Dart bindings compile cleanly against `justifier_audio.h`
  2. `AudioBridge` can create a voice, set its frequency and amplitude, and destroy it — verified by audio output from Dart code (no UI yet)
  3. All audio operations are invokable by name via `CommandRegistry` — no direct AudioBridge calls from widgets
  4. Riverpod notifiers update app state and call AudioBridge in response to commands — UI can watch state without knowing about audio
**Plans**: TBD

### Phase 3: Voice UI
**Goal**: Users can create voices tuned to JI ratios, shape them, and hear the result in real time — the core interaction loop is complete
**Depends on**: Phase 2
**Requirements**: VOC-01, VOC-02, VOC-03, VOC-04, VOC-05, VOC-06, UI-01, UI-02, UI-03
**Success Criteria** (what must be TRUE):
  1. User can create a voice with a JI ratio (numerator/denominator) from the default 172.8 Hz reference and hear it immediately
  2. VoiceCard displays live Hz and cents values that update as the ratio changes
  3. User can adjust amplitude, mute/solo, and select waveform type per voice — all changes are audible within one buffer cycle
  4. Deleting a voice triggers a 10-second sleep-before-destroy: voice fades out, a countdown is visible, and the delete can be undone before the slot is reclaimed
  5. The app renders in M3 dark theme with dense desktop layout; all 6 custom components exist and are functional
  6. ServerConsole panel shows live audio engine state (voice count, xrun count, engine health)
**Plans**: TBD
**UI hint**: yes

### Phase 4: Workspace
**Goal**: Users can organize voices into named wave groups and save/restore sessions — the tool is usable for sustained work
**Depends on**: Phase 3
**Requirements**: WRK-01, WRK-02, WRK-03, WRK-04, WRK-05
**Success Criteria** (what must be TRUE):
  1. User can create named wave groups and assign voices to them; groups provide collective visual organization
  2. User can collapse a wave group to hide its voice cards and expand it to reveal them
  3. Panic button silences all audio with one click — no confirmation dialog, active within one buffer cycle
  4. User can save the current workspace (all voices, ratios, waveforms, amplitudes, group assignments) as a named JSON preset file
  5. User can load a previously saved preset and the workspace is restored exactly, including all voice parameters
**Plans**: TBD
**UI hint**: yes

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. C Audio Engine | 0/? | Not started | - |
| 2. Dart Integration | 0/? | Not started | - |
| 3. Voice UI | 0/? | Not started | - |
| 4. Workspace | 0/? | Not started | - |
