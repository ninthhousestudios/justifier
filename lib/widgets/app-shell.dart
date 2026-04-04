import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:window_manager/window_manager.dart';

import '../models/workspace-state.dart';
import '../providers/workspace-provider.dart';
import '../providers/zoom-provider.dart';
import '../theme/app-theme.dart';
import 'connection-status-badge.dart';
import 'wave-column.dart';

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
          // Main content
          Expanded(
            child: Row(
              children: [
                // Workspace area
                Expanded(
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
                // Right panel (console placeholder)
                const VerticalDivider(width: 1),
                const _ConsolePanel(),
              ],
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

class _ConsolePanel extends StatelessWidget {
  const _ConsolePanel();

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 250,
      child: Container(
        color: const Color(0xFF080808),
        padding: const EdgeInsets.all(8),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('CONSOLE',
                style: AppTheme.monoSmall.copyWith(
                  color: AppTheme.prometheusViolet,
                )),
            const Divider(),
            Expanded(
              child: Center(
                child: Text(
                  'Connected to engine',
                  style: AppTheme.monoSmall.copyWith(
                    color: AppTheme.prometheusGreen.withValues(alpha: 0.3),
                  ),
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
