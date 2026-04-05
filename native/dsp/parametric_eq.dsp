import("stdfaust.lib");

low_gain  = nentry("low_gain",  0.0, -12.0, 12.0, 0.1);
mid_freq  = nentry("mid_freq",  1000.0, 100.0, 10000.0, 1.0);
mid_gain  = nentry("mid_gain",  0.0, -12.0, 12.0, 0.1);
mid_q     = nentry("mid_q",     1.0, 0.1, 10.0, 0.1);
high_gain = nentry("high_gain", 0.0, -12.0, 12.0, 0.1);

eq = fi.low_shelf(low_gain, 200) :
     fi.peak_eq(mid_gain, mid_freq, mid_freq / mid_q) :
     fi.high_shelf(high_gain, 8000);

process = eq, eq;
