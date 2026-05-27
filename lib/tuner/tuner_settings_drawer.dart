import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';

import 'ratio.dart';
import 'tuner_settings_state.dart';

class TunerSettingsDrawer extends ConsumerStatefulWidget {
  const TunerSettingsDrawer({super.key});

  @override
  ConsumerState<TunerSettingsDrawer> createState() =>
      _TunerSettingsDrawerState();
}

class _TunerSettingsDrawerState extends ConsumerState<TunerSettingsDrawer> {
  bool _open = false;
  bool _advanced = false;

  @override
  Widget build(BuildContext context) {
    final settings = ref.watch(tunerSettingsProvider);
    final notifier = ref.read(tunerSettingsProvider.notifier);
    final theme = Theme.of(context);

    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        _Handle(
          open: _open,
          onTap: () => setState(() {
            _open = !_open;
            if (!_open) _advanced = false;
          }),
        ),
        if (_open)
          Flexible(
            child: SingleChildScrollView(
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  _Toggles(settings: settings, notifier: notifier),
                  _Presets(settings: settings, notifier: notifier),
                  const SizedBox(height: 8),
                  _AdvancedHandle(
                    open: _advanced,
                    onTap: () => setState(() => _advanced = !_advanced),
                  ),
                  if (_advanced)
                    Padding(
                      padding: const EdgeInsets.fromLTRB(16, 0, 16, 12),
                      child: _AdvancedRatios(
                          settings: settings, notifier: notifier),
                    ),
                ],
              ),
            ),
          ),
        Divider(height: 1, color: theme.colorScheme.surfaceContainerHighest),
      ],
    );
  }
}

// ---------------------------------------------------------------------------
// Handle
// ---------------------------------------------------------------------------

class _Handle extends StatelessWidget {
  const _Handle({required this.open, required this.onTap});
  final bool open;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return InkWell(
      onTap: onTap,
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 10),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          mainAxisSize: MainAxisSize.min,
          children: [
            Text(
              'Settings',
              style: theme.textTheme.labelMedium?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
            ),
            const SizedBox(width: 4),
            Icon(
              open ? Icons.expand_less : Icons.expand_more,
              size: 18,
              color: theme.colorScheme.onSurfaceVariant,
            ),
          ],
        ),
      ),
    );
  }
}

// ---------------------------------------------------------------------------
// Toggle switches
// ---------------------------------------------------------------------------

class _Toggles extends StatelessWidget {
  const _Toggles({required this.settings, required this.notifier});
  final TunerSettings settings;
  final TunerSettingsNotifier notifier;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final labelStyle = theme.textTheme.bodySmall?.copyWith(
      color: theme.colorScheme.onSurfaceVariant,
    );

    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 16),
      child: Column(
        children: [
          _toggleRow('Octave reduction', settings.octaveReduction,
              notifier.setOctaveReduction, labelStyle),
          _toggleRow('Show cents', settings.showCents, notifier.setShowCents,
              labelStyle),
          _toggleRow(
              'Show Hz', settings.showHz, notifier.setShowHz, labelStyle),
        ],
      ),
    );
  }

  Widget _toggleRow(
      String label, bool value, ValueChanged<bool> onChanged, TextStyle? style) {
    return SizedBox(
      height: 36,
      child: Row(
        children: [
          Text(label, style: style),
          const Spacer(),
          SizedBox(
            height: 28,
            child: FittedBox(
              child: Switch(value: value, onChanged: onChanged),
            ),
          ),
        ],
      ),
    );
  }
}

// ---------------------------------------------------------------------------
// Preset buttons
// ---------------------------------------------------------------------------

