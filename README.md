# HEADJOB Refined 0.5.0 — audition preview

This is an ORIGINAL tentative response to owner feedback, not a hardware-calibrated HEDD emulation. It is a separate plug-in identity (`audio.headjob.refined`, subtype HjR4), so HEADJOB 0.3 sessions and installations remain available for comparison. Both default to zero processing. No previous release file has been replaced.

Triode: shallower asymmetric curve, smaller nonlinear contribution and gentler low/middle knob travel. Pentode (0.5): a flat, memoryless third-harmonic curve. At 14:00 (knob 7.22) a -6 dBFS sine reaches its flat top; past 14:00 that point keeps falling, to -12 dBFS at full. 15% dry signal is kept to preserve the transient edge. Tape (0.5): far less added harmonic content. It acts on level and frequency instead: quiet highs lifted about 1.3 dB at 14:00, loud highs pressed down below unity, and a low-end head bump. Serial ordering, 4x oversampling, 25 ms control smoothing, stereo output metering, manual output gain and RMS Auto Gain are retained.

Sessions saved with Refined 0.4 open with the same knob positions but the new Pentode and Tape sound.

Important: measured behaviour does not demonstrate HEDD accuracy. The 14:00 calibration follows owner listening feedback, not a hardware capture. Pentode presence and Tape brightness require listening/reference assessment. No unmeasured circuit, hysteresis or proprietary formula is claimed.

## Install on macOS

Copy `HEADJOB Refined.component` to `~/Library/Audio/Plug-Ins/Components/`, and `HEADJOB Refined.vst3` to `~/Library/Audio/Plug-Ins/VST3/`. Close and restart the DAW and rescan. Choose **HEADJOB Refined**, keeping HEADJOB 0.3 installed for comparison. Bundles contain arm64 and x86_64; runtime tests were on Apple Silicon. This audition build is ad-hoc signed, NOT Developer ID signed or notarized. The standalone app is optional. Remove the new bundles to uninstall this preview.

## Audition

Use the same source and input gain. Set Output Gain to 0 dB and Auto Gain on in both versions; wait several seconds for matching to settle. Test each stage separately at 2.5, 5, 7.5 and 10, then combine them. Listen for transient preservation, low/mid coloration, Pentode presence and Tape brightness. Also check with Auto Gain off at manually matched listening levels. Auto Gain attenuates only, so it cannot compensate if a processed source is quieter than dry.

For hardware comparison, record the exact same source through a digital HEDD path and capture dry, processed, knob settings, input peak/RMS, sample rate, generation/model and output gain. A short passage with clear transients is more informative than subjective labels alone.

## Build and tests

JUCE 8.0.10 and CMake 3.22+. `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` (or add `-DJUCE_PATH=/absolute/JUCE`). `cmake --build build --config Release`. `ctest --test-dir build -C Release --output-on-failure`. `HEADJOBCompare` compares this preview to the included 0.3 `LegacyEngine` and emits CSV. It does NOT compare against Crane Song hardware. Windows x64 VST3 and Standalone have been built with Visual Studio 2022. Internal tests, silent install/uninstall and pluginval 1.0.4 strictness 5 passed on the Windows Server 2022 runner. No direct Cubase test has been performed. The installer and plug-in have no Windows Authenticode signature. Run scripts/build-windows.ps1 to reproduce; scripts/validate-windows.ps1 is intended for disposable test machines only.

Test CSV measures H2–H9 for a 997 Hz sine, after settling, over one second at 48 kHz, Auto Gain off, Output 0 dB, one active stage. Levels are peak dBFS. Distortion, RMS gain and crest factor are measured separately. A reduction is not automatically a perceptual improvement. Source and JUCE licensing are described by LICENSE and https://juce.com/legal/juce-8-licence/.

## Signed macOS builds

Every push to `refined-preview` or `main` builds a universal AU and VST3 on a GitHub Mac (`.github/workflows/macos.yml`), runs the tests, signs with Developer ID, notarizes with Apple, then installs the result and runs auval and pluginval strictness 5. Signed builds are published to the `refined-latest` release. Signing needs six repository secrets, set once by running `installer/setup_mac_signing.sh` on the Mac; without them the job still builds, ad-hoc signed. No Mac is needed after that.

Local fallback: for a Developer ID signed/notarized macOS installer, the binary preview ZIP includes `Create Signed Refined Installer.command`, using the existing local certificate identities and notary Keychain profile HEADJOB. It is syntax checked but was not run end to end in this agent environment. The supplied ZIP remains ad-hoc signed.

## Resizing (0.4.1)

Drag the visible lower-right corner. The interface scales uniformly from approximately 70% (813×213) to 200% (2324×608), with a fixed 1162:304 aspect ratio. Knobs, text, meter and mouse targets scale together. Editor width is stored in DAW state and restored on reopening. Existing sessions without this property use 1162×304. 
