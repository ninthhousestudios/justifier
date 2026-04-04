import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shared_preferences/shared_preferences.dart';

/// Provider for the SharedPreferences instance, initialized in main().
final sharedPrefsProvider = Provider<SharedPreferences>((ref) {
  throw UnimplementedError('Must be overridden in ProviderScope');
});

class PersistenceService {
  PersistenceService(this._prefs);
  final SharedPreferences _prefs;

  // ── Zoom ──

  void saveZoom(double scale) {
    _prefs.setDouble('zoom', scale);
  }

  double loadZoom() {
    return _prefs.getDouble('zoom') ?? 1.0;
  }

  // ── Theme ──

  void saveTheme(String themeName) {
    _prefs.setString('theme', themeName);
  }

  String loadTheme() {
    return _prefs.getString('theme') ?? 'dark';
  }
}

/// Provider for PersistenceService.
final persistenceProvider = Provider<PersistenceService>((ref) {
  return PersistenceService(ref.watch(sharedPrefsProvider));
});
