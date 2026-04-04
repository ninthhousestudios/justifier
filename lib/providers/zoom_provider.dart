import 'dart:math';

import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../core/persistence.dart';

const _zoomStep = 0.05;
const _zoomMin = 0.6;
const _zoomMax = 2.0;

final zoomProvider = StateProvider<double>((ref) {
  return ref.read(persistenceProvider).loadZoom();
});

void zoomIn(WidgetRef ref) {
  final value = min(_zoomMax, ref.read(zoomProvider) + _zoomStep);
  ref.read(zoomProvider.notifier).state = value;
  ref.read(persistenceProvider).saveZoom(value);
}

void zoomOut(WidgetRef ref) {
  final value = max(_zoomMin, ref.read(zoomProvider) - _zoomStep);
  ref.read(zoomProvider.notifier).state = value;
  ref.read(persistenceProvider).saveZoom(value);
}

void zoomReset(WidgetRef ref) {
  ref.read(zoomProvider.notifier).state = 1.0;
  ref.read(persistenceProvider).saveZoom(1.0);
}
