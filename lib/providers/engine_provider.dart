import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../audio/audio_engine.dart';

/// Singleton AudioEngine — initialized on first access, shut down on dispose.
final engineProvider = Provider<AudioEngine>((ref) {
  final engine = AudioEngine();
  engine.init();
  ref.onDispose(() => engine.shutdown());
  return engine;
});

/// Whether the audio engine is currently running.
final engineRunningProvider = Provider<bool>((ref) {
  return ref.read(engineProvider).isRunning;
});

/// Number of active voices in the engine.
final activeVoiceCountProvider = Provider<int>((ref) {
  return ref.read(engineProvider).activeVoiceCount;
});
