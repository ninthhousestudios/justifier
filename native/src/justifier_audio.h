// justifier_audio.h — the only file Phase 2's Dart FFI binds to
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Symbol visibility for shared library
#ifndef JUSTIFIER_EXPORT
  #ifdef _WIN32
    #ifdef JUSTIFIER_BUILDING
      #define JUSTIFIER_EXPORT __declspec(dllexport)
    #else
      #define JUSTIFIER_EXPORT __declspec(dllimport)
    #endif
  #else
    #define JUSTIFIER_EXPORT __attribute__((visibility("default")))
  #endif
#endif

// Engine constants
#define JUSTIFIER_MAX_VOICES         32
#define JUSTIFIER_DEFAULT_SAMPLE_RATE 48000
#define JUSTIFIER_DEFAULT_BUFFER_SIZE 256
#define JUSTIFIER_MAX_BUFFER_SIZE    4096   // size all temp buffers to this (PipeWire quantum override)

typedef enum {
    WAVEFORM_SINE        = 0,
    WAVEFORM_TRIANGLE    = 1,
    WAVEFORM_SAW         = 2,
    WAVEFORM_SQUARE      = 3,
    WAVEFORM_PULSE       = 4,
    WAVEFORM_WHITE_NOISE = 5,
    WAVEFORM_PINK_NOISE  = 6,
    WAVEFORM_BROWN_NOISE = 7,
    WAVEFORM_LFNOISE0    = 8,
    WAVEFORM_LFNOISE1    = 9,
    WAVEFORM_LFNOISE2    = 10,
    WAVEFORM_FM          = 11,
} WaveformType;

#define NUM_WAVEFORM_TYPES 12

// Engine lifecycle
JUSTIFIER_EXPORT int  justifier_init(int sample_rate, int buffer_size);
JUSTIFIER_EXPORT void justifier_shutdown(void);

// Voice management — all writes go through the SPSC queue
JUSTIFIER_EXPORT int  justifier_voice_add(WaveformType type, float frequency, float amplitude);
JUSTIFIER_EXPORT void justifier_voice_remove(int voice_id);
JUSTIFIER_EXPORT void justifier_voice_set_frequency(int voice_id, float hz);
JUSTIFIER_EXPORT void justifier_voice_set_amplitude(int voice_id, float amplitude);
JUSTIFIER_EXPORT void justifier_voice_set_pan(int voice_id, float pan);
JUSTIFIER_EXPORT void justifier_voice_set_detune(int voice_id, float cents);
JUSTIFIER_EXPORT void justifier_voice_set_waveform(int voice_id, WaveformType type);

// FM-specific
JUSTIFIER_EXPORT void justifier_voice_set_mod_ratio(int voice_id, float ratio);
JUSTIFIER_EXPORT void justifier_voice_set_mod_index(int voice_id, float index);

// Per-voice filter
JUSTIFIER_EXPORT void justifier_voice_set_filter_type(int voice_id, int type);
JUSTIFIER_EXPORT void justifier_voice_set_filter_cutoff(int voice_id, float hz);
JUSTIFIER_EXPORT void justifier_voice_set_filter_resonance(int voice_id, float resonance);

// Reverb send/return
JUSTIFIER_EXPORT void justifier_voice_set_reverb_send(int voice_id, float send);
JUSTIFIER_EXPORT void justifier_set_reverb_return(float level);

// Gate (envelope on/off — attack on create, release on sleep-before-destroy)
JUSTIFIER_EXPORT void justifier_voice_set_gate(int voice_id, int gate_on);
JUSTIFIER_EXPORT void justifier_voice_set_gate_times(int voice_id, float attack_s, float decay_s, float sustain_level, float release_s);

// Global
JUSTIFIER_EXPORT void justifier_panic(void);     // atomic flag — one buffer cycle to silence
JUSTIFIER_EXPORT void justifier_unpanic(void);   // clear the silence flag, resume audio output
JUSTIFIER_EXPORT void justifier_set_master_volume(float volume);

// Status — safe to call from any thread (reads atomics, no locks)
JUSTIFIER_EXPORT int  justifier_is_running(void);
JUSTIFIER_EXPORT int  justifier_get_xrun_count(void);
JUSTIFIER_EXPORT int  justifier_get_active_voice_count(void);

#ifdef __cplusplus
}
#endif
