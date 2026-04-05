// audio_engine.cpp — miniaudio device, audio callback, SPSC queue, voice pool.
//
// All public API functions are extern "C" (declared in justifier_audio.h).
// Written in C style despite the .cpp extension — C++ is required only for
// the ReaderWriterQueue template and faust_wrapper.h linkage.

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <atomic>

// Faust wrapper (C++ linkage — must come before miniaudio to avoid ODR issues)
#include "faust_wrapper.h"

// SPSC queue
#include "readerwriterqueue.h"

// miniaudio — single-header, implement here
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

// Public API and type definitions
#include "justifier_audio.h"
#include "voice_slot.h"

// ---------------------------------------------------------------------------
// Engine state
// ---------------------------------------------------------------------------

typedef struct {
    VoiceSlot       voices[MAX_VOICES];   // pre-allocated, no malloc in callback
    ma_device       device;
    moodycamel::ReaderWriterQueue<ControlMessage>* control_queue;
    std::atomic<bool> is_silent;          // panic flag — checked first in callback
    std::atomic<int>  active_voice_count;
    float           master_volume;
    FaustDSP*       reverb_dsp;        // singleton from faust_wrapper
    float           reverb_return_level; // 0.0..1.0, default 0.3
    FaustDSP*       delay_dsp;
    float           delay_return_level;
    FaustDSP*       chorus_dsp;
    float           chorus_return_level;
    FaustDSP*       phaser_dsp;
    float           phaser_return_level;
    FaustDSP*       flanger_dsp;
    float           flanger_return_level;
    FaustDSP*       eq_dsp;
    float           eq_return_level;
    FaustDSP*       saturation_dsp;
    float           saturation_return_level;
    int             sample_rate;
    int             buffer_size;
    std::atomic<bool> running;
} AudioEngine;

static AudioEngine g_engine;
static std::atomic<int> g_voice_id_counter{0};

// ---------------------------------------------------------------------------
// Control message handler — runs on audio thread only
// ---------------------------------------------------------------------------

