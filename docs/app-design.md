## justifier — app design

a just intonation education app.

### vision

justifier teaches people to hear and produce just intonation intervals. the
primary mode is experiential — listening first, then producing (singing or
playing), with theory available but not required. the goal is a solid
foundation in JI from the inside out: hearing it, producing it, feeling it in
the body.

this is not a composition tool or a complete music theory course. the boundary
is: fundamentals of just intonation through listening and producing. temperament
history and higher primes are in scope because they are about hearing.
composing in JI is out of scope.

### origin

the app grows out of two things: a personal journey from "am i tone deaf?" to
"i am temperament deaf" (see musical-memoir.txt), and a tuning theory primer
that walks through the harmonic series, pythagorean tuning, 5-limit JI,
temperaments, and 12TET (see tuning-theory-primer.txt). the primer is
essentially the curriculum in prose form; the app makes it interactive.

### audience

anyone, potentially. realistically, people with some musical foundation will be
most attracted, but the app should be accessible to someone with no background.
the onboarding handles both — a musician can skip ahead, a beginner follows the
guided path.

---

### platform and tech

- **mobile first** (android and ios). desktop deferred to a later version.
- **flutter** for UI.
- **existing audio engine** (C/C++, faust DSP, miniaudio) via dart:ffi. the
  engine already produces JI drones with multiple waveforms, effects, ADSR,
  and a 32-voice pool. needs build plumbing for mobile (NDK/gradle for
  android, static linking for ios) but no architectural changes.
- **pitch detection** is new work. needed for sing-along exercises. likely
  YIN or mcleod pitch method, running on captured audio from miniaudio in
  duplex mode. the detected pitch is shown as a ratio relative to the drone
  root.

### mobile build requirements

the current engine runs on desktop (linux, macos, windows). porting to mobile
requires:

- pre-generate faust DSP C++ files (faust compiler unavailable during mobile
  builds)
- android: add CMake to gradle via NDK, add `Platform.isAndroid` library
  loading in dart
- ios: static linking or framework bundle (no `DynamicLibrary.open(path)` on
  ios), AVAudioSession category setup, podspec wiring
- remove/guard `window_manager` dependency (desktop-only)
- fix reverb.dsp `fsmax = 48000` hardcoding (will produce incorrect reverb
  on devices with different sample rates)

---

### navigation (v1, mobile)

bottom navigation bar with four tabs:

| tab | purpose |
|-----|---------|
| **home** | the lesson map — curriculum navigation, progress |
| **tuner** | standalone JI tuner — shows ratio relative to reference pitch |
| **drone** | standalone drone player — pick a root, pick intervals, play |
| **settings** | themes, progress, preferences, reference materials |

studio/sound design (the full waveform/effects/lattice playground from current
justifier) is deferred to desktop version. it is a power-user feature that
benefits from screen space and is not core to the education mission.

---

### lesson structure

each lesson is self-contained. the user does not leave the lesson to use tools.
the tuner and drone are **embedded widgets** within the lesson, not separate
screens.

a lesson flows through up to four phases:

1. **listen** — hear the concept. A/B comparisons (JI vs 12TET), drone
   examples, recordings. the app plays; the user listens.

2. **produce** — try it yourself. the lesson starts a drone and activates the
   embedded tuner. the user sings or plays. real-time feedback shows what ratio
   they are hitting and how close to pure they are.

3. **feel** — reinforcement. the detune slider (from current justifier) lets
   the user intentionally detune to hear beats, then tune back to pure. builds
   body-level awareness of what "locked in" feels like.

4. **understand** (optional) — theory text adapted from the primer. expandable,
   not forced. the math is presented honestly but accessibly. available for
   those who want it, invisible to those who don't.

not every lesson needs all four phases. the onboarding might be listen-only.
later lessons might emphasize producing. the phases are a framework, not a
rigid template.

### lesson duration

each lesson node is roughly 2-5 minutes. short enough to do one on a break,
substantial enough to learn something.

---

### the tuner

the tuner is the technical differentiator. existing tuners show cents deviation
from 12TET note names. the justifier tuner shows:

- the **ratio** you are hitting relative to the reference pitch (e.g., "3/2")
- how **locked in** you are — a visual indicator of beat frequency approaching
  zero
- the ratio name if applicable (e.g., "just perfect fifth")
- cents as secondary information (since people may be familiar with it)

the tuner thinks in ratios, not note names. it operates relative to a
reference pitch (the drone root, or a user-set frequency).

the tuner exists as:
- an **embedded widget** inside lessons (with context-appropriate prompts)
- a **standalone screen** in the tuner tab (for free practice)

same component, different context.

### the drone

the drone player provides a sustained reference pitch (or multiple pitches) for
practicing intonation against.

features:
- select a root frequency (by note name, by Hz, or by ratio relative to a
  reference)
