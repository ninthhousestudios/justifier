// pitch_detector.cpp — microphone capture + McLeod Pitch Method (MPM)
//
// Separate capture device from the playback engine. A dedicated detection
// thread runs MPM on a sliding window and writes results to atomics that
// Dart polls via FFI.

#include <cstring>
#include <cstdio>
#include <cmath>
#include <atomic>
#include <thread>
#include <chrono>

#include "miniaudio.h"
#include "justifier_audio.h"

#ifdef __APPLE__
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
extern "C" void justifier_ios_set_session_play_and_record(void);
extern "C" void justifier_ios_set_session_playback(void);
#endif
#endif

#define PITCH_WINDOW_SIZE    2048
#define PITCH_RING_SIZE      32768
#define PITCH_RING_MASK      (PITCH_RING_SIZE - 1)
#define MPM_CLARITY_THRESHOLD 0.7f
#define MPM_CUTOFF           0.93f
#define MPM_MAX_PEAKS        64

static_assert((PITCH_RING_SIZE & PITCH_RING_MASK) == 0, "ring size must be power of 2");

typedef struct {
    ma_device               capture_device;
    int                     sample_rate;

    float                   ring[PITCH_RING_SIZE];
    std::atomic<uint32_t>   ring_write;
    uint32_t                ring_read;

    float                   window[PITCH_WINDOW_SIZE];
    int                     window_fill;

    float                   nsdf[PITCH_WINDOW_SIZE];

    std::atomic<float>      result_hz;
    std::atomic<float>      result_confidence;

    std::atomic<bool>       running;
    std::thread             thread;
} PitchDetector;

static PitchDetector g_pitch;

// ---------------------------------------------------------------------------
// Capture callback — realtime safe, just copies samples into ring buffer
// ---------------------------------------------------------------------------

static void capture_callback(ma_device* device, void* output,
                              const void* input, ma_uint32 frame_count) {
    (void)output;
    (void)device;
    const float* in = (const float*)input;
    uint32_t wp = g_pitch.ring_write.load(std::memory_order_relaxed);

    for (ma_uint32 i = 0; i < frame_count; i++) {
        g_pitch.ring[wp & PITCH_RING_MASK] = in[i];
        wp++;
    }

    g_pitch.ring_write.store(wp, std::memory_order_release);
}

// ---------------------------------------------------------------------------
// McLeod Pitch Method
// ---------------------------------------------------------------------------

// NSDF(τ) = 2·r'(τ) / m'(τ)
// r'(τ) = Σ x[j]·x[j+τ]   (autocorrelation)
// m'(τ) = Σ (x[j]² + x[j+τ]²)
static void mpm_nsdf(const float* x, int W, float* nsdf) {
    int half = W / 2;
    for (int tau = 0; tau <= half; tau++) {
        float acf = 0.0f;
        float m   = 0.0f;
        int n = W - tau;
        for (int j = 0; j < n; j++) {
            acf += x[j] * x[j + tau];
            m   += x[j] * x[j] + x[j + tau] * x[j + tau];
        }
        nsdf[tau] = (m > 1e-12f) ? (2.0f * acf / m) : 0.0f;
    }
}

static float parabolic_interp(const float* y, int idx, int limit) {
    if (idx <= 0 || idx >= limit - 1) return (float)idx;
    float a = y[idx - 1];
    float b = y[idx];
    float c = y[idx + 1];
    float denom = 2.0f * (2.0f * b - a - c);
    if (fabsf(denom) < 1e-10f) return (float)idx;
    return (float)idx + (a - c) / denom;
}

static void mpm_detect(const float* x, int W, int sample_rate,
                        float* out_hz, float* out_confidence) {
    int half = W / 2;
    mpm_nsdf(x, W, g_pitch.nsdf);

    int   peak_pos[MPM_MAX_PEAKS];
    float peak_val[MPM_MAX_PEAKS];
    int   num_peaks = 0;
    float global_max = -1.0f;

    // Skip the initial positive lobe (tau near 0 is always ~1.0)
    int i = 1;
    while (i <= half && g_pitch.nsdf[i] > 0.0f) i++;

    bool  in_pos = false;
    int   local_pos = -1;
    float local_val = -1.0f;

    for (; i <= half; i++) {
        if (g_pitch.nsdf[i] > 0.0f) {
            if (!in_pos) {
                in_pos = true;
                local_pos = i;
                local_val = g_pitch.nsdf[i];
            } else if (g_pitch.nsdf[i] > local_val) {
                local_pos = i;
                local_val = g_pitch.nsdf[i];
            }
        } else {
            if (in_pos && num_peaks < MPM_MAX_PEAKS) {
                peak_pos[num_peaks] = local_pos;
                peak_val[num_peaks] = local_val;
                if (local_val > global_max) global_max = local_val;
                num_peaks++;
            }
            in_pos = false;
            local_val = -1.0f;
        }
    }
    if (in_pos && num_peaks < MPM_MAX_PEAKS) {
        peak_pos[num_peaks] = local_pos;
        peak_val[num_peaks] = local_val;
        if (local_val > global_max) global_max = local_val;
        num_peaks++;
    }

    if (num_peaks == 0 || global_max < MPM_CLARITY_THRESHOLD) {
        *out_hz = 0.0f;
        *out_confidence = 0.0f;
        return;
    }

    // First key maximum exceeding cutoff × global max
    float threshold = MPM_CUTOFF * global_max;
    int best_tau = -1;
    float best_clarity = 0.0f;

    for (int k = 0; k < num_peaks; k++) {
        if (peak_val[k] >= threshold) {
            best_tau = peak_pos[k];
            best_clarity = peak_val[k];
            break;
        }
    }

    if (best_tau < 1) {
        *out_hz = 0.0f;
        *out_confidence = 0.0f;
        return;
    }

    float refined = parabolic_interp(g_pitch.nsdf, best_tau, half + 1);
    *out_hz = (float)sample_rate / refined;
    *out_confidence = best_clarity;
}

