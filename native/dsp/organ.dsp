import("stdfaust.lib");
import("justifier_filter.lib");
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

// Per-rank detuning: slow drift ±0.5 cents (pipe tuning instability)
rank_drift(n) = os.osc(0.07 + n * 0.03) * 0.0003;

// Per-rank amplitude flutter: air pressure variations in the wind chest
rank_flutter(n) = 1.0 + os.osc(0.11 + n * 0.07) * 0.015;

drawbar(mult, level, n) =
    os.osc(effective_freq * mult * (1.0 + rank_drift(n)))
    * level * rank_flutter(n);

pipes = drawbar(0.5,  0.5,  0)    // sub-octave (16')
      + drawbar(1.0,  1.0,  1)    // fundamental (8')
      + drawbar(2.0,  0.7,  2)    // octave (4')
      + drawbar(3.0,  0.45, 3)    // quint (2 2/3')
      + drawbar(4.0,  0.3,  4)    // super-octave (2')
      + drawbar(5.0,  0.15, 5)    // tierce (1 3/5')
      + drawbar(6.0,  0.1,  6)    // larigot (1 1/3')
      + drawbar(8.0,  0.08, 7);   // piccolo (1')

// Wind noise shaped like air across a pipe mouth: bandpass 1–4 kHz
wind = no.noise * 0.06 : fi.resonbp(2200, 1.5, 1);

// Low rumble from the bellows/wind chest
chest = no.noise * 0.008 : fi.lowpass(2, 200.0);

norm = 1.0 / 3.28;
process = (pipes * norm + wind + chest) : jf_filter : *(amp) : *(envelope);
