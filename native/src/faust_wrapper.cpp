// faust_wrapper.cpp — C++ implementation wrapping Faust-generated DSP classes.
//
// The generated DSP files (sine_dsp.cpp, etc.) are NOT compiled as independent
// translation units. They are #included here, after the Faust base class headers,
// because they depend on `dsp`, `UI`, and `Meta` base classes from faust/dsp/dsp.h.
//
// All public functions are extern "C" (declared in faust_wrapper.h).

#include <cstring>
#include <cstdio>

// Faust base class headers — must come before the generated DSP includes.
#include "faust/dsp/dsp.h"
#include "faust/gui/meta.h"
#include "faust/gui/UI.h"
#include "faust/gui/MapUI.h"

// Generated DSP classes — included as headers, not compiled independently.
// Each defines a unique class (SineDSP, TriangleDSP, ...) with header guards.
#include "sine_dsp.cpp"
#include "triangle_dsp.cpp"
#include "saw_dsp.cpp"
#include "square_dsp.cpp"
#include "pulse_dsp.cpp"
#include "white_noise_dsp.cpp"
#include "pink_noise_dsp.cpp"
#include "brown_noise_dsp.cpp"
#include "lfnoise0_dsp.cpp"
#include "lfnoise1_dsp.cpp"
#include "lfnoise2_dsp.cpp"
#include "fm_dsp.cpp"
#include "reverb_dsp.cpp"
#include "delay_dsp.cpp"
#include "chorus_dsp.cpp"
#include "phaser_dsp.cpp"
#include "flanger_dsp.cpp"
#include "parametric_eq_dsp.cpp"
#include "saturation_dsp.cpp"

#include "faust_wrapper.h"

// ---------------------------------------------------------------------------
// DSP pool configuration
// ---------------------------------------------------------------------------

#define POOL_PER_TYPE 10   // 10 instances per waveform type = 70 total

// The opaque FaustDSP struct (forward-declared in voice_slot.h).
struct FaustDSP {
    dsp*         instance;   // Faust-generated DSP subclass
    MapUI        ui;         // parameter name -> zone mapping (built at init)
    WaveformType type;
    bool         in_use;     // pool management flag (audio-thread only after init)
};

