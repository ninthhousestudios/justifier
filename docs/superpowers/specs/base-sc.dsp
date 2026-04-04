import("stdfaust.lib");

// ============================================================
// base.sc translated to Faust
// Sine Ocean + Noise Ocean — JI soundscape at 172.8 Hz
// Paste into https://faustide.grame.fr to run
// ============================================================

// --- Global Controls ---
base = hslider("[0]Base Freq (Hz)", 172.8, 20, 1000, 0.1) : si.smoo;
sineVol = hslider("[1]Sine Ocean Vol", 1, 0, 1, 0.01) : si.smoo;
noiseVol = hslider("[2]Noise Ocean Vol", 1, 0, 1, 0.01) : si.smoo;
master = hslider("[3]Master Vol", 0.5, 0, 1, 0.01) : si.smoo;

// --- Voice: sine pair with beating ---
// Each SC voice was a pure tone + a copy offset by `mod` Hz
// The interference between them creates slow amplitude beating
voice(freq, mod, amp) = os.osc(freq) * amp + os.osc(freq + mod) * amp;

// --- Sine Ocean ---
// Layered JI intervals from the base frequency.
// "octave" in the SC code was a raw multiplier (2 = one octave up, 4 = two, 8 = three).
sineOcean =
    // unison — base frequency, octave 1
    voice(base, 0.1, 0.3)
    // 3/2 perfect fifth, octave 2
  + voice(base * 2 * (3/2), 0.1, 0.05)
    // 5/4 major third, octave 4
  + voice(base * 4 * (5/4), 0.1, 0.05)
    // 7/4 harmonic seventh, octave 4
  + voice(base * 4 * (7/4), 0.1, 0.03)
    // 7/5 septimal tritone, octave 4
  + voice(base * 4 * (7/5), 0.1, 0.03)
    // 11/8 undecimal tritone, octave 8
  + voice(base * 8 * (11/8), 0.1, 0.01)
    // 13/8 tridecimal neutral sixth, octave 8
  + voice(base * 8 * (13/8), 0.005, 0.01);

// --- LFNoise0: sample-and-hold noise ---
// SC's LFNoise0 picks a new random value at a given rate.
// In Faust: white noise through a sample-and-hold triggered by an impulse.
lfnoise0(freq) = no.noise : ba.sAndH(os.lf_imptrain(freq));

// --- Noise Ocean ---
// LFNoise0 layers at JI-related frequencies
noiseOcean =
    lfnoise0(base) * 0.05
  + lfnoise0(base * 8) * 0.05
  + lfnoise0(base * 16) * 0.01
  + lfnoise0(base * 32) * 0.01
  + lfnoise0(base * 64) * 0.01
  + lfnoise0(base * 32 * (7/5)) * 0.03;

// --- Output ---
// Mix both oceans, apply master volume, split to stereo
process = (sineOcean * sineVol + noiseOcean * noiseVol) * master <: _, _;
