#!/usr/bin/env python3
"""Test signal for measuring a hardware unit digital in, digital out. Writes the WAV and a segment map."""
import json, struct, sys, wave
import numpy as np

SR = 48000
rng = np.random.default_rng(1)
parts, segments, now = [], [], [0]

def db(x): return 10 ** (x / 20)
def fade(x, ms=5):
    n = int(SR * ms / 1000); r = np.sin(np.linspace(0, np.pi / 2, n)) ** 2
    x = x.copy(); x[:n] *= r; x[-n:] *= r[::-1]; return x
def add(name, x, **info):
    if name: segments.append({"name": name, "start": now[0], "length": len(x), **info})
    parts.append(x); now[0] += len(x)
def gap(s=0.1): add(None, np.zeros(int(SR * s)))
def sine(f, level, s):
    t = np.arange(int(SR * s)) / SR; return fade(db(level) * np.sin(2 * np.pi * f * t))
def click():
    x = np.zeros(int(SR * 0.002)); x[:8] = db(-6) * np.array([1, 1, -1, -1, 1, 1, -1, -1]); return x

gap(1); add("sync_start", click()); gap(1)
for level in (-42, -36, -30, -24, -18, -12, -9, -6, -3, -1):
    add(f"sine_997_{level}", sine(997, level, 1.2), freq=997, level=level); gap()
for f in (60, 100, 300, 3000, 8000):
    for level in (-36, -24, -18, -12, -6):
        add(f"sine_{f}_{level}", sine(f, level, 0.8), freq=f, level=level); gap()
for level in (-30, -18, -6):
    n = int(SR * 6); t = np.arange(n) / SR; k = np.log(20000 / 20)
    add(f"sweep_{level}", fade(db(level) * np.sin(2 * np.pi * 20 * 6 / k * (np.exp(t / 6 * k) - 1))), level=level); gap(0.3)
t = np.arange(SR * 2) / SR
for level in (-12, -6):
    add(f"imd_60_7000_{level}", fade(db(level) * (0.8 * np.sin(2 * np.pi * 60 * t) + 0.2 * np.sin(2 * np.pi * 7000 * t))), level=level); gap()
    add(f"imd_13k_14k_{level}", fade(db(level) * 0.5 * (np.sin(2 * np.pi * 13000 * t) + np.sin(2 * np.pi * 14000 * t))), level=level); gap()
envelope = np.concatenate([np.tile(np.r_[np.full(int(SR * .3), db(-30)), np.full(int(SR * .3), db(-6))], 6)])
add("level_steps_997", fade(envelope * np.sin(2 * np.pi * 997 * np.arange(len(envelope)) / SR)), low=-30, high=-6); gap(0.3)
white = rng.standard_normal(SR * 12); spectrum = np.fft.rfft(white); freqs = np.fft.rfftfreq(len(white), 1 / SR); freqs[0] = 1
pink = np.fft.irfft(spectrum / np.sqrt(freqs), len(white)); pink /= np.sqrt(np.mean(pink ** 2))
for level, part in ((-24, pink[:SR * 6]), (-14, pink[SR * 6:])):
    x = part * db(level); peak = np.max(np.abs(x))
    add(f"pink_{level}_rms", fade(x * min(1, db(-1) / peak)), level=level); gap()
gap(1); add("sync_end", click()); gap(1)

signal = np.concatenate(parts); out = sys.argv[1]
pcm = np.clip(np.round(signal * 8388607), -8388607, 8388607).astype(np.int32)
with wave.open(out, "wb") as w:
    w.setnchannels(2); w.setsampwidth(3); w.setframerate(SR)
    mono = b"".join(struct.pack("<i", int(v))[:3] for v in pcm)
    w.writeframes(b"".join(mono[i:i + 3] * 2 for i in range(0, len(mono), 3)))
json.dump({"rate": SR, "segments": segments}, open(out.rsplit(".", 1)[0] + ".json", "w"), indent=1)
print(f"{len(signal) / SR:.1f} s, peak {20 * np.log10(np.max(np.abs(signal))):.2f} dBFS, {len(segments)} segments")
