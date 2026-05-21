import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../tuner/pitch_state.dart';

class TunerScreen extends ConsumerWidget {
  const TunerScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final pitch = ref.watch(pitchProvider);
    final notifier = ref.read(pitchProvider.notifier);
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
          ] else if (pitch.isRunning && pitch.isValid) ...[
            Text(
              '${pitch.hz.toStringAsFixed(1)} Hz',
              style: theme.textTheme.displayMedium?.copyWith(
                color: theme.colorScheme.primary,
                fontWeight: FontWeight.bold,
              ),
            ),
            const SizedBox(height: 8),
            _ConfidenceBar(confidence: pitch.confidence),
            const SizedBox(height: 16),
            Text(
              'confidence: ${(pitch.confidence * 100).toStringAsFixed(0)}%',
              style: theme.textTheme.bodyMedium?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
            ),
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
