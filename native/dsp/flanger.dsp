import("stdfaust.lib");

rate  = nentry("rate",  0.3, 0.01, 10.0, 0.01);
depth = nentry("depth", 0.5, 0.0,  1.0,  0.01);
fb    = nentry("fb",    0.6, 0.0,  0.95, 0.01);

max_del = 1024;
min_del = 1;
max_range = 10;
lfo = (1 + os.oscrs(rate)) / 2;
cur_del = min_del + (max_range - min_del) * depth * lfo;

one_ch = _ <: _, (+~(de.fdelay(max_del, cur_del) * fb)) :> /(2);

process = one_ch, one_ch;
