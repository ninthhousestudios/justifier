first decision:

1/1 is set for the whole app globaly. on the settings screen


tuner will have all options configurable:

octave reduce
if on, all ratios between 1 and 2
if off, ratio can be anything, specifically: if sounding pitch is below 1/1, then the
ratio will be < 1

which ratios are we aiming for?
will have multiple config layers
basic will be, 3-limit, 5-limit, 7-limit, etc.

however, this itself has issues because:
1) 7-limit generally means to include all primes up to and including 7
but, the well-tuned piano for instance, is technically 7-limit but does not include 5
2) there are many 5-limit ratios, e.g., 9/8 and 10/9 are very close, so 5-limit wont be
every 5-limit ratio

so, ultimately, there will be presets, but also the user should be able to choose all
the ratios they want to the tuner to target.

thats a pretty big design thing i think, because there are a lot of possibilites. but as
i said we do it in layers in the configuration for the tuner. most users wont need or want full control

these tuner settings should all be on the tuner page, not on the settings page, i think.
though the global 1/1 will be on the settings page...although we should allow it to be
changed elsewhere im thinking. but still the app always only has one signle global 1/1 

defaults:
the tuner will ship with defaults
basically, 5-limit, meaning the 12 ratios that are easiest and standard 5-limit ones

# ways of smoothing tune

Current state: The green zone is effectively 0–5 cents, the IIR smoothing has α=0.15 (~220ms time constant), and there's no hysteresis on the
  color transitions. For a bowed string with natural pitch wobble, 5 cents is genuinely tight — professional vibrato alone is ±10-30 cents.

  Here are the levers we could pull, roughly from simplest to most involved:

  1. Widen the green zone — Move the green→amber crossover from ~5¢ to 8–10¢. Trivial change (_deviationScale and the 0.17 breakpoint in
  _color()). Downside: less precise feedback for someone who actually wants to nail sub-5¢.
  2. More aggressive smoothing — Drop α from 0.15 to ~0.06 (time constant ~550ms). Averages out natural bow wobble. Downside: sluggish response
  to intentional pitch changes.
  3. Hysteresis on color state — Once green, stay green until deviation exceeds a wider threshold (say 8¢), but require ≤5¢ to enter green.
  Prevents the flickering at the boundary without widening the zone for initial lock-on. Moderate complexity.
  4. Adaptive smoothing — Use heavy smoothing when pitch is stable (sustaining a note), light smoothing when it's changing fast (adjusting). Best
   of both worlds but more moving parts.
  5. Median filter stage — Add a small sliding-window median (5–7 samples) before the IIR. Excellent at rejecting harmonic-jump outliers. Can
  combine with any of the above.
  6. User-configurable tolerance — Put a sensitivity slider in the tuner settings drawer. Let the player choose how strict it is.
