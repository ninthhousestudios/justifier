import("stdfaust.lib");

drive       = nentry("drive",  0.5, 0.0, 1.0, 0.01);
tone        = nentry("tone",   0.5, 0.0, 1.0, 0.01);
output_gain = nentry("output", 0.7, 0.0, 1.0, 0.01);

saturate(x) = ma.tanh(x * (1 + drive * 10)) * output_gain;

one_ch = saturate : fi.lowpass(1, 1000 + tone * 15000);

process = one_ch, one_ch;
