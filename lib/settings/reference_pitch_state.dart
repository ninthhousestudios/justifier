import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'prefs_provider.dart';

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
  static const _keyHz = 'reference.hz';
  static const _keyNoteName = 'reference.noteName';

  SharedPreferences get _prefs => ref.read(sharedPrefsProvider);

  @override
  ReferencePitch build() {
    final hz = _prefs.getDouble(_keyHz) ?? 440.0;
    final noteName = _prefs.getString(_keyNoteName) ?? 'A4';
    return ReferencePitch(hz: hz, noteName: noteName);
  }

  void _save() {
    _prefs.setDouble(_keyHz, state.hz);
    if (state.noteName != null) {
      _prefs.setString(_keyNoteName, state.noteName!);
    } else {
      _prefs.remove(_keyNoteName);
    }
  }

  void setHz(double hz) {
    state = ReferencePitch(hz: hz, noteName: null);
    _save();
  }

  void setFromNoteName(String name) {
    final hz = noteFrequencies[name];
    if (hz != null) {
      state = ReferencePitch(hz: hz, noteName: name);
      _save();
    }
  }
}

final referencePitchProvider =
    NotifierProvider<ReferencePitchNotifier, ReferencePitch>(
  ReferencePitchNotifier.new,
);
