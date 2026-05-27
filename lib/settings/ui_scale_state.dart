import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'prefs_provider.dart';

class UiScaleNotifier extends Notifier<double> {
  static const _key = 'ui.scale';
  static const defaultScale = 1.2;

  SharedPreferences get _prefs => ref.read(sharedPrefsProvider);

  @override
  double build() => _prefs.getDouble(_key) ?? defaultScale;

  void set(double scale) {
    state = scale.clamp(0.8, 1.6);
    _prefs.setDouble(_key, state);
  }
}

final uiScaleProvider =
    NotifierProvider<UiScaleNotifier, double>(UiScaleNotifier.new);
