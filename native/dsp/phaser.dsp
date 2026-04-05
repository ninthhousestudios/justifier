import("stdfaust.lib");

speed  = nentry("speed",  0.5, 0.01, 10.0, 0.01);
depth  = nentry("depth",  0.7, 0.0,  1.0,  0.01);
fb     = nentry("fb",     0.7, 0.0,  0.95, 0.01);

// phaser2_mono(Notches, phase01, width, frqmin, fratio, frqmax, speed, depth, fb, invert)
one_ch = pf.phaser2_mono(4, 0.0, 50, 200, 1.5, 4000, speed, depth, fb, 0);

process = one_ch, one_ch;
