import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../audio/audio_engine.dart';
import '../audio/engine_provider.dart';
import '../audio/waveform_type.dart';

class DroneVoice {
  const DroneVoice({
    required this.id,
    this.waveform = WaveformType.sine,
    this.frequency = 220.0,
    this.amplitude = 0.5,
    this.gateOn = true,
  });

  final int id;
  final WaveformType waveform;
  final double frequency;
  final double amplitude;
  final bool gateOn;

  DroneVoice copyWith({
    WaveformType? waveform,
    double? frequency,
    double? amplitude,
    bool? gateOn,
  }) {
    return DroneVoice(
      id: id,
      waveform: waveform ?? this.waveform,
      frequency: frequency ?? this.frequency,
      amplitude: amplitude ?? this.amplitude,
      gateOn: gateOn ?? this.gateOn,
    );
  }

  String get frequencyLabel {
    if (frequency >= 1000) {
      return '${(frequency / 1000).toStringAsFixed(2)} kHz';
    }
    return '${frequency.toStringAsFixed(1)} Hz';
  }
}

class DroneNotifier extends Notifier<List<DroneVoice>> {
  AudioEngine get _engine => ref.read(engineProvider);

  @override
  List<DroneVoice> build() => [];

  void addVoice({
    WaveformType waveform = WaveformType.sine,
    double frequency = 220.0,
    double amplitude = 0.5,
  }) {
    final id = _engine.addVoice(waveform, frequency, amplitude);
    if (id < 0) return;
    _engine.setGate(id, true);
    state = [...state, DroneVoice(id: id, waveform: waveform, frequency: frequency, amplitude: amplitude)];
  }

  void removeVoice(int id) {
    _engine.setGate(id, false);
    _engine.removeVoice(id);
    state = state.where((v) => v.id != id).toList();
  }

  void setWaveform(int id, WaveformType waveform) {
    _engine.setWaveform(id, waveform);
    state = [for (final v in state) if (v.id == id) v.copyWith(waveform: waveform) else v];
  }

  void setFrequency(int id, double hz) {
    _engine.setFrequency(id, hz);
    state = [for (final v in state) if (v.id == id) v.copyWith(frequency: hz) else v];
  }

  void setAmplitude(int id, double amplitude) {
    _engine.setAmplitude(id, amplitude);
    state = [for (final v in state) if (v.id == id) v.copyWith(amplitude: amplitude) else v];
  }

  void toggleGate(int id) {
    final voice = state.firstWhere((v) => v.id == id);
    final newGate = !voice.gateOn;
    _engine.setGate(id, newGate);
    state = [for (final v in state) if (v.id == id) v.copyWith(gateOn: newGate) else v];
  }

  void removeAll() {
    for (final v in state) {
      _engine.setGate(v.id, false);
      _engine.removeVoice(v.id);
    }
    state = [];
  }
}

final droneProvider = NotifierProvider<DroneNotifier, List<DroneVoice>>(DroneNotifier.new);
