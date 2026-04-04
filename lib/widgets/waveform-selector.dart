import 'package:flutter/material.dart';

import '../audio/waveform_type.dart';
import '../theme/app-theme.dart';

/// Compact row of waveform type chips.
class WaveformSelector extends StatelessWidget {
  const WaveformSelector({
    super.key,
    required this.selected,
    required this.onChanged,
    this.color,
  });

  final WaveformType selected;
  final ValueChanged<WaveformType> onChanged;
  final Color? color;

  static const _labels = {
    WaveformType.sine: 'sin',
    WaveformType.triangle: 'tri',
    WaveformType.saw: 'saw',
    WaveformType.square: 'sqr',
    WaveformType.pulse: 'pls',
    WaveformType.whiteNoise: 'wht',
    WaveformType.pinkNoise: 'pnk',
    WaveformType.brownNoise: 'brn',
    WaveformType.lfnoise0: 'ln0',
    WaveformType.lfnoise1: 'ln1',
    WaveformType.lfnoise2: 'ln2',
    WaveformType.fm: 'fm',
  };

  @override
  Widget build(BuildContext context) {
    final accentColor = color ?? AppTheme.prometheusGreen;

    return Wrap(
      spacing: 2,
      children: WaveformType.values.map((type) {
        final isSelected = type == selected;
        return SizedBox(
          height: 22,
          child: TextButton(
            onPressed: () => onChanged(type),
            style: TextButton.styleFrom(
              padding: const EdgeInsets.symmetric(horizontal: 4),
              minimumSize: Size.zero,
              tapTargetSize: MaterialTapTargetSize.shrinkWrap,
              backgroundColor:
                  isSelected ? accentColor.withValues(alpha: 0.2) : null,
              foregroundColor: isSelected ? accentColor : Colors.grey,
              textStyle: AppTheme.monoSmall,
            ),
            child: Text(_labels[type]!),
          ),
        );
      }).toList(),
    );
  }
}
