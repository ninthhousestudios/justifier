import 'dart:ffi';
import 'dart:io';

import 'justifier_audio_bindings.dart';
import 'waveform_type.dart';

/// Dart wrapper around the native justifier audio engine.
///
/// Loads the platform-specific shared library and exposes typed methods
/// using [WaveformType] instead of raw C enum integers.
class AudioEngine {
  AudioEngine() : _bindings = JustifierAudioBindings(_loadLibrary());

  final JustifierAudioBindings _bindings;

  // ---------------------------------------------------------------------------
  // Engine lifecycle
  // ---------------------------------------------------------------------------

  /// Initialize the audio engine. Returns true on success.
  bool init({int sampleRate = 48000, int bufferSize = 256}) {
    return _bindings.justifier_init(sampleRate, bufferSize) == 0;
  }

  /// Shut down the audio engine and release all resources.
  void shutdown() => _bindings.justifier_shutdown();

  // ---------------------------------------------------------------------------
  // Voice management
  // ---------------------------------------------------------------------------

  /// Add a voice and return its ID, or -1 on failure.
  int addVoice(WaveformType type, double frequency, double amplitude) {
    return _bindings.justifier_voice_add(
      NativeWaveformType.fromValue(type.index),
      frequency,
      amplitude,
    );
  }

  void removeVoice(int voiceId) => _bindings.justifier_voice_remove(voiceId);

  void setFrequency(int voiceId, double hz) =>
      _bindings.justifier_voice_set_frequency(voiceId, hz);

  void setAmplitude(int voiceId, double amplitude) =>
      _bindings.justifier_voice_set_amplitude(voiceId, amplitude);

  void setPan(int voiceId, double pan) =>
      _bindings.justifier_voice_set_pan(voiceId, pan);

  void setDetune(int voiceId, double cents) =>
      _bindings.justifier_voice_set_detune(voiceId, cents);

  void setWaveform(int voiceId, WaveformType type) =>
      _bindings.justifier_voice_set_waveform(
        voiceId,
        NativeWaveformType.fromValue(type.index),
      );

  // ---------------------------------------------------------------------------
  // FM-specific
  // ---------------------------------------------------------------------------

  void setModRatio(int voiceId, double ratio) =>
      _bindings.justifier_voice_set_mod_ratio(voiceId, ratio);

  void setModIndex(int voiceId, double index) =>
      _bindings.justifier_voice_set_mod_index(voiceId, index);

  // ---------------------------------------------------------------------------
  // Filter
  // ---------------------------------------------------------------------------

  void setFilterType(int voiceId, int type) =>
      _bindings.justifier_voice_set_filter_type(voiceId, type);

  void setFilterCutoff(int voiceId, double hz) =>
      _bindings.justifier_voice_set_filter_cutoff(voiceId, hz);

  void setFilterResonance(int voiceId, double resonance) =>
      _bindings.justifier_voice_set_filter_resonance(voiceId, resonance);

  // ---------------------------------------------------------------------------
  // Reverb send/return
  // ---------------------------------------------------------------------------

  void setReverbSend(int voiceId, double send) =>
      _bindings.justifier_voice_set_reverb_send(voiceId, send);

  void setReverbReturn(double level) =>
      _bindings.justifier_set_reverb_return(level);

  void setDelaySend(int voiceId, double send) =>
      _bindings.justifier_voice_set_delay_send(voiceId, send);

  void setDelayReturn(double level) =>
      _bindings.justifier_set_delay_return(level);

  void setChorusSend(int voiceId, double send) =>
      _bindings.justifier_voice_set_chorus_send(voiceId, send);

  void setChorusReturn(double level) =>
      _bindings.justifier_set_chorus_return(level);

  void setPhaserSend(int voiceId, double send) =>
      _bindings.justifier_voice_set_phaser_send(voiceId, send);

  void setPhaserReturn(double level) =>
      _bindings.justifier_set_phaser_return(level);

  void setFlangerSend(int voiceId, double send) =>
      _bindings.justifier_voice_set_flanger_send(voiceId, send);

  void setFlangerReturn(double level) =>
      _bindings.justifier_set_flanger_return(level);

  void setEqSend(int voiceId, double send) =>
      _bindings.justifier_voice_set_eq_send(voiceId, send);

  void setEqReturn(double level) =>
      _bindings.justifier_set_eq_return(level);

  void setSaturationSend(int voiceId, double send) =>
      _bindings.justifier_voice_set_saturation_send(voiceId, send);

  void setSaturationReturn(double level) =>
      _bindings.justifier_set_saturation_return(level);

  // ---------------------------------------------------------------------------
  // Gate (envelope control)
  // ---------------------------------------------------------------------------

  void setGate(int voiceId, bool on) =>
      _bindings.justifier_voice_set_gate(voiceId, on ? 1 : 0);

  void setGateTimes(
    int voiceId, {
    double attack = 0.05,
    double decay = 0.3,
    double sustain = 0.8,
    double release = 2.0,
  }) => _bindings.justifier_voice_set_gate_times(
        voiceId,
        attack,
        decay,
        sustain,
        release,
      );

  // ---------------------------------------------------------------------------
  // Global controls
  // ---------------------------------------------------------------------------

  void panic() => _bindings.justifier_panic();
  void unpanic() => _bindings.justifier_unpanic();

  void setMasterVolume(double volume) =>
      _bindings.justifier_set_master_volume(volume);

  // ---------------------------------------------------------------------------
  // Status (thread-safe atomic reads)
  // ---------------------------------------------------------------------------

  bool get isRunning => _bindings.justifier_is_running() != 0;
  int get xrunCount => _bindings.justifier_get_xrun_count();
  int get activeVoiceCount => _bindings.justifier_get_active_voice_count();

  // ---------------------------------------------------------------------------
  // Library loading
  // ---------------------------------------------------------------------------

  static DynamicLibrary _loadLibrary() {
    if (Platform.isMacOS) {
      // CMake builds libjustifier_audio.dylib; the podspec script_phase
      // copies it into the justifier_native framework inside the app bundle.
      final exeDir = File(Platform.resolvedExecutable).parent.path;
      final candidates = [
        // Inside the pod's framework
        '$exeDir/../Frameworks/justifier_native.framework/Versions/A/Frameworks/libjustifier_audio.dylib',
        // Direct in app Frameworks (if copy target changes)
        '$exeDir/../Frameworks/libjustifier_audio.dylib',
      ];
      for (final path in candidates) {
        if (File(path).existsSync()) return DynamicLibrary.open(path);
      }
      return DynamicLibrary.process();
    }

    final exeDir = File(Platform.resolvedExecutable).parent.path;

    // Platform-specific candidates in order of priority.
    final candidates = <String>[
      if (Platform.isLinux) '$exeDir/lib/libjustifier_audio.so',
      if (Platform.isWindows) '$exeDir/justifier_audio.dll',
    ];

    for (final path in candidates) {
      if (File(path).existsSync()) return DynamicLibrary.open(path);
    }

    // Fallback: bare library name (relies on OS linker search path).
    final bareName = Platform.isWindows
        ? 'justifier_audio.dll'
        : 'libjustifier_audio.so';
    try {
      return DynamicLibrary.open(bareName);
    } catch (_) {}

    throw StateError(
      'libjustifier_audio not found. '
      'Ensure the native library is built and bundled with the application.',
    );
  }
}
