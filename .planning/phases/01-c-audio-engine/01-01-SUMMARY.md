---
phase: 01-c-audio-engine
plan: 01
subsystem: audio
tags: [faust, miniaudio, readerwriterqueue, cmake, dsp, c, cpp]

requires: []

provides:
  - "7 Faust .dsp files for all waveform types (sine, triangle, saw, square, pulse, noise, FM)"
  - "native/src/justifier_audio.h: complete public C API for Phase 2 FFI binding"
  - "native/src/voice_slot.h: VoiceSlot, ControlMessage, all C type definitions"
  - "native/CMakeLists.txt: build system with Faust codegen, library, and test targets"
  - "native/include/: vendored miniaudio v0.11.25, readerwriterqueue, atomicops"
  - "Stub sources (audio_engine.c, faust_wrapper.cpp, test_audio.c) for CMake configure"

affects:
  - "01-02 (audio engine implementation: imports justifier_audio.h, voice_slot.h, CMakeLists.txt)"
  - "01-03 (test harness: uses test_audio.c, links justifier_audio)"
  - "02-dart-ffi (ffigen generates bindings from justifier_audio.h)"

tech-stack:
  added:
    - "Faust 2.85.5 compiler (built from source to ~/.local/bin/faust, CPP backend enabled)"
    - "miniaudio v0.11.25 (vendored header-only)"
    - "readerwriterqueue (Cameron Desrochers, vendored header-only)"
    - "atomicops.h (readerwriterqueue dependency, vendored)"
  patterns:
    - "Faust DSP parameters use nentry() not hslider/vslider (pitfall avoidance)"
    - "si.smoo on all smoothed params (freq, amp, pan, mod params)"
    - "en.are gate envelope for attack/release control (D-05)"
    - "Mono DSP output; C wrapper handles stereo panning (D-05)"
    - "Unique class names via -cn flag (SineDSP, TriangleDSP, etc.) prevent mydsp collision"
    - "-i flag inlines Faust stdlib, no runtime library path dependency"

key-files:
  created:
    - "native/dsp/sine.dsp"
    - "native/dsp/triangle.dsp"
    - "native/dsp/saw.dsp"
    - "native/dsp/square.dsp"
    - "native/dsp/pulse.dsp (uses os.lf_pulsetrain with duty cycle)"
    - "native/dsp/noise.dsp (freq/detune kept for API uniformity, not used)"
    - "native/dsp/fm.dsp (adds mod_ratio, mod_index per D-06)"
    - "native/src/justifier_audio.h"
    - "native/src/voice_slot.h"
    - "native/CMakeLists.txt"
    - "native/include/miniaudio.h"
    - "native/include/readerwriterqueue.h"
    - "native/include/atomicops.h"
    - "native/src/audio_engine.c (stub)"
    - "native/src/faust_wrapper.cpp (stub)"
    - "native/test/test_audio.c (stub)"
    - "native/.gitignore"
  modified: []

key-decisions:
  - "Built Faust 2.85.5 from source (no system faust, no root access on Arch Linux); binary at ~/.local/bin/faust"
  - "Faust libraries installed to ~/.local/share/faust/; include headers to ~/.local/include/faust/"
  - "Added stub source files so cmake configure succeeds; CMake requires sources to exist at configure time"
  - "Used os.lf_pulsetrain for pulse waveform (os.pulsetrain not available in Faust stdlib)"
  - "justifier_unpanic() added to public API to complement justifier_panic()"
  - "justifier_voice_set_gate_times() added per Research open question 3"

patterns-established:
  - "Faust DSP: all params via nentry(), smooth via si.smoo, gate via en.are()"
  - "CMake Faust codegen: find_program + faust --includedir + add_custom_command loop"

requirements-completed: [AUD-02]

duration: 9min
completed: 2026-04-01
---

# Phase 01 Plan 01: Build System, DSP Files, and Type Definitions Summary

**7 Faust waveform DSP files compiled to C++ via CMake codegen, public C API header defining the full voice management surface, and vendored miniaudio/readerwriterqueue headers in place**

## Performance

- **Duration:** 9 min
- **Started:** 2026-04-01T15:17:53Z
- **Completed:** 2026-04-01T15:26:42Z
- **Tasks:** 2
- **Files modified:** 16 created, 0 modified

## Accomplishments

- All 7 Faust .dsp files created with correct parameter interfaces (freq, amp, gate, attack, release, detune; FM adds mod_ratio/mod_index)
- Public C API header `justifier_audio.h` defines complete voice management surface including justifier_panic, justifier_unpanic, justifier_voice_set_gate_times
- CMakeLists.txt: Faust codegen loop for all 7 types with unique class names via -cn flag, platform-specific library linking, test harness target
- Faust 2.85.5 built from source with CPP backend enabled (system had no faust, no root access)
- All 7 .dsp files verified to compile to valid C++ via `faust -lang cpp`
- cmake configure: succeeds with all 7 codegen commands registered

## Task Commits

