// fds_string.dsp — finite-difference stiff string (validation step, no jivari yet)
//
// Adapted from Faust's StiffString.dsp (Riccardo Russo) to self-pluck on a
// slow cycle and expose the standard Justifier voice interface. This is the
// linear string we'll later couple to a rigid-wall bridge for the jivari.

import("stdfaust.lib");

// --- standard voice interface -------------------------------------------
amp     = nentry("amp", 0.0, 0.0, 1.0, 0.001) : si.smoo;
gate    = nentry("gate", 0, 0, 1, 1);
attack  = nentry("attack", 0.05, 0.001, 5.0, 0.001);
decay   = nentry("decay", 0.3, 0.001, 5.0, 0.001);
sustain = nentry("sustain", 0.8, 0.0, 1.0, 0.001);
release = nentry("release", 2.0, 0.01, 30.0, 0.01);
envelope = en.adsr(attack, decay, sustain, release, gate);

// --- tunables -----------------------------------------------------------
pluckRate = nentry("pluck_rate", 0.5, 0.05, 4.0, 0.01);
pluckGain = nentry("pluck_gain", 1.0, 0.0, 20.0, 0.001);
tension   = nentry("tension", 150.0, 5.0, 1000.0, 0.1);   // pitch control
outGain   = nentry("out_gain", 1.0, 0.0, 5000.0, 0.1);

// --- string physics (from StiffString.dsp) ------------------------------
nPoints = 100;
k = 1/ma.SR;
coeff = c^2*k^2 + 4*sigma1*k;
h = sqrt((coeff + sqrt((coeff)^2 + 16*k^2*K^2))/2);

T = tension;
radius = 3.5560e-04;
rho = 8.05*10^3;
Area = ma.PI*radius^2;
I = (ma.PI*radius^4)/ 4;
Emod = 174e4;
K = sqrt(Emod*I/rho/Area);
c = sqrt(T/rho/Area);
sigma1 = 0.01;
sigma0 = 0.0005;

den = 1+sigma0*k;
A = (2*h^4-2*c^2*k^2*h^2-4*sigma1*k*h^2-6*K^2*k^2)/den/h^4;
B = (sigma0*k*h^2-h^2+4*sigma1*k)/den/h^2;
C = (c^2*k^2*h^2+2*sigma1*k*h^2+4*K^2*k^2)/den/h^4;
D = -2*sigma1*k/den/h^2;
E = -K^2*k^2/den/h^4;
midCoeff = E,C,A,C,E;
midCoeffDel = 0,D,B,D,0;
r = 2;
t = 1;
scheme(points) = par(i,points,midCoeff,midCoeffDel);

inPoint  = floor(nPoints/4);     // pluck near 1/4 (rich harmonics)
outPoint = floor(nPoints*0.13);  // listen near the (future) bridge end

// --- self-plucking force ------------------------------------------------
// A real finger pluck is NOT a Dirac (which would dump energy at Nyquist and
// destabilise the scheme). Band-limit the impulse to a soft, low-passed burst.
forceModel = os.lf_imptrain(pluckRate) * pluckGain * gate
             : fi.lowpass(2, 800.0);

string = forceModel <: fd.linInterp1D(nPoints,inPoint) :
         fd.model1D(nPoints,r,t,scheme(nPoints)) :
         fd.linInterp1DOut(nPoints,outPoint);

// Gentle ultrasonic roll-off to suppress any residual Nyquist-band ringing.
process = string * outGain : fi.lowpass(2, 12000.0) : *(amp) : *(envelope);
