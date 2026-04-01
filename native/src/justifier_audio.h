// justifier_audio.h — the only file Phase 2's Dart FFI binds to
#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Engine constants
#define JUSTIFIER_MAX_VOICES         32
#define JUSTIFIER_DEFAULT_SAMPLE_RATE 48000
#define JUSTIFIER_DEFAULT_BUFFER_SIZE 256
#define JUSTIFIER_MAX_BUFFER_SIZE    4096   // size all temp buffers to this (PipeWire quantum override)

typedef enum {
    WAVEFORM_SINE     = 0,
    WAVEFORM_TRIANGLE = 1,
    WAVEFORM_SAW      = 2,
    WAVEFORM_SQUARE   = 3,
    WAVEFORM_PULSE    = 4,
    WAVEFORM_NOISE    = 5,
    WAVEFORM_FM       = 6,
} WaveformType;

// Engine lifecycle
int  justifier_init(int sample_rate, int buffer_size);
void justifier_shutdown(void);

// Voice management — all writes go through the SPSC queue
int  justifier_voice_add(WaveformType type, float frequency, float amplitude);
void justifier_voice_remove(int voice_id);
void justifier_voice_set_frequency(int voice_id, float hz);
void justifier_voice_set_amplitude(int voice_id, float amplitude);
void justifier_voice_set_pan(int voice_id, float pan);
void justifier_voice_set_detune(int voice_id, float cents);
void justifier_voice_set_waveform(int voice_id, WaveformType type);

// FM-specific
void justifier_voice_set_mod_ratio(int voice_id, float ratio);
void justifier_voice_set_mod_index(int voice_id, float index);

// Gate (envelope on/off — attack on create, release on sleep-before-destroy)
void justifier_voice_set_gate(int voice_id, int gate_on);
void justifier_voice_set_gate_times(int voice_id, float attack_s, float release_s);

// Global
void justifier_panic(void);     // atomic flag — one buffer cycle to silence
void justifier_unpanic(void);   // clear the silence flag, resume audio output
void justifier_set_master_volume(float volume);

// Status — safe to call from any thread (reads atomics, no locks)
int  justifier_is_running(void);
int  justifier_get_xrun_count(void);
int  justifier_get_active_voice_count(void);

#ifdef __cplusplus
}
#endif
