# HEADJOB Refined 0.4 — audition preview

This is an ORIGINAL tentative response to owner feedback, not a hardware-calibrated HEDD emulation. It is a separate plug-in identity (`audio.headjob.refined`, subtype HjR4), so HEADJOB 0.3 sessions and installations remain available for comparison. Both default to zero processing. No previous release file has been replaced.

Triode: shallower asymmetric curve, smaller nonlinear contribution and gentler low/middle knob travel. Pentode: predominantly odd harmonic contribution blended into the unmodified signal, retaining at least 65% of the linear component at the maximum setting, instead of driving the entire stage deeply into tanh. Tape: reduced record pre-emphasis (0.85 versus 1.8 HF residual gain), reduced saturation drive, restrained nonlinear contribution and matching inverse de-emphasis. Serial ordering, 4x oversampling, 25 ms control smoothing, stereo output metering, manual output gain and RMS Auto Gain are retained. Amount-dependent makeup is much smaller.

Important: lower distortion does not demonstrate greater HEDD accuracy. This preview may be too subtle or have the wrong harmonic balance. The Pentode sense of presence and Tape brightness require listening/reference assessment. No unmeasured circuit, hysteresis or proprietary formula is claimed.

## Install on macOS

Copy `HEADJOB Refined.component` to `~/Library/Audio/Plug-Ins/Components/`, and `HEADJOB Refined.vst3` to `~/Library/Audio/Plug-Ins/VST3/`. Close and restart the DAW and rescan. Choose **HEADJOB Refined**, keeping HEADJOB 0.3 installed for comparison. Bundles contain arm64 and x86_64; runtime tests were on Apple Silicon. This audition build is ad-hoc signed, NOT Developer ID signed or notarized. The standalone app is optional. Remove the new bundles to uninstall this preview.

## Audition

Use the same source and input gain. Set Output Gain to 0 dB and Auto Gain on in both versions; wait several seconds for matching to settle. Test each stage separately at 2.5, 5, 7.5 and 10, then combine them. Listen for transient preservation, low/mid coloration, Pentode presence and Tape brightness. Also check with Auto Gain off at manually matched listening levels. Auto Gain attenuates only, so it cannot compensate if a processed source is quieter than dry.

For hardware comparison, record the exact same source through a digital HEDD path and capture dry, processed, knob settings, input peak/RMS, sample rate, generation/model and output gain. A short passage with clear transients is more informative than subjective labels alone.

## Build and tests

JUCE 8.0.10 and CMake 3.22+. `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` (or add `-DJUCE_PATH=/absolute/JUCE`). `cmake --build build --config Release`. `ctest --test-dir build -C Release --output-on-failure`. `HEADJOBCompare` compares this preview to the included 0.3 `LegacyEngine` and emits CSV. It does NOT compare against Crane Song hardware. Windows conditional targets are present, but this preview's Windows binaries have not been built/tested.

Test CSV measures H2–H9 for a 997 Hz sine, after settling, over one second at 48 kHz, Auto Gain off, Output 0 dB, one active stage. Levels are peak dBFS. Distortion, RMS gain and crest factor are measured separately. A reduction is not automatically a perceptual improvement. Source and JUCE licensing are described by LICENSE and https://juce.com/legal/juce-8-licence/.
