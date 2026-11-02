# HEADJOB Refined 0.4 audition report — 2026-11-01

All comparisons below are against our HEADJOB 0.3 DSP, NOT HEDD hardware.

Conditions: 997 Hz sine at 48 kHz; -12 dBFS peak; one stage at knob 5; other stages 0; Output 0 dB; Auto Gain off; one second of analysis after one second settling. THD includes H2–H9 only.

| Stage | 0.3 THD % | Refined THD % | 0.3 RMS gain dB | Refined RMS gain dB |
|---|---:|---:|---:|---:|
| Triode | 4.470 | 0.318 | 1.624 | 0.091 |
| Pentode | 0.960 | 0.165 | 2.189 | 0.018 |
| Tape | 2.413 | 0.125 | 3.838 | 0.195 |

The 36-condition sweep covers each stage independently at knobs 2.5/5/7.5/10 and input peaks -18/-12/-6 dBFS. Every condition showed lower measured H2–H9 THD. Lower THD is an intentional behavioral change, not proof of greater HEDD fidelity.

At -6 dBFS peak and knob 10, the refined curves still produce approximately 2.64% Triode, 2.71% Pentode and 3.10% Tape H2–H9 THD. They have not been reduced to a clean gain stage.

macOS: Release universal arm64/x86_64 builds; DSP/state/Auto Gain/output tests passed on Apple Silicon; exact zero-control clean null; finite/stable 44.1/48/96/192 kHz; 47 Steinberg VST3 checks passed; direct AU factory/render smoke passed. Full Logic/Cubase tests remain unperformed.

Final perceptual priorities: retain Pentode presence and transient definition, retain desirable Tape brightness, and make Triode coloration easier to distinguish from overt distortion. Same-source HEDD captures are required for quantitative matching.

Windows x64: Release VST3 and Standalone built on private GitHub Actions run 41319138215, commit 8aa7f32. Native internal tests, installer install/uninstall and pluginval strictness 5 passed. No Windows Authenticode signature or direct Cubase validation.
