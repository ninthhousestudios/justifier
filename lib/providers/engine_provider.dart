import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../audio/audio_engine.dart';

/// Singleton AudioEngine — initialized on first access, shut down on dispose.
final engineProvider = Provider<AudioEngine>((ref) {
  final engine = AudioEngine();
  engine.init();
  ref.onDispose(() => engine.shutdown());
  return engine;
});

