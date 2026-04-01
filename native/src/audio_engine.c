// audio_engine.c — stub for CMake configuration
// Full implementation: Plan 02
// See: native/src/justifier_audio.h for the public API

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "justifier_audio.h"
#include "voice_slot.h"

// TODO: Plan 02 implements the full audio engine
// - miniaudio device init/shutdown
// - SPSC queue drain in audio callback
// - Pre-allocated voice pool (MAX_VOICES slots)
// - Stereo panning mix buffer
// - Atomic is_silent panic flag
