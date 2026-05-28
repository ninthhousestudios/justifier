import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../audio/waveform_type.dart';
import '../drone/drone_state.dart';
import '../settings/reference_pitch_state.dart';

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
                Text('Drone',
                    style: theme.textTheme.titleLarge
                        ?.copyWith(color: theme.colorScheme.primary)),
                const Spacer(),
                if (voices.isNotEmpty)
                  IconButton(
                    icon: const Icon(Icons.delete_sweep),
                    tooltip: 'Remove all',
                    onPressed: () =>
                        ref.read(droneProvider.notifier).removeAll(),
                  ),
                IconButton(
                  icon: const Icon(Icons.add_circle_outline),
                  tooltip: 'Add voice',
                  iconSize: 32,
                  color: theme.colorScheme.primary,
                  onPressed: () =>
                      ref.read(droneProvider.notifier).addVoice(),
                ),
              ],
            ),
          ),
          const Divider(height: 1),
          Expanded(
            child: voices.isEmpty
                ? _EmptyState(
                    onAdd: () =>
                        ref.read(droneProvider.notifier).addVoice())
                : ListView.builder(
                    padding: const EdgeInsets.symmetric(vertical: 8),
                    itemCount: voices.length,
                    itemBuilder: (context, index) =>
                        _VoiceCard(voice: voices[index]),
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
          Icon(Icons.music_note,
              size: 48, color: theme.colorScheme.onSurfaceVariant),
          const SizedBox(height: 12),
          Text('No voices',
              style: theme.textTheme.bodyLarge
                  ?.copyWith(color: theme.colorScheme.onSurfaceVariant)),
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
    final refHz = ref.watch(referencePitchProvider).hz;

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
                  icon: Icon(
                      voice.gateOn ? Icons.pause : Icons.play_arrow),
                  color: voice.gateOn
                      ? theme.colorScheme.primary
                      : theme.colorScheme.onSurfaceVariant,
                  tooltip: voice.gateOn ? 'Pause' : 'Resume',
                  onPressed: () => notifier.toggleGate(voice.id),
                ),
                const SizedBox(width: 4),
                Text(
                  voice.useRatio
                      ? '${voice.ratioLabel}  ${voice.frequencyLabel(refHz)}'
                      : voice.frequencyLabel(refHz),
                  style: theme.textTheme.titleMedium?.copyWith(
                    fontFamily: 'Source Code Pro',
                    color: voice.gateOn
                        ? theme.colorScheme.onSurface
                        : theme.colorScheme.onSurfaceVariant,
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
            _WaveformPicker(voice: voice),
            const SizedBox(height: 4),
            _PitchModeToggle(voice: voice),
            const SizedBox(height: 8),
            if (voice.useRatio)
              _RatioInput(voice: voice)
            else
              _FrequencyRow(voice: voice),
            const SizedBox(height: 4),
            _AmplitudeRow(voice: voice),
          ],
        ),
      ),
    );
  }
}

class _WaveformPicker extends ConsumerWidget {
  const _WaveformPicker({required this.voice});
  final DroneVoice voice;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Wrap(
      spacing: 6,
      children: [
        for (final type in WaveformType.droneTypes)
          ChoiceChip(
            label: Text(type.label),
            selected: voice.waveformType == type,
            visualDensity: VisualDensity.compact,
            onSelected: (_) =>
                ref.read(droneProvider.notifier).setWaveform(voice.id, type),
          ),
      ],
    );
  }
}

class _PitchModeToggle extends ConsumerWidget {
  const _PitchModeToggle({required this.voice});
  final DroneVoice voice;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final theme = Theme.of(context);
    return SegmentedButton<bool>(
      segments: const [
        ButtonSegment(value: true, label: Text('Ratio')),
        ButtonSegment(value: false, label: Text('Hz')),
      ],
      selected: {voice.useRatio},
      onSelectionChanged: (sel) =>
          ref.read(droneProvider.notifier).setRatioMode(voice.id, sel.first),
      style: ButtonStyle(
        visualDensity: VisualDensity.compact,
        textStyle: WidgetStatePropertyAll(theme.textTheme.labelMedium),
      ),
    );
  }
}

class _FrequencyRow extends ConsumerWidget {
  const _FrequencyRow({required this.voice});
  final DroneVoice voice;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final theme = Theme.of(context);
    final notifier = ref.read(droneProvider.notifier);
    return Row(
      children: [
        _StepButton(
          icon: Icons.remove,
          onPressed: () =>
              notifier.setFrequency(voice.id, voice.frequency - 1),
        ),
        SizedBox(
          width: 28,
          child: Text('Hz',
              style: theme.textTheme.labelSmall
                  ?.copyWith(color: theme.colorScheme.onSurfaceVariant)),
        ),
        Expanded(
          child: Slider(
            value: _freqToSlider(voice.frequency),
            onChanged: (v) =>
                notifier.setFrequency(voice.id, _sliderToFreq(v)),
            activeColor: theme.colorScheme.primary,
          ),
        ),
        _StepButton(
          icon: Icons.add,
          onPressed: () =>
              notifier.setFrequency(voice.id, voice.frequency + 1),
        ),
      ],
    );
  }

  static double _freqToSlider(double hz) =>
      (log(hz / 27.5) / log(2)) / 7.0;
  static double _sliderToFreq(double v) =>
      27.5 * pow(2, v * 7.0).toDouble();
}

