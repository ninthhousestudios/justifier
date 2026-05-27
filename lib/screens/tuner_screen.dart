import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../tuner/pitch_state.dart';
import '../tuner/ratio.dart';
import '../tuner/ratio_match_provider.dart';
import '../tuner/tuner_indicator.dart';
import '../tuner/tuner_settings_drawer.dart';
import '../tuner/tuner_settings_state.dart';

class TunerScreen extends ConsumerWidget {
  const TunerScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final pitch = ref.watch(pitchProvider);
    final notifier = ref.read(pitchProvider.notifier);
    final match = ref.watch(ratioMatchProvider);
    final settings = ref.watch(tunerSettingsProvider);
    final hasMatch = pitch.isRunning && match != null;

    return SafeArea(
      child: Column(
        children: [
          const Flexible(child: TunerSettingsDrawer()),
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
            if (hasMatch)
              _OptionalReadouts(
                match: match,
                pitch: pitch,
                showCents: settings.showCents,
                showHz: settings.showHz,
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

class _OptionalReadouts extends StatelessWidget {
  const _OptionalReadouts({
    required this.match,
    required this.pitch,
    required this.showCents,
    required this.showHz,
  });

  final RatioMatch match;
  final PitchState pitch;
  final bool showCents;
  final bool showHz;

  @override
  Widget build(BuildContext context) {
    if (!showCents && !showHz) return const SizedBox.shrink();

    final theme = Theme.of(context);
    final style = theme.textTheme.bodySmall?.copyWith(
      color: theme.colorScheme.onSurfaceVariant,
    );

    return Padding(
      padding: const EdgeInsets.only(top: 12),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.center,
        children: [
          if (showCents) ...[
            Text(_formatCents(match.deviationCents), style: style),
            if (showHz) const SizedBox(width: 16),
          ],
          if (showHz) Text('${pitch.hz.toStringAsFixed(1)} Hz', style: style),
        ],
      ),
    );
  }

  static String _formatCents(double cents) {
    final sign = cents >= 0 ? '+' : '−';
    return '$sign${cents.abs().toStringAsFixed(1)}¢';
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
