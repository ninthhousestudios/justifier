import("stdfaust.lib");

rate  = nentry("rate",  1.5, 0.1, 10.0, 0.1);
depth = nentry("depth", 0.002, 0.0, 0.01, 0.0001);

one_ch(i) = _ <: _, de.fdelay(1024, mod) :> /(2)
with {
    mod = (depth * ma.SR / 2) * (1 + os.oscrs(rate + i * 0.1)) + 10;
};

process = one_ch(0), one_ch(1);
