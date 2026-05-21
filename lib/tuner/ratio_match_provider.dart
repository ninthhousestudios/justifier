import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../settings/reference_pitch_state.dart';
import 'pitch_state.dart';
import 'ratio.dart';

const _alpha = 0.15;

class RatioMatchNotifier extends Notifier<RatioMatch?> {
  double _smoothedCents = 0;
  JiRatio? _lastRatio;

  @override
  RatioMatch? build() {
    final pitch = ref.watch(pitchProvider);
    final reference = ref.watch(referencePitchProvider);

    if (!pitch.isValid) {
      _lastRatio = null;
      return null;
    }

    final raw = findNearestRatio(pitch.hz, reference.hz, defaultFiveLimitRatios);

    if (_lastRatio != raw.ratio) {
      _smoothedCents = raw.deviationCents;
      _lastRatio = raw.ratio;
    } else {
      _smoothedCents = _alpha * raw.deviationCents + (1 - _alpha) * _smoothedCents;
    }

    return RatioMatch(ratio: raw.ratio, deviationCents: _smoothedCents);
  }
}

final ratioMatchProvider =
    NotifierProvider<RatioMatchNotifier, RatioMatch?>(
  RatioMatchNotifier.new,
);
