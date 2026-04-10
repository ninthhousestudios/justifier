import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import '../../models/lattice_state.dart';
import '../../models/workspace_state.dart';
import '../../providers/lattice_provider.dart';
import '../../providers/workspace_provider.dart';
import 'lattice_canvas.dart';
import 'lattice_controls.dart';
import 'lattice_node_widget.dart';

/// The main lattice explorer panel. Shows a bounded grid of JI ratios
/// (±[latticeRange] steps on each axis) with pan/zoom via [InteractiveViewer].
class LatticePanel extends ConsumerStatefulWidget {
  const LatticePanel({super.key});

  @override
  ConsumerState<LatticePanel> createState() => _LatticePanelState();
}

class _LatticePanelState extends ConsumerState<LatticePanel> {
  /// How many steps in each direction from origin.
  static const int latticeRange = 4;
  static const double gridSpacing = 90.0;
  static const double _nodeSize = 56.0;
  static const double _padding = 60.0;

  /// Canvas size derived from the fixed range.
  static const double _canvasSize =
      (latticeRange * 2 + 1) * gridSpacing + _padding * 2;

  /// Origin pixel position within the canvas.
  static const double _originX = _canvasSize / 2;
  static const double _originY = _canvasSize / 2;

  late final TransformationController _ctrl;
  bool _needsCenter = true;

  @override
  void initState() {
    super.initState();
    _ctrl = TransformationController();
    _ctrl.addListener(_onTransformChanged);
  }

  @override
  void dispose() {
    _ctrl.removeListener(_onTransformChanged);
    _ctrl.dispose();
    super.dispose();
  }

  void _onTransformChanged() {
    setState(() {});
  }

  void _centerView(Size widgetSize) {
    _ctrl.removeListener(_onTransformChanged);
    _ctrl.value = Matrix4.identity()
      ..setTranslationRaw(
        -_originX + widgetSize.width / 2,
        -_originY + widgetSize.height / 2,
        0,
      );
    _ctrl.addListener(_onTransformChanged);
  }

  List<Widget> _buildNodes() {
    final lattice = ref.watch(latticeProvider);
    final workspace = ref.watch(workspaceProvider);

    // Active voice lookup: (numerator, denominator) → list of (waveId, Voice).
    final Map<(int, int), List<(String, Voice)>> activeByRatio = {};
    for (final wave in workspace.waves) {
      for (final voice in wave.voices) {
        if (!voice.dying) {
          final key = (voice.numerator, voice.denominator);
          activeByRatio.putIfAbsent(key, () => []).add((wave.id, voice));
        }
      }
    }

    final mode = lattice.viewMode;
    // Generate nodes for the fixed range.
    final viewport = Rect.fromLTRB(
      -latticeRange.toDouble(),
      -latticeRange.toDouble(),
      latticeRange.toDouble(),
      latticeRange.toDouble(),
    );
    final visibleNodes = generateVisibleNodes(mode, viewport);

    // Collect nodes + higher-prime expansions.
    final allNodes = <LatticeNode>[];
    for (final node in visibleNodes) {
      allNodes.add(node);
      if (lattice.expandedNodes.contains((node.numerator, node.denominator))) {
        allNodes.addAll(getHigherPrimeNeighbors(node, [7, 11, 13]));
      }
    }

    // Deduplicate by grid position. Base nodes take priority over
    // higher-prime neighbors (which can round to the same grid cell).
    final Map<(int, int), LatticeNode> byGridPos = {};
    for (final n in allNodes) {
      byGridPos.putIfAbsent(n.gridPosition, () => n);
    }

    // Build connection list for active nodes.
    final activeGridPositions = <(int, int)>{};
    for (final node in byGridPos.values) {
      if (activeByRatio.containsKey((node.numerator, node.denominator))) {
        activeGridPositions.add(node.gridPosition);
      }
    }

    final connections = <((int, int), (int, int))>[];
    final activeList = activeGridPositions.toList();
    for (var i = 0; i < activeList.length; i++) {
      for (var j = i + 1; j < activeList.length; j++) {
        final (ai, bi) = activeList[i];
        final (aj, bj) = activeList[j];
        if ((ai - aj).abs() + (bi - bj).abs() == 1) {
          connections.add((activeList[i], activeList[j]));
        }
      }
    }

    // Active node color map for canvas painter.
    final Map<(int, int), Color> activeNodeColors = {};
    for (final entry in activeByRatio.entries) {
      final waveId = entry.value.first.$1;
      final wave = workspace.waves.firstWhere(
        (w) => w.id == waveId,
        orElse: () => workspace.waves.first,
      );
      activeNodeColors[entry.key] = wave.color;
    }

    final widgets = <Widget>[];

    // Canvas painter (grid dots, crosshair, connections).
    widgets.add(
      CustomPaint(
        painter: LatticeCanvasPainter(
          activeNodes: activeNodeColors,
          connections: connections,
          gridSpacing: gridSpacing,
          canvasCenter: const Offset(_originX, _originY),
          range: latticeRange,
        ),
        size: const Size(_canvasSize, _canvasSize),
      ),
    );

    // Node widgets.
    for (final node in byGridPos.values) {
      final (a, b) = node.gridPosition;
      final left = _originX + a * gridSpacing - _nodeSize / 2;
      final top = _originY - b * gridSpacing - _nodeSize / 2;

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

          if (_needsCenter) {
            _needsCenter = false;
            _centerView(widgetSize);
          }

          return Stack(
            children: [
              Center(
                child: InteractiveViewer(
                  transformationController: _ctrl,
                  constrained: false,
                  minScale: 0.4,
                  maxScale: 3.0,
                  boundaryMargin: const EdgeInsets.all(100),
                  child: SizedBox(
                    width: _canvasSize,
                    height: _canvasSize,
                    child: Stack(
                      children: _buildNodes(),
                    ),
                  ),
                ),
              ),
              // Controls overlay.
              Positioned(
                top: 0,
                left: 0,
                right: 0,
                child: LatticeControls(
                  onHome: () {
                    if (context.mounted) {
                      _centerView(widgetSize);
                      setState(() {});
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
