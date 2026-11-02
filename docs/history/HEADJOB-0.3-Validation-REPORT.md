# HEADJOB 0.3.0 verification — 2026-10-31

Built on macOS Apple Silicon with Xcode and JUCE 8.0.10, Release, deployment target macOS 11. Universal arm64/x86_64 Mach-O code is present in AU, VST3 and Standalone. Intel runtime has not been exercised.

- CMake build: successful (AU, VST3, Standalone, tests).
- CTest: 1/1 suite passed. Includes DAW parameter-state serialization, harmonic balance, four sample rates, processed/bypassed bounded output, independent channels and exact clean null. Measured host latency: 6 samples. Null error with zero controls: 0.
- Steinberg VST 3.8.1 validator: 47 passed, 0 failed. Full log attached. Host audio processing is float; DSP internally is double.
- AU direct bundle smoke: exported factory load, initialize, and 100 stereo render blocks passed. This exercises the AU wrapper directly, not system registration. Attempting local AudioComponentRegister returned no component in this execution environment; full auval/Logic validation remains to be performed after installation.
- Ad-hoc signature verification: AU, VST3 and Standalone passed. Xcode displays Developer ID Application/Installer certificates, but signing CLI reports zero usable identities even after keychain file read access was granted. No Developer ID signature or notarization is claimed for this download. A syntax-checked local signing/notarization helper is included; its end-to-end execution remains pending.
- UI: supplied photographic panel embedded; live rotary crop rendering, stereo LED dimming and bypass switch verified. Three knob attachments and bypass attachment tested. Actual JUCE editor rendered at 2× and visually inspected at zero, representative active values, and bypass. Active preview uses representative meter values for display.

No HEDD reference recordings were supplied. No hardware null, aliasing suppression in dB, perceptual loudness matching, full DAW automation stress test, or Intel runtime result is claimed. Fixed gain compensation is approximate. Parameter and bypass ramps are implemented; host-specific bypass/suspend policies may differ.

Version 0.2.1 restricts the editor viewport to the silver faceplate (1936×382 source coordinates), with translated knob/switch hit regions and meter rendering. DSP is unchanged.

Version 0.2.2 replaces rotating photographic knobs with cached GUI-thread height-field shading. Light direction is fixed in panel coordinates; only grip geometry and pointer rotate. Editor renders visually checked at controls 0, 2.5, 5, 7.5 and 10. Full reference style is approximated by the new renderer; DSP unchanged.

Version 0.3.0 adds output gain and attenuation-only stereo RMS Auto Gain. Steady 1 kHz/0.03 amplitude Triode=10 test: Auto Gain off +3.0439 dB, on -0.000528386 dB after settling; stereo difference 0. Manual +6 dB amplitude, host bypass with manual +12 dB/Auto Gain enabled, six UI attachments and new parameter state recall passed. This is one deterministic test stimulus, not a universal perceptual loudness measurement.
