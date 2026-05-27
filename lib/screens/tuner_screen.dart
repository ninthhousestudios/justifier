import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../tuner/pitch_state.dart';
import '../tuner/ratio.dart';
import '../tuner/ratio_match_provider.dart';
import '../tuner/tuner_indicator.dart';

class TunerScreen extends ConsumerWidget {
  const TunerScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final pitch = ref.watch(pitchProvider);
    final notifier = ref.read(pitchProvider.notifier);
    final match = ref.watch(ratioMatchProvider);
    final hasMatch = pitch.isRunning && match != null;

    return SafeArea(
      child: Column(
        children: [
          const _SettingsDrawer(),
          if (pitch.permissionDenied)
            const Expanded(child: _PermissionDenied()),
          if (!pitch.permissionDenied) ...[
            const Spacer(flex: 2),
            _RatioDisplay(
              match: hasMatch ? match : null,
              isListening: pitch.isRunning && !hasMatch,
            ),
            const SizedBox(height: 24),
            SizedBox(
              height: 200,
              child: Padding(
                padding: const EdgeInsets.symmetric(horizontal: 24),
                child: TunerIndicator(
                  deviationCents: match?.deviationCents,
                  isActive: hasMatch,
                ),
              ),
            ),
            const Spacer(flex: 3),
          ],
          Padding(
            padding: const EdgeInsets.only(bottom: 32),
            child: FilledButton.icon(
              onPressed: () {
                if (pitch.isRunning) {
                  notifier.stop();
                } else {
                  notifier.start();
                }
              },
              icon: Icon(pitch.isRunning ? Icons.stop : Icons.mic),
              label: Text(pitch.isRunning ? 'Stop' : 'Start'),
            ),
          ),
        ],
      ),
    );
  }
}

class _RatioDisplay extends StatelessWidget {
  const _RatioDisplay({
    this.match,
    this.isListening = false,
  });

  final RatioMatch? match;
  final bool isListening;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);

    if (match != null) {
      return Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(
            match!.ratio.label,
            style: theme.textTheme.displayLarge?.copyWith(
              color: theme.colorScheme.primary,
              fontWeight: FontWeight.bold,
            ),
          ),
          if (match!.ratio.name != null)
            Padding(
              padding: const EdgeInsets.only(top: 4),
              child: Text(
                match!.ratio.name!,
                style: theme.textTheme.titleMedium?.copyWith(
                  color: theme.colorScheme.onSurfaceVariant,
                ),
              ),
            ),
        ],
      );
    }

    return Text(
      isListening ? 'Listening...' : 'Tap to start',
      style: theme.textTheme.headlineMedium?.copyWith(
        color: theme.colorScheme.onSurfaceVariant,
      ),
    );
  }
}

class _PermissionDenied extends StatelessWidget {
  const _PermissionDenied();

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return Center(
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Icon(Icons.mic_off, size: 64, color: theme.colorScheme.error),
          const SizedBox(height: 16),
          Text(
            'Microphone access denied',
            style: theme.textTheme.headlineMedium?.copyWith(
              color: theme.colorScheme.error,
            ),
          ),
          const SizedBox(height: 8),
          Text(
            'Enable in system settings to use the tuner',
            style: theme.textTheme.bodyMedium?.copyWith(
              color: theme.colorScheme.onSurfaceVariant,
            ),
          ),
        ],
      ),
    );
  }
}

class _SettingsDrawer extends StatefulWidget {
  const _SettingsDrawer();

  @override
  State<_SettingsDrawer> createState() => _SettingsDrawerState();
}

class _SettingsDrawerState extends State<_SettingsDrawer> {
  bool _open = false;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        InkWell(
          onTap: () => setState(() => _open = !_open),
          child: Padding(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.center,
              mainAxisSize: MainAxisSize.min,
              children: [
                Text(
                  'Settings',
                  style: theme.textTheme.labelMedium?.copyWith(
                    color: theme.colorScheme.onSurfaceVariant,
                  ),
                ),
                const SizedBox(width: 4),
                Icon(
                  _open ? Icons.expand_less : Icons.expand_more,
                  size: 18,
                  color: theme.colorScheme.onSurfaceVariant,
                ),
              ],
            ),
          ),
        ),
        AnimatedCrossFade(
          duration: const Duration(milliseconds: 200),
          crossFadeState:
              _open ? CrossFadeState.showSecond : CrossFadeState.showFirst,
          firstChild: const SizedBox(height: 0, width: double.infinity),
          secondChild: Container(
            width: double.infinity,
            padding: const EdgeInsets.fromLTRB(16, 0, 16, 12),
            child: Text(
              'Tuner settings will go here',
              style: theme.textTheme.bodySmall?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
              textAlign: TextAlign.center,
            ),
          ),
        ),
        Divider(height: 1, color: theme.colorScheme.surfaceContainerHighest),
      ],
    );
  }
}
