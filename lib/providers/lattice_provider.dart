import 'dart:math';

import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../audio/audio_engine.dart';
import '../audio/waveform_type.dart';
import '../models/lattice_state.dart';
import 'engine_provider.dart';

final latticeProvider =
    NotifierProvider<LatticeNotifier, LatticeState>(LatticeNotifier.new);

class LatticeNotifier extends Notifier<LatticeState> {
  late AudioEngine _engine;

  @override
  LatticeState build() {
    _engine = ref.read(engineProvider);
    return const LatticeState();
  }

  void setViewMode(LatticeViewMode mode) {
    // Pre-mortem fix N1: clear expandedNodes when switching modes
    state = state.copyWith(viewMode: mode, expandedNodes: const {});
  }

  void setSpawnOctave(int octave) {
    state = state.copyWith(spawnOctave: octave.clamp(0, 9));
  }

  void toggleExpanded(int n, int d) {
    final key = (n, d);
    final current = Set<(int, int)>.from(state.expandedNodes);
    if (current.contains(key)) {
      current.remove(key);
    } else {
      current.add(key);
    }
    state = state.copyWith(expandedNodes: current);
  }

  void setHovered(int? n, int? d) {
    if (n != null && d != null) {
      state = state.copyWith(hoveredRatio: (n, d));
    } else {
      state = state.copyWith(hoveredRatio: null);
    }
  }

  void setDividerFraction(double f) {
    state = state.copyWith(dividerFraction: f.clamp(0.0, 1.0));
  }

  void toggleLabels() {
    state = state.copyWith(showLabels: !state.showLabels);
  }

  void startPreview(int n, int d, double referenceHz) {
    // Kill existing preview if any
    stopPreview();
    final hz = referenceHz * (n / d) * pow(2, state.spawnOctave);
    final engineId = _engine.addVoice(WaveformType.sine, hz, 0.5);
    if (engineId < 0) return; // voice pool full
    _engine.setGate(engineId, true);
    state = state.copyWith(previewVoiceId: engineId, previewRatio: (n, d));
  }

  void stopPreview() {
    if (state.previewVoiceId != null) {
      _engine.removeVoice(state.previewVoiceId!);
      state = state.copyWith(previewVoiceId: null, previewRatio: null);
    }
  }
}
