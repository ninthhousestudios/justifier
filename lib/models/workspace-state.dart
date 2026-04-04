import 'dart:math';

import 'package:flutter/material.dart';

import '../audio/waveform_type.dart';

/// Immutable workspace state tree.
class WorkspaceState {
  const WorkspaceState({
    this.referenceHz = 172.8,
    this.masterVolume = 0.7,
    this.waves = const [],
  });

  final double referenceHz;
  final double masterVolume;
  final List<Wave> waves;

  WorkspaceState copyWith({
    double? referenceHz,
    double? masterVolume,
    List<Wave>? waves,
  }) {
    return WorkspaceState(
      referenceHz: referenceHz ?? this.referenceHz,
      masterVolume: masterVolume ?? this.masterVolume,
      waves: waves ?? this.waves,
    );
  }
}

/// A wave column containing multiple voices.
class Wave {
  const Wave({
    required this.id,
    required this.name,
    required this.color,
    this.masterVolume = 1.0,
    this.muted = false,
    this.soloed = false,
    this.collapsed = false,
    this.voices = const [],
  });

  final String id;
  final String name;
  final Color color;
  final double masterVolume;
  final bool muted;
  final bool soloed;
  final bool collapsed;
  final List<Voice> voices;

  Wave copyWith({
    String? name,
    Color? color,
    double? masterVolume,
    bool? muted,
    bool? soloed,
    bool? collapsed,
    List<Voice>? voices,
  }) {
    return Wave(
      id: id,
      name: name ?? this.name,
      color: color ?? this.color,
      masterVolume: masterVolume ?? this.masterVolume,
      muted: muted ?? this.muted,
      soloed: soloed ?? this.soloed,
      collapsed: collapsed ?? this.collapsed,
      voices: voices ?? this.voices,
    );
  }
}

/// A single voice (oscillator) within a wave.
class Voice {
  const Voice({
    required this.id,
    this.engineVoiceId,
    this.waveform = WaveformType.sine,
    this.numerator = 1,
    this.denominator = 1,
    this.octave = 1,
    this.amplitude = 0.5,
    this.pan = 0.0,
    this.detuneCents = 0.0,
    this.fadeTime = 8.0,
    this.enabled = true,
    this.dying = false,
    this.modRatio = 1.0,
    this.modIndex = 0.0,
  });

  final String id;
  final int? engineVoiceId;
  final WaveformType waveform;
  final int numerator;
  final int denominator;
  final int octave;
  final double amplitude;
  final double pan;
  final double detuneCents;
  final double fadeTime;
  final bool enabled;
  final bool dying;
  final double modRatio;
  final double modIndex;

  /// Compute the actual frequency from a reference Hz.
  double frequencyHz(double referenceHz) {
    return referenceHz * (numerator / denominator) * pow(2, octave);
  }

  /// Cents offset from the reference (ignoring octave).
  double get centsFromReference {
    if (numerator == denominator) return octave * 1200.0;
    return 1200.0 * log(numerator / denominator) / ln2 + octave * 1200.0;
  }

  /// Interval name for display (e.g. "3/2" or "1/1").
  String get ratioLabel => '$numerator/$denominator';

  Voice copyWith({
    int? engineVoiceId,
    WaveformType? waveform,
    int? numerator,
    int? denominator,
    int? octave,
    double? amplitude,
    double? pan,
    double? detuneCents,
    double? fadeTime,
    bool? enabled,
    bool? dying,
    double? modRatio,
    double? modIndex,
  }) {
    return Voice(
      id: id,
      engineVoiceId: engineVoiceId ?? this.engineVoiceId,
      waveform: waveform ?? this.waveform,
      numerator: numerator ?? this.numerator,
      denominator: denominator ?? this.denominator,
      octave: octave ?? this.octave,
      amplitude: amplitude ?? this.amplitude,
      pan: pan ?? this.pan,
      detuneCents: detuneCents ?? this.detuneCents,
      fadeTime: fadeTime ?? this.fadeTime,
      enabled: enabled ?? this.enabled,
      dying: dying ?? this.dying,
      modRatio: modRatio ?? this.modRatio,
      modIndex: modIndex ?? this.modIndex,
    );
  }
}
