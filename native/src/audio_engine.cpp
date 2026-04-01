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
    std::atomic<int>  xrun_count;
    std::atomic<int>  active_voice_count;
    float           master_volume;
    int             sample_rate;
    int             buffer_size;
    bool            running;
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
            if (slot->state.load() != VOICE_FREE) break;  // slot raced, skip

            slot->dsp = faust_wrapper_acquire((WaveformType)msg->voice_add.waveform_type);
            if (!slot->dsp) break;  // pool exhausted

            slot->waveform_type = msg->voice_add.waveform_type;
            slot->frequency = msg->voice_add.frequency;
            slot->amplitude = msg->voice_add.amplitude;
            slot->pan = 0.0f;
            slot->detune_cents = 0.0f;
            slot->attack_time = 0.05f;
            slot->release_time = 10.0f;
            slot->dsp_pending = NULL;
            slot->crossfade_samples_remaining = 0;
            slot->crossfade_gain = 0.0f;

            faust_wrapper_set_param(slot->dsp, "freq",    slot->frequency);
            faust_wrapper_set_param(slot->dsp, "amp",     slot->amplitude);
            faust_wrapper_set_param(slot->dsp, "attack",  slot->attack_time);
            faust_wrapper_set_param(slot->dsp, "release", slot->release_time);
            faust_wrapper_set_param(slot->dsp, "gate",    1.0f);

            slot->state.store(VOICE_ACTIVE);
            eng->active_voice_count.fetch_add(1);
            break;
        }

        case MSG_VOICE_REMOVE: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;

            faust_wrapper_set_param(slot->dsp, "gate", 0.0f);
            faust_wrapper_release(slot->dsp);
            slot->dsp = NULL;
            if (slot->dsp_pending) {
                faust_wrapper_release(slot->dsp_pending);
                slot->dsp_pending = NULL;
            }
            slot->state.store(VOICE_FREE);
            eng->active_voice_count.fetch_sub(1);
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
            if (slot->waveform_type == msg->int_value) break;  // no-op

            FaustDSP* new_dsp = faust_wrapper_acquire((WaveformType)msg->int_value);
            if (!new_dsp) break;

            faust_wrapper_set_param(new_dsp, "freq",      slot->frequency);
            faust_wrapper_set_param(new_dsp, "amp",       slot->amplitude);
            faust_wrapper_set_param(new_dsp, "detune",    slot->detune_cents);
            faust_wrapper_set_param(new_dsp, "attack",    slot->attack_time);
            faust_wrapper_set_param(new_dsp, "release",   slot->release_time);
            faust_wrapper_set_param(new_dsp, "gate",      1.0f);

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
            if (slot->state.load() == VOICE_FREE) break;
            faust_wrapper_set_param(slot->dsp, "gate", (float)msg->int_value);
            break;
        }

        case MSG_SET_GATE_TIMES: {
            if (id < 0 || id >= MAX_VOICES) break;
            VoiceSlot* slot = &eng->voices[id];
            if (slot->state.load() == VOICE_FREE) break;
            slot->attack_time  = msg->gate_times.attack_s;
            slot->release_time = msg->gate_times.release_s;
            faust_wrapper_set_param(slot->dsp, "attack",  slot->attack_time);
            faust_wrapper_set_param(slot->dsp, "release", slot->release_time);
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

    // 4. Per-voice mono compute buffer (stack — no allocation)
    float tmp[JUSTIFIER_MAX_BUFFER_SIZE];

    // 5. Mix all active voices
    for (int i = 0; i < MAX_VOICES; i++) {
        int state = eng->voices[i].state.load(std::memory_order_relaxed);
        if (state == VOICE_FREE) continue;

        VoiceSlot* slot = &eng->voices[i];

        if (state == VOICE_FADING && slot->dsp_pending) {
            // Crossfade: run both old and new DSP for 20ms overlap
            float tmp_old[JUSTIFIER_MAX_BUFFER_SIZE];
            float tmp_new[JUSTIFIER_MAX_BUFFER_SIZE];
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
            }

            slot->crossfade_samples_remaining = xfade_remaining;

            if (xfade_remaining <= 0) {
                faust_wrapper_release(slot->dsp);
                slot->dsp         = slot->dsp_pending;
                slot->dsp_pending = NULL;
                slot->state.store(VOICE_ACTIVE);
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
        }
    }
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
    g_engine.xrun_count.store(0);
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
        if (g_engine.voices[idx].state.load() == VOICE_FREE) {
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

void justifier_voice_set_gate_times(int voice_id, float attack_s, float release_s) {
    if (!g_engine.running) return;
    ControlMessage msg = {};
    msg.type                = MSG_SET_GATE_TIMES;
    msg.voice_id            = voice_id;
    msg.gate_times.attack_s  = attack_s;
    msg.gate_times.release_s = release_s;
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

int justifier_get_xrun_count(void) {
    return g_engine.xrun_count.load();
}

int justifier_get_active_voice_count(void) {
    return g_engine.active_voice_count.load();
}

} // extern "C"
