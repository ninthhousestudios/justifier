import 'package:flutter/material.dart';

class AppTheme {
  static const _amberGold = Color(0xFFD4A017);
  static const _violet = Color(0xFF8B5CF6);
  static const _black = Color(0xFF000000);
  static const _surface = Color(0xFF0A0A0A);
  static const _surfaceContainer = Color(0xFF141414);
  static const _onSurface = Color(0xFFE0E0E0);
  static const _onSurfaceDim = Color(0xFF808080);

  static ThemeData justifier() {
    return ThemeData(
      useMaterial3: true,
      brightness: Brightness.dark,
      scaffoldBackgroundColor: _black,
      colorScheme: const ColorScheme.dark(
        primary: _amberGold,
        secondary: _violet,
        surface: _surface,
        surfaceContainerHighest: _surfaceContainer,
        onSurface: _onSurface,
        onSurfaceVariant: _onSurfaceDim,
      ),
      navigationBarTheme: NavigationBarThemeData(
        backgroundColor: _surface,
        indicatorColor: _amberGold.withAlpha(40),
        iconTheme: WidgetStateProperty.resolveWith((states) {
          if (states.contains(WidgetState.selected)) {
            return const IconThemeData(color: _amberGold);
          }
          return const IconThemeData(color: _onSurfaceDim);
        }),
        labelTextStyle: WidgetStateProperty.resolveWith((states) {
          if (states.contains(WidgetState.selected)) {
            return const TextStyle(
              color: _amberGold,
              fontSize: 12,
              fontWeight: FontWeight.w600,
            );
          }
          return const TextStyle(color: _onSurfaceDim, fontSize: 12);
        }),
      ),
      fontFamily: 'Source Code Pro',
      appBarTheme: const AppBarTheme(
        backgroundColor: _black,
        foregroundColor: _onSurface,
        elevation: 0,
      ),
    );
  }
}
