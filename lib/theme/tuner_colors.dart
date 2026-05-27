import 'package:flutter/material.dart';

class TunerColors extends ThemeExtension<TunerColors> {
  const TunerColors({
    required this.inTune,
    required this.close,
    required this.off,
    required this.guideLine,
  });

  final Color inTune;
  final Color close;
  final Color off;
  final Color guideLine;

  @override
  TunerColors copyWith({
    Color? inTune,
    Color? close,
    Color? off,
    Color? guideLine,
  }) {
    return TunerColors(
      inTune: inTune ?? this.inTune,
      close: close ?? this.close,
      off: off ?? this.off,
      guideLine: guideLine ?? this.guideLine,
    );
  }

  @override
  TunerColors lerp(TunerColors? other, double t) {
    if (other is! TunerColors) return this;
    return TunerColors(
      inTune: Color.lerp(inTune, other.inTune, t)!,
      close: Color.lerp(close, other.close, t)!,
      off: Color.lerp(off, other.off, t)!,
      guideLine: Color.lerp(guideLine, other.guideLine, t)!,
    );
  }
}