static void apply_control_message(AudioEngine* eng, const ControlMessage* msg) {
    int id = msg->voice_id;

    switch (msg->type) {
        case MSG_VOICE_ADD: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() != VOICE_PENDING) break;  // not reserved by UI thread

            slot->dsp = faust_wrapper_acquire((WaveformType)msg->voice_add.waveform_type);
            if (!slot->dsp) break;  // pool exhausted

            slot->waveform_type = msg->voice_add.waveform_type;
            slot->frequency = msg->voice_add.frequency;
            slot->amplitude = msg->voice_add.amplitude;
            slot->pan = 0.0f;
            slot->detune_cents = 0.0f;
            slot->attack_time = 0.05f;
            slot->decay_time    = 0.3f;
            slot->sustain_level = 0.8f;
            slot->release_time = 2.0f;
            slot->release_samples_remaining = 0;
            slot->filter_type      = 0.0f;      // LP
            slot->filter_cutoff    = 20000.0f;   // effectively bypass
            slot->filter_resonance = 0.0f;
            slot->reverb_send      = 0.0f;
            slot->delay_send       = 0.0f;
            slot->chorus_send      = 0.0f;
            slot->phaser_send      = 0.0f;
            slot->flanger_send     = 0.0f;
            slot->eq_send          = 0.0f;
            slot->saturation_send  = 0.0f;
            slot->dsp_pending = NULL;
            slot->crossfade_samples_remaining = 0;
            slot->crossfade_gain = 0.0f;

            faust_wrapper_set_param(slot->dsp, "freq",    slot->frequency);
            faust_wrapper_set_param(slot->dsp, "amp",     slot->amplitude);
            faust_wrapper_set_param(slot->dsp, "attack",  slot->attack_time);
            faust_wrapper_set_param(slot->dsp, "decay",   slot->decay_time);
            faust_wrapper_set_param(slot->dsp, "sustain", slot->sustain_level);
            faust_wrapper_set_param(slot->dsp, "release", slot->release_time);
            faust_wrapper_set_param(slot->dsp, "gate",    1.0f);
            faust_wrapper_set_param(slot->dsp, "filter_type",   slot->filter_type);
            faust_wrapper_set_param(slot->dsp, "filter_cutoff", slot->filter_cutoff);
            faust_wrapper_set_param(slot->dsp, "filter_res",    slot->filter_resonance);

            slot->state.store(VOICE_ACTIVE);  // PENDING -> ACTIVE, now safe for audio callback
            eng->active_voice_count.fetch_add(1);
            break;
        }

        case MSG_VOICE_REMOVE: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            int state = slot->state.load();
            if (state == VOICE_FREE || state == VOICE_RELEASING) break;

            // If mid-crossfade, finish it immediately
            if (state == VOICE_FADING && slot->dsp_pending) {
                faust_wrapper_release(slot->dsp);
                slot->dsp = slot->dsp_pending;
                slot->dsp_pending = NULL;
            }

            faust_wrapper_set_param(slot->dsp, "gate", 0.0f);
            // Timeout = release_time + 0.5s safety margin
            slot->release_samples_remaining =
                (int)((slot->release_time + 0.5f) * (float)eng->sample_rate);
            slot->state.store(VOICE_RELEASING);
            break;
        }

        case MSG_SET_FREQUENCY: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            slot->frequency = msg->float_value;
            faust_wrapper_set_param(slot->dsp, "freq", slot->frequency);
            if (slot->dsp_pending)
                faust_wrapper_set_param(slot->dsp_pending, "freq", slot->frequency);
            break;
        }

        case MSG_SET_AMPLITUDE: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            slot->amplitude = msg->float_value;
            break;
        }

        case MSG_SET_PAN: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            slot->pan = msg->float_value;
            break;
        }

        case MSG_SET_DETUNE: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            slot->detune_cents = msg->float_value;
            faust_wrapper_set_param(slot->dsp, "detune", slot->detune_cents);
            if (slot->dsp_pending)
                faust_wrapper_set_param(slot->dsp_pending, "detune", slot->detune_cents);
            break;
        }

        case MSG_SET_WAVEFORM: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            if (slot->state.load() == VOICE_RELEASING) break;  // don't crossfade a dying voice
            if (slot->waveform_type == msg->int_value) break;  // no-op

            FaustDSP* new_dsp = faust_wrapper_acquire((WaveformType)msg->int_value);
            if (!new_dsp) break;

            faust_wrapper_set_param(new_dsp, "freq",      slot->frequency);
            faust_wrapper_set_param(new_dsp, "amp",       slot->amplitude);
            faust_wrapper_set_param(new_dsp, "detune",    slot->detune_cents);
            faust_wrapper_set_param(new_dsp, "attack",    slot->attack_time);
            faust_wrapper_set_param(new_dsp, "decay",     slot->decay_time);
            faust_wrapper_set_param(new_dsp, "sustain",   slot->sustain_level);
            faust_wrapper_set_param(new_dsp, "release",   slot->release_time);
            faust_wrapper_set_param(new_dsp, "gate",      1.0f);
            faust_wrapper_set_param(new_dsp, "filter_type",   slot->filter_type);
            faust_wrapper_set_param(new_dsp, "filter_cutoff", slot->filter_cutoff);
            faust_wrapper_set_param(new_dsp, "filter_res",    slot->filter_resonance);

            if (slot->dsp_pending) {
                faust_wrapper_release(slot->dsp_pending);
            }
            slot->dsp_pending = new_dsp;
            slot->crossfade_samples_remaining = (int)(0.020f * (float)eng->sample_rate);
            slot->crossfade_gain = 0.0f;
            slot->waveform_type = msg->int_value;
            slot->state.store(VOICE_FADING);
            break;
        }

        case MSG_SET_GATE: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            int state = slot->state.load();
            if (state == VOICE_FREE) break;
            float gate_val = msg->int_value ? 1.0f : 0.0f;
            faust_wrapper_set_param(slot->dsp, "gate", gate_val);
            if (slot->dsp_pending)
                faust_wrapper_set_param(slot->dsp_pending, "gate", gate_val);
            // Re-gate: if turning gate ON while releasing, return to ACTIVE
            if (msg->int_value && state == VOICE_RELEASING) {
                slot->release_samples_remaining = 0;
                slot->state.store(VOICE_ACTIVE);
            }
            break;
        }

        case MSG_SET_GATE_TIMES: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE || slot->state.load() == VOICE_RELEASING) break;
            slot->attack_time   = msg->gate_times.attack_s;
            slot->decay_time    = msg->gate_times.decay_s;
            slot->sustain_level = msg->gate_times.sustain_level;
            slot->release_time  = msg->gate_times.release_s;
            faust_wrapper_set_param(slot->dsp, "attack",  slot->attack_time);
            faust_wrapper_set_param(slot->dsp, "decay",   slot->decay_time);
            faust_wrapper_set_param(slot->dsp, "sustain", slot->sustain_level);
            faust_wrapper_set_param(slot->dsp, "release", slot->release_time);
            if (slot->dsp_pending) {
                faust_wrapper_set_param(slot->dsp_pending, "attack",  slot->attack_time);
                faust_wrapper_set_param(slot->dsp_pending, "decay",   slot->decay_time);
                faust_wrapper_set_param(slot->dsp_pending, "sustain", slot->sustain_level);
                faust_wrapper_set_param(slot->dsp_pending, "release", slot->release_time);
            }
            break;
        }

        case MSG_SET_MOD_RATIO: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            faust_wrapper_set_param(slot->dsp, "mod_ratio", msg->float_value);
            break;
        }

        case MSG_SET_MOD_INDEX: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            faust_wrapper_set_param(slot->dsp, "mod_index", msg->float_value);
            break;
        }

        case MSG_SET_MASTER_VOLUME: {
            eng->master_volume = msg->float_value;
            break;
        }

        case MSG_SET_FILTER_TYPE: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            slot->filter_type = (float)msg->int_value;
            faust_wrapper_set_param(slot->dsp, "filter_type", slot->filter_type);
            if (slot->dsp_pending)
                faust_wrapper_set_param(slot->dsp_pending, "filter_type", slot->filter_type);
            break;
        }

        case MSG_SET_FILTER_CUTOFF: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            slot->filter_cutoff = msg->float_value;
            faust_wrapper_set_param(slot->dsp, "filter_cutoff", slot->filter_cutoff);
            if (slot->dsp_pending)
                faust_wrapper_set_param(slot->dsp_pending, "filter_cutoff", slot->filter_cutoff);
            break;
        }

        case MSG_SET_FILTER_RESONANCE: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            slot->filter_resonance = msg->float_value;
            faust_wrapper_set_param(slot->dsp, "filter_res", slot->filter_resonance);
            if (slot->dsp_pending)
                faust_wrapper_set_param(slot->dsp_pending, "filter_res", slot->filter_resonance);
            break;
        }

        case MSG_SET_REVERB_SEND: {
            if (id < 0 || id >= MAX_VOICES) break;
            eng->voices[id].reverb_send = msg->float_value;
            break;
        }

        case MSG_SET_REVERB_RETURN: {
            eng->reverb_return_level = msg->float_value;
            break;
        }

        case MSG_SET_DELAY_SEND: {
            if (id < 0 || id >= MAX_VOICES) break;
            eng->voices[id].delay_send = msg->float_value;
            break;
        }
        case MSG_SET_DELAY_RETURN: {
            eng->delay_return_level = msg->float_value;
            break;
        }
        case MSG_SET_CHORUS_SEND: {
            if (id < 0 || id >= MAX_VOICES) break;
            eng->voices[id].chorus_send = msg->float_value;
            break;
        }
        case MSG_SET_CHORUS_RETURN: {
            eng->chorus_return_level = msg->float_value;
            break;
        }
        case MSG_SET_PHASER_SEND: {
            if (id < 0 || id >= MAX_VOICES) break;
            eng->voices[id].phaser_send = msg->float_value;
            break;
        }
        case MSG_SET_PHASER_RETURN: {
            eng->phaser_return_level = msg->float_value;
            break;
        }
        case MSG_SET_FLANGER_SEND: {
            if (id < 0 || id >= MAX_VOICES) break;
            eng->voices[id].flanger_send = msg->float_value;
            break;
        }
        case MSG_SET_FLANGER_RETURN: {
            eng->flanger_return_level = msg->float_value;
            break;
        }
        case MSG_SET_EQ_SEND: {
            if (id < 0 || id >= MAX_VOICES) break;
            eng->voices[id].eq_send = msg->float_value;
            break;
        }
        case MSG_SET_EQ_RETURN: {
            eng->eq_return_level = msg->float_value;
            break;
        }
        case MSG_SET_SATURATION_SEND: {
            if (id < 0 || id >= MAX_VOICES) break;
            eng->voices[id].saturation_send = msg->float_value;
            break;
        }
        case MSG_SET_SATURATION_RETURN: {
            eng->saturation_return_level = msg->float_value;
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Audio callback — real-time safe: no malloc, no blocking, no Dart
// ---------------------------------------------------------------------------

static void audio_callback(ma_device* device, void* output,
                           const void* input, ma_uint32 frame_count) {
    (void)input;
    AudioEngine* eng = (AudioEngine*)device->pUserData;
    float* out = (float*)output;

    // 1. Panic check — FIRST action, before anything else
    if (eng->is_silent.load(std::memory_order_relaxed)) {
        memset(out, 0, frame_count * 2 * sizeof(float));
        return;
    }

    // 2. Drain SPSC control queue (caller thread -> audio thread, lock-free)
    ControlMessage msg;
    while (eng->control_queue->try_dequeue(msg)) {
        apply_control_message(eng, &msg);
    }

    // 3. Clear stereo output buffer
    memset(out, 0, frame_count * 2 * sizeof(float));

    // 4. Per-voice mono compute buffer (static — callback is single-threaded)
    static float tmp[JUSTIFIER_MAX_BUFFER_SIZE];

    // 4b. Send bus buffers — one stereo pair per effect (static to avoid ~240KB stack pressure)
    static float send_reverb_L[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_reverb_R[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_delay_L[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_delay_R[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_chorus_L[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_chorus_R[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_phaser_L[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_phaser_R[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_flanger_L[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_flanger_R[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_eq_L[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_eq_R[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_sat_L[JUSTIFIER_MAX_BUFFER_SIZE];
    static float send_sat_R[JUSTIFIER_MAX_BUFFER_SIZE];
    memset(send_reverb_L, 0, frame_count * sizeof(float));
    memset(send_reverb_R, 0, frame_count * sizeof(float));
    memset(send_delay_L, 0, frame_count * sizeof(float));
    memset(send_delay_R, 0, frame_count * sizeof(float));
    memset(send_chorus_L, 0, frame_count * sizeof(float));
    memset(send_chorus_R, 0, frame_count * sizeof(float));
    memset(send_phaser_L, 0, frame_count * sizeof(float));
    memset(send_phaser_R, 0, frame_count * sizeof(float));
    memset(send_flanger_L, 0, frame_count * sizeof(float));
    memset(send_flanger_R, 0, frame_count * sizeof(float));
    memset(send_eq_L, 0, frame_count * sizeof(float));
    memset(send_eq_R, 0, frame_count * sizeof(float));
    memset(send_sat_L, 0, frame_count * sizeof(float));
    memset(send_sat_R, 0, frame_count * sizeof(float));
    bool send_reverb_active = false;
    bool send_delay_active = false;
    bool send_chorus_active = false;
    bool send_phaser_active = false;
    bool send_flanger_active = false;
    bool send_eq_active = false;
    bool send_sat_active = false;

    // 5. Mix all active voices
    for (int i = 0; i < MAX_VOICES; i++) {
        int state = eng->voices[i].state.load(std::memory_order_relaxed);
        if (state == VOICE_FREE || state == VOICE_PENDING) continue;

        VoiceSlot* slot = &eng->voices[i];

        if (state == VOICE_FADING && slot->dsp_pending) {
            // Crossfade: run both old and new DSP for 20ms overlap
            static float tmp_old[JUSTIFIER_MAX_BUFFER_SIZE];
            static float tmp_new[JUSTIFIER_MAX_BUFFER_SIZE];
            faust_wrapper_compute(slot->dsp,         (int)frame_count, tmp_old);
            faust_wrapper_compute(slot->dsp_pending, (int)frame_count, tmp_new);

            int xfade_total     = (int)(0.020f * (float)eng->sample_rate);
            int xfade_remaining = slot->crossfade_samples_remaining;

            float amp_L = slot->amplitude * (1.0f - slot->pan) * 0.5f;
            float amp_R = slot->amplitude * (1.0f + slot->pan) * 0.5f;

            for (ma_uint32 s = 0; s < frame_count; s++) {
                float progress;
                if (xfade_remaining <= 0) {
                    progress = 1.0f;
                } else {
                    progress = 1.0f - ((float)xfade_remaining / (float)xfade_total);
                    xfade_remaining--;
                }
                float sample = tmp_old[s] * (1.0f - progress) + tmp_new[s] * progress;
                out[s * 2]     += sample * amp_L * eng->master_volume;
                out[s * 2 + 1] += sample * amp_R * eng->master_volume;

                // Accumulate effect sends
                #define ACCUM_SEND_SAMPLE(field, bus_L, bus_R, active_flag) \
                    if (slot->field > 0.0f) { \
                        active_flag = true; \
                        bus_L[s] += sample * slot->field * (1.0f - slot->pan) * 0.5f; \
                        bus_R[s] += sample * slot->field * (1.0f + slot->pan) * 0.5f; \
                    }
                ACCUM_SEND_SAMPLE(reverb_send,     send_reverb_L,  send_reverb_R,  send_reverb_active)
                ACCUM_SEND_SAMPLE(delay_send,      send_delay_L,   send_delay_R,   send_delay_active)
                ACCUM_SEND_SAMPLE(chorus_send,     send_chorus_L,  send_chorus_R,  send_chorus_active)
                ACCUM_SEND_SAMPLE(phaser_send,     send_phaser_L,  send_phaser_R,  send_phaser_active)
                ACCUM_SEND_SAMPLE(flanger_send,    send_flanger_L, send_flanger_R, send_flanger_active)
                ACCUM_SEND_SAMPLE(eq_send,         send_eq_L,      send_eq_R,      send_eq_active)
                ACCUM_SEND_SAMPLE(saturation_send, send_sat_L,     send_sat_R,     send_sat_active)
                #undef ACCUM_SEND_SAMPLE
            }

            slot->crossfade_samples_remaining = xfade_remaining;

            if (xfade_remaining <= 0) {
                faust_wrapper_release(slot->dsp);
                slot->dsp         = slot->dsp_pending;
                slot->dsp_pending = NULL;
                slot->state.store(VOICE_ACTIVE);
            }
        } else if (state == VOICE_RELEASING) {
            // Voice is in release phase — keep rendering until timeout
            faust_wrapper_compute(slot->dsp, (int)frame_count, tmp);

            float amp_L = slot->amplitude * (1.0f - slot->pan) * 0.5f;
            float amp_R = slot->amplitude * (1.0f + slot->pan) * 0.5f;

            for (ma_uint32 s = 0; s < frame_count; s++) {
                out[s * 2]     += tmp[s] * amp_L * eng->master_volume;
                out[s * 2 + 1] += tmp[s] * amp_R * eng->master_volume;
            }

            // Accumulate effect sends for releasing voice
            #define ACCUM_SEND_BLOCK(field, bus_L, bus_R, active_flag) \
                if (slot->field > 0.0f) { \
                    active_flag = true; \
                    float sL = slot->field * (1.0f - slot->pan) * 0.5f; \
                    float sR = slot->field * (1.0f + slot->pan) * 0.5f; \
                    for (ma_uint32 s = 0; s < frame_count; s++) { \
                        bus_L[s] += tmp[s] * sL; \
                        bus_R[s] += tmp[s] * sR; \
                    } \
                }
            ACCUM_SEND_BLOCK(reverb_send,     send_reverb_L,  send_reverb_R,  send_reverb_active)
            ACCUM_SEND_BLOCK(delay_send,      send_delay_L,   send_delay_R,   send_delay_active)
            ACCUM_SEND_BLOCK(chorus_send,     send_chorus_L,  send_chorus_R,  send_chorus_active)
            ACCUM_SEND_BLOCK(phaser_send,     send_phaser_L,  send_phaser_R,  send_phaser_active)
            ACCUM_SEND_BLOCK(flanger_send,    send_flanger_L, send_flanger_R, send_flanger_active)
            ACCUM_SEND_BLOCK(eq_send,         send_eq_L,      send_eq_R,      send_eq_active)
            ACCUM_SEND_BLOCK(saturation_send, send_sat_L,     send_sat_R,     send_sat_active)
            #undef ACCUM_SEND_BLOCK

            slot->release_samples_remaining -= (int)frame_count;
            if (slot->release_samples_remaining <= 0) {
                faust_wrapper_release(slot->dsp);
                slot->dsp = NULL;
                slot->state.store(VOICE_FREE);
                if (eng->active_voice_count.load() > 0)
                    eng->active_voice_count.fetch_sub(1);
            }
        } else {
            // Normal active voice
            faust_wrapper_compute(slot->dsp, (int)frame_count, tmp);

            float amp_L = slot->amplitude * (1.0f - slot->pan) * 0.5f;
            float amp_R = slot->amplitude * (1.0f + slot->pan) * 0.5f;

            for (ma_uint32 s = 0; s < frame_count; s++) {
                out[s * 2]     += tmp[s] * amp_L * eng->master_volume;
                out[s * 2 + 1] += tmp[s] * amp_R * eng->master_volume;
            }

            // Accumulate effect sends for active voice
            #define ACCUM_SEND_BLOCK(field, bus_L, bus_R, active_flag) \
                if (slot->field > 0.0f) { \
                    active_flag = true; \
                    float sL = slot->field * (1.0f - slot->pan) * 0.5f; \
                    float sR = slot->field * (1.0f + slot->pan) * 0.5f; \
                    for (ma_uint32 s = 0; s < frame_count; s++) { \
                        bus_L[s] += tmp[s] * sL; \
                        bus_R[s] += tmp[s] * sR; \
                    } \
                }
            ACCUM_SEND_BLOCK(reverb_send,     send_reverb_L,  send_reverb_R,  send_reverb_active)
            ACCUM_SEND_BLOCK(delay_send,      send_delay_L,   send_delay_R,   send_delay_active)
            ACCUM_SEND_BLOCK(chorus_send,     send_chorus_L,  send_chorus_R,  send_chorus_active)
            ACCUM_SEND_BLOCK(phaser_send,     send_phaser_L,  send_phaser_R,  send_phaser_active)
            ACCUM_SEND_BLOCK(flanger_send,    send_flanger_L, send_flanger_R, send_flanger_active)
            ACCUM_SEND_BLOCK(eq_send,         send_eq_L,      send_eq_R,      send_eq_active)
            ACCUM_SEND_BLOCK(saturation_send, send_sat_L,     send_sat_R,     send_sat_active)
            #undef ACCUM_SEND_BLOCK
        }
    }

    // 6. Process effect send buses (each skipped when inactive)
    #define PROCESS_EFFECT(active, dsp_ptr, return_level, sL, sR) \
        if (active && dsp_ptr && return_level > 0.0f) { \
            float ret_L[JUSTIFIER_MAX_BUFFER_SIZE]; \
            float ret_R[JUSTIFIER_MAX_BUFFER_SIZE]; \
            faust_wrapper_compute_stereo(dsp_ptr, (int)frame_count, \
                                         sL, sR, ret_L, ret_R); \
            float wet = return_level * eng->master_volume; \
            for (ma_uint32 s = 0; s < frame_count; s++) { \
                out[s * 2]     += ret_L[s] * wet; \
                out[s * 2 + 1] += ret_R[s] * wet; \
            } \
        }
    PROCESS_EFFECT(send_reverb_active,  eng->reverb_dsp,     eng->reverb_return_level,     send_reverb_L,  send_reverb_R)
    PROCESS_EFFECT(send_delay_active,   eng->delay_dsp,      eng->delay_return_level,      send_delay_L,   send_delay_R)
    PROCESS_EFFECT(send_chorus_active,  eng->chorus_dsp,     eng->chorus_return_level,     send_chorus_L,  send_chorus_R)
    PROCESS_EFFECT(send_phaser_active,  eng->phaser_dsp,     eng->phaser_return_level,     send_phaser_L,  send_phaser_R)
    PROCESS_EFFECT(send_flanger_active, eng->flanger_dsp,    eng->flanger_return_level,    send_flanger_L, send_flanger_R)
    PROCESS_EFFECT(send_eq_active,      eng->eq_dsp,         eng->eq_return_level,         send_eq_L,      send_eq_R)
    PROCESS_EFFECT(send_sat_active,     eng->saturation_dsp, eng->saturation_return_level, send_sat_L,     send_sat_R)
    #undef PROCESS_EFFECT
}

// ---------------------------------------------------------------------------
// Public API — extern "C" via justifier_audio.h
// ---------------------------------------------------------------------------

extern "C" {

int justifier_init(int sample_rate, int buffer_size) {
    memset(&g_engine, 0, sizeof(g_engine));
    g_engine.sample_rate   = sample_rate;
    g_engine.buffer_size   = buffer_size;
    g_engine.master_volume = 1.0f;
    g_engine.is_silent.store(false);
    g_engine.active_voice_count.store(0);

    for (int i = 0; i < MAX_VOICES; i++) {
        g_engine.voices[i].state.store(VOICE_FREE);
        g_engine.voices[i].dsp         = NULL;
        g_engine.voices[i].dsp_pending = NULL;
    }

    if (faust_wrapper_init(sample_rate) != 0) {
        fprintf(stderr, "justifier_init: faust_wrapper_init failed\n");
        return -1;
    }

    g_engine.reverb_dsp = faust_wrapper_reverb_acquire();
    g_engine.reverb_return_level = 0.3f;
    g_engine.delay_dsp = faust_wrapper_delay_acquire();
    g_engine.delay_return_level = 0.3f;
    g_engine.chorus_dsp = faust_wrapper_chorus_acquire();
    g_engine.chorus_return_level = 0.3f;
    g_engine.phaser_dsp = faust_wrapper_phaser_acquire();
    g_engine.phaser_return_level = 0.3f;
    g_engine.flanger_dsp = faust_wrapper_flanger_acquire();
    g_engine.flanger_return_level = 0.3f;
    g_engine.eq_dsp = faust_wrapper_eq_acquire();
    g_engine.eq_return_level = 0.3f;
    g_engine.saturation_dsp = faust_wrapper_saturation_acquire();
    g_engine.saturation_return_level = 0.3f;

    g_engine.control_queue =
        new moodycamel::ReaderWriterQueue<ControlMessage>(1024);

    ma_device_config config      = ma_device_config_init(ma_device_type_playback);
    config.playback.format       = ma_format_f32;
    config.playback.channels     = 2;
    config.sampleRate            = (ma_uint32)sample_rate;
    config.periodSizeInFrames    = (ma_uint32)buffer_size;
    config.dataCallback          = audio_callback;
    config.pUserData             = &g_engine;

    if (ma_device_init(NULL, &config, &g_engine.device) != MA_SUCCESS) {
        fprintf(stderr, "justifier_init: ma_device_init failed\n");
        faust_wrapper_shutdown();
        delete g_engine.control_queue;
        return -2;
    }

    if (ma_device_start(&g_engine.device) != MA_SUCCESS) {
        fprintf(stderr, "justifier_init: ma_device_start failed\n");
        ma_device_uninit(&g_engine.device);
        faust_wrapper_shutdown();
        delete g_engine.control_queue;
        return -3;
    }

    g_engine.running = true;
    return 0;
}

void justifier_shutdown(void) {
    if (!g_engine.running) return;

    ma_device_uninit(&g_engine.device);  // stops callback first

    for (int i = 0; i < MAX_VOICES; i++) {
        if (g_engine.voices[i].dsp) {
            faust_wrapper_release(g_engine.voices[i].dsp);
            g_engine.voices[i].dsp = NULL;
        }
        if (g_engine.voices[i].dsp_pending) {
            faust_wrapper_release(g_engine.voices[i].dsp_pending);
            g_engine.voices[i].dsp_pending = NULL;
        }
    }

    faust_wrapper_reverb_release();
    g_engine.reverb_dsp = NULL;
    faust_wrapper_delay_release();
    g_engine.delay_dsp = NULL;
    faust_wrapper_chorus_release();
    g_engine.chorus_dsp = NULL;
    faust_wrapper_phaser_release();
    g_engine.phaser_dsp = NULL;
    faust_wrapper_flanger_release();
    g_engine.flanger_dsp = NULL;
    faust_wrapper_eq_release();
    g_engine.eq_dsp = NULL;
    faust_wrapper_saturation_release();
    g_engine.saturation_dsp = NULL;

    faust_wrapper_shutdown();
    delete g_engine.control_queue;
    g_engine.control_queue = NULL;
    g_engine.running = false;
}

int justifier_voice_add(WaveformType type, float frequency, float amplitude) {
    if (!g_engine.running) return -1;

    int start = g_voice_id_counter.fetch_add(1) % MAX_VOICES;
    for (int i = 0; i < MAX_VOICES; i++) {
        int idx = (start + i) % MAX_VOICES;
        // Atomically reserve the slot to prevent TOCTOU race.
        // VOICE_PENDING keeps the audio callback from rendering before DSP is init'd.
        int expected = VOICE_FREE;
        if (g_engine.voices[idx].state.compare_exchange_strong(expected, VOICE_PENDING)) {
            ControlMessage msg = {};
            msg.type                    = MSG_VOICE_ADD;
            msg.voice_id                = idx;
            msg.voice_add.waveform_type = (int)type;
            msg.voice_add.frequency     = frequency;
            msg.voice_add.amplitude     = amplitude;
            g_engine.control_queue->enqueue(msg);
            return idx;
        }
    }
    return -1;  // all slots full
}

void justifier_voice_remove(int voice_id) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type     = MSG_VOICE_REMOVE;
    msg.voice_id = voice_id;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_frequency(int voice_id, float hz) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_FREQUENCY;
    msg.voice_id    = voice_id;
    msg.float_value = hz;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_amplitude(int voice_id, float amplitude) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_AMPLITUDE;
    msg.voice_id    = voice_id;
    msg.float_value = amplitude;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_pan(int voice_id, float pan) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_PAN;
    msg.voice_id    = voice_id;
    msg.float_value = pan;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_detune(int voice_id, float cents) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_DETUNE;
    msg.voice_id    = voice_id;
    msg.float_value = cents;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_waveform(int voice_id, WaveformType type) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type      = MSG_SET_WAVEFORM;
    msg.voice_id  = voice_id;
    msg.int_value = (int)type;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_mod_ratio(int voice_id, float ratio) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_MOD_RATIO;
    msg.voice_id    = voice_id;
    msg.float_value = ratio;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_mod_index(int voice_id, float index) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_MOD_INDEX;
    msg.voice_id    = voice_id;
    msg.float_value = index;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_gate(int voice_id, int gate_on) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type      = MSG_SET_GATE;
    msg.voice_id  = voice_id;
    msg.int_value = gate_on;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_gate_times(int voice_id, float attack_s, float decay_s, float sustain_level, float release_s) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type                     = MSG_SET_GATE_TIMES;
    msg.voice_id                 = voice_id;
    msg.gate_times.attack_s      = attack_s;
    msg.gate_times.decay_s       = decay_s;
    msg.gate_times.sustain_level = sustain_level;
    msg.gate_times.release_s     = release_s;
    g_engine.control_queue->enqueue(msg);
}

void justifier_panic(void) {
    g_engine.is_silent.store(true, std::memory_order_relaxed);
}

void justifier_unpanic(void) {
    g_engine.is_silent.store(false, std::memory_order_relaxed);
}

void justifier_set_master_volume(float volume) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_MASTER_VOLUME;
    msg.voice_id    = -1;
    msg.float_value = volume;
    g_engine.control_queue->enqueue(msg);
}

int justifier_is_running(void) {
    return g_engine.running ? 1 : 0;
}

int justifier_get_active_voice_count(void) {
    return g_engine.active_voice_count.load();
}

void justifier_voice_set_filter_type(int voice_id, int type) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type      = MSG_SET_FILTER_TYPE;
    msg.voice_id  = voice_id;
    msg.int_value = type;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_filter_cutoff(int voice_id, float hz) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_FILTER_CUTOFF;
    msg.voice_id    = voice_id;
    msg.float_value = hz;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_filter_resonance(int voice_id, float resonance) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_FILTER_RESONANCE;
    msg.voice_id    = voice_id;
    msg.float_value = resonance;
    g_engine.control_queue->enqueue(msg);
}

void justifier_voice_set_reverb_send(int voice_id, float send) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_REVERB_SEND;
    msg.voice_id    = voice_id;
    msg.float_value = (send < 0.0f) ? 0.0f : (send > 1.0f) ? 1.0f : send;
    g_engine.control_queue->enqueue(msg);
}

void justifier_set_reverb_return(float level) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_REVERB_RETURN;
    msg.voice_id    = -1;
    msg.float_value = (level < 0.0f) ? 0.0f : (level > 1.0f) ? 1.0f : level;
    g_engine.control_queue->enqueue(msg);
}

// --- Delay ---

void justifier_voice_set_delay_send(int voice_id, float send) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_DELAY_SEND;
    msg.voice_id    = voice_id;
    msg.float_value = (send < 0.0f) ? 0.0f : (send > 1.0f) ? 1.0f : send;
    g_engine.control_queue->enqueue(msg);
}

void justifier_set_delay_return(float level) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_DELAY_RETURN;
    msg.voice_id    = -1;
    msg.float_value = (level < 0.0f) ? 0.0f : (level > 1.0f) ? 1.0f : level;
    g_engine.control_queue->enqueue(msg);
}

// --- Chorus ---

void justifier_voice_set_chorus_send(int voice_id, float send) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_CHORUS_SEND;
    msg.voice_id    = voice_id;
    msg.float_value = (send < 0.0f) ? 0.0f : (send > 1.0f) ? 1.0f : send;
    g_engine.control_queue->enqueue(msg);
}

void justifier_set_chorus_return(float level) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_CHORUS_RETURN;
    msg.voice_id    = -1;
    msg.float_value = (level < 0.0f) ? 0.0f : (level > 1.0f) ? 1.0f : level;
    g_engine.control_queue->enqueue(msg);
}

// --- Phaser ---

void justifier_voice_set_phaser_send(int voice_id, float send) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_PHASER_SEND;
    msg.voice_id    = voice_id;
    msg.float_value = (send < 0.0f) ? 0.0f : (send > 1.0f) ? 1.0f : send;
    g_engine.control_queue->enqueue(msg);
}

void justifier_set_phaser_return(float level) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_PHASER_RETURN;
    msg.voice_id    = -1;
    msg.float_value = (level < 0.0f) ? 0.0f : (level > 1.0f) ? 1.0f : level;
    g_engine.control_queue->enqueue(msg);
}

// --- Flanger ---

void justifier_voice_set_flanger_send(int voice_id, float send) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_FLANGER_SEND;
    msg.voice_id    = voice_id;
    msg.float_value = (send < 0.0f) ? 0.0f : (send > 1.0f) ? 1.0f : send;
    g_engine.control_queue->enqueue(msg);
}

void justifier_set_flanger_return(float level) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_FLANGER_RETURN;
    msg.voice_id    = -1;
    msg.float_value = (level < 0.0f) ? 0.0f : (level > 1.0f) ? 1.0f : level;
    g_engine.control_queue->enqueue(msg);
}

// --- Parametric EQ ---

void justifier_voice_set_eq_send(int voice_id, float send) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_EQ_SEND;
    msg.voice_id    = voice_id;
    msg.float_value = (send < 0.0f) ? 0.0f : (send > 1.0f) ? 1.0f : send;
    g_engine.control_queue->enqueue(msg);
}

void justifier_set_eq_return(float level) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_EQ_RETURN;
    msg.voice_id    = -1;
    msg.float_value = (level < 0.0f) ? 0.0f : (level > 1.0f) ? 1.0f : level;
    g_engine.control_queue->enqueue(msg);
}

// --- Saturation ---

void justifier_voice_set_saturation_send(int voice_id, float send) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_SATURATION_SEND;
    msg.voice_id    = voice_id;
    msg.float_value = (send < 0.0f) ? 0.0f : (send > 1.0f) ? 1.0f : send;
    g_engine.control_queue->enqueue(msg);
}

void justifier_set_saturation_return(float level) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type        = MSG_SET_SATURATION_RETURN;
    msg.voice_id    = -1;
    msg.float_value = (level < 0.0f) ? 0.0f : (level > 1.0f) ? 1.0f : level;
    g_engine.control_queue->enqueue(msg);
}

} // extern "C"
