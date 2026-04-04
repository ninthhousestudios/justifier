import 'package:flutter/material.dart';

/// Prometheus color scheme: black background, #00ff00 green, #6b00ff violet.
class AppTheme {
  AppTheme._();

  static const Color prometheusGreen = Color(0xFF00FF00);
  static const Color prometheusViolet = Color(0xFF6B00FF);

  static ThemeData dark() {
    final colorScheme = ColorScheme.fromSeed(
      seedColor: prometheusGreen,
      brightness: Brightness.dark,
      surface: const Color(0xFF0A0A0A),
      onSurface: prometheusGreen,
      primary: prometheusGreen,
      secondary: prometheusViolet,
      tertiary: prometheusViolet,
    );

    return ThemeData(
      useMaterial3: true,
      brightness: Brightness.dark,
      colorScheme: colorScheme,
      scaffoldBackgroundColor: Colors.black,
      visualDensity: VisualDensity.compact,
      cardTheme: CardThemeData(
        elevation: 0,
        color: const Color(0xFF111111),
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(6),
          side: BorderSide(color: prometheusGreen.withValues(alpha: 0.2)),
        ),
        margin: const EdgeInsets.all(4),
      ),
      sliderTheme: SliderThemeData(
        activeTrackColor: prometheusGreen,
        inactiveTrackColor: prometheusGreen.withValues(alpha: 0.15),
        thumbColor: prometheusGreen,
        overlayColor: prometheusGreen.withValues(alpha: 0.12),
        trackHeight: 2,
        thumbShape: const RoundSliderThumbShape(enabledThumbRadius: 6),
      ),
      inputDecorationTheme: InputDecorationTheme(
        isDense: true,
        contentPadding: const EdgeInsets.symmetric(horizontal: 8, vertical: 6),
        border: OutlineInputBorder(
          borderRadius: BorderRadius.circular(4),
          borderSide: BorderSide(color: prometheusGreen.withValues(alpha: 0.3)),
        ),
        enabledBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(4),
          borderSide: BorderSide(color: prometheusGreen.withValues(alpha: 0.3)),
        ),
        focusedBorder: OutlineInputBorder(
          borderRadius: BorderRadius.circular(4),
          borderSide: const BorderSide(color: prometheusGreen),
        ),
      ),
      iconTheme: const IconThemeData(color: prometheusGreen, size: 18),
      fontFamily: 'Source Code Pro',
      textTheme: const TextTheme(
        bodyMedium: TextStyle(color: prometheusGreen, fontWeight: FontWeight.w600),
        bodySmall: TextStyle(color: prometheusGreen, fontWeight: FontWeight.w600),
        labelMedium: TextStyle(color: prometheusGreen, fontWeight: FontWeight.w600),
      ),
      dividerColor: prometheusGreen.withValues(alpha: 0.15),
    );
  }

  static const String _fontFamily = 'Source Code Pro';
  static const FontWeight _fontWeight = FontWeight.w600;

  /// Monospace style for numeric values.
  static const TextStyle mono = TextStyle(
    fontFamily: _fontFamily,
    fontWeight: _fontWeight,
    fontSize: 16,
    color: prometheusGreen,
  );

  /// Monospace style for secondary numeric values (violet accent).
  static const TextStyle monoSecondary = TextStyle(
    fontFamily: _fontFamily,
    fontWeight: _fontWeight,
    fontSize: 16,
    color: prometheusViolet,
  );

  /// Smaller mono style for labels and status text.
  static const TextStyle monoSmall = TextStyle(
    fontFamily: _fontFamily,
    fontWeight: _fontWeight,
    fontSize: 12,
    color: prometheusGreen,
  );
}
