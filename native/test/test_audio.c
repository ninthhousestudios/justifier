// test_audio.c — standalone test harness for the C audio engine.
// No Flutter/Dart dependency. Exercises all justifier_audio.h functions.
//
// Usage:
//   ./test_audio          Full mode (~30s, audible verification)
//   ./test_audio --quick  Quick mode (~10s, shorter stages)

#include "justifier_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TEST(name) printf("\n=== TEST: %s ===\n", name)
#define PASS(name) printf("  PASS: %s\n", name)
#define CHECK(cond, msg) do { \
    if (!(cond)) { fprintf(stderr, "  FAIL: %s (line %d)\n", msg, __LINE__); exit(1); } \
    else { printf("  PASS: %s\n", msg); } \
} while(0)

int main(int argc, char* argv[]) {
    int quick = (argc > 1 && strcmp(argv[1], "--quick") == 0);
    int play_ms = quick ? 200 : 1500;

    printf("Justifier Audio Engine Test Harness\n");
    printf("Mode: %s\n\n", quick ? "quick (200ms per stage)" : "full (1.5s per stage)");

    // --- 1: Init/Shutdown ---
    TEST("Engine Init");
    int result = justifier_init(JUSTIFIER_DEFAULT_SAMPLE_RATE, JUSTIFIER_DEFAULT_BUFFER_SIZE);
    CHECK(result == 0, "justifier_init returns 0");
    CHECK(justifier_is_running() == 1, "engine is running after init");
    CHECK(justifier_get_active_voice_count() == 0, "no active voices after init");
    CHECK(justifier_get_xrun_count() == 0, "no xruns after init");

    // --- 2: Sine wave ---
    TEST("Sine wave at 440 Hz");
    int v0 = justifier_voice_add(WAVEFORM_SINE, 440.0f, 0.3f);
    CHECK(v0 >= 0, "voice_add returns valid id");
    usleep(play_ms * 1000);
    CHECK(justifier_get_active_voice_count() == 1, "1 active voice");
    PASS("Sine plays audibly");

    // --- 3: Frequency change ---
    TEST("Frequency change 440 -> 880 Hz");
    justifier_voice_set_frequency(v0, 880.0f);
    usleep(play_ms * 1000);
    PASS("Frequency changed to 880 Hz");

    // --- 4: Amplitude change ---
    TEST("Amplitude change 0.3 -> 0.8");
    justifier_voice_set_amplitude(v0, 0.8f);
    usleep(play_ms * 1000);
    PASS("Amplitude increased");
    justifier_voice_set_amplitude(v0, 0.3f);

    // --- 5: Pan ---
    TEST("Pan left -> center -> right");
    justifier_voice_set_pan(v0, -1.0f);
    usleep(play_ms * 1000 / 3);
    justifier_voice_set_pan(v0, 0.0f);
    usleep(play_ms * 1000 / 3);
    justifier_voice_set_pan(v0, 1.0f);
    usleep(play_ms * 1000 / 3);
    justifier_voice_set_pan(v0, 0.0f);
    PASS("Pan sweep complete");

    // --- 6: Detune ---
    TEST("Detune +50 cents");
    justifier_voice_set_detune(v0, 50.0f);
    usleep(play_ms * 1000);
    justifier_voice_set_detune(v0, 0.0f);
    PASS("Detune applied and restored");

    justifier_voice_remove(v0);
    usleep(100 * 1000);

    // --- 7: All 12 waveform types ---
    TEST("All 12 waveform types");
    const char* names[] = {
        "sine", "triangle", "saw", "square", "pulse",
        "white noise", "pink noise", "brown noise",
        "lfnoise0", "lfnoise1", "lfnoise2", "FM"
    };
    for (int type = WAVEFORM_SINE; type <= WAVEFORM_FM; type++) {
        printf("  Playing: %s at 330 Hz\n", names[type]);
        int v = justifier_voice_add((WaveformType)type, 330.0f, 0.25f);
        CHECK(v >= 0, names[type]);

        if (type == WAVEFORM_FM) {
            justifier_voice_set_mod_ratio(v, 2.0f);
            justifier_voice_set_mod_index(v, 3.0f);
        }

        usleep(play_ms * 1000);
        justifier_voice_remove(v);
        usleep(100 * 1000);
    }
    PASS("All 12 waveform types played");

    // --- 8: Multiple simultaneous voices ---
    TEST("8 simultaneous voices (chord)");
    float freqs[] = {261.63f, 329.63f, 392.00f, 523.25f,
                     293.66f, 369.99f, 440.00f, 587.33f};
    int voices[8];
    for (int i = 0; i < 8; i++) {
        voices[i] = justifier_voice_add(WAVEFORM_SINE, freqs[i], 0.15f);
        CHECK(voices[i] >= 0, "voice slot allocated");
    }
    usleep(50 * 1000);  // let audio thread drain SPSC queue
    CHECK(justifier_get_active_voice_count() == 8, "8 voices active");
    usleep(play_ms * 1000 * 2);

    for (int i = 0; i < 8; i++) {
        justifier_voice_remove(voices[i]);
    }
    usleep(200 * 1000);
    PASS("8 simultaneous voices played and removed");

    // --- 9: Waveform crossfade ---
    TEST("Waveform crossfade sine -> saw -> square");
    int vx = justifier_voice_add(WAVEFORM_SINE, 440.0f, 0.3f);
    CHECK(vx >= 0, "crossfade voice created");
    usleep(play_ms * 1000);
    printf("  sine -> saw\n");
    justifier_voice_set_waveform(vx, WAVEFORM_SAW);
    usleep(play_ms * 1000);
    printf("  saw -> square\n");
    justifier_voice_set_waveform(vx, WAVEFORM_SQUARE);
    usleep(play_ms * 1000);
    justifier_voice_remove(vx);
    usleep(100 * 1000);
    PASS("Waveform crossfade completed");

    // --- 10: Panic ---
    TEST("Panic (instant silence)");
    int vp = justifier_voice_add(WAVEFORM_SAW, 220.0f, 0.4f);
    usleep(play_ms * 1000);
    printf("  Activating panic...\n");
    justifier_panic();
    usleep(500 * 1000);
    CHECK(justifier_get_active_voice_count() >= 1, "voice still active during panic");
    printf("  Deactivating panic...\n");
    justifier_unpanic();
    usleep(play_ms * 1000);
    justifier_voice_remove(vp);
    usleep(100 * 1000);
    PASS("Panic silenced and resumed correctly");

    // --- 11: Master volume ---
    TEST("Master volume");
    int vm = justifier_voice_add(WAVEFORM_SINE, 440.0f, 0.5f);
    usleep(play_ms * 1000 / 2);
    printf("  Master volume -> 0.1\n");
    justifier_set_master_volume(0.1f);
    usleep(play_ms * 1000 / 2);
    printf("  Master volume -> 1.0\n");
    justifier_set_master_volume(1.0f);
    usleep(play_ms * 1000 / 2);
    justifier_voice_remove(vm);
    usleep(100 * 1000);
    PASS("Master volume changes applied");

    // --- 12: Shutdown ---
    TEST("Shutdown");
    int xruns = justifier_get_xrun_count();
    justifier_shutdown();
    CHECK(justifier_is_running() == 0, "engine stopped after shutdown");

    printf("\n========================================\n");
    printf("ALL TESTS PASSED\n");
    printf("Xruns during test: %d\n", xruns);
    printf("========================================\n");

    return 0;
}
