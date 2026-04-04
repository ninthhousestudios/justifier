import("stdfaust.lib");
freq = nentry("freq", 440.0, 0.1, 20000.0, 0.01) : si.smoo;
amp = nentry("amp", 0.0, 0.0, 1.0, 0.001) : si.smoo;
gate = nentry("gate", 0, 0, 1, 1);
attack = nentry("attack", 0.05, 0.001, 1.0, 0.001);
release = nentry("release", 10.0, 0.01, 30.0, 0.01);
detune = nentry("detune", 0.0, -100.0, 100.0, 0.01);
detuned_freq = freq * 2.0^(detune / 1200.0);
envelope = en.are(attack, release, gate);
// Smooth random: linear interpolated noise through a 2nd-order lowpass
process = no.lfnoise(detuned_freq) : fi.lowpass(2, detuned_freq * 2) * amp * envelope;
