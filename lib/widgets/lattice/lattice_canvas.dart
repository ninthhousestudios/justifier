import 'dart:ui' as ui;

import 'package:flutter/material.dart';

/// Paints the lattice background: grid dots, 1/1 crosshair, and
/// connection lines between active nodes.
class LatticeCanvasPainter extends CustomPainter {
  const LatticeCanvasPainter({
    required this.activeNodes,
    required this.connections,
    required this.gridSpacing,
    required this.canvasCenter,
    required this.range,
  });

  final Map<(int, int), Color> activeNodes;
  final List<((int, int), (int, int))> connections;
  final double gridSpacing;
  final Offset canvasCenter;
  final int range;

  Offset _gridToPixel(int a, int b) {
    return Offset(
      canvasCenter.dx + a * gridSpacing,
      canvasCenter.dy - b * gridSpacing,
    );
  }

  @override
  void paint(Canvas canvas, Size size) {
    _drawGridDots(canvas);
    _drawCrosshair(canvas, size);
    _drawConnections(canvas);
  }

  void _drawGridDots(Canvas canvas) {
    final dotPaint = Paint()
      ..color = const Color(0xFF00FF00).withValues(alpha: 0.10)
      ..style = PaintingStyle.fill;

    for (var a = -range; a <= range; a++) {
      for (var b = -range; b <= range; b++) {
        final pixel = _gridToPixel(a, b);
        canvas.drawCircle(pixel, 2.0, dotPaint);
      }
    }
  }

  void _drawCrosshair(Canvas canvas, Size size) {
    final crosshairPaint = Paint()
      ..color = const Color(0xFF6B00FF).withValues(alpha: 0.20)
      ..strokeWidth = 0.5
      ..style = PaintingStyle.stroke;

    // Horizontal through origin.
    final leftEdge = _gridToPixel(-range, 0);
    final rightEdge = _gridToPixel(range, 0);
    canvas.drawLine(leftEdge, rightEdge, crosshairPaint);

    // Vertical through origin.
    final topEdge = _gridToPixel(0, range);
    final bottomEdge = _gridToPixel(0, -range);
    canvas.drawLine(topEdge, bottomEdge, crosshairPaint);
  }

  void _drawConnections(Canvas canvas) {
    final linePaint = Paint()
      ..color = const Color(0xFF6B00FF).withValues(alpha: 0.60)
      ..strokeWidth = 1.5
      ..style = PaintingStyle.stroke
      ..strokeCap = ui.StrokeCap.round;

    for (final connection in connections) {
      final (aPos, bPos) = connection;
      final from = _gridToPixel(aPos.$1, aPos.$2);
      final to = _gridToPixel(bPos.$1, bPos.$2);
      canvas.drawLine(from, to, linePaint);
    }
  }

  @override
  bool shouldRepaint(LatticeCanvasPainter oldDelegate) {
    return true;
  }
}
