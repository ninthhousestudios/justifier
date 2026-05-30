// tanpura_ensemble.dsp — 4-string self-playing tanpura drone (one voice).
//
// Promotes the tuned single-string algorithm (native/experiments/tanpura.dsp,
// approved by Josh) and runs it four times internally, summed, so one app voice
// IS the whole tanpura ensemble. Each string is a Karplus-Strong loop with a
// signal-model jivari (passive nonlinear allpass, Faust-STK nonLinearModulator)
// whose buzz blooms per pluck. Strings self-pluck on a slow staggered cycle.
//
// Tuning (relative to Sa = the voice's `freq`, which tracks the app's global 1/1):
//   string 1: freq * tone1_ratio   (the "first tone" / Pa — user-tunable, default 3/2)
//   string 2: freq                 (Sa)
//   string 3: freq                 (Sa)
//   string 4: freq * 0.5           (Sa, octave below)
// The four self-plucks are staggered a quarter-cycle apart for a rolling wash.
//
// Standard Justifier voice interface (freq/amp/gate/adsr/detune + jf_filter).
// The drone timbre's other character knobs (jivari, brightness, sustain_time,
// warmth, pluck_attack, pluck_rate) are baked at the approved defaults here; only
// tone1_ratio is exposed to the app for now.

import("stdfaust.lib");
import("justifier_filter.lib");
import("instruments.lib");   // nonLinearModulator (passive nonlinear allpass)

// --- standard voice interface -------------------------------------------
freq    = nentry("freq", 220.0, 20.0, 20000.0, 0.01) : si.smoo;
amp     = nentry("amp", 0.0, 0.0, 1.0, 0.001) : si.smoo;
gate    = nentry("gate", 0, 0, 1, 1);
attack  = nentry("attack", 0.05, 0.001, 5.0, 0.001);
decay   = nentry("decay", 0.3, 0.001, 5.0, 0.001);
sustain = nentry("sustain", 0.8, 0.0, 1.0, 0.001);
release = nentry("release", 2.0, 0.01, 30.0, 0.01);
detune  = nentry("detune", 0.0, -100.0, 100.0, 0.01);
effective_freq = freq * 2.0^(detune / 1200.0);
envelope = en.adsr(attack, decay, sustain, release, gate);

// --- the one app-exposed knob: the first tone (Pa) vs. Sa ----------------
tone1Ratio = nentry("tone1_ratio", 1.5, 0.25, 4.0, 0.001) : si.smoo;

// --- baked tanpura character (the approved tuning) -----------------------
pluckRate  = 0.208;   // plucks/sec per string
jivari     = 0.35;    // buzz / shimmer depth
brightness = 0.30;    // loop damping (0 = dark)
sustainTime= 0.85;    // ring length
nlSweep    = 8.0;     // buzz-bloom decay rate
warmthHz   = 2800.0;  // post-loop darkening cutoff
pluckGain  = 0.35;    // excitation level (gentle finger pluck)
pluckAtt   = 0.035;   // finger-release onset softness (s)
nlOrder    = 4;       // allpass order (must be a compile-time constant)
nlType     = 0;       // 0: theta modulated by the loop signal (sitar-like)

// --- one string: KS loop + signal-model jivari, self-plucking ------------
// phase0 (0..1) offsets this string's self-pluck clock for the ensemble stagger.
tanpuraString(sfreq, phase0) =
    ( *(lg) : ((lf : nl) + exc) ) ~ de.fdelay(8192, ma.SR / sfreq - 1.0)
with {
    ph   = (os.lf_sawpos(pluckRate) + phase0) : ma.frac;        // staggered pluck phase
    pEnv = exp(-ph * 12.0) : si.smooth(ba.tau2pole(pluckAtt));  // soft, broad burst
    exc  = no.noise : fi.lowpass(2, 1000.0) : *(pEnv * gate * pluckGain);
    jEnv = exp(-ph * (nlSweep * pluckRate)) * gate;             // per-pluck buzz bloom
    lg   = 0.9960 + sustainTime * 0.0040 + (sfreq * 0.0000003) : min(0.9996);
    lf   = si.smooth((1.0 - brightness) * 0.5);
    nl   = nonLinearModulator(jivari, jEnv, sfreq, nlType, 0, nlOrder);
};

// --- the ensemble: four staggered strings, summed ------------------------
ensemble = ( tanpuraString(effective_freq * tone1Ratio, 0.00)   // first tone (Pa)
           + tanpuraString(effective_freq,               0.25)  // Sa
           + tanpuraString(effective_freq,               0.50)  // Sa
           + tanpuraString(effective_freq * 0.5,         0.75)  // Sa, octave below
           ) * 0.5;   // staggered strings sum incoherently; 0.5 ~= single-voice level

// dcblocker: the nonlinear allpass introduces DC. warmth: gentle post-loop
// lowpass to keep the timbre warm rather than metallic.
warmth = fi.lowpass(2, warmthHz);
process = ensemble : fi.dcblocker : warmth : jf_filter : *(amp) : *(envelope);
