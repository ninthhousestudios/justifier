import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../models/lattice_state.dart';
import '../../providers/lattice_provider.dart';
import '../../providers/workspace_provider.dart';
import '../../theme/app_theme.dart';

class LatticeNodeWidget extends ConsumerWidget {
  const LatticeNodeWidget({
    super.key,
    required this.numerator,
    required this.denominator,
    required this.isActive,
    this.activeColor,
    this.voiceCount = 0,
    this.isPreview = false,
    this.is1_1 = false,
    this.isHigherPrime = false,
    required this.referenceHz,
    required this.spawnOctave,
  });

  final int numerator;
  final int denominator;
  final bool isActive;
  final Color? activeColor;
  final int voiceCount;
  final bool isPreview;
  final bool is1_1;
  final bool isHigherPrime;
  final double referenceHz;
  final int spawnOctave;

  static const _green = AppTheme.prometheusGreen;
  static const _violet = AppTheme.prometheusViolet;

  double get _size {
    if (is1_1) return 30.0;
    if (isActive) return 28.0;
    if (isHigherPrime) return 18.0;
    return 24.0;
  }

  double get _fontSize => is1_1 ? 10.0 : 8.0;

  String _tooltipText() {
    final cents = ratioToCents(numerator, denominator);
    final hz = referenceHz * numerator / denominator * pow(2, spawnOctave);
    return '$numerator/$denominator\n${cents.toStringAsFixed(1)}¢\n${hz.toStringAsFixed(1)} Hz';
  }

  BoxDecoration _decoration() {
    if (is1_1) {
      return BoxDecoration(
        shape: BoxShape.circle,
        color: _green.withValues(alpha: 0.20),
        border: Border.all(color: _green, width: 2),
      );
    }
    if (isActive) {
      return BoxDecoration(
        shape: BoxShape.circle,
        color: activeColor ?? _green,
      );
    }
    if (isPreview) {
      return BoxDecoration(
        shape: BoxShape.circle,
        color: Colors.transparent,
        border: Border.all(color: _green, width: 1.5),
      );
    }
    // Empty normal or higher-prime child — same style, different size
    return BoxDecoration(
      shape: BoxShape.circle,
      color: const Color(0xFF1A1A1A),
      border: Border.all(color: _green.withValues(alpha: 0.30), width: 1),
    );
  }

  bool get _isAltPressed {
    final pressed = HardwareKeyboard.instance.logicalKeysPressed;
    return pressed.contains(LogicalKeyboardKey.altLeft) ||
        pressed.contains(LogicalKeyboardKey.altRight);
  }

  void _onTap(WidgetRef ref) {
    if (_isAltPressed) {
      _togglePreview(ref);
      return;
    }
    if (isActive) {
      _selectVoice(ref);
    } else {
      _spawnVoice(ref);
    }
  }

  void _togglePreview(WidgetRef ref) {
    final latticeState = ref.read(latticeProvider);
    final currentPreview = latticeState.previewRatio;
    if (currentPreview != null &&
        currentPreview.$1 == numerator &&
        currentPreview.$2 == denominator) {
      ref.read(latticeProvider.notifier).stopPreview();
    } else {
      ref.read(latticeProvider.notifier).startPreview(numerator, denominator, referenceHz);
    }
  }

  void _spawnVoice(WidgetRef ref) {
    final workspace = ref.read(workspaceProvider);
    String waveId = workspace.focusedWaveId ?? ref.read(workspaceProvider.notifier).addWave();
    ref.read(workspaceProvider.notifier).addVoiceFromLattice(waveId, numerator, denominator, spawnOctave);
  }

  void _selectVoice(WidgetRef ref) {
    final workspace = ref.read(workspaceProvider);
    for (final wave in workspace.waves) {
      for (final voice in wave.voices) {
        if (voice.numerator == numerator && voice.denominator == denominator) {
          ref.read(workspaceProvider.notifier).focusWave(wave.id);
          return;
        }
      }
    }
  }

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    final size = _size;

    Widget node = Container(
      width: size,
      height: size,
      decoration: _decoration(),
      child: Stack(
        clipBehavior: Clip.none,
        children: [
          Center(
            child: Text(
              '$numerator/$denominator',
              style: AppTheme.monoSmall.copyWith(
                fontSize: _fontSize,
                color: isActive
                    ? Colors.black.withValues(alpha: 0.85)
                    : AppTheme.prometheusGreen,
              ),
              textAlign: TextAlign.center,
              overflow: TextOverflow.clip,
            ),
          ),
          if (is1_1)
            Positioned.fill(
              child: Container(
                decoration: BoxDecoration(
                  shape: BoxShape.circle,
                  border: Border.all(color: _violet, width: 1),
                ),
              ),
            ),
          if (isActive && voiceCount > 1)
            Positioned(
              top: -2,
              right: -2,
              child: Container(
                padding: const EdgeInsets.all(2),
                decoration: const BoxDecoration(
                  shape: BoxShape.circle,
                  color: Color(0xFF1A1A1A),
                ),
                child: Text(
                  '$voiceCount',
                  style: AppTheme.monoSmall.copyWith(fontSize: 7),
                ),
              ),
            ),
        ],
      ),
    );

    node = Tooltip(
      message: _tooltipText(),
      child: node,
    );

    node = GestureDetector(
      onTap: () => _onTap(ref),
      onSecondaryTap: () => ref
          .read(latticeProvider.notifier)
          .toggleExpanded(numerator, denominator),
      child: node,
    );

    node = MouseRegion(
      cursor: SystemMouseCursors.click,
      onEnter: (_) =>
          ref.read(latticeProvider.notifier).setHovered(numerator, denominator),
      onExit: (_) => ref.read(latticeProvider.notifier).setHovered(null, null),
      child: node,
    );

    return node;
  }
}
