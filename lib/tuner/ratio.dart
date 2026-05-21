import 'dart:math';

class JiRatio {
  const JiRatio(this.numerator, this.denominator, {this.name});

  final int numerator;
  final int denominator;
  final String? name;

  double get decimal => numerator / denominator;
  double get cents => 1200 * log(decimal) / ln2;
  String get label => '$numerator/$denominator';

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

const defaultFiveLimitRatios = [
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
