import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../audio/waveform_type.dart';
import '../models/workspace_state.dart';
import '../providers/workspace_provider.dart';
import '../theme/app_theme.dart';
import 'voice_card.dart';

/// A vertical column representing a Wave and its voices.
class WaveColumn extends ConsumerWidget {
  const WaveColumn({
    super.key,
    required this.wave,
    required this.referenceHz,
  });

  final Wave wave;
  final double referenceHz;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Container(
      width: 280,
      margin: const EdgeInsets.all(4),
      decoration: BoxDecoration(
        border: Border(top: BorderSide(color: wave.color, width: 2)),
        color: const Color(0xFF111111),
        borderRadius: BorderRadius.circular(6),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          // Header
          _WaveHeader(wave: wave),
          const Divider(height: 1),
          // Voice list
          Expanded(
            child: wave.voices.isEmpty
                ? Center(
                    child: Text(
                      'no voices',
                      style: AppTheme.monoSmall.copyWith(
                        color: wave.color.withValues(alpha: 0.3),
                      ),
                    ),
                  )
                : ListView.builder(
                    padding: const EdgeInsets.all(4),
                    itemCount: wave.voices.length,
                    itemBuilder: (context, index) {
                      final voice = wave.voices[index];
                      return VoiceCard(
                        waveId: wave.id,
                        voice: voice,
                        referenceHz: referenceHz,
                        color: wave.color,
                      );
                    },
                  ),
          ),
        ],
      ),
    );
  }
}

class _WaveHeader extends ConsumerWidget {
  const _WaveHeader({required this.wave});

  final Wave wave;

  @override
  Widget build(BuildContext context, WidgetRef ref) {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 8, vertical: 4),
      child: Row(
        children: [
          Expanded(
            child: Text(
              wave.name,
              style: AppTheme.mono.copyWith(color: wave.color),
              overflow: TextOverflow.ellipsis,
            ),
          ),
          SizedBox(
            width: 24,
            height: 24,
            child: IconButton(
              onPressed: () => ref
                  .read(workspaceProvider.notifier)
                  .addVoice(wave.id, WaveformType.sine),
              icon: const Icon(Icons.add, size: 14),
              padding: EdgeInsets.zero,
              constraints:
                  const BoxConstraints(minWidth: 24, minHeight: 24),
              tooltip: 'Add voice',
            ),
          ),
          SizedBox(
            width: 24,
            height: 24,
            child: IconButton(
              onPressed: () => ref
                  .read(workspaceProvider.notifier)
                  .removeWave(wave.id),
              icon: const Icon(Icons.close, size: 12),
              padding: EdgeInsets.zero,
              constraints:
                  const BoxConstraints(minWidth: 24, minHeight: 24),
              tooltip: 'Remove wave',
            ),
          ),
        ],
      ),
    );
  }
}
