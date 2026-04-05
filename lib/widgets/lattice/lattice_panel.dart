import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../models/lattice_state.dart';
import '../../models/workspace_state.dart';
import '../../providers/lattice_provider.dart';
import '../../providers/workspace_provider.dart';
import 'lattice_canvas.dart';
import 'lattice_controls.dart';
import 'lattice_node_widget.dart';

/// The main lattice explorer panel. Owns an [InteractiveViewer] over a 10000×10000
/// virtual canvas with the JI lattice origin at (5000, 5000).
class LatticePanel extends ConsumerStatefulWidget {
  const LatticePanel({super.key});

  @override
  ConsumerState<LatticePanel> createState() => _LatticePanelState();
}

class _LatticePanelState extends ConsumerState<LatticePanel> {
  static const double gridSpacing = 80.0;
  static const double _canvasSize = 10000.0;
  static const double _canvasOrigin = 5000.0;
  static const double _nodeSize = 48.0;

  late final TransformationController _ctrl;

  /// Set to false after we center the transform on first layout.
  bool _needsCenter = true;

  @override
  void initState() {
    super.initState();
    // Set a reasonable default transform before adding the listener.
    // Will be adjusted to actual widget size on first layout.
    _ctrl = TransformationController(
      Matrix4.identity()
        ..setTranslationRaw(-_canvasOrigin, -_canvasOrigin, 0),
    );
    _ctrl.addListener(_onTransformChanged);
  }

  @override
  void dispose() {
    _ctrl.removeListener(_onTransformChanged);
    _ctrl.dispose();
    super.dispose();
  }

  void _onTransformChanged() {
    // Trigger a rebuild so _buildNodes() recomputes the visible viewport.
    setState(() {});
  }

  /// Invert the current transform to find which part of the canvas is visible.
  /// Returns the viewport in lattice coordinate space (grid steps, not pixels).
  Rect _computeVisibleViewport(Size widgetSize) {
    final inverse = _ctrl.value.clone()..invert();

    // Transform the four viewport corners into canvas space.
    final tl = MatrixUtils.transformPoint(inverse, Offset.zero);
    final tr = MatrixUtils.transformPoint(inverse, Offset(widgetSize.width, 0));
    final bl = MatrixUtils.transformPoint(inverse, Offset(0, widgetSize.height));
    final br = MatrixUtils.transformPoint(
        inverse, Offset(widgetSize.width, widgetSize.height));

    final canvasRect = Rect.fromPoints(
      Offset(
        [tl.dx, tr.dx, bl.dx, br.dx].reduce((a, b) => a < b ? a : b),
        [tl.dy, tr.dy, bl.dy, br.dy].reduce((a, b) => a < b ? a : b),
      ),
      Offset(
        [tl.dx, tr.dx, bl.dx, br.dx].reduce((a, b) => a > b ? a : b),
        [tl.dy, tr.dy, bl.dy, br.dy].reduce((a, b) => a > b ? a : b),
      ),
    );

    // Convert canvas pixels → lattice coordinates.
    // Canvas pixel x = _canvasOrigin + a * gridSpacing  →  a = (x - _canvasOrigin) / gridSpacing
    // Canvas pixel y = _canvasOrigin - b * gridSpacing  →  b = (_canvasOrigin - y) / gridSpacing
    // Note: left/right in Rect align with a-axis; top/bottom are inverted for b-axis.
    return Rect.fromLTRB(
      (canvasRect.left - _canvasOrigin) / gridSpacing,
      (_canvasOrigin - canvasRect.bottom) / gridSpacing, // b increases upward
      (canvasRect.right - _canvasOrigin) / gridSpacing,
      (_canvasOrigin - canvasRect.top) / gridSpacing,
    );
  }

