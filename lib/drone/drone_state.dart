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
    this.approximateRatio = false,
    this.waveformType = WaveformType.sine,
    this.tone1Num = 3,
    this.tone1Den = 2,
  });

  final int id;
  final double frequency;
  final double amplitude;
  final bool gateOn;
  final bool useRatio;
  final int numerator;
  final int denominator;
  final bool approximateRatio;
  final WaveformType waveformType;

  // Tanpura first-tone (Pa) string, as a JI ratio against Sa. Default Pa = 3/2.
  final int tone1Num;
  final int tone1Den;

  DroneVoice copyWith({
    double? frequency,
    double? amplitude,
    bool? gateOn,
    bool? useRatio,
    int? numerator,
    int? denominator,
    bool? approximateRatio,
    WaveformType? waveformType,
    int? tone1Num,
    int? tone1Den,
  }) {
    return DroneVoice(
      id: id,
      frequency: frequency ?? this.frequency,
      amplitude: amplitude ?? this.amplitude,
      gateOn: gateOn ?? this.gateOn,
      useRatio: useRatio ?? this.useRatio,
      numerator: numerator ?? this.numerator,
      denominator: denominator ?? this.denominator,
      approximateRatio: approximateRatio ?? this.approximateRatio,
      waveformType: waveformType ?? this.waveformType,
      tone1Num: tone1Num ?? this.tone1Num,
      tone1Den: tone1Den ?? this.tone1Den,
    );
  }

  double ratioHz(double refHz) => refHz * numerator / denominator;

  double get tone1Ratio => tone1Num / tone1Den;

  String frequencyLabel(double refHz) {
    final hz = useRatio ? ratioHz(refHz) : frequency;
    if (hz >= 1000) {
      return '${(hz / 1000).toStringAsFixed(2)} kHz';
    }
    return '${hz.toStringAsFixed(1)} Hz';
  }

  String get ratioLabel =>
      '${approximateRatio ? '≈ ' : ''}$numerator/$denominator';
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
    final refHz = _refHz;
    DroneVoice updated;
    if (useRatio) {
      final ratio = voice.frequency / refHz;
      final (n, d) = _approximateRatio(ratio);
      final isApprox = (ratio - n / d).abs() > 1e-9;
      updated = voice.copyWith(
        useRatio: true,
        numerator: n,
        denominator: d,
        approximateRatio: isApprox,
      );
      _engine.setFrequency(id, updated.ratioHz(refHz));
    } else {
      updated = voice.copyWith(
        useRatio: false,
        frequency: voice.ratioHz(refHz),
      );
    }
    state = [
      for (final v in state) if (v.id == id) updated else v,
    ];
  }

  static (int, int) _approximateRatio(double value, {int maxDenom = 128}) {
    if (value <= 0) return (1, 1);
    int bestN = 1, bestD = 1;
    double bestErr = (value - 1).abs();
    int prevN = 0, prevD = 1, currN = 1, currD = 0;
    double x = value;
    for (int i = 0; i < 20; i++) {
      final a = x.floor();
      final n = a * currN + prevN;
      final d = a * currD + prevD;
      if (d > maxDenom) break;
      final err = (value - n / d).abs();
      if (err < bestErr) {
        bestN = n;
        bestD = d;
        bestErr = err;
        if (err < 1e-9) break;
      }
      prevN = currN;
      prevD = currD;
      currN = n;
      currD = d;
      final remainder = x - a;
      if (remainder < 1e-9) break;
      x = 1.0 / remainder;
    }
    return (bestN.clamp(1, 9999), bestD.clamp(1, 9999));
  }

  void setNumerator(int id, int n) {
    if (n < 1) return;
    final voice = state.firstWhere((v) => v.id == id);
    final updated = voice.copyWith(numerator: n, approximateRatio: false);
    _engine.setFrequency(id, updated.ratioHz(_refHz));
    state = [
      for (final v in state) if (v.id == id) updated else v,
    ];
  }

  void setDenominator(int id, int d) {
    if (d < 1) return;
    final voice = state.firstWhere((v) => v.id == id);
    final updated = voice.copyWith(denominator: d, approximateRatio: false);
    _engine.setFrequency(id, updated.ratioHz(_refHz));
    state = [
      for (final v in state) if (v.id == id) updated else v,
    ];
  }

  void setWaveform(int id, WaveformType type) {
    _engine.setWaveform(id, type);
    // The freshly-swapped pool DSP starts at its param defaults; push the voice's
    // current first-tone ratio so a tanpura reflects any earlier user change.
    if (type == WaveformType.tanpura) {
      final v = state.firstWhere((v) => v.id == id);
      _engine.setTanpuraTone1(id, v.tone1Ratio);
    }
    state = [
      for (final v in state)
        if (v.id == id) v.copyWith(waveformType: type) else v,
    ];
  }

  void setTone1Num(int id, int n) {
    if (n < 1) return;
    final voice = state.firstWhere((v) => v.id == id);
    final updated = voice.copyWith(tone1Num: n);
    _engine.setTanpuraTone1(id, updated.tone1Ratio);
    state = [
      for (final v in state) if (v.id == id) updated else v,
    ];
  }

  void setTone1Den(int id, int d) {
    if (d < 1) return;
    final voice = state.firstWhere((v) => v.id == id);
    final updated = voice.copyWith(tone1Den: d);
    _engine.setTanpuraTone1(id, updated.tone1Ratio);
    state = [
      for (final v in state) if (v.id == id) updated else v,
    ];
  }

  void pauseAll() {
    for (final v in state) {
      if (v.gateOn) _engine.setGate(v.id, false);
    }
    state = [for (final v in state) v.copyWith(gateOn: false)];
  }

  void resumeAll() {
    for (final v in state) {
      if (!v.gateOn) _engine.setGate(v.id, true);
    }
    state = [for (final v in state) v.copyWith(gateOn: true)];
  }

  bool get allPaused => state.isNotEmpty && state.every((v) => !v.gateOn);

  void removeAll() {
    for (final v in state) {
      _engine.removeVoice(v.id);
    }
    state = [];
  }
}

final droneProvider =
    NotifierProvider<DroneNotifier, List<DroneVoice>>(DroneNotifier.new);