class _AmplitudeRow extends ConsumerWidget {
  const _AmplitudeRow({required this.voice});
  final DroneVoice voice;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final theme = Theme.of(context);
    final notifier = ref.read(droneProvider.notifier);
    return Row(
      children: [
        _StepButton(
          icon: Icons.remove,
          onPressed: () =>
              notifier.setAmplitude(voice.id, voice.amplitude - 0.05),
        ),
        SizedBox(
          width: 28,
          child: Text('Vol',
              style: theme.textTheme.labelSmall
                  ?.copyWith(color: theme.colorScheme.onSurfaceVariant)),
        ),
        Expanded(
          child: Slider(
            value: voice.amplitude,
            onChanged: (v) => notifier.setAmplitude(voice.id, v),
            activeColor: theme.colorScheme.secondary,
          ),
        ),
        _StepButton(
          icon: Icons.add,
          onPressed: () =>
              notifier.setAmplitude(voice.id, voice.amplitude + 0.05),
        ),
      ],
    );
  }
}

class _RatioInput extends ConsumerStatefulWidget {
  const _RatioInput({required this.voice});
  final DroneVoice voice;

  @override
  ConsumerState<_RatioInput> createState() => _RatioInputState();
}

class _RatioInputState extends ConsumerState<_RatioInput> {
  late final TextEditingController _numCtrl;
  late final TextEditingController _denCtrl;

  @override
  void initState() {
    super.initState();
    _numCtrl = TextEditingController(text: '${widget.voice.numerator}');
    _denCtrl = TextEditingController(text: '${widget.voice.denominator}');
  }

  @override
  void didUpdateWidget(_RatioInput old) {
    super.didUpdateWidget(old);
    if (old.voice.numerator != widget.voice.numerator) {
      _numCtrl.text = '${widget.voice.numerator}';
    }
    if (old.voice.denominator != widget.voice.denominator) {
      _denCtrl.text = '${widget.voice.denominator}';
    }
  }

  @override
  void dispose() {
    _numCtrl.dispose();
    _denCtrl.dispose();
    super.dispose();
  }

  void _submitNum() {
    final n = int.tryParse(_numCtrl.text);
    if (n != null && n >= 1) {
      ref.read(droneProvider.notifier).setNumerator(widget.voice.id, n);
    } else {
      _numCtrl.text = '${widget.voice.numerator}';
    }
  }

  void _submitDen() {
    final d = int.tryParse(_denCtrl.text);
    if (d != null && d >= 1) {
      ref.read(droneProvider.notifier).setDenominator(widget.voice.id, d);
    } else {
      _denCtrl.text = '${widget.voice.denominator}';
    }
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final notifier = ref.read(droneProvider.notifier);
    final voice = widget.voice;

    return Row(
      mainAxisAlignment: MainAxisAlignment.center,
      children: [
        _StepButton(
          icon: Icons.remove,
          onPressed: () => notifier.setNumerator(voice.id, voice.numerator - 1),
        ),
        SizedBox(
          width: 48,
          child: TextField(
            controller: _numCtrl,
            textAlign: TextAlign.center,
            keyboardType: TextInputType.number,
            inputFormatters: [FilteringTextInputFormatter.digitsOnly],
            style: theme.textTheme.titleMedium,
            decoration: const InputDecoration(
              isDense: true,
              contentPadding: EdgeInsets.symmetric(vertical: 8, horizontal: 4),
            ),
            onSubmitted: (_) => _submitNum(),
            onTapOutside: (_) => _submitNum(),
          ),
        ),
        _StepButton(
          icon: Icons.add,
          onPressed: () => notifier.setNumerator(voice.id, voice.numerator + 1),
        ),
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 8),
          child: Text('/',
              style: theme.textTheme.titleLarge
                  ?.copyWith(color: theme.colorScheme.onSurfaceVariant)),
        ),
        _StepButton(
          icon: Icons.remove,
          onPressed: () =>
              notifier.setDenominator(voice.id, voice.denominator - 1),
        ),
        SizedBox(
          width: 48,
          child: TextField(
            controller: _denCtrl,
            textAlign: TextAlign.center,
            keyboardType: TextInputType.number,
            inputFormatters: [FilteringTextInputFormatter.digitsOnly],
            style: theme.textTheme.titleMedium,
            decoration: const InputDecoration(
              isDense: true,
              contentPadding: EdgeInsets.symmetric(vertical: 8, horizontal: 4),
            ),
            onSubmitted: (_) => _submitDen(),
            onTapOutside: (_) => _submitDen(),
          ),
        ),
        _StepButton(
          icon: Icons.add,
          onPressed: () =>
              notifier.setDenominator(voice.id, voice.denominator + 1),
        ),
      ],
    );
  }
}

class _StepButton extends StatelessWidget {
  const _StepButton({required this.icon, required this.onPressed});
  final IconData icon;
  final VoidCallback onPressed;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 32,
      height: 32,
      child: IconButton(
        icon: Icon(icon, size: 18),
        onPressed: onPressed,
        padding: EdgeInsets.zero,
        visualDensity: VisualDensity.compact,
      ),
    );
  }
}