- add intervals above the root (from a curated list: octave, fifth, fourth,
  major third, minor third, etc., labeled by ratio)
- multiple simultaneous drones
- waveform selection (sine, saw, triangle, etc. — subset of the full engine)
- the detune slider from current justifier, for hearing beats

the drone exists as:
- an **embedded widget** inside lessons (pre-configured by the lesson)
- a **standalone screen** in the drone tab (user configures freely)

lessons can open the drone pre-loaded — "now try singing a 5/4 above this
drone" starts the drone at the right pitch automatically.

---

### curriculum map

the curriculum is presented as a **map**, not a linear track. there is a
recommended path highlighted, but all accessible nodes are available. someone
who already knows what a pure fifth sounds like can skip ahead. the map should
feel inviting and adventurous, not like a homework checklist.

the style is somewhere between a clean skill tree and a path through a
landscape — visual enough to inspire exploration, simple enough not to
overwhelm or distract from the actual content.

#### curriculum arc

adapted from the tuning theory primer. each item is a lesson node or a small
cluster of related nodes.

**1. orientation (the hook)**

- the "jacob moment": hear a JI triad vs a 12TET triad. did you notice a
  difference?
- if yes: great, here is what you heard
- if no: more examples with bigger contrasts (unison vs wolf fifth, pure
  major triad vs 12TET), different timbres, suggestion to try headphones
- goal: the user has the experience of hearing that JI and 12TET are
  different

**2. foundations**

- what is a vibrating string? the harmonic series. overtones.
- what is a frequency? what is a ratio?
- the octave: 2/1. hearing that two pitches an octave apart share identity.
- what "in tune" means: a relationship between two pitches, not an absolute
  property

**3. the pure intervals (3-limit)**

- the perfect fifth: 3/2. listen, then sing/play against a drone.
- the perfect fourth: 4/3.
- stacking fifths — how pythagorean tuning works
- why the spiral of fifths never closes (incommensurability of primes)
- the pythagorean comma

**4. the sweet spot (5-limit)**

- the just major third: 5/4. the big contrast with 12TET (12TET thirds are
  far off).
- the just minor third: 6/5.
- building a 5-limit scale from combinations of 3/2 and 5/4
- the just major triad: hearing beats disappear
- the wolf fifth and the keyboard problem

**5. temperament as compromise**

- why keyboards with 12 keys created a problem
- meantone: keep the thirds, bend the fifths
- well-temperament: some fifths pure, some not; keys have character
- 12TET: equal everything, maximum flexibility, maximum compromise
- the cents system and why it is "unfortunate"

**6. higher primes (extended JI)**

- the septimal intervals: 7/4, 7/6, 7/5. alien-beautiful territory.
- 11-limit, 13-limit intervals
- where JI composition goes from here (pointers, not instruction)

**7. context and culture**

- indian classical music and the tanpura: a living JI tradition
- overtone singing
- composers: partch, riley, young, harrison, lamb, johnston, twining, radigue
- resources: books, recordings, further listening

**8. ongoing practice**

- not a lesson node but a persistent feature: the standalone tuner and drone
  tabs are the practice space. the curriculum teaches; the tools let you keep
  going.

---

### themes

three themes available in settings:

| theme | description |
|-------|-------------|
| **light** | conventional light mode for readability |
| **dark** | conventional dark mode |
| **justifier** | the signature aesthetic: black ground, amber-gold and violet accents applied as glow, cinzel display type. sacred/contemplative darkness. |

the justifier theme is the default. users can switch in settings.

---

### what v1 ships with

- mobile app (android + ios)
- bottom nav: home, tuner, drone, settings
- lesson map with curriculum through at least sections 1-5 (orientation through
  temperament)
- embedded tuner and drone within lessons
- standalone tuner (ratio-based)
- standalone drone (single or multiple, with interval selection and detune)
- pitch detection for sing-along exercises
- three themes
- audio engine ported from desktop justifier

### what v1 does not include

- studio/sound design (deferred to desktop)
- desktop builds
- monetization
- user accounts or cloud sync
- sections 6-7 of curriculum (higher primes, cultural context) — can be added
  post-launch

---

### open questions

- lesson map visual design: what does the map actually look like? how spatial
  vs. how structured? needs mockups.
- pitch detection UX: how to visualize "you are hitting 3/2" in real time in a
  way that is intuitive for non-musicians
- audio latency on mobile: needs testing on real devices, especially android
- curriculum content: the primer text needs adaptation for interactive format.
  how much text per lesson? how to handle the images/charts from the primer?
- progress model: what does "completing" a lesson mean? just visited? hit a
  pitch accuracy threshold? self-reported?
- apple review: ensure the app reads as education, not utility spam. clear
  onboarding, coherent flow, not a grab bag of tools.
