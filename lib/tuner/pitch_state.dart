import 'dart:async';

import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:permission_handler/permission_handler.dart';

import '../audio/audio_engine.dart';
import '../audio/engine_provider.dart';

class PitchState {
  const PitchState({
    this.hz = 0,
    this.confidence = 0,
    this.isRunning = false,
    this.permissionDenied = false,
  });

  final double hz;
  final double confidence;
  final bool isRunning;
  final bool permissionDenied;

  bool get isValid => hz > 0 && confidence > 0.7;
}

class PitchNotifier extends Notifier<PitchState> {
  Timer? _timer;

  AudioEngine get _engine => ref.read(engineProvider);

  @override
  PitchState build() => const PitchState();

  Future<void> start() async {
    if (state.isRunning) return;

    final status = await Permission.microphone.request();
    if (!status.isGranted) {
      state = const PitchState(permissionDenied: true);
      return;
    }

    if (!_engine.pitchStart()) return;

    state = const PitchState(isRunning: true);
    _timer = Timer.periodic(const Duration(milliseconds: 33), (_) {
      final result = _engine.pitchGet();
      state = PitchState(
        hz: result.hz,
        confidence: result.confidence,
        isRunning: true,
      );
    });
  }

  void stop() {
    _timer?.cancel();
    _timer = null;
    _engine.pitchStop();
    state = const PitchState();
  }
}

final pitchProvider =
    NotifierProvider<PitchNotifier, PitchState>(PitchNotifier.new);
