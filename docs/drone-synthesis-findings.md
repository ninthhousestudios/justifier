# Drone synthesis exploration — tanpura & bowed string

Findings from exploring physical-modeling approaches for rich drone timbres
to sing against (yojana justifier/11). Goal: highest-quality sustained tone,
not a perfect instrument simulation.

## Tooling

Offline Faust→WAV render + spectral analysis, so timbres are iterable without
launching the app:

- `native/tools/render.sh <dsp> <out.wav> <secs> [param=value ...]` — compiles
  any `.dsp` (in `native/dsp/` or `native/experiments/`) and renders it. Sets
  `gate=1`, `amp=0.8` by default; pass any param by suffix name.
- `native/tools/analyze.py <wav> [--f0 Hz] [--at sec]` — prints the harmonic
  series (dB), spectral centroid, and a "richness" proxy (energy above 5·f0).
  Lets you *see* a timbre: rich = many strong harmonics + high centroid.
- `native/tools/render.cpp` — the harness (instantiates the Faust `mydsp`,
  sets params via MapUI suffix-match exactly like the app's faust_wrapper).

"Richness" reference points: pure sine ~0%, pm.lib viola ~15%, KS tanpura ~27%.

## Approaches compared

| Approach | File | Pitch | Richness | Verdict |
|---|---|---|---|---|
| pm.lib viola (shipped) | `native/dsp/bowed_string.dsp` | exact | 15% | immediate, controllable, a bit dark |
| **KS + jivari delay-mod** | `native/experiments/tanpura.dsp` | exact | 27% | **best candidate**: stable, real-time, tunable, musical |
| mi.lib bowed string | `native/experiments/mi_bowed.dsp` | fixed by mesh | very high | rich/gritty but slow to speak, scratchy, hard to pitch |
| mi.lib collision jivari | `native/experiments/mi_tanpura.dsp` | tunable (odd units) | 50–63% when engaged | authentic buzz, but numerically delicate |

## The KS tanpura (recommended for integration)

`native/experiments/tanpura.dsp` — a Karplus-Strong string with a
noise-modulated delay line (the Faust-STK sitar mechanism) approximating the
jivari bridge's shimmer, self-plucking on a slow internal cycle so one voice is
a continuous evolving drone. Standard voice interface plus tunable knobs:
`pluck_rate`, `jivari`, `brightness`, `sustain_time`.

Measured: each pluck blooms the centroid ~8kHz→700Hz over ~1.5s (the jivari
formant sweep), woody (not clicky) attack, stable, ~11dB swell between plucks
(continuous bed, never vanishes). 4-string JI ensemble (Pa–Sa–Sa–Sa) renders
with exact ratios (5th=1.4999, octave=1.9998).

Samples: `native/tools/renders/tanpura_v2.wav` (single), `tanpura_ensemble.wav`.

## mi.lib findings

mi.lib is low-level mass-interaction primitives (mass, spring, damper, nlBow,
nlCollisionClipped). Full instruments are MIMS-generated explicit networks
(the bundled bowed string is ~150 masses of generated routing).

- **Bowed string** (mi.lib's strength — friction/bow): self-plays when driven
  with a slow constant-velocity bow gesture. Very rich and gritty (steady
  centroid ~4500Hz vs viola's 685Hz), but takes ~3–5s to build self-oscillation
  (bad for a toggle-on drone) and is scratchier/less controllable than the
  shipped pm.lib viola. Sample: `mi_bowed.wav`.
- **Collision jivari** (`nlCollisionClipped`): a single resonating mass colliding
  with a rigid boundary below it (the bridge). Hand-authored the routing — it
  works and the collision *does* inject the buzz (richness 50–63%). BUT: mi.lib
  uses arbitrary position units (pitch via `cos ω = 1−K/2M`), and the energy
  balance is narrow — too little collision = plain tone, too much = runaway to
  the clip-bound. A single mass is also too crude (one mode, no modal richness
  or true formant sweep); a faithful tanpura needs a multi-mass string with a
  near-bridge collision, which needs MIMS to generate. Sample (normalized,
  collision-engaged regime): `mi_tanpura_buzz.wav`.

## fds.lib findings (finite-difference + rigid-wall jivari)

Built on Faust's `StiffString.dsp` (real string physics: tension, stiffness,
damping → real pitch) and coupled a one-sided rigid wall at a near-bridge point,
mirroring `HammeredString.dsp`'s feedback coupling. Files:
`native/experiments/fds_string.dsp` (linear validation) and `fds_tanpura.dsp`
(with the wall). Sample: `renders/fds_tanpura_buzz.wav`.

- **Linear FD stiff string**: excellent — stable, exact pitch (tracks tension),
  authentic string physics. But mellow on its own (richness ~7%), no buzz.
  Required band-limiting the pluck: a raw 1-sample Dirac dumps energy at Nyquist
  and destabilises the explicit scheme.
- **Rigid-wall jivari**: produces a genuinely rich (65%), bright (centroid
  ~2.5kHz), *sustained* buzz — the real javari shimmer, harmonics out to ~3.3kHz.
  BUT the explicit scheme is numerically fraught:
  - A conservative stiff contact gains energy numerically and **blows up**
    (raw values → 1e37, NaN).
  - Stabilising it (velocity damping + force clamp) works, but the contact then
    becomes a self-oscillating energy *source* — a limit cycle whose frequency
    does **not** track the string tuning (T=80→467Hz, 150→237, 300→241; gentle
    contact was even wilder: 936/465/479/238). It never decays naturally.
  - For a just-intonation tuner drone, **exact, controllable pitch is
    non-negotiable** — this is the blocker.
- Root cause is well known: stable, pitch-preserving collision needs an
  *implicit / energy-conserving* update (Bilbao, van Walstijn), solving an
  implicit equation per sample — not expressible in plain Faust feedback.

Net: all three physical jivari routes have a fatal flaw for this use case — KS
(stable/exact but sounds cheap), mi.lib (buzz ⇔ instability), fds.lib (authentic
buzz but uncontrollable pitch). A production tanpura likely needs a *signal*
model of the jivari (impose its spectrum/formant-sweep on a stable, exact-pitch
string) rather than a physical collision — or a sample-based approach.

## Recommendation

1. **Integrate the KS tanpura** as a drone timbre — it's the most musical,
   stable, controllable result and is real-time cheap.
2. If chasing maximum authenticity for the jivari later, the literature and
   `docs/tanpura-ideas.md` both point to **`fds.lib`** (finite-difference,
   energy-conserving string + rigid-wall boundary) as better suited than mi.lib
   for the boundary collision — stable by construction, the string chatters
   against the wall automatically. That's the recommended next experiment.

## Side fix found

`jf_filter` (shared by every voice) called `fi.resonlp/hp/bp` without the `gain`
argument, so Faust fed the signal into the gain slot too — squaring/distorting
every voice (the shipped sine and viola included). Fixed + regenerated; see
yojana justifier/12.
