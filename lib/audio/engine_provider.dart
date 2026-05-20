import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'audio_engine.dart';

final engineProvider = Provider<AudioEngine>((ref) {
  final engine = AudioEngine();
  engine.init();
  ref.onDispose(() => engine.shutdown());
  return engine;
});
