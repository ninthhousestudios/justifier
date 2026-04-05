import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:window_manager/window_manager.dart';

import '../models/workspace_state.dart';
import '../providers/lattice_provider.dart';
import '../providers/workspace_provider.dart';
import '../providers/zoom_provider.dart';
import '../theme/app_theme.dart';
import 'connection_status_badge.dart';
import 'lattice/lattice_panel.dart';
import 'wave_column.dart';

/// Top-level app layout: workspace area + right console panel.
class AppShell extends ConsumerWidget {
  const AppShell({super.key});

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final workspace = ref.watch(workspaceProvider);
    final zoom = ref.watch(zoomProvider);

    return Scaffold(
      body: Column(
        children: [
          // Top bar
          _TopBar(
            referenceHz: workspace.referenceHz,
            masterVolume: workspace.masterVolume,
            zoom: zoom,
            onVolumeChanged: (v) =>
                ref.read(workspaceProvider.notifier).setMasterVolume(v),
            onPanic: () => ref.read(workspaceProvider.notifier).panic(),
            onAddWave: () => ref.read(workspaceProvider.notifier).addWave(),
          ),
          const Divider(height: 1),
          // Main content: voice panel + drag divider + lattice panel
          Expanded(
            child: LayoutBuilder(
              builder: (context, constraints) {
                final lattice = ref.watch(latticeProvider);
                final divider = lattice.dividerFraction;
                final totalWidth = constraints.maxWidth;
                final latticeWidth = totalWidth * divider;
                final voiceWidth = totalWidth - latticeWidth - 8;

                return Row(
                  children: [
                    // Voice panel (left)
                    SizedBox(
                      width: voiceWidth.clamp(0, totalWidth),
                      child: workspace.waves.isEmpty
                          ? _EmptyState(
                              onAddWave: () =>
                                  ref.read(workspaceProvider.notifier).addWave(),
                            )
                          : _WaveList(
                              waves: workspace.waves,
                              referenceHz: workspace.referenceHz,
                            ),
                    ),
                    // Drag divider
                    _DragDivider(
                      onDrag: (dx) {
                        final newFraction = divider - dx / totalWidth;
                        ref
                            .read(latticeProvider.notifier)
                            .setDividerFraction(newFraction);
                      },
                      onDoubleTap: () {
                        ref.read(latticeProvider.notifier).setDividerFraction(
                              divider > 0.01 ? 0.0 : 0.6,
                            );
                      },
                    ),
                    // Lattice panel (right)
                    SizedBox(
                      width: latticeWidth.clamp(0, totalWidth),
                      child: const LatticePanel(),
                    ),
                  ],
                );
              },
            ),
          ),
        ],
      ),
    );
  }
}

class _TopBar extends StatelessWidget {
  const _TopBar({
    required this.referenceHz,
    required this.masterVolume,
    required this.zoom,
    required this.onVolumeChanged,
    required this.onPanic,
    required this.onAddWave,
  });

  final double referenceHz;
  final double masterVolume;
  final double zoom;
  final ValueChanged<double> onVolumeChanged;
  final VoidCallback onPanic;
  final VoidCallback onAddWave;

