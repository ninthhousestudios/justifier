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
