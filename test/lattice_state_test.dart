import 'dart:ui';

import 'package:flutter_test/flutter_test.dart';
import 'package:justifier/models/lattice_state.dart';

void main() {
  group('primeFactorize', () {
    test('3/2 => {2: -1, 3: 1}', () {
      expect(primeFactorize(3, 2), equals({2: -1, 3: 1}));
    });

    test('15/8 => {2: -3, 3: 1, 5: 1}', () {
      expect(primeFactorize(15, 8), equals({2: -3, 3: 1, 5: 1}));
    });

    test('1/1 => {}', () {
      expect(primeFactorize(1, 1), equals({}));
    });
  });

  group('octaveReduce', () {
    test('3/2 stays 3/2 (already in [1,2))', () {
      expect(octaveReduce(3, 2), equals((3, 2)));
    });

    test('3/1 reduces to 3/2', () {
      expect(octaveReduce(3, 1), equals((3, 2)));
    });

    test('1/3 reduces to 4/3', () {
      expect(octaveReduce(1, 3), equals((4, 3)));
    });
  });

  group('ratioToCents', () {
    test('3/2 is approximately 702 cents', () {
      expect(ratioToCents(3, 2), closeTo(701.955, 0.1));
    });
  });

  group('generateVisibleNodes', () {
    test('NestedMode 3x3 viewport yields 9 nodes', () {
      final viewport = Rect.fromLTRB(-1.5, -1.5, 1.5, 1.5);
      final nodes = generateVisibleNodes(const NestedMode(), viewport);
      // Grid positions: a in [-1,1], b in [-1,1] => 3x3 = 9 nodes
      expect(nodes.length, equals(9));
    });

    test('NestedMode origin node is 1/1', () {
      final viewport = Rect.fromLTRB(-1.5, -1.5, 1.5, 1.5);
      final nodes = generateVisibleNodes(const NestedMode(), viewport);
      final origin =
          nodes.where((n) => n.gridPosition == (0, 0)).firstOrNull;
      expect(origin, isNotNull);
      expect(origin!.numerator, equals(1));
      expect(origin.denominator, equals(1));
    });

    test('NestedMode (1,0) node is 3/2 (3^1 * 5^0 octave-reduced)', () {
      final viewport = Rect.fromLTRB(-1.5, -1.5, 1.5, 1.5);
      final nodes = generateVisibleNodes(const NestedMode(), viewport);
      final node =
          nodes.where((n) => n.gridPosition == (1, 0)).firstOrNull;
      expect(node, isNotNull);
      expect(node!.numerator, equals(3));
      expect(node.denominator, equals(2));
    });

    test('ProjectionMode with primes 3 and 7 yields correct origin node', () {
      final viewport = Rect.fromLTRB(-1.5, -1.5, 1.5, 1.5);
      final nodes = generateVisibleNodes(
          const ProjectionMode(primeX: 3, primeY: 7), viewport);
      expect(nodes.length, equals(9));
      final origin =
          nodes.where((n) => n.gridPosition == (0, 0)).firstOrNull;
      expect(origin, isNotNull);
      expect(origin!.numerator, equals(1));
      expect(origin.denominator, equals(1));
    });

    test('ProjectionMode (0,1) node uses primeY=7 => 7/4', () {
      final viewport = Rect.fromLTRB(-1.5, -1.5, 1.5, 1.5);
      final nodes = generateVisibleNodes(
          const ProjectionMode(primeX: 3, primeY: 7), viewport);
      final node =
          nodes.where((n) => n.gridPosition == (0, 1)).firstOrNull;
      expect(node, isNotNull);
      // 7^1 octave-reduced: 7 > 2, so 7/4
      expect(node!.numerator, equals(7));
      expect(node.denominator, equals(4));
    });
  });

  group('getHigherPrimeNeighbors', () {
    test('1/1 expanded with [7] gives nodes containing 7/4 and 8/7', () {
      final root = LatticeNode(
        numerator: 1,
        denominator: 1,
        primeFactors: {},
        gridPosition: (0, 0),
      );
      final neighbors = getHigherPrimeNeighbors(root, [7]);
      expect(neighbors.length, equals(2));

      final ratios =
          neighbors.map((n) => (n.numerator, n.denominator)).toSet();

      // 1/1 * 7 = 7/1 => octave-reduced = 7/4
      expect(ratios, contains((7, 4)));
      // 1/1 * 1/7 = 1/7 => octave-reduced = 8/7
      expect(ratios, contains((8, 7)));
    });
  });
}