  @override
  Widget build(BuildContext context) {
    return DragToMoveArea(
      child: Container(
        height: 40,
        padding: const EdgeInsets.symmetric(horizontal: 12),
        color: const Color(0xFF0A0A0A),
        child: Row(
          children: [
            Text('ref: ${referenceHz.toStringAsFixed(1)} Hz',
                style: AppTheme.monoSmall),
            const SizedBox(width: 16),
            if (zoom != 1.0)
              Text('${(zoom * 100).round()}%',
                  style: AppTheme.monoSmall.copyWith(
                    color: AppTheme.prometheusViolet,
                  )),
            const SizedBox(width: 16),
            const ConnectionStatusBadge(),
            const SizedBox(width: 12),
            IconButton(
              onPressed: onAddWave,
              icon: const Icon(Icons.add, size: 16),
              tooltip: 'Add wave',
              padding: EdgeInsets.zero,
              constraints: const BoxConstraints(minWidth: 28, minHeight: 28),
            ),
            const Spacer(),
            SizedBox(
              width: 120,
              child: Slider(
                value: masterVolume,
                onChanged: onVolumeChanged,
              ),
            ),
            const SizedBox(width: 8),
            Text('${(masterVolume * 100).round()}%',
                style: AppTheme.monoSmall),
            const SizedBox(width: 16),
            IconButton(
              onPressed: onPanic,
              icon: const Icon(Icons.stop_circle_outlined),
              tooltip: 'Panic — silence all voices',
              iconSize: 18,
              padding: EdgeInsets.zero,
              constraints: const BoxConstraints(minWidth: 28, minHeight: 28),
            ),
            const SizedBox(width: 8),
            // Window controls
            _WindowButton(
              icon: Icons.minimize,
              tooltip: 'Minimize',
              onPressed: () => windowManager.minimize(),
            ),
            _WindowButton(
              icon: Icons.crop_square,
              tooltip: 'Maximize',
              onPressed: () async {
                if (await windowManager.isMaximized()) {
                  await windowManager.unmaximize();
                } else {
                  await windowManager.maximize();
                }
              },
            ),
            _WindowButton(
              icon: Icons.close,
              tooltip: 'Close',
              onPressed: () => windowManager.close(),
            ),
          ],
        ),
      ),
    );
  }
}

class _EmptyState extends StatelessWidget {
  const _EmptyState({required this.onAddWave});

  final VoidCallback onAddWave;

  @override
  Widget build(BuildContext context) {
    return Center(
      child: Column(
        mainAxisSize: MainAxisSize.min,
        children: [
          Text(
            'No waves yet',
            style: AppTheme.mono.copyWith(
              color: AppTheme.prometheusGreen.withValues(alpha: 0.4),
            ),
          ),
          const SizedBox(height: 16),
          OutlinedButton.icon(
            onPressed: onAddWave,
            icon: const Icon(Icons.add),
            label: const Text('Add Wave'),
          ),
        ],
      ),
    );
  }
}

class _WaveList extends StatelessWidget {
  const _WaveList({required this.waves, required this.referenceHz});

  final List<Wave> waves;
  final double referenceHz;

  @override
  Widget build(BuildContext context) {
    return ListView.builder(
      scrollDirection: Axis.horizontal,
      itemCount: waves.length,
      itemBuilder: (context, index) {
        return WaveColumn(
          wave: waves[index],
          referenceHz: referenceHz,
        );
      },
    );
  }
}

class _WindowButton extends StatelessWidget {
  const _WindowButton({
    required this.icon,
    required this.tooltip,
    required this.onPressed,
  });

  final IconData icon;
  final String tooltip;
  final VoidCallback onPressed;

  @override
  Widget build(BuildContext context) {
    return IconButton(
      onPressed: onPressed,
      icon: Icon(icon, size: 14),
      tooltip: tooltip,
      padding: EdgeInsets.zero,
      constraints: const BoxConstraints(minWidth: 24, minHeight: 24),
    );
  }
}

class _DragDivider extends StatelessWidget {
  const _DragDivider({
    required this.onDrag,
    required this.onDoubleTap,
  });

  final ValueChanged<double> onDrag;
  final VoidCallback onDoubleTap;

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onHorizontalDragUpdate: (details) => onDrag(details.delta.dx),
      onDoubleTap: onDoubleTap,
      child: MouseRegion(
        cursor: SystemMouseCursors.resizeColumn,
        child: Container(
          width: 8,
          color: const Color(0xFF0A0A0A),
          child: Center(
            child: Container(
              width: 2,
              height: 40,
              decoration: BoxDecoration(
                color: AppTheme.prometheusGreen.withValues(alpha: 0.3),
                borderRadius: BorderRadius.circular(1),
              ),
            ),
          ),
        ),
      ),
    );
  }
}
