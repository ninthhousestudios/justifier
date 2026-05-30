// render.cpp — offline Faust DSP renderer for drone-timbre experiments.
//
// Compiles a single generated Faust DSP (class `mydsp`, the Faust default)
// to a WAV file so we can listen/analyze without launching the Flutter app.
//
// Build (see render.sh): faust generates /tmp/<name>_gen.cpp, then:
//   g++ -std=c++17 -O2 -I<faust-include> -DDSP_INCLUDE=\"...gen.cpp\" render.cpp -o render
//
// Usage:
//   render out.wav <seconds> <samplerate> [param=value ...]
// gate defaults to 1, amp to 0.8 unless overridden. Renders mono or stereo
// depending on the DSP's output count.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cmath>
#include <vector>
#include <string>

#include "faust/dsp/dsp.h"
#include "faust/gui/meta.h"
#include "faust/gui/UI.h"
#include "faust/gui/MapUI.h"

#ifndef DSP_INCLUDE
#error "define DSP_INCLUDE to the generated DSP .cpp path"
#endif
#include DSP_INCLUDE

// Suffix-match param setter, mirroring faust_wrapper::set_param.
static void set_param(MapUI& ui, const char* name, float value) {
    int n = ui.getParamsCount();
    size_t name_len = strlen(name);
    for (int i = 0; i < n; i++) {
        const char* path = ui.getParamAddress1(i);
        size_t path_len = strlen(path);
        if (path_len >= name_len &&
            ((path_len == name_len && strcmp(path, name) == 0) ||
             (path_len > name_len && path[path_len - name_len - 1] == '/' &&
              strcmp(path + path_len - name_len, name) == 0))) {
            FAUSTFLOAT* zone = ui.getParamZone(i);
            if (zone) *zone = value;
            return;
        }
    }
    fprintf(stderr, "warning: param '%s' not found\n", name);
}

static void write_wav(const char* path, const std::vector<float>& interleaved,
                      int channels, int sr) {
    size_t frames = interleaved.size() / channels;
    uint32_t data_bytes = (uint32_t)(interleaved.size() * 2); // 16-bit
    FILE* f = fopen(path, "wb");
    if (!f) { fprintf(stderr, "cannot open %s\n", path); exit(1); }
    auto u32 = [&](uint32_t v){ fwrite(&v, 4, 1, f); };
    auto u16 = [&](uint16_t v){ fwrite(&v, 2, 1, f); };
    fwrite("RIFF", 1, 4, f); u32(36 + data_bytes); fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f); u32(16); u16(1); u16(channels);
    u32(sr); u32(sr * channels * 2); u16(channels * 2); u16(16);
    fwrite("data", 1, 4, f); u32(data_bytes);
    for (float s : interleaved) {
        if (s > 1.f) s = 1.f; if (s < -1.f) s = -1.f;
        int16_t v = (int16_t)lrintf(s * 32767.f);
        fwrite(&v, 2, 1, f);
    }
    fclose(f);
    fprintf(stderr, "wrote %s: %zu frames, %d ch, %d Hz\n", path, frames, channels, sr);
}

int main(int argc, char** argv) {
    if (argc < 4) {
        fprintf(stderr, "usage: %s out.wav seconds samplerate [param=value ...]\n", argv[0]);
        return 1;
    }
    const char* out = argv[1];
    double seconds = atof(argv[2]);
    int sr = atoi(argv[3]);

    mydsp dsp;
    dsp.init(sr);
    MapUI ui;
    dsp.buildUserInterface(&ui);

    set_param(ui, "amp", 0.8f);
    set_param(ui, "gate", 1.0f);
    for (int i = 4; i < argc; i++) {
        char* eq = strchr(argv[i], '=');
        if (!eq) continue;
        *eq = '\0';
        set_param(ui, argv[i], (float)atof(eq + 1));
    }

    fprintf(stderr, "params (%d):", ui.getParamsCount());
    for (int i = 0; i < ui.getParamsCount(); i++)
        fprintf(stderr, " %s=%.3f", ui.getParamAddress1(i), *ui.getParamZone(i));
    fprintf(stderr, "\n");

    int nout = dsp.getNumOutputs();
    const int BLK = 512;
    long total = (long)(seconds * sr);

    std::vector<float> buf[8];
    float* outs[8];
    for (int c = 0; c < nout; c++) { buf[c].resize(BLK); outs[c] = buf[c].data(); }

    std::vector<float> interleaved;
    interleaved.reserve(total * nout);

    long done = 0;
    while (done < total) {
        int n = (int)((total - done < BLK) ? (total - done) : BLK);
        dsp.compute(n, nullptr, outs);
        for (int i = 0; i < n; i++)
            for (int c = 0; c < nout; c++)
                interleaved.push_back(outs[c][i]);
        done += n;
    }

    // Diagnostics: catch NaN/inf and report raw range (before WAV clamping).
    {
        double mn = 1e30, mx = -1e30; long bad = 0;
        for (float v : interleaved) {
            if (std::isnan(v) || std::isinf(v)) { bad++; continue; }
            if (v < mn) mn = v; if (v > mx) mx = v;
        }
        fprintf(stderr, "raw range [%.4g, %.4g]  nan/inf: %ld\n", mn, mx, bad);
    }

    write_wav(out, interleaved, nout, sr);
    return 0;
}
