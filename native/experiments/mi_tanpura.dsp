// mi_tanpura.dsp — mass-interaction jivari experiment (mi.lib)
//
// Minimal authentic-collision model: one resonating mass held by a spring
// (sets the pitch), colliding against a rigid boundary (mi.ground) placed
// just below it — the jivari bridge. On each downswing the mass chatters
// against the boundary (nlCollisionClipped), injecting the buzzing harmonics
// a waveguide can't produce. Self-plucked by a periodic force impulse.
//
// Hand-authored routing following the MIMS pattern from the mi-faust examples.

import("stdfaust.lib");
import("mi.lib");

// --- tunables -----------------------------------------------------------
pluckRate = nentry("pluck_rate", 0.5, 0.05, 4.0, 0.01);
pluckGain = nentry("pluck_gain", 0.5, 0.0, 5.0, 0.001);
K         = nentry("k", 0.1, 0.0001, 1.0, 0.0001);     // string tension -> pitch
Z         = nentry("z", 0.0001, 0.0, 0.01, 0.00001);   // string damping -> decay
boundary  = nentry("boundary", -0.02, -0.2, 0.0, 0.0001); // bridge offset below rest
thres     = nentry("thres", 0.005, 0.0, 0.05, 0.0001);   // contact threshold
collS     = nentry("coll_s", 3.0, 0.0, 20.0, 0.01);     // collision stiffness
gate      = nentry("gate", 1, 0, 1, 1);

OutGain = nentry("out_gain", 1.0, 0.0, 100.0, 0.001);

// Self-plucking: periodic force impulse into the mass.
in1 = os.lf_imptrain(pluckRate) * pluckGain * gate;

model = (
    mi.mass(1.0, 0, 0., 0.),     // m0: the string mass
    mi.ground(0.),               // m1: tension anchor (rest position)
    mi.ground(boundary),         // m2: jivari bridge boundary, just below
    par(i, nbFrcIn, _):
    RoutingMassToLink,
    par(i, nbFrcIn, _):
    mi.spring(K, 0., 0.),                                // l0: tension spring (m1-m0)
    mi.damper(Z, 0., 0.),                                // l1: damping (m1-m0)
    mi.nlCollisionClipped(collS, 0.5, 6.0, 0.2, thres, 0., 0.),  // l2: bridge (m2-m0)
    par(i, nbOut + nbFrcIn, _):
    RoutingLinkToMass
)~par(i, nbMass, _):
par(i, nbMass, !), par(i, nbOut, _)
with {
    // positions fed to each link, in link order, then the output tap
    RoutingMassToLink(m0, m1, m2) =
        m1, m0,   // spring: x1=anchor, x2=mass
        m1, m0,   // damper: x1=anchor, x2=mass
        m2, m0,   // collision: x1=boundary, x2=mass (contact when mass dips low)
        m0;       // output tap: mass position
    // sum forces back onto each mass; pass through the output tap
    RoutingLinkToMass(l0_f1, l0_f2, l1_f1, l1_f2, l2_f1, l2_f2, p_out1, f_in1) =
        f_in1 + l0_f2 + l1_f2 + l2_f2,   // m0 (mass): pluck + spring + damper + collision
        l0_f1 + l1_f1,                   // m1 (anchor)
        l2_f1,                           // m2 (boundary)
        p_out1;
    nbMass = 3;
    nbFrcIn = 1;
    nbOut = 1;
};

process = in1 : model : *(OutGain);
