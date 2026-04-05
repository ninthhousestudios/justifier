import 'dart:math';
import 'dart:ui';

/// Returns a map of prime -> exponent for the ratio n/d.
/// Numerator contributes positive exponents, denominator negative.
/// Example: primeFactorize(15, 8) == {2: -3, 3: 1, 5: 1}
Map<int, int> primeFactorize(int n, int d) {
  final result = <int, int>{};

  void factorize(int value, int sign) {
    if (value <= 1) return;
    var remaining = value;
    var p = 2;
    while (p * p <= remaining) {
      while (remaining % p == 0) {
        result[p] = (result[p] ?? 0) + sign;
        remaining ~/= p;
      }
      p++;
    }
    if (remaining > 1) {
      result[remaining] = (result[remaining] ?? 0) + sign;
    }
  }

  factorize(n, 1);
  factorize(d, -1);
  result.removeWhere((_, exp) => exp == 0);
  return result;
}

/// Returns ratio n/d octave-reduced to [1/1, 2/1) as a reduced fraction.
(int, int) octaveReduce(int n, int d) {
  var num = n;
  var den = d;
  // Bring into [1, 2) by multiplying or dividing by 2
  while (num >= 2 * den) {
    den *= 2;
  }
  while (num < den) {
    num *= 2;
  }
  // Reduce
  final g = _gcd(num, den);
  return (num ~/ g, den ~/ g);
}

int _gcd(int a, int b) {
  while (b != 0) {
    final t = b;
    b = a % b;
    a = t;
  }
  return a;
}

/// Returns cents for ratio n/d: 1200 * log2(n/d).
double ratioToCents(int n, int d) {
  return 1200.0 * log(n / d) / ln2;
}

/// Given a view mode and a viewport rect in lattice coordinates,
/// returns all nodes whose grid positions fall within the viewport.
List<LatticeNode> generateVisibleNodes(LatticeViewMode mode, Rect viewport) {
  final nodes = <LatticeNode>[];

  final int aMin = viewport.left.ceil();
  final int aMax = viewport.right.floor();
  final int bMin = viewport.top.ceil();
  final int bMax = viewport.bottom.floor();

  int primeX;
  int primeY;
  if (mode is NestedMode) {
    primeX = 3;
    primeY = 5;
  } else if (mode is ProjectionMode) {
    primeX = mode.primeX;
    primeY = mode.primeY;
  } else {
    return nodes;
  }

  for (var a = aMin; a <= aMax; a++) {
    for (var b = bMin; b <= bMax; b++) {
      // Ratio = primeX^a * primeY^b, then octave-reduce
      // Work in log space to compute numerator/denominator before reducing
      final (n, d) = _computeRatio(primeX, a, primeY, b);
      final (rn, rd) = octaveReduce(n, d);
      final factors = primeFactorize(rn, rd);
      nodes.add(LatticeNode(
        numerator: rn,
        denominator: rd,
        primeFactors: factors,
        gridPosition: (a, b),
      ));
    }
  }
  return nodes;
}

/// Compute numerator and denominator for primeX^a * primeY^b (before octave reduction).
(int, int) _computeRatio(int primeX, int a, int primeY, int b) {
  int num = 1;
  int den = 1;

  if (a >= 0) {
    for (var i = 0; i < a; i++) {
      num *= primeX;
    }
  } else {
    for (var i = 0; i < -a; i++) {
      den *= primeX;
    }
  }

  if (b >= 0) {
    for (var i = 0; i < b; i++) {
      num *= primeY;
    }
  } else {
    for (var i = 0; i < -b; i++) {
      den *= primeY;
    }
  }

  return (num, den);
}

