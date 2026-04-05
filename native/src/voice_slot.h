#pragma once

#ifdef __cplusplus
#include <atomic>
#include <stdbool.h>
// In C++ TUs, declare the VoiceState atomic field using std::atomic<int>
// The C11 _Atomic keyword is not valid C++; we use a conditional typedef.
typedef std::atomic<int> voice_state_atomic_t;
#else
#include <stdatomic.h>
#include <stdbool.h>
typedef _Atomic int voice_state_atomic_t;
#endif

#include "justifier_audio.h"

#define MAX_VOICES         32
#define MAX_BUFFER_SIZE    4096

// Forward declaration — actual type defined in faust_wrapper.cpp
typedef struct FaustDSP FaustDSP;

typedef enum {
    VOICE_FREE      = 0,
    VOICE_ACTIVE    = 1,
    VOICE_FADING    = 2,  // crossfade in progress
    VOICE_RELEASING = 3,  // gate off, release envelope running
    VOICE_PENDING   = 4,  // reserved by UI thread, waiting for audio thread init
} VoiceState;

typedef struct {
    voice_state_atomic_t state;     // VoiceState
    FaustDSP*       dsp;            // current DSP instance
    FaustDSP*       dsp_pending;    // non-null during 20ms crossfade (per D-02)
    float           frequency;
    float           amplitude;
    float           pan;            // -1.0 to 1.0 (per D-05)
    float           detune_cents;   // cents offset (per D-05)
    int             waveform_type;  // WaveformType enum value
    float           crossfade_gain; // 0.0 to 1.0 during crossfade
    int             crossfade_samples_remaining; // 960 at 48kHz for 20ms (per D-02)
    float           attack_time;    // seconds (default 0.05)
    float           release_time;   // seconds (default 10.0)
    float           decay_time;     // seconds (default 0.3)
    float           sustain_level;  // 0.0 to 1.0 (default 0.8)
    int             release_samples_remaining; // countdown for VOICE_RELEASING timeout
    float           filter_type;      // 0=LP, 1=HP, 2=BP, 3=notch
    float           filter_cutoff;    // Hz, 20..20000 (default 20000)
    float           filter_resonance; // 0.0..1.0 (default 0.0)
    float           reverb_send;      // 0.0..1.0 (default 0.0)
    float           delay_send;       // 0.0..1.0 (default 0.0)
    float           chorus_send;      // 0.0..1.0 (default 0.0)
    float           phaser_send;      // 0.0..1.0 (default 0.0)
    float           flanger_send;     // 0.0..1.0 (default 0.0)
    float           eq_send;          // 0.0..1.0 (default 0.0)
    float           saturation_send;  // 0.0..1.0 (default 0.0)
} VoiceSlot;

// Control message types for SPSC queue (per D-08)
typedef enum {
    MSG_VOICE_ADD,
    MSG_VOICE_REMOVE,
    MSG_SET_FREQUENCY,
    MSG_SET_AMPLITUDE,
    MSG_SET_PAN,
    MSG_SET_DETUNE,
    MSG_SET_WAVEFORM,
    MSG_SET_GATE,
    MSG_SET_GATE_TIMES,
    MSG_SET_MOD_RATIO,
    MSG_SET_MOD_INDEX,
    MSG_SET_MASTER_VOLUME,
    MSG_SET_FILTER_TYPE,
    MSG_SET_FILTER_CUTOFF,
    MSG_SET_FILTER_RESONANCE,
    MSG_SET_REVERB_SEND,
    MSG_SET_REVERB_RETURN,
    MSG_SET_DELAY_SEND,
    MSG_SET_DELAY_RETURN,
    MSG_SET_CHORUS_SEND,
    MSG_SET_CHORUS_RETURN,
    MSG_SET_PHASER_SEND,
    MSG_SET_PHASER_RETURN,
    MSG_SET_FLANGER_SEND,
    MSG_SET_FLANGER_RETURN,
    MSG_SET_EQ_SEND,
    MSG_SET_EQ_RETURN,
    MSG_SET_SATURATION_SEND,
    MSG_SET_SATURATION_RETURN,
} ControlMessageType;

typedef struct {
    ControlMessageType type;
    int                voice_id;
    union {
        float          float_value;
        int            int_value;
        struct {
            int        waveform_type;
            float      frequency;
            float      amplitude;
        } voice_add;
        struct {
            float      attack_s;
            float      decay_s;
            float      sustain_level;
            float      release_s;
        } gate_times;
    };
} ControlMessage;