1. **Task 1: Vendor deps, create 7 Faust DSP files, define type headers** - `9e876bb` (feat)
2. **Task 2: CMake build system with Faust codegen and stub sources** - `c138df8` (feat)

## Files Created/Modified

- `native/dsp/{sine,triangle,saw,square,pulse,noise,fm}.dsp` - Faust oscillator programs
- `native/src/justifier_audio.h` - Public C API: WaveformType enum, all lifecycle/voice/gate/panic functions, constants
- `native/src/voice_slot.h` - VoiceSlot struct, ControlMessage union, VoiceState enum, MAX_VOICES=32
- `native/CMakeLists.txt` - Build system: Faust codegen + library + test harness targets
- `native/include/miniaudio.h` - Vendored v0.11.25 (single-header audio I/O)
- `native/include/readerwriterqueue.h` - Vendored SPSC lock-free queue
- `native/include/atomicops.h` - readerwriterqueue dependency
- `native/src/audio_engine.c` - Stub (Plan 02)
- `native/src/faust_wrapper.cpp` - Stub (Plan 02)
- `native/test/test_audio.c` - Stub (Plan 02)
- `native/.gitignore` - Excludes build/ (Faust-generated C++ output)

## Decisions Made

- **Built Faust from source**: No system faust on Arch Linux, no root access. Built 2.85.5 from source tarball with CPP backend explicitly enabled (-DCPP_BACKEND=COMPILER). Binary at ~/.local/bin/faust.
- **Faust stdlib at ~/.local/share/faust/**: Copied from source tree; `faust --includedir` reports ~/.local/include where architecture headers live.
- **os.lf_pulsetrain for pulse**: os.pulsetrain not available in Faust stdlib; os.lf_pulsetrain with duty cycle parameter is the correct alternative.
- **-cn flag for unique class names**: Without -cn, all generated classes are named `mydsp` — collision when multiple DSP types linked together.
- **-i flag inlines libraries**: Avoids runtime Faust library path issues; generated .cpp is self-contained.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical] Added stub source files for CMake configure**
- **Found during:** Task 2 (CMake configuration)
- **Issue:** CMake requires source files to exist at configure time. audio_engine.c, faust_wrapper.cpp, and test/test_audio.c are Plan 02 deliverables but CMake errored at configure without them.
- **Fix:** Created minimal stub files with TODO comments and correct #include structure. These replace nothing since the files didn't exist; Plan 02 will overwrite them with full implementations.
- **Files modified:** native/src/audio_engine.c, native/src/faust_wrapper.cpp, native/test/test_audio.c
- **Verification:** `cmake ..` from native/build/ exits 0, codegen commands registered for all 7 DSP types
- **Committed in:** c138df8 (Task 2 commit)

---

**Total deviations:** 1 auto-fixed (1 missing critical)
**Impact on plan:** Required for cmake configure success (explicit acceptance criterion). Stubs don't add behavior; Plan 02 replaces them entirely.

## Issues Encountered

- **Faust CLI missing, no root access**: Arch Linux had no faust package installed. `sudo pacman` and AUR helpers (paru, yay) require terminal password. Built Faust 2.85.5 from source tarball; took 3 extra minutes of build time.
- **Faust CPP backend disabled by default in first build**: First binary build succeeded but lacked the CPP backend. Required a fresh cmake configure with `-DCPP_BACKEND=COMPILER` and rebuild. All other Faust backends were present.
- **Faust stdlib not on path after binary install**: `faust --includedir` returns `~/.local/include` but standard libraries live in source at `libraries/*.lib`. Manually copied to `~/.local/share/faust/`.

## Known Stubs

| File | Lines | Reason |
|------|-------|--------|
| `native/src/audio_engine.c` | all | Plan 02 implementation; stub added for CMake configure |
| `native/src/faust_wrapper.cpp` | all | Plan 02 implementation; stub added for CMake configure |
| `native/test/test_audio.c` | all | Plan 02 implementation; stub added for CMake configure |

These stubs prevent `justifier_audio` from building, but CMake configure (the goal of this plan) works. Plan 02 replaces all three with full implementations.

## User Setup Required

**Faust compiler must be installed for Plan 02 to build.** Current faust binary was built from source and is at `~/.local/bin/faust` with:
- Binary: `~/.local/bin/faust`
- Libraries: `~/.local/share/faust/*.lib`
- Architecture headers: `~/.local/include/faust/`

For a system-wide install (recommended for CI): `sudo pacman -S faust` (Arch) or `sudo apt install faust` (Ubuntu/Debian).

## Next Phase Readiness

Plan 02 can now implement `audio_engine.c` and `faust_wrapper.cpp` with complete context:
- All type definitions are in place (`voice_slot.h`)
- Public API surface is locked (`justifier_audio.h`)
- CMake will compile the Faust DSP files when Plan 02 sources are written
- Vendored dependencies (miniaudio, readerwriterqueue) are available in `native/include/`

No blockers for Plan 02 execution.

---
*Phase: 01-c-audio-engine*
*Completed: 2026-04-01*
