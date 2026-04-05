import("stdfaust.lib");

rate  = nentry("rate",  0.5, 0.01, 10.0, 0.01);
depth = nentry("depth", 0.7, 0.0,  1.0,  0.01);
fb    = nentry("fb",    0.7, 0.0,  0.95, 0.01);

stages = 4;

one_ch = pf.phaser2_mono(stages, 200, 1600, rate, depth, fb, 0);

process = one_ch, one_ch;
