import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../models/lattice_state.dart';
import '../../providers/lattice_provider.dart';
import '../../theme/app_theme.dart';

class LatticeControls extends ConsumerWidget {
  const LatticeControls({super.key, required this.onHome});

  final VoidCallback onHome;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final state = ref.watch(latticeProvider);
    final notifier = ref.read(latticeProvider.notifier);
    final isProjection = state.viewMode is ProjectionMode;

    return Container(
      decoration: const BoxDecoration(
        color: Color(0xCC0A0A0A),
        borderRadius: BorderRadius.only(
          bottomLeft: Radius.circular(8),
          bottomRight: Radius.circular(8),
        ),
      ),
      padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        crossAxisAlignment: CrossAxisAlignment.center,
        children: [
          // --- View mode toggle ---
          _ModeButton(
            label: 'Nested',
            active: !isProjection,
            onTap: () => notifier.setViewMode(const NestedMode()),
          ),
          const SizedBox(width: 4),
          _ModeButton(
            label: 'Proj',
            active: isProjection,
            onTap: () {
              if (!isProjection) {
                notifier.setViewMode(const ProjectionMode(primeX: 3, primeY: 5));
              }
            },
          ),

          // --- Prime dropdowns (only in projection mode) ---
          if (isProjection) ...[
            const SizedBox(width: 8),
            _PrimeDropdown(
              label: 'X',
              value: (state.viewMode as ProjectionMode).primeX,
              onChanged: (p) {
                if (p == null) return;
                notifier.setViewMode(ProjectionMode(
                  primeX: p,
                  primeY: (state.viewMode as ProjectionMode).primeY,
                ));
              },
            ),
            const SizedBox(width: 4),
            _PrimeDropdown(
              label: 'Y',
              value: (state.viewMode as ProjectionMode).primeY,
              onChanged: (p) {
                if (p == null) return;
                notifier.setViewMode(ProjectionMode(
                  primeX: (state.viewMode as ProjectionMode).primeX,
                  primeY: p,
                ));
              },
            ),
          ],

          const SizedBox(width: 12),

          // --- Octave spinner ---
          IconButton(
            icon: const Icon(Icons.remove, size: 14),
            onPressed: () => notifier.setSpawnOctave(state.spawnOctave - 1),
            padding: EdgeInsets.zero,
            constraints: const BoxConstraints(minWidth: 24, minHeight: 24),
            color: AppTheme.prometheusGreen,
            tooltip: 'Decrease octave',
          ),
          Text('Oct ${state.spawnOctave}', style: AppTheme.monoSmall),
          IconButton(
            icon: const Icon(Icons.add, size: 14),
            onPressed: () => notifier.setSpawnOctave(state.spawnOctave + 1),
            padding: EdgeInsets.zero,
            constraints: const BoxConstraints(minWidth: 24, minHeight: 24),
            color: AppTheme.prometheusGreen,
            tooltip: 'Increase octave',
          ),

          const SizedBox(width: 12),

          // --- Home button ---
          IconButton(
            icon: const Icon(Icons.home, size: 16),
            onPressed: onHome,
            padding: EdgeInsets.zero,
            constraints: const BoxConstraints(minWidth: 24, minHeight: 24),
            color: AppTheme.prometheusGreen,
            tooltip: 'Center on 1/1',
          ),

          // --- Label toggle ---
          IconButton(
            icon: Icon(
              state.showLabels ? Icons.label : Icons.label_outline,
              size: 16,
            ),
            onPressed: () => notifier.toggleLabels(),
            padding: EdgeInsets.zero,
            constraints: const BoxConstraints(minWidth: 24, minHeight: 24),
            color: AppTheme.prometheusGreen,
            tooltip: 'Toggle labels',
          ),
        ],
      ),
    );
  }
}

class _ModeButton extends StatelessWidget {
  const _ModeButton({
    required this.label,
    required this.active,
    required this.onTap,
  });

  final String label;
  final bool active;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final color = active
        ? AppTheme.prometheusGreen
        : AppTheme.prometheusGreen.withValues(alpha: 0.30);

    return TextButton(
      onPressed: onTap,
      style: TextButton.styleFrom(
        padding: const EdgeInsets.symmetric(horizontal: 6, vertical: 2),
        minimumSize: Size.zero,
        tapTargetSize: MaterialTapTargetSize.shrinkWrap,
      ),
      child: Text(label, style: AppTheme.monoSmall.copyWith(color: color)),
    );
  }
}

class _PrimeDropdown extends StatelessWidget {
  const _PrimeDropdown({
    required this.label,
    required this.value,
    required this.onChanged,
  });

  static const _primeOptions = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31];

  final String label;
  final int value;
  final ValueChanged<int?> onChanged;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Text(label, style: AppTheme.monoSmall),
        const SizedBox(width: 2),
        DropdownButton<int>(
          value: value,
          isDense: true,
          underline: const SizedBox.shrink(),
          dropdownColor: const Color(0xFF111111),
          style: AppTheme.monoSmall,
          items: _primeOptions
              .map((p) => DropdownMenuItem(
                    value: p,
                    child: Text('$p', style: AppTheme.monoSmall),
                  ))
              .toList(),
          onChanged: onChanged,
        ),
      ],
    );
  }
}
