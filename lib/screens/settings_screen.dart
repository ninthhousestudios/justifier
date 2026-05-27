import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../settings/reference_pitch_state.dart';
import '../settings/ui_scale_state.dart';

class SettingsScreen extends ConsumerWidget {
  const SettingsScreen({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final reference = ref.watch(referencePitchProvider);
    final pitchNotifier = ref.read(referencePitchProvider.notifier);
    final uiScale = ref.watch(uiScaleProvider);
    final scaleNotifier = ref.read(uiScaleProvider.notifier);
    final theme = Theme.of(context);

    return SafeArea(
      child: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          Text(
            'Settings',
            style: theme.textTheme.headlineMedium?.copyWith(
              color: theme.colorScheme.primary,
            ),
          ),
          const SizedBox(height: 24),
          Text(
            'Reference pitch (1/1)',
            style: theme.textTheme.titleMedium,
          ),
          const SizedBox(height: 12),
          Row(
            children: [
              DropdownButton<String>(
                value: reference.noteName,
                hint: const Text('Note'),
                items: noteFrequencies.keys
                    .map((name) => DropdownMenuItem(
                          value: name,
                          child: Text(name),
                        ))
                    .toList(),
                onChanged: (name) {
                  if (name != null) pitchNotifier.setFromNoteName(name);
                },
              ),
              const SizedBox(width: 16),
              SizedBox(
                width: 130,
                child: TextFormField(
                  key: ValueKey(reference.hz),
                  initialValue: reference.hz.toStringAsFixed(2),
                  decoration: const InputDecoration(
                    suffixText: 'Hz',
                    border: OutlineInputBorder(),
                    isDense: true,
                  ),
                  keyboardType:
                      const TextInputType.numberWithOptions(decimal: true),
                  inputFormatters: [
                    FilteringTextInputFormatter.allow(RegExp(r'[\d.]')),
                  ],
                  onFieldSubmitted: (value) {
                    final hz = double.tryParse(value);
                    if (hz != null && hz > 0) pitchNotifier.setHz(hz);
                  },
                ),
              ),
            ],
          ),
          const SizedBox(height: 32),
          Text('UI scale', style: theme.textTheme.titleMedium),
          const SizedBox(height: 8),
          Row(
            children: [
              const Text('A', style: TextStyle(fontSize: 12)),
              Expanded(
                child: Slider(
                  value: uiScale,
                  min: 0.8,
                  max: 1.6,
                  divisions: 16,
                  label: '${(uiScale * 100).round()}%',
                  onChanged: (v) => scaleNotifier.set(v),
                ),
              ),
              const Text('A', style: TextStyle(fontSize: 20)),
            ],
          ),
          Center(
            child: Text(
              '${(uiScale * 100).round()}%',
              style: theme.textTheme.bodySmall?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
            ),
          ),
        ],
      ),
    );
  }
}
