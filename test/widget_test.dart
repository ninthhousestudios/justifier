import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:shared_preferences/shared_preferences.dart';

import 'package:justifier/audio/audio_engine.dart';
import 'package:justifier/core/persistence.dart';
import 'package:justifier/providers/engine_provider.dart';
import 'package:justifier/providers/zoom_provider.dart';
import 'package:justifier/theme/app_theme.dart';
import 'package:justifier/audio/waveform_type.dart';
import 'package:justifier/providers/workspace_provider.dart';
import 'package:justifier/widgets/app_shell.dart';

/// Fake AudioEngine that doesn't load the native library.
class FakeAudioEngine implements AudioEngine {
  bool _running = false;

  @override
  bool init({int sampleRate = 48000, int bufferSize = 256}) {
    _running = true;
    return true;
  }

  @override
  void shutdown() => _running = false;

  @override
  bool get isRunning => _running;

  @override
  int get xrunCount => 0;

  @override
  int get activeVoiceCount => 0;

  @override
  int addVoice(waveformType, double frequency, double amplitude) => 0;

  @override
  void removeVoice(int voiceId) {}

  @override
  void setFrequency(int voiceId, double hz) {}

  @override
  void setAmplitude(int voiceId, double amplitude) {}

  @override
  void setPan(int voiceId, double pan) {}

  @override
  void setDetune(int voiceId, double cents) {}

  @override
  void setWaveform(int voiceId, waveformType) {}

  @override
  void setModRatio(int voiceId, double ratio) {}

  @override
  void setModIndex(int voiceId, double index) {}

  @override
  void setGate(int voiceId, bool on) {}

  @override
  void setGateTimes(int voiceId,
      {double attack = 0.05, double decay = 0.3, double sustain = 0.8, double release = 2.0}) {}

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
  dynamic noSuchMethod(Invocation invocation) => super.noSuchMethod(invocation);
}

Future<Widget> testApp() async {
  SharedPreferences.setMockInitialValues({});
  final prefs = await SharedPreferences.getInstance();
  final fakeEngine = FakeAudioEngine()..init();
  return ProviderScope(
    overrides: [
      sharedPrefsProvider.overrideWithValue(prefs),
      engineProvider.overrideWithValue(fakeEngine),
    ],
    child: MaterialApp(
      theme: AppTheme.dark(),
      home: const AppShell(),
    ),
  );
}

void main() {
  testWidgets('app shell renders with dark theme', (tester) async {
    await tester.pumpWidget(await testApp());

    final materialApp = tester.widget<MaterialApp>(find.byType(MaterialApp));
    expect(materialApp.theme!.brightness, Brightness.dark);
    expect(materialApp.theme!.useMaterial3, isTrue);

    expect(find.byType(AppShell), findsOneWidget);
  });

  testWidgets('empty workspace shows add wave button', (tester) async {
    await tester.pumpWidget(await testApp());

    expect(find.text('No waves yet'), findsOneWidget);
    expect(find.text('Add Wave'), findsOneWidget);
  });

  testWidgets('console panel is visible', (tester) async {
    await tester.pumpWidget(await testApp());

    expect(find.text('CONSOLE'), findsOneWidget);
  });

  testWidgets('adding a wave shows wave column with add voice button',
      (tester) async {
    await tester.pumpWidget(await testApp());

    // Add a wave via the button
    await tester.tap(find.text('Add Wave'));
    await tester.pumpAndSettle();

    // Wave column should appear with its name and add voice icon
    expect(find.text('Wave 1'), findsOneWidget);
    expect(find.text('no voices'), findsOneWidget);
  });

  testWidgets('adding a voice shows voice card with ratio display',
      (tester) async {
    SharedPreferences.setMockInitialValues({});
    final prefs = await SharedPreferences.getInstance();
    final fakeEngine = FakeAudioEngine()..init();
    final container = ProviderContainer(
      overrides: [
        sharedPrefsProvider.overrideWithValue(prefs),
        engineProvider.overrideWithValue(fakeEngine),
      ],
    );

    // Pre-populate state: add a wave, then a voice
    final waveId = container.read(workspaceProvider.notifier).addWave();
    container
        .read(workspaceProvider.notifier)
        .addVoice(waveId, WaveformType.sine);

    await tester.pumpWidget(
      UncontrolledProviderScope(
        container: container,
        child: MaterialApp(
          theme: AppTheme.dark(),
          home: const AppShell(),
        ),
      ),
    );
    await tester.pumpAndSettle();

    // Should show waveform selector with "sin" selected
    expect(find.text('sin'), findsOneWidget);
    // Should show frequency
    expect(find.textContaining('Hz'), findsWidgets);
  });

  testWidgets('connection status badge renders', (tester) async {
    await tester.pumpWidget(await testApp());

    // Badge should show running status with voice count
    expect(find.text('v:0'), findsOneWidget);
  });

  testWidgets('zoom provider defaults to 1.0', (tester) async {
    SharedPreferences.setMockInitialValues({});
    final prefs = await SharedPreferences.getInstance();
    final container = ProviderContainer(
      overrides: [sharedPrefsProvider.overrideWithValue(prefs)],
    );
    addTearDown(container.dispose);

    expect(container.read(zoomProvider), 1.0);
  });
}
