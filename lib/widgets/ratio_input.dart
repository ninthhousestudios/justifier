import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

import '../theme/app_theme.dart';

/// Two integer fields with "/" separator for JI ratio entry,
/// each with +/- nudge buttons.
class RatioInput extends StatelessWidget {
  const RatioInput({
    super.key,
    required this.numerator,
    required this.denominator,
    required this.onChanged,
    this.color,
  });

  final int numerator;
  final int denominator;
  final void Function(int numerator, int denominator) onChanged;
  final Color? color;

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        _IntField(
          value: numerator,
          onChanged: (v) => onChanged(v, denominator),
          color: color,
        ),
        Padding(
          padding: const EdgeInsets.symmetric(horizontal: 2),
          child: Text('/', style: AppTheme.monoSmall.copyWith(color: color)),
        ),
        _IntField(
          value: denominator,
          onChanged: (v) => onChanged(numerator, v),
          color: color,
        ),
      ],
    );
  }
}

class _IntField extends StatefulWidget {
  const _IntField({
    required this.value,
    required this.onChanged,
    this.color,
  });

  final int value;
  final ValueChanged<int> onChanged;
  final Color? color;

  @override
  State<_IntField> createState() => _IntFieldState();
}

class _IntFieldState extends State<_IntField> {
  late TextEditingController _controller;

  @override
  void initState() {
    super.initState();
    _controller = TextEditingController(text: widget.value.toString());
  }

  @override
  void didUpdateWidget(_IntField old) {
    super.didUpdateWidget(old);
    if (old.value != widget.value) {
      _controller.text = widget.value.toString();
    }
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  void _submit(String text) {
    final v = int.tryParse(text);
    if (v != null && v > 0) {
      widget.onChanged(v);
    } else {
      _controller.text = widget.value.toString();
    }
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisSize: MainAxisSize.min,
      children: [
        SizedBox(
          width: 18,
          height: 18,
          child: IconButton(
            onPressed: widget.value > 1
                ? () => widget.onChanged(widget.value - 1)
                : null,
            icon: const Icon(Icons.remove, size: 10),
            padding: EdgeInsets.zero,
            constraints: const BoxConstraints(minWidth: 18, minHeight: 18),
          ),
        ),
        SizedBox(
          width: 32,
          height: 24,
          child: TextField(
            controller: _controller,
            style: AppTheme.monoSmall.copyWith(color: widget.color),
            textAlign: TextAlign.center,
            keyboardType: TextInputType.number,
            inputFormatters: [FilteringTextInputFormatter.digitsOnly],
            decoration: const InputDecoration(
              isDense: true,
              contentPadding: EdgeInsets.symmetric(horizontal: 2, vertical: 4),
              border: OutlineInputBorder(),
            ),
            onSubmitted: _submit,
            onTapOutside: (_) => _submit(_controller.text),
          ),
        ),
        SizedBox(
          width: 18,
          height: 18,
          child: IconButton(
            onPressed: () => widget.onChanged(widget.value + 1),
            icon: const Icon(Icons.add, size: 10),
            padding: EdgeInsets.zero,
            constraints: const BoxConstraints(minWidth: 18, minHeight: 18),
          ),
        ),
      ],
    );
  }
}
