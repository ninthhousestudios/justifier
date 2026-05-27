import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../audio/audio_engine.dart';
import '../audio/engine_provider.dart';
import '../audio/waveform_type.dart';
import '../settings/reference_pitch_state.dart';

class DroneVoice {
  const DroneVoice({
    required this.id,
    this.frequency = 440.0,
    this.amplitude = 0.5,
    this.gateOn = true,
    this.useRatio = true,
    this.numerator = 1,
    this.denominator = 1,
  });

  final int id;
  final double frequency;
  final double amplitude;
  final bool gateOn;
  final bool useRatio;
  final int numerator;
  final int denominator;

  DroneVoice copyWith({
    double? frequency,
    double? amplitude,
    bool? gateOn,
    bool? useRatio,
    int? numerator,
    int? denominator,
  }) {
    return DroneVoice(
      id: id,
      frequency: frequency ?? this.frequency,
      amplitude: amplitude ?? this.amplitude,
      gateOn: gateOn ?? this.gateOn,
      useRatio: useRatio ?? this.useRatio,
      numerator: numerator ?? this.numerator,
      denominator: denominator ?? this.denominator,
    );
  }

  double ratioHz(double refHz) => refHz * numerator / denominator;

  String frequencyLabel(double refHz) {
    final hz = useRatio ? ratioHz(refHz) : frequency;
    if (hz >= 1000) {
      return '${(hz / 1000).toStringAsFixed(2)} kHz';
    }
    return '${hz.toStringAsFixed(1)} Hz';
  }

  String get ratioLabel => '$numerator/$denominator';
}

class DroneNotifier extends Notifier<List<DroneVoice>> {
  AudioEngine get _engine => ref.read(engineProvider);
  double get _refHz => ref.read(referencePitchProvider).hz;

  @override
  List<DroneVoice> build() {
    ref.listen(referencePitchProvider, (_, next) {
      _onReferencePitchChanged(next.hz);
    });
    return [];
  }

  void _onReferencePitchChanged(double refHz) {
    for (final v in state) {
      if (v.useRatio) {
        _engine.setFrequency(v.id, v.ratioHz(refHz));
      }
    }
    state = [...state];
  }

  void addVoice({double? frequency, double amplitude = 0.5}) {
    final hz = frequency ?? _refHz;
    final id = _engine.addVoice(WaveformType.sine, hz, amplitude);
    if (id < 0) return;
    _engine.setGate(id, true);
    state = [
      ...state,
      DroneVoice(id: id, frequency: hz, amplitude: amplitude),
    ];
  }

  void removeVoice(int id) {
    _engine.removeVoice(id);
    state = state.where((v) => v.id != id).toList();
  }

  void setFrequency(int id, double hz) {
    final clamped = hz.clamp(20.0, 20000.0);
    _engine.setFrequency(id, clamped);
    state = [
      for (final v in state)
        if (v.id == id) v.copyWith(frequency: clamped) else v,
    ];
  }

  void setAmplitude(int id, double amplitude) {
    final clamped = amplitude.clamp(0.0, 1.0);
    _engine.setAmplitude(id, clamped);
    state = [
      for (final v in state)
        if (v.id == id) v.copyWith(amplitude: clamped) else v,
    ];
  }

  void toggleGate(int id) {
    final voice = state.firstWhere((v) => v.id == id);
    final newGate = !voice.gateOn;
    _engine.setGate(id, newGate);
    state = [
      for (final v in state)
        if (v.id == id) v.copyWith(gateOn: newGate) else v,
    ];
  }

  void setRatioMode(int id, bool useRatio) {
    final voice = state.firstWhere((v) => v.id == id);
    if (voice.useRatio == useRatio) return;
    final updated = voice.copyWith(useRatio: useRatio);
    if (useRatio) {
      _engine.setFrequency(id, updated.ratioHz(_refHz));
    }
    state = [
      for (final v in state) if (v.id == id) updated else v,
    ];
  }

  void setNumerator(int id, int n) {
    if (n < 1) return;
    final voice = state.firstWhere((v) => v.id == id);
    final updated = voice.copyWith(numerator: n);
    _engine.setFrequency(id, updated.ratioHz(_refHz));
    state = [
      for (final v in state) if (v.id == id) updated else v,
    ];
  }

  void setDenominator(int id, int d) {
    if (d < 1) return;
    final voice = state.firstWhere((v) => v.id == id);
    final updated = voice.copyWith(denominator: d);
    _engine.setFrequency(id, updated.ratioHz(_refHz));
    state = [
      for (final v in state) if (v.id == id) updated else v,
    ];
  }

  void removeAll() {
    for (final v in state) {
      _engine.removeVoice(v.id);
    }
    state = [];
  }
}

final droneProvider =
    NotifierProvider<DroneNotifier, List<DroneVoice>>(DroneNotifier.new);
