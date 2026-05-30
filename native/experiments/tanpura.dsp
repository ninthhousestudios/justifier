// tanpura.dsp — self-plucking tanpura drone (v3: signal-model jivari)
//
// Karplus-Strong string (exact, fixed delay -> exact pitch) with a SIGNAL
// model of the jivari bridge: a nonlinear passive allpass (Faust-STK
// nonLinearModulator) inside the loop, its buzz depth driven by the per-pluck
// envelope. Harmonics bloom at each pluck and sweep down as it decays — the
// tanpura's javari formant sweep — without the random delay jitter of v1/v2
// (which sounded chorusy) and without the pitch instability of the physical
// collision models. Standard Justifier voice interface.

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

// --- tanpura-specific (tunable; sane defaults for in-app use) -----------
pluckRate  = nentry("pluck_rate", 0.25, 0.05, 4.0, 0.01);  // plucks per second (v3.1: halved — 2x gap)
jivari     = nentry("jivari", 0.35, 0.0, 1.0, 0.01);       // buzz / shimmer depth (v3.1: down from 0.6 — less buzzy)
brightness = nentry("brightness", 0.3, 0.0, 1.0, 0.01);    // loop damping (0=dark) (v3.1: down from 0.5 — warmer)
sustainTime= nentry("sustain_time", 0.85, 0.5, 1.0, 0.001);// string ring time
nlSweep    = nentry("nl_sweep", 8.0, 0.5, 40.0, 0.01);     // buzz-bloom decay rate
warmthHz   = nentry("warmth_hz", 2800.0, 800.0, 12000.0, 1.0); // post-loop darkening cutoff

nlOrder = 4;   // allpass order (must be a compile-time constant) (v3.1: down from 6 — less inharmonic dispersion = less metallic)
nlType  = 0;   // 0: theta modulated by the loop signal (sitar-like)

// --- self-plucking excitation -------------------------------------------
pluckAttack = nentry("pluck_attack", 0.035, 0.001, 0.15, 0.001); // finger-release softness (s)
pluckPhase = os.lf_sawpos(pluckRate);
// soft finger pluck: the raw burst onset is instantaneous (reads as a hard
// "strike"/bow-scrape); a one-pole smoother with a ~35ms time constant turns
// it into a gentle rise. Decay rate lowered (12) so the burst is broad and
// low-peak — energy fed in gently rather than as a transient. Noise rolled off
// at 1000Hz to kill the high-frequency "scrape" that read as a bow attack.
pluckEnv   = exp(-pluckPhase * 12.0) : si.smooth(ba.tau2pole(pluckAttack));
excitation = no.noise : fi.lowpass(2, 1000.0) : *(pluckEnv * gate * 0.35);

// jivari bloom envelope: bright buzz right after the pluck, sweeping down.
// Slower than the excitation burst so the shimmer rings on past the attack.
jivariEnv = exp(-pluckPhase * (nlSweep * pluckRate)) * gate;

// --- string loop (KS + signal-model jivari) -----------------------------
// Fixed delay length keeps pitch exact (no random jitter).
targetDelay = ma.SR / effective_freq;
delayLine   = de.fdelay(8192, targetDelay - 1.0);

// sustainTime maps smoothly to ring length. The old `sustainTime + 0.14`
// formula was hypersensitive (0.85->0.99 but 0.86->cap); this gives a usable
// range. Default 0.85 ~= 90s T60 at Sa (up from ~5s), so the ring barely fades
// between plucks — continuous overlapping resonance, the pluck moment blurs.
loopGain = 0.9960 + sustainTime * 0.0040 + (effective_freq * 0.0000003) : min(0.9996);
damp = (1.0 - brightness) * 0.5;
loopFilter = si.smooth(damp);

// The nonlinear bridge: a passive allpass whose phase is modulated by the
// loop signal, scaled by jivari (depth) and jivariEnv (per-pluck bloom).
// Passive => adds harmonics / disperses without injecting energy => stable.
jivariNL = nonLinearModulator(jivari, jivariEnv, effective_freq, nlType, 0, nlOrder);

string = ( *(loopGain) : ((loopFilter : jivariNL) + excitation) ) ~ delayLine;

// dcblocker: the nonlinear allpass introduces a DC offset — remove it.
// warmth: gentle 2nd-order lowpass after the loop rolls off the high-harmonic
// energy / inharmonic shimmer that read as "metallic" (borrowed from
// bowed_string.dsp). Cutoff is tunable via warmth_hz.
warmth = fi.lowpass(2, warmthHz);
process = string : fi.dcblocker : warmth : jf_filter : *(amp) : *(envelope);
