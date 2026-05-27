import 'dart:math';

class JiRatio {
  const JiRatio(this.numerator, this.denominator, {this.name});

  final int numerator;
  final int denominator;
  final String? name;

  double get decimal => numerator / denominator;
  double get cents => 1200 * log(decimal) / ln2;
  String get label => '$numerator/$denominator';

  int get primeLimit {
    var n = numerator, d = denominator;
    for (final p in [2, 3]) {
      while (n % p == 0) { n ~/= p; }
      while (d % p == 0) { d ~/= p; }
    }
    if (n == 1 && d == 1) return 3;
    while (n % 5 == 0) { n ~/= 5; }
    while (d % 5 == 0) { d ~/= 5; }
    if (n == 1 && d == 1) return 5;
    while (n % 7 == 0) { n ~/= 7; }
    while (d % 7 == 0) { d ~/= 7; }
    if (n == 1 && d == 1) return 7;
    return 11;
  }

  @override
  String toString() => name != null ? '$label ($name)' : label;

  @override
  bool operator ==(Object other) =>
      other is JiRatio &&
      other.numerator == numerator &&
      other.denominator == denominator;

  @override
  int get hashCode => Object.hash(numerator, denominator);
}

class RatioMatch {
  const RatioMatch({required this.ratio, required this.deviationCents});

  final JiRatio ratio;
  final double deviationCents;
}

// ---------------------------------------------------------------------------
// Built-in ratio library
// ---------------------------------------------------------------------------

const allBuiltInRatios = [
  // 3-limit (Pythagorean)
  JiRatio(1, 1, name: 'unison'),
  JiRatio(9, 8, name: 'major second'),
  JiRatio(32, 27, name: 'Pythagorean minor third'),
  JiRatio(81, 64, name: 'Pythagorean major third'),
  JiRatio(4, 3, name: 'perfect fourth'),
  JiRatio(3, 2, name: 'perfect fifth'),
  JiRatio(27, 16, name: 'Pythagorean major sixth'),
  JiRatio(16, 9, name: 'Pythagorean minor seventh'),
  // 5-limit
  JiRatio(16, 15, name: 'minor second'),
  JiRatio(10, 9, name: 'minor whole tone'),
  JiRatio(6, 5, name: 'minor third'),
  JiRatio(5, 4, name: 'major third'),
  JiRatio(45, 32, name: 'tritone'),
  JiRatio(8, 5, name: 'minor sixth'),
  JiRatio(5, 3, name: 'major sixth'),
  JiRatio(9, 5, name: 'minor seventh'),
  JiRatio(15, 8, name: 'major seventh'),
  // 7-limit
  JiRatio(8, 7, name: 'septimal major second'),
  JiRatio(7, 6, name: 'septimal minor third'),
  JiRatio(9, 7, name: 'septimal major third'),
  JiRatio(7, 5, name: 'septimal tritone'),
  JiRatio(10, 7, name: "Euler's tritone"),
  JiRatio(14, 9, name: 'septimal minor sixth'),
  JiRatio(12, 7, name: 'septimal major sixth'),
  JiRatio(7, 4, name: 'harmonic seventh'),
];

// ---------------------------------------------------------------------------
// Curated presets
// ---------------------------------------------------------------------------

const preset3Limit = [
  JiRatio(1, 1, name: 'unison'),
  JiRatio(9, 8, name: 'major second'),
  JiRatio(32, 27, name: 'Pythagorean minor third'),
  JiRatio(81, 64, name: 'Pythagorean major third'),
  JiRatio(4, 3, name: 'perfect fourth'),
  JiRatio(3, 2, name: 'perfect fifth'),
  JiRatio(27, 16, name: 'Pythagorean major sixth'),
  JiRatio(16, 9, name: 'Pythagorean minor seventh'),
];

