#!/usr/bin/env python3
"""render_ensemble.py — render a 4-string JI tanpura ensemble.

Renders each string of a classic Pa–Sa–Sa–Sa tanpura with render.sh, then
mixes them with per-string time staggers so the strings sound one after the
other (the tanpura's rolling cycle) rather than all at once.

  python3 render_ensemble.py <out.wav> [secs]

Tuning (just intonation, relative to the low Sa = C2 = 65.405 Hz):
  Pa  = 3/2 * Sa_low  (98.11 Hz, the fifth)
  Sa  = 2/1 * Sa_low  (130.81 Hz = C3, the single-voice reference)
  Sa  = 2/1 * Sa_low  (130.81 Hz)
  Sa_low = 1/1        (65.405 Hz)

v3.1: plucking halved (pluck_rate 0.417 -> 0.208) and staggers doubled
([0,0.6,1.2,1.8] -> [0,1.2,2.4,3.6]) so the gap between strings is 2x.
"""
import os, subprocess, sys, tempfile, wave
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
RENDER = os.path.join(HERE, "render.sh")
SR = 48000

SA = 130.81           # C3 — matches the single-voice render
SA_LOW = SA / 2.0     # C2 = 65.405

# (label, frequency Hz, pluck stagger seconds)
STRINGS = [
    ("Pa",     SA_LOW * 3.0 / 2.0, 0.0),
    ("Sa",     SA,                 1.2),
    ("Sa",     SA,                 2.4),
    ("Sa_low", SA_LOW,             3.6),
]

PLUCK_RATE = 0.208    # plucks/sec per string (half of v3's 0.417)


def render_one(freq, secs):
    fd, path = tempfile.mkstemp(suffix=".wav")
    os.close(fd)
    subprocess.run(
        [RENDER, "tanpura", path, str(secs),
         f"freq={freq:.4f}", f"pluck_rate={PLUCK_RATE}"],
        check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
    )
    w = wave.open(path, "rb")
    data = np.frombuffer(w.readframes(w.getnframes()), dtype=np.int16).astype(np.float64) / 32768.0
    w.close()
    os.remove(path)
    return data


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "renders/tanpura_v3_ensemble.wav")
    secs = float(sys.argv[2]) if len(sys.argv) > 2 else 24.0
    total = int(secs * SR)

    mix = np.zeros(total)
    for label, freq, stagger in STRINGS:
        sys.stderr.write(f"rendering {label:7s} {freq:8.3f} Hz  stagger {stagger}s\n")
        sig = render_one(freq, secs)
        off = int(stagger * SR)
        n = min(len(sig), total - off)
        if n > 0:
            mix[off:off + n] += sig[:n]

    peak = np.max(np.abs(mix))
    if peak > 0:
        mix *= 0.9 / peak  # normalize to -0.9 dBFS headroom
    bad = np.sum(~np.isfinite(mix))
    sys.stderr.write(f"mix: peak(pre-norm) {peak:.4f}  nan/inf {bad}  dur {secs}s\n")

    pcm = np.clip(mix, -1.0, 1.0)
    pcm = (pcm * 32767.0).astype(np.int16)
    w = wave.open(out, "wb")
    w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR)
    w.writeframes(pcm.tobytes())
    w.close()
    sys.stderr.write(f"wrote {out}: {total} frames, 1 ch, {SR} Hz\n")


if __name__ == "__main__":
    main()
