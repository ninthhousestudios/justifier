import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../audio/waveform_type.dart';
import '../drone/drone_state.dart';

class DroneScreen extends ConsumerWidget {
  const DroneScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final voices = ref.watch(droneProvider);
    final theme = Theme.of(context);

    return SafeArea(
      child: Column(
        children: [
          Padding(
            padding: const EdgeInsets.fromLTRB(16, 12, 16, 0),
            child: Row(
              children: [
                Text('Drone', style: theme.textTheme.titleLarge?.copyWith(color: theme.colorScheme.primary)),
                const Spacer(),
                if (voices.isNotEmpty)
                  IconButton(
                    icon: const Icon(Icons.delete_sweep),
                    tooltip: 'Remove all',
                    onPressed: () => ref.read(droneProvider.notifier).removeAll(),
                  ),
                IconButton(
                  icon: const Icon(Icons.add_circle_outline),
                  tooltip: 'Add voice',
                  iconSize: 32,
                  color: theme.colorScheme.primary,
                  onPressed: () => ref.read(droneProvider.notifier).addVoice(),
                ),
              ],
            ),
          ),
          const Divider(height: 1),
          Expanded(
            child: voices.isEmpty
                ? _EmptyState(onAdd: () => ref.read(droneProvider.notifier).addVoice())
                : ListView.builder(
                    padding: const EdgeInsets.symmetric(vertical: 8),
                    itemCount: voices.length,
                    itemBuilder: (context, index) => _VoiceCard(voice: voices[index]),
                  ),
          ),
        ],
      ),
    );
  }
}

class _EmptyState extends StatelessWidget {
  const _EmptyState({required this.onAdd});
  final VoidCallback onAdd;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return Center(
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Icon(Icons.music_note, size: 48, color: theme.colorScheme.onSurfaceVariant),
          const SizedBox(height: 12),
          Text('No voices', style: theme.textTheme.bodyLarge?.copyWith(color: theme.colorScheme.onSurfaceVariant)),
          const SizedBox(height: 16),
          FilledButton.icon(
            onPressed: onAdd,
            icon: const Icon(Icons.add),
            label: const Text('Add voice'),
          ),
        ],
      ),
    );
  }
}

class _VoiceCard extends ConsumerWidget {
  const _VoiceCard({required this.voice});
  final DroneVoice voice;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final theme = Theme.of(context);
    final notifier = ref.read(droneProvider.notifier);

    return Card(
      margin: const EdgeInsets.symmetric(horizontal: 12, vertical: 4),
      color: theme.colorScheme.surfaceContainerHighest,
      child: Padding(
        padding: const EdgeInsets.all(12),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Row(
              children: [
                IconButton(
                  icon: Icon(voice.gateOn ? Icons.volume_up : Icons.volume_off),
                  color: voice.gateOn ? theme.colorScheme.primary : theme.colorScheme.onSurfaceVariant,
                  onPressed: () => notifier.toggleGate(voice.id),
                ),
                const SizedBox(width: 4),
                Text(
                  voice.frequencyLabel,
                  style: theme.textTheme.titleMedium?.copyWith(
                    fontFamily: 'Source Code Pro',
                    color: voice.gateOn ? theme.colorScheme.onSurface : theme.colorScheme.onSurfaceVariant,
                  ),
                ),
                const Spacer(),
                IconButton(
                  icon: const Icon(Icons.close, size: 20),
                  onPressed: () => notifier.removeVoice(voice.id),
                ),
              ],
            ),
            const SizedBox(height: 4),
            _WaveformChips(voice: voice),
            const SizedBox(height: 8),
            _FrequencySlider(voice: voice),
            const SizedBox(height: 4),
            _AmplitudeSlider(voice: voice),
          ],
        ),
      ),
    );
  }
}

class _WaveformChips extends ConsumerWidget {
  const _WaveformChips({required this.voice});
  final DroneVoice voice;

  static const _droneWaveforms = [
    WaveformType.sine,
    WaveformType.triangle,
    WaveformType.saw,
    WaveformType.square,
    WaveformType.fm,
  ];

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final theme = Theme.of(context);
    return Wrap(
      spacing: 6,
      children: [
        for (final wf in _droneWaveforms)
          ChoiceChip(
            label: Text(wf.label, style: const TextStyle(fontSize: 12)),
            selected: voice.waveform == wf,
            selectedColor: theme.colorScheme.primary.withAlpha(50),
            onSelected: (_) => ref.read(droneProvider.notifier).setWaveform(voice.id, wf),
            visualDensity: VisualDensity.compact,
          ),
      ],
    );
  }
}

class _FrequencySlider extends ConsumerWidget {
  const _FrequencySlider({required this.voice});
  final DroneVoice voice;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final theme = Theme.of(context);
    return Row(
      children: [
        SizedBox(
          width: 36,
          child: Text('Hz', style: theme.textTheme.labelSmall?.copyWith(color: theme.colorScheme.onSurfaceVariant)),
        ),
        Expanded(
          child: Slider(
            value: _freqToSlider(voice.frequency),
            onChanged: (v) => ref.read(droneProvider.notifier).setFrequency(voice.id, _sliderToFreq(v)),
            activeColor: theme.colorScheme.primary,
          ),
        ),
      ],
    );
  }

  static double _freqToSlider(double hz) => (log(hz / 27.5) / log(2)) / 7.0;
  static double _sliderToFreq(double v) => 27.5 * pow(2, v * 7.0).toDouble();
}

class _AmplitudeSlider extends ConsumerWidget {
  const _AmplitudeSlider({required this.voice});
  final DroneVoice voice;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final theme = Theme.of(context);
    return Row(
      children: [
        SizedBox(
          width: 36,
          child: Text('Vol', style: theme.textTheme.labelSmall?.copyWith(color: theme.colorScheme.onSurfaceVariant)),
        ),
        Expanded(
          child: Slider(
            value: voice.amplitude,
            onChanged: (v) => ref.read(droneProvider.notifier).setAmplitude(voice.id, v),
            activeColor: theme.colorScheme.secondary,
          ),
        ),
      ],
    );
  }
}
