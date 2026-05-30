#!/usr/bin/env python3
"""analyze.py — inspect a rendered WAV's harmonic content.

  python3 analyze.py file.wav [--f0 HZ] [--at SECONDS]

Prints fundamental estimate, harmonic series amplitudes (dB), spectral
centroid, and an ASCII bar spectrum. Lets us "see" a drone timbre without
listening: rich = many strong harmonics + high centroid; thin = few.
"""
import sys, wave, struct
import numpy as np

def read_wav(path):
    w = wave.open(path, 'rb')
    ch, sw, sr, n = w.getnchannels(), w.getsampwidth(), w.getframerate(), w.getnframes()
    raw = w.readframes(n)
    w.close()
    data = np.frombuffer(raw, dtype=np.int16).astype(np.float64) / 32768.0
    if ch > 1:
        data = data.reshape(-1, ch).mean(axis=1)
    return data, sr

def main():
    args = sys.argv[1:]
    path = args[0]
    f0 = None
    at = None
    for i, a in enumerate(args):
        if a == '--f0': f0 = float(args[i+1])
        if a == '--at': at = float(args[i+1])
    data, sr = read_wav(path)
    dur = len(data) / sr

    # Analysis window: 1s (or available), centered at --at or 60% through.
    win = int(min(sr, len(data)))
    center = int((at if at is not None else dur * 0.6) * sr)
    start = max(0, min(center - win // 2, len(data) - win))
    seg = data[start:start + win] * np.hanning(win)
    spec = np.abs(np.fft.rfft(seg))
    freqs = np.fft.rfftfreq(win, 1 / sr)

    # Fundamental: use given f0 or pick the strongest peak below 1kHz.
    if f0 is None:
        lo = (freqs > 30) & (freqs < 1000)
        f0 = freqs[lo][np.argmax(spec[lo])]

    rms = np.sqrt(np.mean(data**2))
    peak = np.max(np.abs(data))
    centroid = np.sum(freqs * spec) / (np.sum(spec) + 1e-12)

    # Harmonic amplitudes (nearest bin to each multiple of f0).
    print(f"file: {path}")
    print(f"dur {dur:.2f}s  sr {sr}  rms {rms:.4f}  peak {peak:.4f}  "
          f"centroid {centroid:.0f} Hz  f0 {f0:.2f} Hz")
    print("harmonics (dB rel. strongest):")
    h_amps = []
    for h in range(1, 21):
        fh = f0 * h
        if fh > sr / 2: break
        bin_i = int(round(fh / (sr / win)))
        a = spec[max(0, bin_i-1):bin_i+2].max()
        h_amps.append((h, fh, a))
    mx = max(a for _, _, a in h_amps) + 1e-12
    for h, fh, a in h_amps:
        db = 20 * np.log10(a / mx + 1e-12)
        bar = '#' * int(max(0, (db + 60) / 60 * 40))
        print(f"  h{h:2d} {fh:7.1f}Hz {db:6.1f} |{bar}")

    # Energy above 5*f0 as a "richness" proxy.
    hi = spec[freqs > 5 * f0].sum()
    tot = spec.sum() + 1e-12
    print(f"richness (energy >5*f0): {100*hi/tot:.1f}%")

if __name__ == '__main__':
    main()
