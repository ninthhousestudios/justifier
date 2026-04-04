import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:uuid/uuid.dart';

import '../audio/audio_engine.dart';
import '../audio/waveform_type.dart';
import '../models/workspace-state.dart';
import 'engine-provider.dart';

const _uuid = Uuid();

/// Default wave colors — cycled as waves are added.
const _waveColors = [
  Color(0xFF00FF00), // green
  Color(0xFF6B00FF), // violet
  Color(0xFF00CCFF), // cyan
  Color(0xFFFF6600), // orange
  Color(0xFFFF0066), // pink
  Color(0xFFFFFF00), // yellow
];

final workspaceProvider =
    NotifierProvider<WorkspaceNotifier, WorkspaceState>(WorkspaceNotifier.new);

class WorkspaceNotifier extends Notifier<WorkspaceState> {
  late AudioEngine _engine;

  @override
  WorkspaceState build() {
    _engine = ref.read(engineProvider);
    return const WorkspaceState();
  }

  void setReferenceHz(double hz) {
    state = state.copyWith(referenceHz: hz);
    // Push updated frequencies to all active voices.
    for (final wave in state.waves) {
      for (final voice in wave.voices) {
        if (voice.engineVoiceId != null) {
          _engine.setFrequency(voice.engineVoiceId!, voice.frequencyHz(hz));
        }
      }
    }
  }

  void setMasterVolume(double vol) {
    state = state.copyWith(masterVolume: vol);
    _engine.setMasterVolume(vol);
  }

  String addWave() {
    final id = _uuid.v4();
    final index = state.waves.length;
    final wave = Wave(
      id: id,
      name: 'Wave ${index + 1}',
      color: _waveColors[index % _waveColors.length],
    );
    state = state.copyWith(waves: [...state.waves, wave]);
    return id;
  }

  void removeWave(String waveId) {
    final wave = state.waves.firstWhere((w) => w.id == waveId);
    // Fade out all voices, then remove the wave.
    for (final voice in wave.voices) {
      if (voice.engineVoiceId != null && !voice.dying) {
        _engine.setGateTimes(voice.engineVoiceId!, release: voice.fadeTime);
        _engine.setGate(voice.engineVoiceId!, false);
      }
    }
    final maxFade = wave.voices.isEmpty
        ? 0.0
        : wave.voices.map((v) => v.fadeTime).reduce((a, b) => a > b ? a : b);
    Future.delayed(
      Duration(milliseconds: (maxFade * 1000).round() + 50),
      () {
        // Remove all engine voices and the wave from state.
        for (final voice in wave.voices) {
          if (voice.engineVoiceId != null) {
            _engine.removeVoice(voice.engineVoiceId!);
          }
        }
        state = state.copyWith(
          waves: state.waves.where((w) => w.id != waveId).toList(),
        );
      },
    );
  }

  String addVoice(String waveId, WaveformType type) {
    final voiceId = _uuid.v4();
    final hz = Voice(waveform: type, id: voiceId).frequencyHz(state.referenceHz);
    final engineId = _engine.addVoice(type, hz, 0.5);
    if (engineId >= 0) {
      _engine.setGate(engineId, true);
    }
    final voice = Voice(
      id: voiceId,
      waveform: type,
      engineVoiceId: engineId >= 0 ? engineId : null,
    );
    state = state.copyWith(
      waves: state.waves.map((w) {
        if (w.id != waveId) return w;
        return w.copyWith(voices: [...w.voices, voice]);
      }).toList(),
    );
    return voiceId;
  }

