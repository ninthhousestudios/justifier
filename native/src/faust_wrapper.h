// faust_wrapper.h — C-callable interface to Faust DSP instances.
// faust_wrapper.cpp (C++) implements this; audio_engine.cpp (C++) calls it.
#pragma once
#include "voice_slot.h"
#include "justifier_audio.h"

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the Faust wrapper.
// Pre-allocates the DSP instance pool: POOL_PER_TYPE (10) instances per
// waveform type = 120 instances total. No allocation happens after this call.
// Returns 0 on success, -1 on failure.
int faust_wrapper_init(int sample_rate);

// Destroy all DSP instances and release the pool.
void faust_wrapper_shutdown(void);

// Acquire a DSP instance of the given waveform type from the pre-allocated pool.
// Returns NULL if all slots of that type are in use (should not happen with
// 32 max voices and 10 slots per type).
// Safe to call from the audio thread — no allocation.
FaustDSP* faust_wrapper_acquire(WaveformType type);

// Return a DSP instance to the pool.
// Safe to call from the audio thread — no deallocation.
void faust_wrapper_release(FaustDSP* dsp);

// Run DSP compute for one buffer. Writes mono output to `output`.
// frame_count MUST be <= JUSTIFIER_MAX_BUFFER_SIZE (4096).
// Safe to call from the audio thread.
void faust_wrapper_compute(FaustDSP* dsp, int frame_count, float* output);

// Set a named parameter on a DSP instance.
// param_name is one of: "freq", "amp", "gate", "attack", "release",
//                        "detune", "mod_ratio", "mod_index"
// Safe to call from the audio thread.
void faust_wrapper_set_param(FaustDSP* dsp, const char* param_name, float value);

// Return the sample rate the wrapper was initialized with.
int faust_wrapper_get_sample_rate(void);

#ifdef __cplusplus
}
#endif