class _Presets extends StatelessWidget {
  const _Presets({required this.settings, required this.notifier});
  final TunerSettings settings;
  final TunerSettingsNotifier notifier;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
      child: Wrap(
        spacing: 6,
        runSpacing: 4,
        crossAxisAlignment: WrapCrossAlignment.center,
        children: [
          Text(
            'Presets',
            style: theme.textTheme.bodySmall?.copyWith(
              color: theme.colorScheme.onSurfaceVariant,
            ),
          ),
          const SizedBox(width: 6),
          _presetChip(context, '3', preset3Limit),
          _presetChip(context, '5', preset5Limit),
          _presetChip(context, '7', preset7Limit),
        ],
      ),
    );
  }

  Widget _presetChip(
      BuildContext context, String label, List<JiRatio> preset) {
    final theme = Theme.of(context);
    final builtInSelected = settings.selectedRatios
        .where((r) => !settings.isCustom(r))
        .toList();
    final isActive = _sameSet(builtInSelected, preset);

    return ActionChip(
      label: Text('$label-limit',
          style: TextStyle(
            fontSize: 11,
            color: isActive
                ? theme.colorScheme.primary
                : theme.colorScheme.onSurfaceVariant,
          )),
      side: BorderSide(
        color: isActive
            ? theme.colorScheme.primary
            : theme.colorScheme.surfaceContainerHighest,
      ),
      visualDensity: VisualDensity.compact,
      onPressed: () => notifier.applyPreset(preset),
    );
  }

  static bool _sameSet(List<JiRatio> a, List<JiRatio> b) {
    if (a.length != b.length) return false;
    final setA = a.toSet();
    return b.every(setA.contains);
  }
}

// ---------------------------------------------------------------------------
// Advanced toggle handle
// ---------------------------------------------------------------------------

class _AdvancedHandle extends StatelessWidget {
  const _AdvancedHandle({required this.open, required this.onTap});
  final bool open;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return InkWell(
      onTap: onTap,
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 6),
        child: Row(
          children: [
            Text(
              'Advanced',
              style: theme.textTheme.labelSmall?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
            ),
            const SizedBox(width: 4),
            Icon(
              open ? Icons.expand_less : Icons.expand_more,
              size: 16,
              color: theme.colorScheme.onSurfaceVariant,
            ),
          ],
        ),
      ),
    );
  }
}

// ---------------------------------------------------------------------------
// Advanced ratio picker
// ---------------------------------------------------------------------------

class _AdvancedRatios extends StatelessWidget {
  const _AdvancedRatios({required this.settings, required this.notifier});
  final TunerSettings settings;
  final TunerSettingsNotifier notifier;

  @override
  Widget build(BuildContext context) {
    final groups = <int, List<JiRatio>>{};
    for (final r in allBuiltInRatios) {
      (groups[r.primeLimit] ??= []).add(r);
    }

    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        for (final limit in [3, 5, 7])
          if (groups[limit] != null)
            _RatioGroup(
              label: '$limit-limit',
              ratios: groups[limit]!,
              settings: settings,
              notifier: notifier,
              showGroupToggle: true,
            ),
        if (settings.customRatios.isNotEmpty)
          _RatioGroup(
            label: 'Custom',
            ratios: settings.customRatios,
            settings: settings,
            notifier: notifier,
            showDelete: true,
          ),
        const SizedBox(height: 8),
        _AddCustomRatio(notifier: notifier),
      ],
    );
  }
}

class _RatioGroup extends StatelessWidget {
  const _RatioGroup({
    required this.label,
    required this.ratios,
    required this.settings,
    required this.notifier,
    this.showDelete = false,
    this.showGroupToggle = false,
  });

  final String label;
  final List<JiRatio> ratios;
  final TunerSettings settings;
  final TunerSettingsNotifier notifier;
  final bool showDelete;
  final bool showGroupToggle;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final allSelected = ratios.every(settings.isSelected);
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        const SizedBox(height: 8),
        Row(
          children: [
            Text(
              label,
              style: theme.textTheme.labelSmall?.copyWith(
                color: theme.colorScheme.onSurfaceVariant,
              ),
            ),
            if (showGroupToggle) ...[
              const SizedBox(width: 4),
              SizedBox(
                height: 20,
                width: 20,
                child: Checkbox(
                  value: allSelected
                      ? true
                      : ratios.any(settings.isSelected)
                          ? null
                          : false,
                  tristate: true,
                  onChanged: (_) =>
                      notifier.setGroupSelected(ratios, !allSelected),
                  visualDensity: VisualDensity.compact,
                  materialTapTargetSize: MaterialTapTargetSize.shrinkWrap,
                  side: BorderSide(
                    color: theme.colorScheme.onSurfaceVariant,
                    width: 1.5,
                  ),
                ),
              ),
            ],
          ],
        ),
        const SizedBox(height: 4),
        Wrap(
          spacing: 6,
          runSpacing: 4,
          children: [
            for (final r in ratios)
              _RatioChip(
                ratio: r,
                selected: settings.isSelected(r),
                onToggle: () => notifier.toggleRatio(r),
                onDelete: showDelete ? () => notifier.removeCustomRatio(r) : null,
              ),
          ],
        ),
      ],
    );
  }
}