  void removeVoice(String waveId, String voiceId) {
    final wave = state.waves.firstWhere((w) => w.id == waveId);
    final voice = wave.voices.firstWhere((v) => v.id == voiceId);

    if (voice.engineVoiceId != null && !voice.dying) {
      // Phase 1: gate off with short release, mark as dying.
      final releaseTime = voice.fadeTime;
      _engine.setGateTimes(voice.engineVoiceId!, release: releaseTime);
      _engine.setGate(voice.engineVoiceId!, false);
      updateVoice(waveId, voiceId, voice.copyWith(dying: true));

      // Phase 2: after the release envelope completes, actually remove.
      Future.delayed(
        Duration(milliseconds: (releaseTime * 1000).round() + 50),
        () => _finalizeRemove(waveId, voiceId),
      );
    } else {
      // No engine voice or already dying — remove immediately.
      _finalizeRemove(waveId, voiceId);
    }
  }

  void _finalizeRemove(String waveId, String voiceId) {
    final waveIdx = state.waves.indexWhere((w) => w.id == waveId);
    if (waveIdx < 0) return; // wave already removed
    final wave = state.waves[waveIdx];
    final voice = wave.voices.where((v) => v.id == voiceId).firstOrNull;
    if (voice == null) return; // voice already removed
    if (!voice.dying) return; // undo was clicked — don't remove

    if (voice.engineVoiceId != null) {
      _engine.removeVoice(voice.engineVoiceId!);
    }
    state = state.copyWith(
      waves: state.waves.map((w) {
        if (w.id != waveId) return w;
        return w.copyWith(
          voices: w.voices.where((v) => v.id != voiceId).toList(),
        );
      }).toList(),
    );
  }

  void cancelRemoveVoice(String waveId, String voiceId) {
    final wave = state.waves.firstWhere((w) => w.id == waveId);
    final voice = wave.voices.firstWhere((v) => v.id == voiceId);
    if (!voice.dying) return;

    if (voice.engineVoiceId != null) {
      // Restore the gate — voice comes back to life.
      _engine.setGateTimes(voice.engineVoiceId!, attack: 0.05);
      _engine.setGate(voice.engineVoiceId!, true);
    }
    updateVoice(waveId, voiceId, voice.copyWith(dying: false));
  }

  void updateVoice(String waveId, String voiceId, Voice updated) {
    final wave = state.waves.firstWhere((w) => w.id == waveId);
    final old = wave.voices.firstWhere((v) => v.id == voiceId);
    final eid = old.engineVoiceId;

    if (eid != null) {
      final refHz = state.referenceHz;
      if (updated.waveform != old.waveform) {
        _engine.setWaveform(eid, updated.waveform);
      }
      if (updated.numerator != old.numerator ||
          updated.denominator != old.denominator ||
          updated.octave != old.octave) {
        _engine.setFrequency(eid, updated.frequencyHz(refHz));
      }
      if (updated.amplitude != old.amplitude) {
        _engine.setAmplitude(eid, updated.amplitude);
      }
      if (updated.pan != old.pan) {
        _engine.setPan(eid, updated.pan);
      }
      if (updated.detuneCents != old.detuneCents) {
        _engine.setDetune(eid, updated.detuneCents);
      }
      if (updated.modRatio != old.modRatio) {
        _engine.setModRatio(eid, updated.modRatio);
      }
      if (updated.modIndex != old.modIndex) {
        _engine.setModIndex(eid, updated.modIndex);
      }
    }

    state = state.copyWith(
      waves: state.waves.map((w) {
        if (w.id != waveId) return w;
        return w.copyWith(
          voices: w.voices.map((v) => v.id == voiceId ? updated : v).toList(),
        );
      }).toList(),
    );
  }

  void toggleVoiceEnabled(String waveId, String voiceId) {
    final wave = state.waves.firstWhere((w) => w.id == waveId);
    final voice = wave.voices.firstWhere((v) => v.id == voiceId);
    final newEnabled = !voice.enabled;
    if (voice.engineVoiceId != null) {
      _engine.setGate(voice.engineVoiceId!, newEnabled);
    }
    updateVoice(waveId, voiceId, voice.copyWith(enabled: newEnabled));
  }

  void panic() {
    _engine.panic();
  }
}
