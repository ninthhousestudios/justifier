import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../providers/engine_provider.dart';
import '../theme/app_theme.dart';

/// Displays engine running status and active voice count.
/// Polls the engine atomics every 500ms.
class ConnectionStatusBadge extends ConsumerStatefulWidget {
  const ConnectionStatusBadge({super.key});

  @override
  ConsumerState<ConnectionStatusBadge> createState() =>
      _ConnectionStatusBadgeState();
}

class _ConnectionStatusBadgeState extends ConsumerState<ConnectionStatusBadge> {
  Timer? _timer;
  bool _running = false;
  int _voiceCount = 0;

  @override
  void initState() {
    super.initState();
    _poll();
    _timer = Timer.periodic(const Duration(milliseconds: 500), (_) => _poll());
  }

  void _poll() {
    final engine = ref.read(engineProvider);
    final running = engine.isRunning;
    final count = engine.activeVoiceCount;
    if (running != _running || count != _voiceCount) {
      setState(() {
        _running = running;
        _voiceCount = count;
      });
    }
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final color = _running ? AppTheme.prometheusGreen : Colors.red;
    final label = _running ? 'v:$_voiceCount' : 'stopped';

    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        Container(
          width: 6,
          height: 6,
          decoration: BoxDecoration(
            color: color,
            shape: BoxShape.circle,
          ),
        ),
        const SizedBox(width: 4),
        Text(label, style: AppTheme.monoSmall.copyWith(color: color)),
      ],
    );
  }
}
