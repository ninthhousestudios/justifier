import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../audio/waveform_type.dart';
import '../models/workspace_state.dart';
import '../providers/workspace_provider.dart';
import '../theme/app_theme.dart';
import 'ratio_input.dart';
import 'waveform_selector.dart';

const _filterTypeLabels = ['LP', 'HP', 'BP', 'NT'];

double _sliderToCutoff(double v) => 20.0 * pow(1000.0, v);
double _cutoffToSlider(double hz) => log(hz / 20.0) / log(1000.0);

String _cutoffDisplay(double hz) {
  if (hz >= 1000) {
    return '${(hz / 1000).toStringAsFixed(1)}k';
  }
  return '${hz.round()} Hz';
}

/// Compact card displaying all controls for a single voice.
class VoiceCard extends ConsumerWidget {
  const VoiceCard({
    super.key,
    required this.waveId,
    required this.voice,
    required this.referenceHz,
    this.color,
  });

  final String waveId;
  final Voice voice;
  final double referenceHz;
  final Color? color;

  void _update(WidgetRef ref, Voice updated) {
    ref.read(workspaceProvider.notifier).updateVoice(waveId, voice.id, updated);
  }

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final accentColor = color ?? AppTheme.prometheusGreen;
    final dimColor = accentColor.withValues(alpha: 0.5);
    final hz = voice.frequencyHz(referenceHz);

    if (voice.dying) {
      return _DyingBar(
        fadeTime: voice.fadeTime,
        color: accentColor,
        onUndo: () => ref
            .read(workspaceProvider.notifier)
            .cancelRemoveVoice(waveId, voice.id),
      );
    }

