import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shared_preferences/shared_preferences.dart';

import '../settings/prefs_provider.dart';
import 'ratio.dart';

class TunerSettings {
  const TunerSettings({
    this.octaveReduction = true,
    this.showCents = false,
    this.showHz = false,
    this.selectedRatios = preset5Limit,
    this.customRatios = const [],
    this.toleranceCents = 5.0,
  });

  final bool octaveReduction;
  final bool showCents;
  final bool showHz;
  final List<JiRatio> selectedRatios;
  final List<JiRatio> customRatios;
  final double toleranceCents;

  bool isSelected(JiRatio r) => selectedRatios.contains(r);
  bool isCustom(JiRatio r) => customRatios.contains(r);

  List<JiRatio> get activeRatios =>
      selectedRatios.isEmpty ? const [JiRatio(1, 1, name: 'unison')] : selectedRatios;

  TunerSettings copyWith({
    bool? octaveReduction,
    bool? showCents,
    bool? showHz,
    List<JiRatio>? selectedRatios,
    List<JiRatio>? customRatios,
    double? toleranceCents,
  }) {
    return TunerSettings(
      octaveReduction: octaveReduction ?? this.octaveReduction,
      showCents: showCents ?? this.showCents,
      showHz: showHz ?? this.showHz,
      selectedRatios: selectedRatios ?? this.selectedRatios,
      customRatios: customRatios ?? this.customRatios,
      toleranceCents: toleranceCents ?? this.toleranceCents,
    );
  }
}

class TunerSettingsNotifier extends Notifier<TunerSettings> {
  static const _keyOctaveReduction = 'tuner.octaveReduction';
  static const _keyShowCents = 'tuner.showCents';
  static const _keyShowHz = 'tuner.showHz';
  static const _keySelected = 'tuner.selectedRatios';
  static const _keyCustom = 'tuner.customRatios';
  static const _keyTolerance = 'tuner.toleranceCents';

  SharedPreferences get _prefs => ref.read(sharedPrefsProvider);

  @override
  TunerSettings build() => _load();

  TunerSettings _load() {
    final octaveReduction = _prefs.getBool(_keyOctaveReduction) ?? true;
    final showCents = _prefs.getBool(_keyShowCents) ?? false;
    final showHz = _prefs.getBool(_keyShowHz) ?? false;

    final selectedKeys = _prefs.getStringList(_keySelected);
    final customKeys = _prefs.getStringList(_keyCustom);

    final selectedRatios = selectedKeys != null
        ? selectedKeys.map(parseRatio).whereType<JiRatio>().toList()
        : List<JiRatio>.of(preset5Limit);

    final customRatios =
        customKeys?.map(parseRatio).whereType<JiRatio>().toList() ?? [];

    final toleranceCents = _prefs.getDouble(_keyTolerance) ?? 5.0;

    return TunerSettings(
      octaveReduction: octaveReduction,
      showCents: showCents,
      showHz: showHz,
      selectedRatios: selectedRatios,
      customRatios: customRatios,
      toleranceCents: toleranceCents,
    );
  }

  void _save() {
    _prefs.setBool(_keyOctaveReduction, state.octaveReduction);
    _prefs.setBool(_keyShowCents, state.showCents);
    _prefs.setBool(_keyShowHz, state.showHz);
    _prefs.setStringList(
      _keySelected,
      state.selectedRatios.map(serializeRatio).toList(),
    );
    _prefs.setStringList(
      _keyCustom,
      state.customRatios.map(serializeRatio).toList(),
    );
    _prefs.setDouble(_keyTolerance, state.toleranceCents);
  }

  void setOctaveReduction(bool v) {
    state = state.copyWith(octaveReduction: v);
    _save();
  }

  void setShowCents(bool v) {
    state = state.copyWith(showCents: v);
    _save();
  }

  void setShowHz(bool v) {
    state = state.copyWith(showHz: v);
    _save();
  }

  void setToleranceCents(double v) {
    state = state.copyWith(toleranceCents: v.clamp(2.0, 15.0));
    _save();
  }

  void applyPreset(List<JiRatio> preset) {
    final customSelected =
        state.selectedRatios.where((r) => state.isCustom(r)).toList();
    state = state.copyWith(selectedRatios: [...preset, ...customSelected]);
    _save();
  }

  void setGroupSelected(List<JiRatio> ratios, bool select) {
    final selected = List.of(state.selectedRatios);
    if (select) {
      for (final r in ratios) {
        if (!selected.contains(r)) selected.add(r);
      }
    } else {
      selected.removeWhere(ratios.contains);
    }
    state = state.copyWith(selectedRatios: selected);
    _save();
  }

  void toggleRatio(JiRatio ratio) {
    final selected = List.of(state.selectedRatios);
    if (selected.contains(ratio)) {
      selected.remove(ratio);
    } else {
      selected.add(ratio);
    }
    state = state.copyWith(selectedRatios: selected);
    _save();
  }

  void addCustomRatio(int numerator, int denominator, String? name) {
    final ratio = JiRatio(numerator, denominator, name: name);
    if (state.customRatios.contains(ratio) ||
        allBuiltInRatios.contains(ratio)) {
      return;
    }
    state = state.copyWith(
      customRatios: [...state.customRatios, ratio],
      selectedRatios: [...state.selectedRatios, ratio],
    );
    _save();
  }

  void removeCustomRatio(JiRatio ratio) {
    state = state.copyWith(
      customRatios: state.customRatios.where((r) => r != ratio).toList(),
      selectedRatios: state.selectedRatios.where((r) => r != ratio).toList(),
    );
    _save();
  }
}

final tunerSettingsProvider =
    NotifierProvider<TunerSettingsNotifier, TunerSettings>(
  TunerSettingsNotifier.new,
);
