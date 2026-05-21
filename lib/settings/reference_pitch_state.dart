import 'package:flutter_riverpod/flutter_riverpod.dart';

class ReferencePitch {
  const ReferencePitch({this.hz = 440.0, this.noteName = 'A4'});

  final double hz;
  final String? noteName;
}

const noteFrequencies = {
  'C3': 130.81,
  'D3': 146.83,
  'E3': 164.81,
  'F3': 174.61,
  'G3': 196.00,
  'A3': 220.00,
  'B3': 246.94,
  'C4': 261.63,
  'D4': 293.66,
  'E4': 329.63,
  'F4': 349.23,
  'G4': 392.00,
  'A4': 440.00,
  'B4': 493.88,
  'C5': 523.25,
};

class ReferencePitchNotifier extends Notifier<ReferencePitch> {
  @override
  ReferencePitch build() => const ReferencePitch();

  void setHz(double hz) {
    state = ReferencePitch(hz: hz, noteName: null);
  }

  void setFromNoteName(String name) {
    final hz = noteFrequencies[name];
    if (hz != null) {
      state = ReferencePitch(hz: hz, noteName: name);
    }
  }
}

final referencePitchProvider =
    NotifierProvider<ReferencePitchNotifier, ReferencePitch>(
  ReferencePitchNotifier.new,
);
