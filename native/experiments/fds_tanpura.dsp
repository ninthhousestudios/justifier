// fds_tanpura.dsp — finite-difference tanpura with a rigid-wall jivari bridge
//
// A linear stiff string (fds.lib model1D, from StiffString.dsp) coupled to a
// stationary one-sided rigid wall just below a near-bridge point. As the string
// vibrates it grazes/chatters against the wall (a nonlinear contact force fed
// back into that point), producing the tanpura's javari buzz automatically —
// the energy-conserving FD approach the literature favours over waveguides for
// this collision. Coupling mirrors fds.lib's HammeredString (Riccardo Russo).

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
pluckGain = nentry("pluck_gain", 2.5, 0.0, 50.0, 0.001);
tension   = nentry("tension", 150.0, 5.0, 1000.0, 0.1);   // pitch control
wallPos   = nentry("wall_pos", -0.02, -0.5, 0.0, 0.0001);  // bridge offset below rest
wallK     = nentry("wall_k", 50.0, 0.0, 1e6, 1.0);         // contact stiffness
wallAlpha = nentry("wall_alpha", 1.5, 1.0, 3.0, 0.01);     // contact nonlinearity
wallZ     = nentry("wall_z", 5.0, 0.0, 1000.0, 0.01);      // contact damping (stabilises)
wallFmax  = nentry("wall_fmax", 3.0, 0.0, 100.0, 0.01);    // contact force clamp
outGain   = nentry("out_gain", 1.0, 0.0, 5000.0, 0.1);

// --- string physics (StiffString.dsp) -----------------------------------
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

inPoint    = floor(nPoints/4);   // pluck near 1/4
bridgePoint = 6;                 // jivari contact, near the bridge end
outPoint   = 5;                  // listen at the bridge

// --- forces -------------------------------------------------------------
pluck = os.lf_imptrain(pluckRate) * pluckGain * gate : fi.lowpass(2, 800.0);

// One-sided contact: force only when the point dips below wallPos. An
// instantaneous stiff spring runs away in an explicit scheme, so we add
// velocity damping (removes energy on impact) and clamp the force. max(0,...)
// keeps the power base non-negative (select2 would evaluate both branches).
wallForce(u) = force
with {
    pen   = max(0.0, wallPos - u);          // penetration depth (>=0)
    vel   = u - u';                          // string velocity at contact point
    raw   = wallK*(pen ^ wallAlpha) - wallZ*vel*(pen > 0);  // spring + damping
    force = max(0.0, min(wallFmax, raw));    // one-sided, bounded
};

// computeForce: read state at bridgePoint -> wall force -> distribute back,
// then add the external pluck at inPoint. Input/output are nPoints buses.
computeForce =
    ( fd.linInterp1D(nPoints, bridgePoint) :> wallForce
      <: fd.linInterp1D(nPoints, bridgePoint) )
    : par(i, nPoints, _ + pluck*(i == inPoint));

string = ( computeForce : fd.model1D(nPoints, r, t, scheme(nPoints)) )
         ~ si.bus(nPoints)
         : fd.linInterp1DOut(nPoints, outPoint);

process = string * outGain : fi.lowpass(2, 12000.0) : *(amp) : *(envelope);