const preset5Limit = [
  JiRatio(1, 1, name: 'unison'),
  JiRatio(16, 15, name: 'minor second'),
  JiRatio(9, 8, name: 'major second'),
  JiRatio(6, 5, name: 'minor third'),
  JiRatio(5, 4, name: 'major third'),
  JiRatio(4, 3, name: 'perfect fourth'),
  JiRatio(45, 32, name: 'tritone'),
  JiRatio(3, 2, name: 'perfect fifth'),
  JiRatio(8, 5, name: 'minor sixth'),
  JiRatio(5, 3, name: 'major sixth'),
  JiRatio(9, 5, name: 'minor seventh'),
  JiRatio(15, 8, name: 'major seventh'),
];

const preset7Limit = [
  JiRatio(1, 1, name: 'unison'),
  JiRatio(16, 15, name: 'minor second'),
  JiRatio(9, 8, name: 'major second'),
  JiRatio(8, 7, name: 'septimal major second'),
  JiRatio(7, 6, name: 'septimal minor third'),
  JiRatio(6, 5, name: 'minor third'),
  JiRatio(5, 4, name: 'major third'),
  JiRatio(9, 7, name: 'septimal major third'),
  JiRatio(4, 3, name: 'perfect fourth'),
  JiRatio(7, 5, name: 'septimal tritone'),
  JiRatio(45, 32, name: 'tritone'),
  JiRatio(3, 2, name: 'perfect fifth'),
  JiRatio(8, 5, name: 'minor sixth'),
  JiRatio(5, 3, name: 'major sixth'),
  JiRatio(12, 7, name: 'septimal major sixth'),
  JiRatio(7, 4, name: 'harmonic seventh'),
  JiRatio(9, 5, name: 'minor seventh'),
  JiRatio(15, 8, name: 'major seventh'),
];

// ---------------------------------------------------------------------------
// Matching
// ---------------------------------------------------------------------------

RatioMatch findNearestRatio(
  double detectedHz,
  double referenceHz,
  List<JiRatio> targets,
) {
  var rawRatio = detectedHz / referenceHz;

  // Octave-reduce to [1, 2)
  while (rawRatio < 1) {
    rawRatio *= 2;
  }
  while (rawRatio >= 2) {
    rawRatio /= 2;
  }

  final rawCents = 1200 * log(rawRatio) / ln2;

  var bestMatch = targets.first;
  var bestDistance = (rawCents - bestMatch.cents).abs();

  for (var i = 1; i < targets.length; i++) {
    final distance = (rawCents - targets[i].cents).abs();
    if (distance < bestDistance) {
      bestDistance = distance;
      bestMatch = targets[i];
    }
  }

  // Wrap-around: check distance to 1/1 as if it were at 1200¢ (octave)
  final distToOctave = (1200 - rawCents).abs();
  if (distToOctave < bestDistance) {
    bestMatch = targets.first;
    bestDistance = distToOctave;
  }

  var deviation = rawCents - bestMatch.cents;
  // If we matched via octave wrap-around, deviation should be positive
  // (sharp of the lower octave copy)
  if (bestMatch.cents == 0 && rawCents > 600) {
    deviation = rawCents - 1200;
  }

  return RatioMatch(ratio: bestMatch, deviationCents: deviation);
}

// ---------------------------------------------------------------------------
// Serialization helpers (for persistence)
// ---------------------------------------------------------------------------

String serializeRatio(JiRatio r) {
  return r.name != null ? '${r.numerator}/${r.denominator}:${r.name}' : r.label;
}

JiRatio? parseRatio(String s) {
  final colonIdx = s.indexOf(':');
  final String numDen;
  final String? name;
  if (colonIdx >= 0) {
    numDen = s.substring(0, colonIdx);
    name = s.substring(colonIdx + 1);
  } else {
    numDen = s;
    name = null;
  }
  final parts = numDen.split('/');
  if (parts.length != 2) return null;
  final n = int.tryParse(parts[0]);
  final d = int.tryParse(parts[1]);
  if (n == null || d == null || n <= 0 || d <= 0) return null;

  // Prefer built-in definition for canonical names
  for (final r in allBuiltInRatios) {
    if (r.numerator == n && r.denominator == d) return r;
  }
  return JiRatio(n, d, name: name);
}