/// For a given node, compute neighbors at the given higher primes.
/// Each prime p yields two neighbors: parent * p and parent * 1/p, octave-reduced.
/// Positions are offset radially from parent at evenly spaced angles, 0.3 units out.
List<LatticeNode> getHigherPrimeNeighbors(
    LatticeNode parent, List<int> primes) {
  final nodes = <LatticeNode>[];
  final totalNeighbors = primes.length * 2;
  final (pa, pb) = parent.gridPosition;

  for (var i = 0; i < primes.length; i++) {
    final p = primes[i];

    // Upward neighbor: parent * p
    final upAngle = 2 * pi * (i * 2) / totalNeighbors;
    final upOffsetA = (pa + 0.3 * cos(upAngle));
    final upOffsetB = (pb + 0.3 * sin(upAngle));
    final (upN, upD) = octaveReduce(parent.numerator * p, parent.denominator);
    nodes.add(LatticeNode(
      numerator: upN,
      denominator: upD,
      primeFactors: primeFactorize(upN, upD),
      gridPosition: (upOffsetA.round(), upOffsetB.round()),
    ));

    // Downward neighbor: parent * 1/p
    final downAngle = 2 * pi * (i * 2 + 1) / totalNeighbors;
    final downOffsetA = (pa + 0.3 * cos(downAngle));
    final downOffsetB = (pb + 0.3 * sin(downAngle));
    final (downN, downD) =
        octaveReduce(parent.numerator, parent.denominator * p);
    nodes.add(LatticeNode(
      numerator: downN,
      denominator: downD,
      primeFactors: primeFactorize(downN, downD),
      gridPosition: (downOffsetA.round(), downOffsetB.round()),
    ));
  }

  return nodes;
}

// ---------------------------------------------------------------------------
// Data classes
// ---------------------------------------------------------------------------

/// An immutable node in the JI lattice.
class LatticeNode {
  const LatticeNode({
    required this.numerator,
    required this.denominator,
    required this.primeFactors,
    required this.gridPosition,
  });

  final int numerator;
  final int denominator;
  final Map<int, int> primeFactors;
  final (int, int) gridPosition; // (a, b) on current axes
}

/// Sealed view-mode hierarchy.
sealed class LatticeViewMode {
  const LatticeViewMode();
}

class NestedMode extends LatticeViewMode {
  const NestedMode();
}

class ProjectionMode extends LatticeViewMode {
  const ProjectionMode({required this.primeX, required this.primeY});
  final int primeX;
  final int primeY;
}

/// Immutable lattice UI state.
class LatticeState {
  const LatticeState({
    this.viewMode = const NestedMode(),
    this.spawnOctave = 1,
    this.expandedNodes = const {},
    this.previewVoiceId,
    this.previewRatio,
    this.hoveredRatio,
    this.dividerFraction = 0.6,
    this.showLabels = false,
  });

  static const _sentinel = Object();

  final LatticeViewMode viewMode;
  final int spawnOctave;
  final Set<(int, int)> expandedNodes;
  final int? previewVoiceId;
  final (int, int)? previewRatio;
  final (int, int)? hoveredRatio;
  final double dividerFraction;
  final bool showLabels;

  LatticeState copyWith({
    LatticeViewMode? viewMode,
    int? spawnOctave,
    Set<(int, int)>? expandedNodes,
    Object? previewVoiceId = _sentinel,
    Object? previewRatio = _sentinel,
    Object? hoveredRatio = _sentinel,
    double? dividerFraction,
    bool? showLabels,
  }) {
    return LatticeState(
      viewMode: viewMode ?? this.viewMode,
      spawnOctave: spawnOctave ?? this.spawnOctave,
      expandedNodes: expandedNodes ?? this.expandedNodes,
      previewVoiceId: previewVoiceId == _sentinel
          ? this.previewVoiceId
          : previewVoiceId as int?,
      previewRatio: previewRatio == _sentinel
          ? this.previewRatio
          : previewRatio as (int, int)?,
      hoveredRatio: hoveredRatio == _sentinel
          ? this.hoveredRatio
          : hoveredRatio as (int, int)?,
      dividerFraction: dividerFraction ?? this.dividerFraction,
      showLabels: showLabels ?? this.showLabels,
    );
  }
}
