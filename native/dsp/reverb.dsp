import("stdfaust.lib");

rdel  = nentry("rdel",  60.0, 20.0, 100.0, 1.0);
f1    = nentry("f1",   200.0, 50.0, 1000.0, 1.0);
f2    = nentry("f2",  6000.0, 1000.0, 10000.0, 1.0);
t60dc = nentry("t60dc", 3.0, 0.1, 8.0, 0.1);
t60m  = nentry("t60m",  2.0, 0.1, 8.0, 0.1);
fsmax = 48000;

process = re.zita_rev1_stereo(rdel, f1, f2, t60dc, t60m, fsmax);
