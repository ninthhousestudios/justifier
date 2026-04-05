import("stdfaust.lib");
import("justifier_filter.lib");
amp = nentry("amp", 0.0, 0.0, 1.0, 0.001) : si.smoo;
gate = nentry("gate", 0, 0, 1, 1);
attack = nentry("attack", 0.05, 0.001, 5.0, 0.001);
decay = nentry("decay", 0.3, 0.001, 5.0, 0.001);
sustain = nentry("sustain", 0.8, 0.0, 1.0, 0.001);
release = nentry("release", 2.0, 0.01, 30.0, 0.01);
envelope = en.adsr(attack, decay, sustain, release, gate);
// Brown noise: pink noise through a 2nd-order lowpass at 500 Hz
process = no.pink_noise : fi.lowpass(2, 500) : jf_filter : *(amp) : *(envelope);
