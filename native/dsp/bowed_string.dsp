import("stdfaust.lib");
import("justifier_filter.lib");
pm = library("physmodels.lib");

freq = nentry("freq", 440.0, 20.0, 20000.0, 0.01) : si.smoo;
amp = nentry("amp", 0.0, 0.0, 1.0, 0.001) : si.smoo;
gate = nentry("gate", 0, 0, 1, 1);
attack = nentry("attack", 0.05, 0.001, 5.0, 0.001);
decay = nentry("decay", 0.3, 0.001, 5.0, 0.001);
sustain = nentry("sustain", 0.8, 0.0, 1.0, 0.001);
release = nentry("release", 2.0, 0.01, 30.0, 0.01);
detune = nentry("detune", 0.0, -100.0, 100.0, 0.01);
effective_freq = freq * 2.0^(detune / 1200.0);
envelope = en.adsr(attack, decay, sustain, release, gate);

stringLength = effective_freq : pm.f2l;

bowPressure = 0.5;
bowVelocity = 0.2;
bowPosition = 0.15;

// Stock violinModel internals, but with maxSL = 7 (~49 Hz floor)
maxSL = 7;
stringTuning = 0.08;
stringL = stringLength - stringTuning;

bowedString = pm.chain(
    pm.stringSegment(maxSL, stringL * bowPosition) :
    pm.violinBow(bowPressure, bowVelocity) :
    pm.stringSegment(maxSL, stringL * (1 - bowPosition))
);

modelChain = pm.chain(
    pm.violinNuts :
    bowedString :
    pm.violinBridge :
    pm.violinBody :
    pm.out
);

// Gentle darkening — roll off above 3kHz for warmth
warmth = fi.lowpass(2, 3000);

process = pm.endChain(modelChain) : warmth : jf_filter : *(amp) : *(envelope);
