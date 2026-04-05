import("stdfaust.lib");

delay_time = nentry("delay_time", 0.3, 0.01, 2.0, 0.01);
feedback   = nentry("feedback",   0.4, 0.0,  0.95, 0.01);
damp       = nentry("damp",       0.5, 0.0,  1.0,  0.01);

max_delay = 2.0;

one_ch = +~(de.delay(int(ma.SR * max_delay), int(delay_time * ma.SR)) *
            feedback : fi.lowpass(1, 800 + (1.0 - damp) * 12000));

process = one_ch, one_ch;
