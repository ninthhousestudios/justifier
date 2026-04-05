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

#ifdef _WIN32
#include <windows.h>
#define usleep(us) Sleep((us) / 1000)
#else
#include <unistd.h>
#endif

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

    // --- 2: Sine wave ---
    TEST("Sine wave at 440 Hz");
    int v0 = justifier_voice_add(WAVEFORM_SINE, 440.0f, 0.3f);
    CHECK(v0 >= 0, "voice_add returns valid id");
    justifier_voice_set_gate_times(v0, 0.01f, 0.01f, 0.8f, 0.05f);  // short release for tests
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
    usleep(600 * 1000);  // wait for VOICE_RELEASING to finish

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
        justifier_voice_set_gate_times(v, 0.01f, 0.01f, 0.8f, 0.05f);

        if (type == WAVEFORM_FM) {
            justifier_voice_set_mod_ratio(v, 2.0f);
            justifier_voice_set_mod_index(v, 3.0f);
        }

        usleep(play_ms * 1000);
        justifier_voice_remove(v);
        usleep(600 * 1000);
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
        justifier_voice_set_gate_times(voices[i], 0.01f, 0.01f, 0.8f, 0.05f);
    }
    usleep(50 * 1000);  // let audio thread drain SPSC queue
    CHECK(justifier_get_active_voice_count() == 8, "8 voices active");
    usleep(play_ms * 1000 * 2);

    for (int i = 0; i < 8; i++) {
        justifier_voice_remove(voices[i]);
    }
    usleep(600 * 1000);
    PASS("8 simultaneous voices played and removed");

    // --- 9: Waveform crossfade ---
    TEST("Waveform crossfade sine -> saw -> square");
    int vx = justifier_voice_add(WAVEFORM_SINE, 440.0f, 0.3f);
    CHECK(vx >= 0, "crossfade voice created");
    justifier_voice_set_gate_times(vx, 0.01f, 0.01f, 0.8f, 0.05f);
    usleep(play_ms * 1000);
    printf("  sine -> saw\n");
    justifier_voice_set_waveform(vx, WAVEFORM_SAW);
    usleep(play_ms * 1000);
    printf("  saw -> square\n");
    justifier_voice_set_waveform(vx, WAVEFORM_SQUARE);
    usleep(play_ms * 1000);
    justifier_voice_remove(vx);
    usleep(600 * 1000);
    PASS("Waveform crossfade completed");

    // --- 10: Panic ---
    TEST("Panic (instant silence)");
    int vp = justifier_voice_add(WAVEFORM_SAW, 220.0f, 0.4f);
    justifier_voice_set_gate_times(vp, 0.01f, 0.01f, 0.8f, 0.05f);
    usleep(play_ms * 1000);
    printf("  Activating panic...\n");
    justifier_panic();
    usleep(500 * 1000);
    CHECK(justifier_get_active_voice_count() >= 1, "voice still active during panic");
    printf("  Deactivating panic...\n");
    justifier_unpanic();
    usleep(play_ms * 1000);
    justifier_voice_remove(vp);
    usleep(600 * 1000);
    PASS("Panic silenced and resumed correctly");

    // --- 11: Master volume ---
    TEST("Master volume");
    int vm = justifier_voice_add(WAVEFORM_SINE, 440.0f, 0.5f);
    justifier_voice_set_gate_times(vm, 0.01f, 0.01f, 0.8f, 0.05f);
    usleep(play_ms * 1000 / 2);
    printf("  Master volume -> 0.1\n");
    justifier_set_master_volume(0.1f);
    usleep(play_ms * 1000 / 2);
    printf("  Master volume -> 1.0\n");
    justifier_set_master_volume(1.0f);
    usleep(play_ms * 1000 / 2);
    justifier_voice_remove(vm);
    usleep(600 * 1000);
    PASS("Master volume changes applied");

    // --- 12: Per-voice filter ---
    TEST("Per-voice filter");
    int vf = justifier_voice_add(WAVEFORM_SAW, 440.0f, 0.3f);
    CHECK(vf >= 0, "filter test voice created");
    justifier_voice_set_gate_times(vf, 0.01f, 0.01f, 0.8f, 0.05f);
    usleep(play_ms * 1000);

    // All 4 filter types
    for (int ft = 0; ft <= 3; ft++) {
        printf("  Filter type %d\n", ft);
        justifier_voice_set_filter_type(vf, ft);
        justifier_voice_set_filter_cutoff(vf, 800.0f);
        justifier_voice_set_filter_resonance(vf, 0.5f);
        usleep(play_ms * 1000 / 2);
    }
    PASS("All 4 filter types applied");

    // Cutoff sweep
    justifier_voice_set_filter_type(vf, 0);  // LP
    justifier_voice_set_filter_resonance(vf, 0.3f);
    for (float cutoff = 200.0f; cutoff <= 10000.0f; cutoff += 2000.0f) {
        justifier_voice_set_filter_cutoff(vf, cutoff);
        usleep(play_ms * 1000 / 5);
    }
    justifier_voice_set_filter_cutoff(vf, 20000.0f);
    PASS("Cutoff sweep complete");

    // Invalid voice IDs — must not crash
    justifier_voice_set_filter_type(-1, 0);
    justifier_voice_set_filter_type(99, 0);
    justifier_voice_set_filter_cutoff(-1, 1000.0f);
    justifier_voice_set_filter_cutoff(99, 1000.0f);
    justifier_voice_set_filter_resonance(-1, 0.5f);
    justifier_voice_set_filter_resonance(99, 0.5f);
    usleep(100 * 1000);
    PASS("Invalid voice IDs handled safely");

    // Filter survives waveform change
    justifier_voice_set_filter_type(vf, 1);       // HP
    justifier_voice_set_filter_cutoff(vf, 500.0f);
    justifier_voice_set_filter_resonance(vf, 0.6f);
    usleep(100 * 1000);
    justifier_voice_set_waveform(vf, WAVEFORM_SQUARE);
    usleep(play_ms * 1000);
    PASS("Filter survives waveform crossfade");

    justifier_voice_remove(vf);
    usleep(600 * 1000);
    PASS("Per-voice filter tests complete");

    // --- 13: ADSR envelope + voice lifecycle ---
    TEST("ADSR envelope + voice lifecycle");

    // 13a: Set all 4 ADSR params, voice stays active
    {
        int va = justifier_voice_add(WAVEFORM_SINE, 440.0f, 0.3f);
        CHECK(va >= 0, "ADSR: voice created");
        justifier_voice_set_gate_times(va, 0.01f, 0.1f, 0.7f, 0.5f);
        usleep(50 * 1000);  // let audio thread process
        CHECK(justifier_get_active_voice_count() == 1, "ADSR: voice active after set_gate_times");
        justifier_voice_remove(va);
        usleep(1200 * 1000);  // wait past release_s=0.5 + 0.5s safety margin + overhead
        PASS("ADSR: all 4 params set, voice lifecycle clean");
    }

    // 13b: Voice stays in VOICE_RELEASING immediately after remove
    {
        int vr = justifier_voice_add(WAVEFORM_SINE, 330.0f, 0.3f);
        CHECK(vr >= 0, "ADSR releasing: voice created");
        usleep(50 * 1000);  // let voice start
        justifier_voice_remove(vr);
        usleep(50 * 1000);  // let audio thread process MSG_VOICE_REMOVE
        // Shortly after remove: should be VOICE_RELEASING, not freed
        CHECK(justifier_get_active_voice_count() == 1,
              "ADSR releasing: voice count still 1 immediately after remove");
        // Default release = 2.0s, margin = 0.5s -> wait 3s for it to free
        usleep(3000 * 1000);
        CHECK(justifier_get_active_voice_count() == 0,
              "ADSR releasing: voice freed after release completes");
        PASS("ADSR: voice releasing state confirmed");
    }

    // 13c: Re-gate during release keeps voice alive
    {
        int vg = justifier_voice_add(WAVEFORM_SINE, 550.0f, 0.3f);
        CHECK(vg >= 0, "ADSR re-gate: voice created");
        usleep(50 * 1000);
        justifier_voice_remove(vg);          // enters VOICE_RELEASING
        usleep(50 * 1000);                  // let audio thread process remove
        justifier_voice_set_gate(vg, 1);    // re-gate
        // Sleep past what would have been the release timeout (~3s)
        usleep(3000 * 1000);
        CHECK(justifier_get_active_voice_count() == 1,
              "ADSR re-gate: voice still alive after re-gate during release");
        justifier_voice_remove(vg);
        usleep(3000 * 1000);  // wait for final release
        PASS("ADSR: re-gate during release keeps voice alive");
    }

    // 13d: Default ADSR params work without explicit set
    {
        int vd = justifier_voice_add(WAVEFORM_SINE, 220.0f, 0.3f);
        CHECK(vd >= 0, "ADSR defaults: voice created");
        usleep(50 * 1000);
        CHECK(justifier_get_active_voice_count() == 1,
              "ADSR defaults: voice plays with default ADSR");
        justifier_voice_remove(vd);
        usleep(3000 * 1000);  // wait for default release (2.0s + margin)
        PASS("ADSR: default params work");
    }

    // --- 14: Reverb send/return ---
    TEST("Reverb send/return");

    // 14a: Set reverb send on a voice
    int vr = justifier_voice_add(WAVEFORM_SAW, 220.0f, 0.4f);
    CHECK(vr >= 0, "reverb test voice created");
    justifier_voice_set_gate_times(vr, 0.01f, 0.01f, 0.8f, 0.05f);
    justifier_voice_set_reverb_send(vr, 0.7f);
    usleep(play_ms * 1000);
    PASS("Reverb send applied");

    // 14b: Adjust reverb return level
    justifier_set_reverb_return(0.5f);
    usleep(play_ms * 1000);
    PASS("Reverb return level changed");

    // 14c: Zero send = dry only
    justifier_voice_set_reverb_send(vr, 0.0f);
    usleep(play_ms * 1000);
    PASS("Zero send = dry signal");

    // 14d: Full send
    justifier_voice_set_reverb_send(vr, 1.0f);
    usleep(play_ms * 1000);
    PASS("Full reverb send");

    // 14e: Invalid voice IDs — must not crash
    justifier_voice_set_reverb_send(-1, 0.5f);
    justifier_voice_set_reverb_send(99, 0.5f);
    usleep(100 * 1000);
    PASS("Invalid reverb send voice IDs handled safely");

    // 14f: Reverb tail persists after voice removal
    justifier_voice_set_reverb_send(vr, 1.0f);
    justifier_set_reverb_return(0.8f);
    usleep(play_ms * 1000);
    justifier_voice_remove(vr);
    usleep(play_ms * 1000);  // reverb tail should still be audible
    PASS("Reverb tail persists after voice removal");
    justifier_set_reverb_return(0.3f);  // restore default
    usleep(600 * 1000);
    PASS("Reverb send/return tests complete");

    // --- 15: Shutdown ---
    TEST("Shutdown");
    justifier_shutdown();
    CHECK(justifier_is_running() == 0, "engine stopped after shutdown");

    printf("\n========================================\n");
    printf("ALL TESTS PASSED\n");
    printf("========================================\n");

    return 0;
}
