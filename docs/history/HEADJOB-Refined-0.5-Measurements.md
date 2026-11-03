# HEADJOB Refined 0.5 measurements — 2026-11-03

All figures are from the engine itself, one stage active, Auto Gain off, Output 0 dB, sine input, 48 kHz. Knob 7.22 is the 14:00 position. Nothing here is compared to HEDD hardware.

Why the change: in 0.4, Pentode and Tape at 14:00 were close to the same process.

| 14:00, 997 Hz | H3 at -12 dBFS | H3 at -6 dBFS | Gain at -6 dBFS |
|---|---:|---:|---:|
| Pentode 0.4 | 0.40% | 1.16% | -0.24 dB |
| Tape 0.4 | 0.36% | 1.13% | +0.13 dB |
| Pentode 0.5 | 1.89% | 9.05% | -1.66 dB |
| Tape 0.5 | 0.11% | 0.34% | -0.10 dB |

Pentode 0.5 H3 at 997 Hz, -6 dBFS: knob 2.5 0.39%, knob 5 2.64%, knob 7.22 9.05%, knob 8.5 16.4%, knob 10 18.8%. Response is flat with frequency.

Tape 0.5 at 14:00: 8 kHz gain +1.26 dB at -36 dBFS, +0.19 dB at -18 dBFS, -1.23 dB at -6 dBFS. 60 Hz +0.65 dB at -6 dBFS.

## 0.5.3 Tape, rebuilt from the HEDD 192 manual's description

The 0.5.0 Tape above removed too many harmonics. The manual describes Tape as odd harmonic, with highs compressing before lows.

14:00, 997 Hz: H3 0.27% at -18 dBFS, 1.03% at -12 dBFS, 3.35% at -6 dBFS. H5 0.27% at -6 dBFS. Gain -0.98 dB at -6 dBFS.
14:00 gain, -30 dBFS to -6 dBFS: 60 Hz +0.79 to +0.14 dB, 8 kHz +0.16 to -2.96 dB. Quiet 150 Hz +1.08 dB.

## 0.5.3 Triode, raised after listening to 0.5.2

14:00, 997 Hz, H2: 0.36 to 1.29% at -18 dBFS, 0.70 to 2.54% at -12 dBFS, 1.30 to 4.76% at -6 dBFS. H3 0.33% at -6 dBFS.

## 0.5.4, from the HEDD 192 manual and a secondary research summary

Nothing here is measured against hardware. Hardware captures are still pending.

Pentode 14:00, 997 Hz: H5 0.03% at -12 dBFS, 0.63% at -6 dBFS. H3 9.24% at -6 dBFS.
Triode 14:00 H2 at -6 dBFS: 100 Hz 8.5%, 997 Hz 5.2%. Quiet response stays within 0.2 dB.
Tape 14:00: +0.2 dB level at -12 dBFS with the peak behaviour unchanged.
A soft ceiling prevents the process from creating overs from a -0.1 dBFS sine.
