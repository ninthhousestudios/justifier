/// Waveform types available in the Justifier audio engine.
///
/// Values match the C enum in justifier_audio.h — [index] maps directly
/// to the native integer value.
enum WaveformType {
  sine,
  triangle,
  saw,
  square,
  pulse,
  whiteNoise,
  pinkNoise,
  brownNoise,
  lfnoise0,
  lfnoise1,
  lfnoise2,
  fm;

  /// Whether this waveform responds to pitch (frequency/detune) controls.
  bool get isPitched => switch (this) {
    whiteNoise || pinkNoise || brownNoise => false,
    _ => true,
  };

  /// Human-readable label for the UI.
  String get label => switch (this) {
    sine => 'Sine',
    triangle => 'Triangle',
    saw => 'Saw',
    square => 'Square',
    pulse => 'Pulse',
    whiteNoise => 'White',
    pinkNoise => 'Pink',
    brownNoise => 'Brown',
    lfnoise0 => 'LFNoise0',
    lfnoise1 => 'LFNoise1',
    lfnoise2 => 'LFNoise2',
    fm => 'FM',
  };
}