    return Container(
      margin: const EdgeInsets.only(bottom: 4),
      padding: const EdgeInsets.all(6),
      decoration: BoxDecoration(
        color: const Color(0xFF0D0D0D),
        borderRadius: BorderRadius.circular(4),
        border: Border.all(
          color: voice.enabled ? accentColor.withValues(alpha: 0.3) : const Color(0xFF333333),
          width: 1,
        ),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          // Row 1: waveform selector + Hz display + enable toggle + remove
          Row(
            children: [
              Expanded(
                child: WaveformSelector(
                  selected: voice.waveform,
                  onChanged: (t) => _update(ref, voice.copyWith(waveform: t)),
                  color: accentColor,
                ),
              ),
              if (voice.waveform.isPitched)
                Padding(
                  padding: const EdgeInsets.only(right: 4),
                  child: Text(
                    '${hz.toStringAsFixed(1)} Hz',
                    style: AppTheme.monoSmall.copyWith(color: dimColor),
                    overflow: TextOverflow.ellipsis,
                  ),
                ),
              SizedBox(
                width: 24,
                height: 24,
                child: IconButton(
                  onPressed: () => ref
                      .read(workspaceProvider.notifier)
                      .toggleVoiceEnabled(waveId, voice.id),
                  icon: Icon(
                    voice.enabled
                        ? Icons.volume_up_rounded
                        : Icons.volume_off_rounded,
                    size: 14,
                    color: voice.enabled ? accentColor : Colors.grey,
                  ),
                  padding: EdgeInsets.zero,
                  constraints:
                      const BoxConstraints(minWidth: 24, minHeight: 24),
                  tooltip: voice.enabled ? 'Mute voice' : 'Unmute voice',
                ),
              ),
              SizedBox(
                width: 24,
                height: 24,
                child: IconButton(
                  onPressed: () => ref
                      .read(workspaceProvider.notifier)
                      .removeVoice(waveId, voice.id),
                  icon: const Icon(Icons.close, size: 12),
                  padding: EdgeInsets.zero,
                  constraints:
                      const BoxConstraints(minWidth: 24, minHeight: 24),
                  tooltip: 'Remove voice',
                ),
              ),
            ],
          ),

          // Row 2: ratio input + octave + frequency display (pitched waveforms only)
          if (voice.waveform.isPitched) ...[
            const SizedBox(height: 4),
            Row(
              children: [
                RatioInput(
                  numerator: voice.numerator,
                  denominator: voice.denominator,
                  onChanged: (n, d) =>
                      _update(ref, voice.copyWith(numerator: n, denominator: d)),
                  color: accentColor,
                ),
                const SizedBox(width: 4),
                SizedBox(
                  width: 18,
                  height: 18,
                  child: IconButton(
                    onPressed: voice.octave > 0
                        ? () =>
                            _update(ref, voice.copyWith(octave: voice.octave - 1))
                        : null,
                    icon: const Icon(Icons.remove, size: 10),
                    padding: EdgeInsets.zero,
                    constraints:
                        const BoxConstraints(minWidth: 18, minHeight: 18),
                  ),
                ),
                Text(
                  'o${voice.octave}',
                  style: AppTheme.monoSmall.copyWith(color: dimColor),
                ),
                SizedBox(
                  width: 18,
                  height: 18,
                  child: IconButton(
                    onPressed: voice.octave < 9
                        ? () =>
                            _update(ref, voice.copyWith(octave: voice.octave + 1))
                        : null,
                    icon: const Icon(Icons.add, size: 10),
                    padding: EdgeInsets.zero,
                    constraints:
                        const BoxConstraints(minWidth: 18, minHeight: 18),
                  ),
                ),
              ],
            ),
          ],

          const SizedBox(height: 4),

          // Row 3: amplitude + pan sliders
          _SliderRow(
            label: 'amp',
            value: voice.amplitude,
            step: 0.01,
            onChanged: (v) => _update(ref, voice.copyWith(amplitude: v)),
            color: accentColor,
          ),
          _SliderRow(
            label: 'pan',
            value: voice.pan,
            min: -1,
            max: 1,
            step: 0.01,
            onChanged: (v) => _update(ref, voice.copyWith(pan: v)),
            color: accentColor,
            displayValue: voice.pan.toStringAsFixed(2),
          ),

          // Row 4: detune slider (pitched waveforms only)
          if (voice.waveform.isPitched)
            _SliderRow(
              label: 'det',
              value: voice.detuneCents,
              min: -100,
              max: 100,
              step: 1,
              onChanged: (v) => _update(ref, voice.copyWith(detuneCents: v)),
              color: accentColor,
              displayValue: '${voice.detuneCents.toStringAsFixed(0)}c',
            ),

          // Filter controls (all voices)
          const SizedBox(height: 2),
          _FilterTypeRow(
            selected: voice.filterType,
            onChanged: (t) => _update(ref, voice.copyWith(filterType: t)),
            color: accentColor,
          ),
          _SliderRow(
            label: 'cut',
            value: _cutoffToSlider(voice.filterCutoff),
            step: 0.01,
            onChanged: (v) =>
                _update(ref, voice.copyWith(filterCutoff: _sliderToCutoff(v))),
            color: accentColor,
            displayValue: _cutoffDisplay(voice.filterCutoff),
          ),
          _SliderRow(
            label: 'res',
            value: voice.filterResonance,
            step: 0.01,
            onChanged: (v) =>
                _update(ref, voice.copyWith(filterResonance: v)),
            color: accentColor,
          ),

          // FM controls (FM waveform only)
          if (voice.waveform == WaveformType.fm) ...[
            _SliderRow(
              label: 'mrt',
              value: voice.modRatio,
              min: 0.1,
              max: 16,
              step: 0.1,
              onChanged: (v) => _update(ref, voice.copyWith(modRatio: v)),
              color: accentColor,
              displayValue: voice.modRatio.toStringAsFixed(1),
            ),
            _SliderRow(
              label: 'mdx',
              value: voice.modIndex,
              min: 0,
              max: 20,
              step: 0.1,
              onChanged: (v) => _update(ref, voice.copyWith(modIndex: v)),
              color: accentColor,
              displayValue: voice.modIndex.toStringAsFixed(1),
            ),
          ],
        ],
      ),
    );
  }
}

class _SliderRow extends StatelessWidget {
  const _SliderRow({
    required this.label,
    required this.value,
    required this.onChanged,
    this.min = 0,
    this.max = 1,
    this.step = 0.01,
    this.color,
    this.displayValue,
  });

  final String label;
  final double value;
  final double min;
  final double max;
  final double step;
  final ValueChanged<double> onChanged;
  final Color? color;
  final String? displayValue;

  void _nudge(double delta) {
    onChanged((value + delta).clamp(min, max));
  }

  @override
  Widget build(BuildContext context) {
    final dimColor = (color ?? AppTheme.prometheusGreen).withValues(alpha: 0.5);

    return SizedBox(
      height: 20,
      child: Row(
        children: [
          SizedBox(
            width: 24,
            child: Text(label, style: AppTheme.monoSmall.copyWith(color: dimColor)),
          ),
          _NudgeButton(
            icon: Icons.remove,
            onPressed: value > min ? () => _nudge(-step) : null,
          ),
          Expanded(
            child: SliderTheme(
              data: SliderTheme.of(context).copyWith(
                trackHeight: 2,
                thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 4),
                overlayShape: const RoundSliderOverlayShape(overlayRadius: 8),
              ),
              child: Slider(
                value: value,
                min: min,
                max: max,
                onChanged: onChanged,
              ),
            ),
          ),
          _NudgeButton(
            icon: Icons.add,
            onPressed: value < max ? () => _nudge(step) : null,
          ),
          SizedBox(
            width: 36,
            child: Text(
              displayValue ?? '${(value * 100).round()}%',
              style: AppTheme.monoSmall.copyWith(color: dimColor),
              textAlign: TextAlign.right,
            ),
          ),
        ],
      ),
    );
  }
}

