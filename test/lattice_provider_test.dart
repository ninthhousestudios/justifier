import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'package:justifier/audio/audio_engine.dart';
import 'package:justifier/audio/waveform_type.dart';
import 'package:justifier/models/lattice_state.dart';
import 'package:justifier/providers/engine_provider.dart';
import 'package:justifier/providers/lattice_provider.dart';

/// Fake AudioEngine that tracks voice calls without loading the native library.
class FakeAudioEngine implements AudioEngine {
  int _nextVoiceId = 0;
  final List<int> addedVoices = [];
  final List<int> removedVoices = [];
  final Map<int, bool> gateStates = {};

  // Control whether addVoice succeeds (set nextVoiceId to -1 to simulate full pool)
  int nextVoiceId = 0;

  @override
  bool init({int sampleRate = 48000, int bufferSize = 256}) => true;

  @override
  void shutdown() {}

  @override
  bool get isRunning => true;

  @override
  int get activeVoiceCount => addedVoices.length - removedVoices.length;

  @override
  int addVoice(WaveformType type, double frequency, double amplitude) {
    final id = nextVoiceId;
    if (id >= 0) {
      addedVoices.add(id);
      _nextVoiceId = id + 1;
      nextVoiceId = _nextVoiceId;
    }
    return id;
  }

  @override
  void removeVoice(int voiceId) => removedVoices.add(voiceId);

  @override
  void setGate(int voiceId, bool on) => gateStates[voiceId] = on;

  @override
  void setFrequency(int voiceId, double hz) {}

  @override
  void setAmplitude(int voiceId, double amplitude) {}

  @override
  void setPan(int voiceId, double pan) {}

  @override
  void setDetune(int voiceId, double cents) {}

  @override
  void setWaveform(int voiceId, WaveformType type) {}

  @override
  void setModRatio(int voiceId, double ratio) {}

  @override
  void setModIndex(int voiceId, double index) {}

  @override
  void setFilterType(int voiceId, int type) {}

  @override
  void setFilterCutoff(int voiceId, double hz) {}

  @override
  void setFilterResonance(int voiceId, double resonance) {}

  @override
  void setGateTimes(int voiceId,
      {double attack = 0.05,
      double decay = 0.3,
      double sustain = 0.8,
      double release = 2.0}) {}

  @override
  void panic() {}

  @override
  void unpanic() {}

  @override
  void setMasterVolume(double volume) {}

  @override
  void setReverbSend(int voiceId, double send) {}

  @override
  void setReverbReturn(double level) {}

  @override
  void setDelaySend(int voiceId, double send) {}

  @override
  void setDelayReturn(double level) {}

  @override
  void setChorusSend(int voiceId, double send) {}

  @override
  void setChorusReturn(double level) {}

  @override
  void setPhaserSend(int voiceId, double send) {}

  @override
  void setPhaserReturn(double level) {}

  @override
  void setFlangerSend(int voiceId, double send) {}

  @override
  void setFlangerReturn(double level) {}

  @override
  void setEqSend(int voiceId, double send) {}

  @override
  void setEqReturn(double level) {}

  @override
  void setSaturationSend(int voiceId, double send) {}

  @override
  void setSaturationReturn(double level) {}

  @override
  dynamic noSuchMethod(Invocation invocation) => super.noSuchMethod(invocation);
}

ProviderContainer makeContainer(FakeAudioEngine engine) {
  final container = ProviderContainer(
    overrides: [engineProvider.overrideWithValue(engine)],
  );
  addTearDown(container.dispose);
  return container;
}

