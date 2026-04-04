import("stdfaust.lib");
amp = nentry("amp", 0.0, 0.0, 1.0, 0.001) : si.smoo;
gate = nentry("gate", 0, 0, 1, 1);
attack = nentry("attack", 0.05, 0.001, 1.0, 0.001);
release = nentry("release", 10.0, 0.01, 30.0, 0.01);
envelope = en.are(attack, release, gate);
process = no.noise * amp * envelope;
