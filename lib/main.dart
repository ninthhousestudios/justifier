import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:window_manager/window_manager.dart';

import 'core/persistence.dart';
import 'providers/zoom-provider.dart';
import 'theme/app-theme.dart';
import 'widgets/app-shell.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  final prefs = await SharedPreferences.getInstance();

  await windowManager.ensureInitialized();
  const windowOptions = WindowOptions(
    size: Size(1280, 720),
    center: true,
    backgroundColor: Colors.black,
    titleBarStyle: TitleBarStyle.hidden,
    skipTaskbar: false,
  );
  await windowManager.waitUntilReadyToShow(windowOptions, () async {
    await windowManager.show();
    await windowManager.focus();
  });

  runApp(
    ProviderScope(
      overrides: [
        sharedPrefsProvider.overrideWithValue(prefs),
      ],
      child: const JustifierApp(),
    ),
  );
}

class JustifierApp extends ConsumerWidget {
  const JustifierApp({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final scale = ref.watch(zoomProvider);

    return MaterialApp(
      title: 'Justifier',
      debugShowCheckedModeBanner: false,
      theme: AppTheme.dark(),
      builder: (context, child) {
        final mq = MediaQuery.of(context);
        return MediaQuery(
          data: mq.copyWith(
            textScaler: TextScaler.linear(scale),
          ),
          child: child!,
        );
      },
      home: CallbackShortcuts(
        bindings: {
          const SingleActivator(LogicalKeyboardKey.equal, control: true):
              () => zoomIn(ref),
          const SingleActivator(LogicalKeyboardKey.minus, control: true):
              () => zoomOut(ref),
          const SingleActivator(LogicalKeyboardKey.digit0, control: true):
              () => zoomReset(ref),
        },
        child: const Focus(
          autofocus: true,
          child: AppShell(),
        ),
      ),
    );
  }
}