static FaustDSP g_pool[NUM_WAVEFORM_TYPES][POOL_PER_TYPE];
static FaustDSP g_reverb;      // singleton reverb DSP (NOT in the pool)
static FaustDSP g_delay;       // singleton delay DSP
static FaustDSP g_chorus;      // singleton chorus DSP
static FaustDSP g_phaser;      // singleton phaser DSP
static FaustDSP g_flanger;     // singleton flanger DSP
static FaustDSP g_eq;          // singleton parametric EQ DSP
static FaustDSP g_saturation;  // singleton saturation DSP
static int      g_sample_rate = 0;
static bool     g_initialized = false;

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static dsp* create_dsp_instance(WaveformType type) {
    switch (type) {
        case WAVEFORM_SINE:        return new SineDSP();
        case WAVEFORM_TRIANGLE:    return new TriangleDSP();
        case WAVEFORM_SAW:         return new SawDSP();
        case WAVEFORM_SQUARE:      return new SquareDSP();
        case WAVEFORM_PULSE:       return new PulseDSP();
        case WAVEFORM_WHITE_NOISE: return new White_noiseDSP();
        case WAVEFORM_PINK_NOISE:  return new Pink_noiseDSP();
        case WAVEFORM_BROWN_NOISE: return new Brown_noiseDSP();
        case WAVEFORM_LFNOISE0:    return new Lfnoise0DSP();
        case WAVEFORM_LFNOISE1:    return new Lfnoise1DSP();
        case WAVEFORM_LFNOISE2:    return new Lfnoise2DSP();
        case WAVEFORM_FM:          return new FmDSP();
        default:
            return nullptr;
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

extern "C" {

int faust_wrapper_init(int sample_rate) {
    if (g_initialized) return 0;  // idempotent

    g_sample_rate = sample_rate;

    for (int t = 0; t < NUM_WAVEFORM_TYPES; t++) {
        for (int s = 0; s < POOL_PER_TYPE; s++) {
            FaustDSP* slot = &g_pool[t][s];

            slot->type   = static_cast<WaveformType>(t);
            slot->in_use = false;

            slot->instance = create_dsp_instance(static_cast<WaveformType>(t));
            if (!slot->instance) {
                fprintf(stderr, "faust_wrapper_init: failed to create DSP instance type=%d slot=%d\n", t, s);
                return -1;
            }

            slot->instance->init(sample_rate);
            slot->instance->buildUserInterface(&slot->ui);
        }
    }

    // Initialize singleton effect DSPs (not part of the voice pool)
    g_reverb.instance = new ReverbDSP();
    g_reverb.instance->init(sample_rate);
    g_reverb.instance->buildUserInterface(&g_reverb.ui);
    g_reverb.in_use = false;

    g_delay.instance = new DelayDSP();
    g_delay.instance->init(sample_rate);
    g_delay.instance->buildUserInterface(&g_delay.ui);
    g_delay.in_use = false;

    g_chorus.instance = new ChorusDSP();
    g_chorus.instance->init(sample_rate);
    g_chorus.instance->buildUserInterface(&g_chorus.ui);
    g_chorus.in_use = false;

    g_phaser.instance = new PhaserDSP();
    g_phaser.instance->init(sample_rate);
    g_phaser.instance->buildUserInterface(&g_phaser.ui);
    g_phaser.in_use = false;

    g_flanger.instance = new FlangerDSP();
    g_flanger.instance->init(sample_rate);
    g_flanger.instance->buildUserInterface(&g_flanger.ui);
    g_flanger.in_use = false;

    g_eq.instance = new Parametric_eqDSP();
    g_eq.instance->init(sample_rate);
    g_eq.instance->buildUserInterface(&g_eq.ui);
    g_eq.in_use = false;

    g_saturation.instance = new SaturationDSP();
    g_saturation.instance->init(sample_rate);
    g_saturation.instance->buildUserInterface(&g_saturation.ui);
    g_saturation.in_use = false;

    g_initialized = true;
    return 0;
}

void faust_wrapper_shutdown(void) {
    if (!g_initialized) return;

    for (int t = 0; t < NUM_WAVEFORM_TYPES; t++) {
        for (int s = 0; s < POOL_PER_TYPE; s++) {
            delete g_pool[t][s].instance;
            g_pool[t][s].instance = nullptr;
            g_pool[t][s].in_use   = false;
        }
    }

    // Destroy singleton effect DSPs
    delete g_reverb.instance;
    g_reverb.instance = nullptr;
    g_reverb.in_use = false;

    delete g_delay.instance;
    g_delay.instance = nullptr;
    g_delay.in_use = false;

    delete g_chorus.instance;
    g_chorus.instance = nullptr;
    g_chorus.in_use = false;

    delete g_phaser.instance;
    g_phaser.instance = nullptr;
    g_phaser.in_use = false;

    delete g_flanger.instance;
    g_flanger.instance = nullptr;
    g_flanger.in_use = false;

    delete g_eq.instance;
    g_eq.instance = nullptr;
    g_eq.in_use = false;

    delete g_saturation.instance;
    g_saturation.instance = nullptr;
    g_saturation.in_use = false;

    g_initialized  = false;
    g_sample_rate  = 0;
}

FaustDSP* faust_wrapper_acquire(WaveformType type) {
    if (!g_initialized || (int)type >= NUM_WAVEFORM_TYPES) return nullptr;

    for (int s = 0; s < POOL_PER_TYPE; s++) {
        FaustDSP* slot = &g_pool[(int)type][s];
        if (!slot->in_use) {
            // Reset DSP state without reallocation so previous voice's buffer
            // state doesn't bleed into the new voice.
            slot->instance->instanceClear();
            slot->in_use = true;
            return slot;
        }
    }

    fprintf(stderr, "faust_wrapper_acquire: pool exhausted for type=%d\n", (int)type);
    return nullptr;
}

void faust_wrapper_release(FaustDSP* dsp) {
    if (!dsp) return;
    dsp->in_use = false;
    // Instance stays allocated in the pool — zero allocation on audio thread.
}

void faust_wrapper_compute(FaustDSP* dsp, int frame_count, float* output) {
    if (!dsp || !dsp->instance || !output) return;

    // Faust compute signature: compute(count, float** inputs, float** outputs)
    // These DSP programs have 0 audio inputs (oscillators/noise generators).
    float* outputs[1] = { output };
    dsp->instance->compute(frame_count, nullptr, outputs);
}

void faust_wrapper_set_param(FaustDSP* dsp, const char* param_name, float value) {
    if (!dsp || !param_name) return;
    // MapUI stores params with full Faust path (e.g. "/sine/freq").
    // Try the zone pointer directly via getParamZone for suffix match.
    int n = dsp->ui.getParamsCount();
    size_t name_len = strlen(param_name);
    for (int i = 0; i < n; i++) {
        const char* path = dsp->ui.getParamAddress1(i);
        size_t path_len = strlen(path);
        if (path_len >= name_len) {
            // Check if path ends with "/<param_name>" or equals param_name
            if ((path_len == name_len && strcmp(path, param_name) == 0) ||
                (path_len > name_len &&
                 path[path_len - name_len - 1] == '/' &&
                 strcmp(path + path_len - name_len, param_name) == 0)) {
                FAUSTFLOAT* zone = dsp->ui.getParamZone(i);
                if (zone) *zone = value;
                return;
            }
        }
    }
}

void faust_wrapper_compute_stereo(FaustDSP* dsp, int frame_count,
                                  float* in_L, float* in_R,
                                  float* out_L, float* out_R) {
    if (!dsp || !dsp->instance) return;
    float* inputs[2]  = { in_L, in_R };
    float* outputs[2] = { out_L, out_R };
    dsp->instance->compute(frame_count, inputs, outputs);
}

FaustDSP* faust_wrapper_reverb_acquire(void) {
    if (!g_initialized) return nullptr;
    g_reverb.in_use = true;
    return &g_reverb;
}

void faust_wrapper_reverb_release(void) {
    g_reverb.in_use = false;
}

FaustDSP* faust_wrapper_delay_acquire(void) {
    if (!g_initialized) return nullptr;
    g_delay.in_use = true;
    return &g_delay;
}
void faust_wrapper_delay_release(void) { g_delay.in_use = false; }

FaustDSP* faust_wrapper_chorus_acquire(void) {
    if (!g_initialized) return nullptr;
    g_chorus.in_use = true;
    return &g_chorus;
}
void faust_wrapper_chorus_release(void) { g_chorus.in_use = false; }

FaustDSP* faust_wrapper_phaser_acquire(void) {
    if (!g_initialized) return nullptr;
    g_phaser.in_use = true;
    return &g_phaser;
}
void faust_wrapper_phaser_release(void) { g_phaser.in_use = false; }

FaustDSP* faust_wrapper_flanger_acquire(void) {
    if (!g_initialized) return nullptr;
    g_flanger.in_use = true;
    return &g_flanger;
}
void faust_wrapper_flanger_release(void) { g_flanger.in_use = false; }

FaustDSP* faust_wrapper_eq_acquire(void) {
    if (!g_initialized) return nullptr;
    g_eq.in_use = true;
    return &g_eq;
}
void faust_wrapper_eq_release(void) { g_eq.in_use = false; }

FaustDSP* faust_wrapper_saturation_acquire(void) {
    if (!g_initialized) return nullptr;
    g_saturation.in_use = true;
    return &g_saturation;
}
void faust_wrapper_saturation_release(void) { g_saturation.in_use = false; }

int faust_wrapper_get_sample_rate(void) {
    return g_sample_rate;
}

} // extern "C"
