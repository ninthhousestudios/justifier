import 'dart:math';

import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';

import '../theme/tuner_colors.dart';

class TunerIndicator extends StatefulWidget {
  const TunerIndicator({
    super.key,
    this.deviationCents,
    this.isActive = false,
  });

  final double? deviationCents;
  final bool isActive;

  @override
  State<TunerIndicator> createState() => _TunerIndicatorState();
}

class _TunerIndicatorState extends State<TunerIndicator>
    with SingleTickerProviderStateMixin {
  late final Ticker _ticker;
  final _repaint = _FrameNotifier();

  double _elapsed = 0;
  double _beatFreq = 0;
  double _beatMin = 1.0;
  double _beatMax = 1.0;
  double _amplitude = 0;
  double _deviationFactor = 0;

  @override
  void initState() {
    super.initState();
    _ticker = createTicker(_onTick)..start();
  }

  @override
  void dispose() {
    _ticker.dispose();
    _repaint.dispose();
    super.dispose();
  }

  void _onTick(Duration elapsed) {
    final now = elapsed.inMicroseconds / 1e6;
    final dt = now - _elapsed;
    _elapsed = now;

    if (dt <= 0 || dt > 0.1) return;

    final hasSignal = widget.isActive && widget.deviationCents != null;
    final absCents = widget.deviationCents?.abs() ?? 0.0;

    final tBeatFreq = hasSignal ? 6.0 * (1.0 - exp(-absCents / 25.0)) : 0.0;
    final tBeatMin =
        hasSignal ? 1.0 - 0.8 * (1.0 - exp(-absCents / 15.0)) : 1.0;
    final tBeatMax =
        hasSignal ? 1.0 + 0.25 * (1.0 - exp(-absCents / 25.0)) : 1.0;
    final tAmplitude = hasSignal ? 1.0 : 0.0;
    final tDeviation = hasSignal ? (absCents / 30.0).clamp(0.0, 1.0) : 0.0;

    final s = 1.0 - exp(-dt * 5.0);
    _beatFreq += (tBeatFreq - _beatFreq) * s;
    _beatMin += (tBeatMin - _beatMin) * s;
    _beatMax += (tBeatMax - _beatMax) * s;
    _amplitude += (tAmplitude - _amplitude) * s;
    _deviationFactor += (tDeviation - _deviationFactor) * s;

    _repaint.notify();
  }

  @override
  Widget build(BuildContext context) {
    final colors = Theme.of(context).extension<TunerColors>()!;
    return CustomPaint(
      painter: _WaveformPainter(
        repaint: _repaint,
        state: this,
        colors: colors,
      ),
      child: const SizedBox.expand(),
    );
  }
}

class _FrameNotifier extends ChangeNotifier {
  void notify() => notifyListeners();
}

class _WaveformPainter extends CustomPainter {
  _WaveformPainter({
    required _FrameNotifier repaint,
    required this.state,
    required this.colors,
  }) : super(repaint: repaint);

  final _TunerIndicatorState state;
  final TunerColors colors;

  static const _scrollPxPerSec = 80.0;
  static const _waveFreqHz = 0.8;
  static const _waveStroke = 2.5;

  @override
  void paint(Canvas canvas, Size size) {
    final centerY = size.height / 2;
    final guideOffset = size.height * 0.3;

    _drawGuides(canvas, size, centerY, guideOffset);

    if (state._amplitude < 0.005) return;

    final path = _buildWavePath(size, centerY, guideOffset);
    final waveColor = _color();

    canvas.drawPath(
      path,
      Paint()
        ..color = waveColor.withAlpha((40 * state._amplitude).round())
        ..strokeWidth = _waveStroke * 4
        ..style = PaintingStyle.stroke
        ..strokeCap = StrokeCap.round
        ..maskFilter = const MaskFilter.blur(BlurStyle.normal, 4),
    );

    canvas.drawPath(
      path,
      Paint()
        ..color = waveColor.withAlpha((255 * state._amplitude).round())
        ..strokeWidth = _waveStroke
        ..style = PaintingStyle.stroke
        ..strokeCap = StrokeCap.round
        ..strokeJoin = StrokeJoin.round,
    );
  }

  void _drawGuides(
      Canvas canvas, Size size, double centerY, double guideOffset) {
    final centerPaint = Paint()
      ..color = colors.guideLine
      ..strokeWidth = 0.5;
    final outerPaint = Paint()
      ..color = colors.guideLine.withAlpha(100)
      ..strokeWidth = 0.5;

    canvas.drawLine(
        Offset(0, centerY), Offset(size.width, centerY), centerPaint);
    canvas.drawLine(Offset(0, centerY - guideOffset),
        Offset(size.width, centerY - guideOffset), outerPaint);
    canvas.drawLine(Offset(0, centerY + guideOffset),
        Offset(size.width, centerY + guideOffset), outerPaint);
  }

  Path _buildWavePath(Size size, double centerY, double guideOffset) {
    final path = Path();
    final baseAmp = guideOffset;
    final elapsed = state._elapsed;
    final width = size.width.toInt();

    for (int i = 0; i <= width; i++) {
      final x = i.toDouble();
      final t = elapsed - (size.width - x) / _scrollPxPerSec;

      final envelope = state._beatMin +
          (state._beatMax - state._beatMin) *
              0.5 *
              (1.0 + cos(2 * pi * state._beatFreq * t));

      final phase = 2 * pi * _waveFreqHz * t;
      final y = centerY - baseAmp * state._amplitude * envelope * sin(phase);

      if (i == 0) {
        path.moveTo(x, y);
      } else {
        path.lineTo(x, y);
      }
    }

    return path;
  }

  Color _color() {
    final f = state._deviationFactor;
    if (f < 0.17) {
      return Color.lerp(colors.inTune, colors.close, f / 0.17)!;
    }
    return Color.lerp(
        colors.close, colors.off, ((f - 0.17) / 0.83).clamp(0.0, 1.0))!;
  }

  @override
  bool shouldRepaint(_WaveformPainter old) => colors != old.colors;
}