class _RatioChip extends StatelessWidget {
  const _RatioChip({
    required this.ratio,
    required this.selected,
    required this.onToggle,
    this.onDelete,
  });

  final JiRatio ratio;
  final bool selected;
  final VoidCallback onToggle;
  final VoidCallback? onDelete;

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    return GestureDetector(
      onLongPress: onDelete,
      child: FilterChip(
        label: Text(
          ratio.label,
          style: TextStyle(
            fontSize: 11,
            color: selected
                ? theme.colorScheme.primary
                : theme.colorScheme.onSurfaceVariant,
          ),
        ),
        tooltip: ratio.name,
        selected: selected,
        selectedColor: theme.colorScheme.surface,
        checkmarkColor: theme.colorScheme.primary,
        side: BorderSide(
          color: selected
              ? theme.colorScheme.primary
              : theme.colorScheme.surfaceContainerHighest,
        ),
        onSelected: (_) => onToggle(),
        visualDensity: VisualDensity.compact,
        materialTapTargetSize: MaterialTapTargetSize.shrinkWrap,
      ),
    );
  }
}

// ---------------------------------------------------------------------------
// Add custom ratio
// ---------------------------------------------------------------------------

class _AddCustomRatio extends StatefulWidget {
  const _AddCustomRatio({required this.notifier});
  final TunerSettingsNotifier notifier;

  @override
  State<_AddCustomRatio> createState() => _AddCustomRatioState();
}

class _AddCustomRatioState extends State<_AddCustomRatio> {
  final _numCtrl = TextEditingController();
  final _denCtrl = TextEditingController();
  final _nameCtrl = TextEditingController();

  @override
  void dispose() {
    _numCtrl.dispose();
    _denCtrl.dispose();
    _nameCtrl.dispose();
    super.dispose();
  }

  void _submit() {
    final n = int.tryParse(_numCtrl.text);
    final d = int.tryParse(_denCtrl.text);
    if (n == null || d == null || n <= 0 || d <= 0) return;
    final name = _nameCtrl.text.trim().isEmpty ? null : _nameCtrl.text.trim();
    widget.notifier.addCustomRatio(n, d, name);
    _numCtrl.clear();
    _denCtrl.clear();
    _nameCtrl.clear();
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final inputStyle = theme.textTheme.bodySmall;
    const inputDecoration = InputDecoration(
      border: OutlineInputBorder(),
      isDense: true,
      contentPadding: EdgeInsets.symmetric(horizontal: 8, vertical: 8),
    );

    return Row(
      children: [
        SizedBox(
          width: 48,
          child: TextField(
            controller: _numCtrl,
            style: inputStyle,
            decoration: inputDecoration.copyWith(hintText: 'n'),
            keyboardType: TextInputType.number,
            inputFormatters: [FilteringTextInputFormatter.digitsOnly],
            onSubmitted: (_) => _submit(),
          ),
        ),
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 4),
          child: Text('/',
              style: theme.textTheme.bodySmall
                  ?.copyWith(color: theme.colorScheme.onSurfaceVariant)),
        ),
        SizedBox(
          width: 48,
          child: TextField(
            controller: _denCtrl,
            style: inputStyle,
            decoration: inputDecoration.copyWith(hintText: 'd'),
            keyboardType: TextInputType.number,
            inputFormatters: [FilteringTextInputFormatter.digitsOnly],
            onSubmitted: (_) => _submit(),
          ),
        ),
        const SizedBox(width: 8),
        Expanded(
          child: TextField(
            controller: _nameCtrl,
            style: inputStyle,
            decoration: inputDecoration.copyWith(hintText: 'name (optional)'),
            onSubmitted: (_) => _submit(),
          ),
        ),
        const SizedBox(width: 4),
        IconButton(
          icon: const Icon(Icons.add_circle_outline, size: 20),
          color: theme.colorScheme.primary,
          visualDensity: VisualDensity.compact,
          onPressed: _submit,
        ),
      ],
    );
  }
}