// ---------------------------------------------------------------------------
// Detection thread
// ---------------------------------------------------------------------------

static void detection_thread_func() {
    while (g_pitch.running.load(std::memory_order_relaxed)) {
        uint32_t wp = g_pitch.ring_write.load(std::memory_order_acquire);
        uint32_t rp = g_pitch.ring_read;
        uint32_t avail = wp - rp;

        if (avail > PITCH_RING_SIZE) {
            // Reader fell behind — skip to most recent data
            rp = wp - PITCH_WINDOW_SIZE;
            avail = PITCH_WINDOW_SIZE;
        }

        if (avail > 0) {
            if (avail >= (uint32_t)PITCH_WINDOW_SIZE) {
                uint32_t start = wp - PITCH_WINDOW_SIZE;
                for (int j = 0; j < PITCH_WINDOW_SIZE; j++)
                    g_pitch.window[j] = g_pitch.ring[(start + j) & PITCH_RING_MASK];
                g_pitch.window_fill = PITCH_WINDOW_SIZE;
            } else {
                int shift = (int)avail;
                if (g_pitch.window_fill >= PITCH_WINDOW_SIZE) {
                    memmove(g_pitch.window, g_pitch.window + shift,
                            (PITCH_WINDOW_SIZE - shift) * sizeof(float));
                    for (int j = 0; j < shift; j++)
                        g_pitch.window[PITCH_WINDOW_SIZE - shift + j] =
                            g_pitch.ring[(rp + j) & PITCH_RING_MASK];
                } else {
                    int space = PITCH_WINDOW_SIZE - g_pitch.window_fill;
                    int n = ((int)avail < space) ? (int)avail : space;
                    for (int j = 0; j < n; j++)
                        g_pitch.window[g_pitch.window_fill + j] =
                            g_pitch.ring[(rp + j) & PITCH_RING_MASK];
                    g_pitch.window_fill += n;
                }
            }
            g_pitch.ring_read = wp;
        }

        if (g_pitch.window_fill >= PITCH_WINDOW_SIZE) {
            float hz, conf;
            mpm_detect(g_pitch.window, PITCH_WINDOW_SIZE, g_pitch.sample_rate,
                       &hz, &conf);
            g_pitch.result_hz.store(hz, std::memory_order_relaxed);
            g_pitch.result_confidence.store(conf, std::memory_order_relaxed);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

extern "C" {

JUSTIFIER_EXPORT int justifier_pitch_start(void) {
    if (g_pitch.running.load()) return 0;

    memset(g_pitch.ring, 0, sizeof(g_pitch.ring));
    memset(g_pitch.window, 0, sizeof(g_pitch.window));
    g_pitch.ring_write.store(0);
    g_pitch.ring_read = 0;
    g_pitch.window_fill = 0;
    g_pitch.result_hz.store(0.0f);
    g_pitch.result_confidence.store(0.0f);

#if defined(__APPLE__) && TARGET_OS_IPHONE
    justifier_ios_set_session_play_and_record();
#endif

    ma_device_config cfg = ma_device_config_init(ma_device_type_capture);
    cfg.capture.format   = ma_format_f32;
    cfg.capture.channels = 1;
    cfg.sampleRate       = 0;
    cfg.dataCallback     = capture_callback;
    cfg.pUserData        = NULL;

    if (ma_device_init(NULL, &cfg, &g_pitch.capture_device) != MA_SUCCESS) {
        fprintf(stderr, "justifier_pitch_start: capture device init failed\n");
        return -1;
    }

    g_pitch.sample_rate = (int)g_pitch.capture_device.sampleRate;

    if (ma_device_start(&g_pitch.capture_device) != MA_SUCCESS) {
        fprintf(stderr, "justifier_pitch_start: capture device start failed\n");
        ma_device_uninit(&g_pitch.capture_device);
        return -2;
    }

    g_pitch.running.store(true);
    g_pitch.thread = std::thread(detection_thread_func);
    return 0;
}

JUSTIFIER_EXPORT void justifier_pitch_stop(void) {
    if (!g_pitch.running.load()) return;

    g_pitch.running.store(false);
    if (g_pitch.thread.joinable()) g_pitch.thread.join();

    ma_device_uninit(&g_pitch.capture_device);

#if defined(__APPLE__) && TARGET_OS_IPHONE
    justifier_ios_set_session_playback();
#endif

    g_pitch.result_hz.store(0.0f);
    g_pitch.result_confidence.store(0.0f);
}

JUSTIFIER_EXPORT void justifier_pitch_get(float* hz, float* confidence) {
    if (hz)         *hz         = g_pitch.result_hz.load(std::memory_order_relaxed);
    if (confidence)  *confidence = g_pitch.result_confidence.load(std::memory_order_relaxed);
}

JUSTIFIER_EXPORT int justifier_pitch_is_running(void) {
    return g_pitch.running.load() ? 1 : 0;
}

} // extern "C"
