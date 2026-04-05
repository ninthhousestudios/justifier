import 'dart:ffi';
import 'dart:io';

import 'package:flutter_test/flutter_test.dart';
import 'package:justifier/audio/waveform_type.dart';
import 'package:justifier/audio/justifier_audio_bindings.dart';

/// Path to the shared library built by native/CMakeLists.txt.
/// In CI or dev, build with: cd native && cmake -B build-shared && cmake --build build-shared
String? _findNativeLib() {
  final candidates = [
    'native/build-shared/libjustifier_audio.so',
    'native/build/libjustifier_audio.so',
    'build/linux/x64/debug/bundle/lib/libjustifier_audio.so',
  ];
  for (final path in candidates) {
    if (File(path).existsSync()) return path;
  }
  return null;
}

void main() {
  group('WaveformType', () {
    test('enum indices match C values', () {
      expect(WaveformType.sine.index, 0);
      expect(WaveformType.triangle.index, 1);
      expect(WaveformType.saw.index, 2);
      expect(WaveformType.square.index, 3);
      expect(WaveformType.pulse.index, 4);
      expect(WaveformType.whiteNoise.index, 5);
      expect(WaveformType.pinkNoise.index, 6);
      expect(WaveformType.brownNoise.index, 7);
      expect(WaveformType.lfnoise0.index, 8);
      expect(WaveformType.lfnoise1.index, 9);
      expect(WaveformType.lfnoise2.index, 10);
      expect(WaveformType.fm.index, 11);
    });

    test('has 12 values', () {
      expect(WaveformType.values.length, 12);
    });
  });

  group('AudioEngine native bindings', () {
    late JustifierAudioBindings bindings;

    setUpAll(() {
      final libPath = _findNativeLib();
      if (libPath == null) {
        fail(
          'Native library not found. Build it first:\n'
          '  cd native && cmake -B build-shared && cmake --build build-shared',
        );
      }
      bindings = JustifierAudioBindings(DynamicLibrary.open(libPath));
    });

    test('can load native library', () {
      // If setUpAll passed, the library loaded successfully.
      expect(bindings, isNotNull);
    });

    test('init, add voice, panic, shutdown lifecycle', () {
      // Single test to avoid multiple init/shutdown cycles in one process.
      final result = bindings.justifier_init(48000, 256);
      expect(result, 0, reason: 'init should return 0 on success');
      expect(bindings.justifier_is_running(), isNot(0));
      expect(bindings.justifier_get_active_voice_count(), 0);

      // Add a voice — returns synchronously, but SPSC queue means the
      // audio thread hasn't processed it yet. The ID should be valid.
      final voiceId = bindings.justifier_voice_add(
        NativeWaveformType.WAVEFORM_SINE,
        440.0,
        0.3,
      );
      expect(voiceId, greaterThanOrEqualTo(0));

      // Panic and unpanic should not crash.
      bindings.justifier_panic();
      bindings.justifier_unpanic();

      // Clean up.
      bindings.justifier_voice_remove(voiceId);
      bindings.justifier_shutdown();
    });
  });
}
