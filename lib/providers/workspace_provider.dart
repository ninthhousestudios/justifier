import 'package:flutter/material.dart';
import 'package:flutter_riverpod/flutter_riverpod.dart';
import 'package:uuid/uuid.dart';

import '../audio/audio_engine.dart';
import '../audio/waveform_type.dart';
import '../models/workspace_state.dart';
import 'engine_provider.dart';

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
    for (final voice in wave.voices) {
      if (voice.engineVoiceId != null && !voice.dying) {
        _engine.removeVoice(voice.engineVoiceId!);
      }
    }
    final maxRelease = wave.voices.isEmpty
        ? 0.0
        : wave.voices.map((v) => v.releaseTime).reduce((a, b) => a > b ? a : b);
    Future.delayed(
      Duration(milliseconds: (maxRelease * 1000).round() + 1000),
      () {
        state = state.copyWith(
          waves: state.waves.where((w) => w.id != waveId).toList(),
        );
      },
    );
  }

  String addVoice(String waveId, WaveformType type) {
    final voiceId = _uuid.v4();
    final defaults = Voice(waveform: type, id: voiceId);
    final hz = defaults.frequencyHz(state.referenceHz);
    final engineId = _engine.addVoice(type, hz, 0.5);
    if (engineId >= 0) {
      _engine.setGateTimes(engineId,
          attack: defaults.attackTime,
          decay: defaults.decayTime,
          sustain: defaults.sustainLevel,
          release: defaults.releaseTime);
      _engine.setGate(engineId, true);
      _engine.setReverbSend(engineId, defaults.reverbSend);
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
      // C engine owns lifecycle: MSG_VOICE_REMOVE sets gate=0 and
      // enters VOICE_RELEASING with timeout = release_time + 0.5s.
      _engine.removeVoice(voice.engineVoiceId!);
      updateVoice(waveId, voiceId, voice.copyWith(dying: true));

      // Dart cleanup timer: remove from workspace state after release.
      // Slightly longer than C timeout to ensure DSP is freed first.
      final releaseTime = voice.releaseTime;
      Future.delayed(
        Duration(milliseconds: (releaseTime * 1000).round() + 1000),
        () => _finalizeRemove(waveId, voiceId),
      );
    } else {
      _finalizeRemove(waveId, voiceId);
    }
  }

  void _finalizeRemove(String waveId, String voiceId) {
    final waveIdx = state.waves.indexWhere((w) => w.id == waveId);
    if (waveIdx < 0) return;
    final wave = state.waves[waveIdx];
    final voice = wave.voices.where((v) => v.id == voiceId).firstOrNull;
    if (voice == null) return;
    if (!voice.dying) return; // undo was clicked

    // Just remove from Dart state. C engine already freed the DSP.
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
      // Voice is in VOICE_RELEASING — re-gate returns it to VOICE_ACTIVE.
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
      if (updated.filterType != old.filterType) {
        _engine.setFilterType(eid, updated.filterType);
      }
      if (updated.filterCutoff != old.filterCutoff) {
        _engine.setFilterCutoff(eid, updated.filterCutoff);
      }
      if (updated.filterResonance != old.filterResonance) {
        _engine.setFilterResonance(eid, updated.filterResonance);
      }
      if (updated.reverbSend != old.reverbSend) {
        _engine.setReverbSend(eid, updated.reverbSend);
      }
      if (updated.attackTime != old.attackTime ||
          updated.decayTime != old.decayTime ||
          updated.sustainLevel != old.sustainLevel ||
          updated.releaseTime != old.releaseTime) {
        _engine.setGateTimes(eid,
            attack: updated.attackTime,
            decay: updated.decayTime,
            sustain: updated.sustainLevel,
            release: updated.releaseTime);
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

  void setFilterType(String waveId, String voiceId, int type) {
    final wave = state.waves.firstWhere((w) => w.id == waveId);
    final voice = wave.voices.firstWhere((v) => v.id == voiceId);
    updateVoice(waveId, voiceId, voice.copyWith(filterType: type));
  }

  void setFilterCutoff(String waveId, String voiceId, double hz) {
    final wave = state.waves.firstWhere((w) => w.id == waveId);
    final voice = wave.voices.firstWhere((v) => v.id == voiceId);
    updateVoice(waveId, voiceId, voice.copyWith(filterCutoff: hz));
  }

  void setFilterResonance(String waveId, String voiceId, double resonance) {
    final wave = state.waves.firstWhere((w) => w.id == waveId);
    final voice = wave.voices.firstWhere((v) => v.id == voiceId);
    updateVoice(waveId, voiceId, voice.copyWith(filterResonance: resonance));
  }

  void setReverbReturn(double level) {
    _engine.setReverbReturn(level);
    state = state.copyWith(reverbReturn: level);
  }

  void panic() {
    _engine.panic();
  }
}