class _FilterTypeRow extends StatelessWidget {
  const _FilterTypeRow({
    required this.selected,
    required this.onChanged,
    this.color,
  });

  final int selected;
  final ValueChanged<int> onChanged;
  final Color? color;

  @override
  Widget build(BuildContext context) {
    final accentColor = color ?? AppTheme.prometheusGreen;
    final dimColor = accentColor.withValues(alpha: 0.5);

    return SizedBox(
      height: 20,
      child: Row(
        children: [
          SizedBox(
            width: 24,
            child: Text('flt', style: AppTheme.monoSmall.copyWith(color: dimColor)),
          ),
          for (int i = 0; i < _filterTypeLabels.length; i++)
            Padding(
              padding: const EdgeInsets.only(right: 2),
              child: SizedBox(
                height: 18,
                child: OutlinedButton(
                  onPressed: () => onChanged(i),
                  style: OutlinedButton.styleFrom(
                    padding: const EdgeInsets.symmetric(horizontal: 6),
                    minimumSize: Size.zero,
                    tapTargetSize: MaterialTapTargetSize.shrinkWrap,
                    side: BorderSide(
                      color: i == selected ? accentColor : const Color(0xFF333333),
                    ),
                    backgroundColor:
                        i == selected ? accentColor.withValues(alpha: 0.15) : Colors.transparent,
                    foregroundColor: i == selected ? accentColor : dimColor,
                    textStyle: AppTheme.monoSmall,
                  ),
                  child: Text(_filterTypeLabels[i]),
                ),
              ),
            ),
        ],
      ),
    );
  }
}

class _DyingBar extends StatefulWidget {
  const _DyingBar({
    required this.fadeTime,
    required this.color,
    required this.onUndo,
  });

  final double fadeTime;
  final Color color;
  final VoidCallback onUndo;

  @override
  State<_DyingBar> createState() => _DyingBarState();
}

class _DyingBarState extends State<_DyingBar> {
  late final Stopwatch _stopwatch;
  late final Duration _total;

  @override
  void initState() {
    super.initState();
    _total = Duration(milliseconds: (widget.fadeTime * 1000).round());
    _stopwatch = Stopwatch()..start();
    _tick();
  }

  void _tick() {
    if (!mounted) return;
    setState(() {});
    if (_stopwatch.elapsed < _total) {
      Future.delayed(const Duration(milliseconds: 100), _tick);
    }
  }

  @override
  Widget build(BuildContext context) {
    final remaining = (_total - _stopwatch.elapsed).inMilliseconds.clamp(0, _total.inMilliseconds);
    final seconds = (remaining / 1000).ceil();
    final progress = 1.0 - (remaining / _total.inMilliseconds);

    return Container(
      margin: const EdgeInsets.only(bottom: 4),
      padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 4),
      decoration: BoxDecoration(
        color: const Color(0xFF0D0D0D),
        borderRadius: BorderRadius.circular(4),
        border: Border.all(color: const Color(0xFF333333)),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          Row(
            children: [
              Text(
                'fading... ${seconds}s',
                style: AppTheme.monoSmall.copyWith(color: Colors.grey),
              ),
              const Spacer(),
              SizedBox(
                height: 22,
                child: TextButton(
                  onPressed: widget.onUndo,
                  style: TextButton.styleFrom(
                    padding: const EdgeInsets.symmetric(horizontal: 8),
                    minimumSize: Size.zero,
                    tapTargetSize: MaterialTapTargetSize.shrinkWrap,
                    foregroundColor: widget.color,
                    textStyle: AppTheme.monoSmall,
                  ),
                  child: const Text('undo'),
                ),
              ),
            ],
          ),
          const SizedBox(height: 2),
          ClipRRect(
            borderRadius: BorderRadius.circular(1),
            child: LinearProgressIndicator(
              value: progress,
              minHeight: 2,
              backgroundColor: const Color(0xFF333333),
              valueColor: AlwaysStoppedAnimation<Color>(
                widget.color.withValues(alpha: 0.4),
              ),
            ),
          ),
        ],
      ),
    );
  }
}

class _NudgeButton extends StatelessWidget {
  const _NudgeButton({required this.icon, required this.onPressed});

  final IconData icon;
  final VoidCallback? onPressed;

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 18,
      height: 18,
      child: IconButton(
        onPressed: onPressed,
        icon: Icon(icon, size: 10),
        padding: EdgeInsets.zero,
        constraints: const BoxConstraints(minWidth: 18, minHeight: 18),
      ),
    );
  }
}