void main() {
  group('LatticeNotifier', () {
    test('testInitialState: default values match spec', () {
      final engine = FakeAudioEngine();
      final container = makeContainer(engine);

      final state = container.read(latticeProvider);

      expect(state.viewMode, isA<NestedMode>());
      expect(state.spawnOctave, 1);
      expect(state.expandedNodes, isEmpty);
      expect(state.previewVoiceId, isNull);
      expect(state.previewRatio, isNull);
      expect(state.hoveredRatio, isNull);
      expect(state.dividerFraction, 0.6);
      expect(state.showLabels, isFalse);
    });

    test('testSetViewMode_clearsExpanded: switching mode clears expandedNodes',
        () {
      final engine = FakeAudioEngine();
      final container = makeContainer(engine);
      final notifier = container.read(latticeProvider.notifier);

      // Add some expanded nodes
      notifier.toggleExpanded(3, 2);
      notifier.toggleExpanded(5, 4);
      expect(container.read(latticeProvider).expandedNodes, hasLength(2));

      // Switch view mode
      notifier.setViewMode(const ProjectionMode(primeX: 3, primeY: 7));

      final state = container.read(latticeProvider);
      expect(state.viewMode, isA<ProjectionMode>());
      expect(state.expandedNodes, isEmpty);
    });

    test('testSetSpawnOctave_clamped: values outside [0, 9] are clamped', () {
      final engine = FakeAudioEngine();
      final container = makeContainer(engine);
      final notifier = container.read(latticeProvider.notifier);

      notifier.setSpawnOctave(-5);
      expect(container.read(latticeProvider).spawnOctave, 0);

      notifier.setSpawnOctave(99);
      expect(container.read(latticeProvider).spawnOctave, 9);

      notifier.setSpawnOctave(4);
      expect(container.read(latticeProvider).spawnOctave, 4);
    });

    test('testToggleExpanded: add and remove from expandedNodes', () {
      final engine = FakeAudioEngine();
      final container = makeContainer(engine);
      final notifier = container.read(latticeProvider.notifier);

      // Toggle on
      notifier.toggleExpanded(3, 2);
      expect(container.read(latticeProvider).expandedNodes, contains((3, 2)));

      // Toggle again removes it
      notifier.toggleExpanded(3, 2);
      expect(container.read(latticeProvider).expandedNodes, isNot(contains((3, 2))));

      // Multiple nodes coexist
      notifier.toggleExpanded(3, 2);
      notifier.toggleExpanded(5, 4);
      expect(container.read(latticeProvider).expandedNodes, hasLength(2));
    });

    test('testPreviewLifecycle: startPreview adds voice, stopPreview removes it',
        () {
      final engine = FakeAudioEngine();
      final container = makeContainer(engine);
      final notifier = container.read(latticeProvider.notifier);

      notifier.startPreview(3, 2, 440.0);

      var state = container.read(latticeProvider);
      expect(engine.addedVoices, hasLength(1));
      expect(state.previewVoiceId, isNotNull);
      expect(state.previewVoiceId, engine.addedVoices.first);
      expect(state.previewRatio, (3, 2));
      expect(engine.gateStates[state.previewVoiceId], isTrue);

      notifier.stopPreview();

      state = container.read(latticeProvider);
      expect(engine.removedVoices, contains(engine.addedVoices.first));
      expect(state.previewVoiceId, isNull);
      expect(state.previewRatio, isNull);
    });

    test('testPreviewReplace: starting new preview kills old one first', () {
      final engine = FakeAudioEngine();
      final container = makeContainer(engine);
      final notifier = container.read(latticeProvider.notifier);

      notifier.startPreview(3, 2, 440.0);
      final firstId = container.read(latticeProvider).previewVoiceId!;

      // Start a second preview without stopping the first
      notifier.startPreview(5, 4, 440.0);

      expect(engine.removedVoices, contains(firstId));
      final state = container.read(latticeProvider);
      expect(state.previewVoiceId, isNot(firstId));
      expect(state.previewRatio, (5, 4));
    });

    test('testSetDividerFraction_clamped: values outside [0, 1] are clamped',
        () {
      final engine = FakeAudioEngine();
      final container = makeContainer(engine);
      final notifier = container.read(latticeProvider.notifier);

      notifier.setDividerFraction(-0.5);
      expect(container.read(latticeProvider).dividerFraction, 0.0);

      notifier.setDividerFraction(2.0);
      expect(container.read(latticeProvider).dividerFraction, 1.0);

      notifier.setDividerFraction(0.75);
      expect(container.read(latticeProvider).dividerFraction, 0.75);
    });
  });
}
