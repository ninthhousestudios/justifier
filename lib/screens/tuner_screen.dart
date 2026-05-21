import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../tuner/pitch_state.dart';
import '../tuner/ratio_match_provider.dart';

class TunerScreen extends ConsumerWidget {
  const TunerScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final pitch = ref.watch(pitchProvider);
    final notifier = ref.read(pitchProvider.notifier);
    final match = ref.watch(ratioMatchProvider);
    final theme = Theme.of(context);

    return SafeArea(
      child: Column(
        children: [
          const Spacer(),
          if (pitch.permissionDenied) ...[
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
          ] else if (pitch.isRunning && match != null) ...[
            Text(
              match.ratio.label,
              style: theme.textTheme.displayLarge?.copyWith(
                color: theme.colorScheme.primary,
                fontWeight: FontWeight.bold,
              ),
            ),
            if (match.ratio.name != null)
              Padding(
                padding: const EdgeInsets.only(top: 4),
                child: Text(
                  match.ratio.name!,
                  style: theme.textTheme.titleMedium?.copyWith(
                    color: theme.colorScheme.onSurfaceVariant,
                  ),
                ),
              ),
            const SizedBox(height: 16),
            _CentsDisplay(cents: match.deviationCents),
            const SizedBox(height: 16),
            Text(
              '${pitch.hz.toStringAsFixed(1)} Hz',
              style: theme.textTheme.bodyMedium?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
            ),
            const SizedBox(height: 8),
            _ConfidenceBar(confidence: pitch.confidence),
          ] else if (pitch.isRunning) ...[
            Icon(Icons.mic, size: 64, color: theme.colorScheme.onSurfaceVariant),
            const SizedBox(height: 16),
            Text(
              'Listening...',
              style: theme.textTheme.headlineMedium?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
            ),
          ] else ...[
            Icon(
              Icons.mic_off,
              size: 64,
              color: theme.colorScheme.onSurfaceVariant,
            ),
            const SizedBox(height: 16),
            Text(
              'Tap to start',
              style: theme.textTheme.headlineMedium?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
            ),
          ],
          const Spacer(),
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

class _CentsDisplay extends StatelessWidget {
  const _CentsDisplay({required this.cents});

  final double cents;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final absCents = cents.abs();
    final sign = cents >= 0 ? '+' : '−';
    final color = absCents < 5
        ? Colors.green
        : absCents < 15
            ? Colors.amber
            : theme.colorScheme.error;

    return Text(
      '$sign${absCents.toStringAsFixed(1)}¢',
      style: theme.textTheme.headlineMedium?.copyWith(
        color: color,
        fontWeight: FontWeight.bold,
      ),
    );
  }
}

class _ConfidenceBar extends StatelessWidget {
  const _ConfidenceBar({required this.confidence});

  final double confidence;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return SizedBox(
      width: 200,
      height: 8,
      child: ClipRRect(
        borderRadius: BorderRadius.circular(4),
        child: LinearProgressIndicator(
          value: confidence.clamp(0.0, 1.0),
          backgroundColor: theme.colorScheme.surfaceContainerHighest,
          color: confidence > 0.9
              ? Colors.green
              : confidence > 0.8
                  ? Colors.amber
                  : theme.colorScheme.primary,
        ),
      ),
    );
  }
}
