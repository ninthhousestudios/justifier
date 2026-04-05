import 'dart:ui' as ui;

import 'package:flutter/material.dart';

/// Paints the static lattice background: grid dots, 1/1 crosshair, and
/// connection lines between active nodes.
class LatticeCanvasPainter extends CustomPainter {
  const LatticeCanvasPainter({
    required this.activeNodes,
    required this.connections,
    required this.gridSpacing,
    required this.canvasCenter,
  });

  /// Ratio (numerator, denominator) → wave color for active nodes.
  final Map<(int, int), Color> activeNodes;

  /// Pairs of grid positions (a, b) to draw connection lines between.
  final List<((int, int), (int, int))> connections;

  final double gridSpacing;

  /// The pixel coordinate of lattice origin (0, 0) — typically (5000, 5000).
  final Offset canvasCenter;

  Offset _gridToPixel(int a, int b) {
    return Offset(
      canvasCenter.dx + a * gridSpacing,
      canvasCenter.dy - b * gridSpacing,
    );
  }

  @override
  void paint(Canvas canvas, Size size) {
    _drawGridDots(canvas, size);
    _drawCrosshair(canvas, size);
    _drawConnections(canvas);
  }

  void _drawGridDots(Canvas canvas, Size size) {
    final dotPaint = Paint()
      ..color = const Color(0xFF00FF00).withValues(alpha: 0.10)
      ..style = PaintingStyle.fill;

    const int range = 20;
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

    // Horizontal line through canvasCenter
    canvas.drawLine(
      Offset(0, canvasCenter.dy),
      Offset(size.width, canvasCenter.dy),
      crosshairPaint,
    );
    // Vertical line through canvasCenter
    canvas.drawLine(
      Offset(canvasCenter.dx, 0),
      Offset(canvasCenter.dx, size.height),
      crosshairPaint,
    );
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
    // Always repaint — cheap for <200 nodes and correctness is paramount.
    return true;
  }
}