  List<Widget> _buildNodes(Size widgetSize) {
    final lattice = ref.watch(latticeProvider);
    final workspace = ref.watch(workspaceProvider);

    // Build lookup: (numerator, denominator) → list of (waveId, Voice).
    final Map<(int, int), List<(String, Voice)>> activeByRatio = {};
    for (final wave in workspace.waves) {
      for (final voice in wave.voices) {
        if (!voice.dying) {
          final key = (voice.numerator, voice.denominator);
          activeByRatio.putIfAbsent(key, () => []).add((wave.id, voice));
        }
      }
    }

    final viewport = _computeVisibleViewport(widgetSize);
    final mode = lattice.viewMode;
    final visibleNodes = generateVisibleNodes(mode, viewport);

    // Collect all nodes to display: visible base nodes + higher-prime neighbors
    // of expanded nodes.
    final allNodes = <LatticeNode>[];
    for (final node in visibleNodes) {
      allNodes.add(node);
      if (lattice.expandedNodes.contains((node.numerator, node.denominator))) {
        allNodes.addAll(getHigherPrimeNeighbors(node, [7, 11, 13]));
      }
    }

    // Deduplicate by grid position (last writer wins — fine for display).
    final Map<(int, int), LatticeNode> byGridPos = {};
    for (final n in allNodes) {
      byGridPos[n.gridPosition] = n;
    }

    // Build connection list: pairs of active nodes that are Manhattan-distance 1 apart.
    final activeGridPositions = <(int, int)>{};
    for (final node in byGridPos.values) {
      final key = (node.numerator, node.denominator);
      if (activeByRatio.containsKey(key)) {
        activeGridPositions.add(node.gridPosition);
      }
    }

    final connections = <((int, int), (int, int))>[];
    final activeList = activeGridPositions.toList();
    for (var i = 0; i < activeList.length; i++) {
      for (var j = i + 1; j < activeList.length; j++) {
        final (ai, bi) = activeList[i];
        final (aj, bj) = activeList[j];
        final manhattan = (ai - aj).abs() + (bi - bj).abs();
        if (manhattan == 1) {
          connections.add((activeList[i], activeList[j]));
        }
      }
    }

    // Build active node color map for the canvas painter.
    final Map<(int, int), Color> activeNodeColors = {};
    for (final entry in activeByRatio.entries) {
      final waveId = entry.value.first.$1;
      final wave = workspace.waves.firstWhere(
        (w) => w.id == waveId,
        orElse: () => workspace.waves.first,
      );
      activeNodeColors[entry.key] = wave.color;
    }

    // Positioned node widgets.
    final widgets = <Widget>[];

    // Canvas painter as the bottommost layer.
    widgets.add(
      CustomPaint(
        painter: LatticeCanvasPainter(
          activeNodes: activeNodeColors,
          connections: connections,
          gridSpacing: gridSpacing,
          canvasCenter: const Offset(_canvasOrigin, _canvasOrigin),
        ),
        size: const Size(_canvasSize, _canvasSize),
      ),
    );

    // Node widgets.
    for (final node in byGridPos.values) {
      final (a, b) = node.gridPosition;
      final left = _canvasOrigin + a * gridSpacing - _nodeSize / 2;
      final top = _canvasOrigin - b * gridSpacing - _nodeSize / 2;

      final ratioKey = (node.numerator, node.denominator);
      final voicesHere = activeByRatio[ratioKey] ?? [];
      final Color? nodeColor = voicesHere.isNotEmpty
          ? workspace.waves
              .firstWhere(
                (w) => w.id == voicesHere.first.$1,
                orElse: () => workspace.waves.first,
              )
              .color
          : null;

      final isPreview = lattice.previewRatio == ratioKey;
      final is1_1 = node.numerator == 1 && node.denominator == 1;

      widgets.add(
        Positioned(
          left: left,
          top: top,
          width: _nodeSize,
          height: _nodeSize,
          child: LatticeNodeWidget(
            numerator: node.numerator,
            denominator: node.denominator,
            isActive: voicesHere.isNotEmpty,
            activeColor: nodeColor,
            voiceCount: voicesHere.length,
            isPreview: isPreview,
            is1_1: is1_1,
            referenceHz: workspace.referenceHz,
            spawnOctave: lattice.spawnOctave,
          ),
        ),
      );
    }

    return widgets;
  }

  @override
  Widget build(BuildContext context) {
    return ClipRect(
      child: LayoutBuilder(
        builder: (context, constraints) {
          final widgetSize =
              Size(constraints.maxWidth, constraints.maxHeight);

          // Center the transform on first layout. We remove the listener
          // temporarily to avoid setState-during-build.
          if (_needsCenter) {
            _needsCenter = false;
            _ctrl.removeListener(_onTransformChanged);
            _ctrl.value = Matrix4.identity()
              ..setTranslationRaw(
                -_canvasOrigin + widgetSize.width / 2,
                -_canvasOrigin + widgetSize.height / 2,
                0,
              );
            _ctrl.addListener(_onTransformChanged);
          }

          return Stack(
            children: [
              InteractiveViewer(
                transformationController: _ctrl,
                constrained: false,
                minScale: 0.3,
                maxScale: 4.0,
                boundaryMargin: const EdgeInsets.all(double.infinity),
                child: SizedBox(
                  width: _canvasSize,
                  height: _canvasSize,
                  child: Stack(
                    children: _buildNodes(widgetSize),
                  ),
                ),
              ),
              // Controls overlay — always on top, not scrolled with the canvas.
              Positioned(
                top: 0,
                left: 0,
                right: 0,
                child: LatticeControls(
                  onHome: () {
                    if (context.mounted) {
                      final size = MediaQuery.sizeOf(context);
                      _ctrl.value = Matrix4.identity()
                        ..setTranslationRaw(
                          -_canvasOrigin + size.width / 2,
                          -_canvasOrigin + size.height / 2,
                          0,
                        );
                    }
                  },
                ),
              ),
            ],
          );
        },
      ),
    );
  }
}
