# Phoenix II 2.0.0 (VST3), black-box measurements — 2026-11-11

Measured from outside on Yishay's licensed copy with Tools/PluginProbe: sine tones in, audio out, as a DAW uses it. Nothing inside the plug-in was read. Phoenix II is a tape emulation by the same designer as the HEDD; it is a reference for direction, not the HEDD Tape itself.

Findings (Input and Output Trim at 0):
- Odd harmonics only. The second harmonic is below 0.01% in every setting.
- Harmonics rise steeply with level: Iridescent / Gold, Process 100, 997 Hz, third harmonic 0.16% at -18 dBFS, 0.63% at -12, 2.38% at -6, 4.37% at -3 (about 4 times per 6 dB, a cubic law).
- Process also adds level: about +1 dB per 25 at low level (+4 dB at 100, Iridescent / Gold).
- Highs compress before lows: Iridescent / Gold, Process 100, gain at -12 dBFS relative to -40 dBFS is -0.2 dB up to 1 kHz, -1.0 dB at 5 kHz, -2.0 dB at 8 kHz, -3.4 dB at 12.5 kHz, -4.7 dB at 20 kHz.
- Brightness is a static tilt: Opal about 2.7 dB darker above 1 kHz, Gold almost flat, Sapphire 2.5 to 3.5 dB brighter (Luminescent and Iridescent).
- Types: Luminescent and Iridescent smooth and flat; Radiant very clean below -12 dBFS with a high roll-off; Luster the heaviest (11% third harmonic at -6 dBFS, Process 100, Gold) with the largest level boost; Dark Essence between them, darker.

Files: parameters.csv, harmonics.csv (all types x brightness x Process 0/25/50/75/100, 997 Hz at five levels, 60/300/3000/8000 Hz at -30 and -6), response.csv (third-octave response at -40 and -12 dBFS, Process 50 and 100).
