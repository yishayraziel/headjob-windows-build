# HEADJOB 0.3.0

An original stereo harmonic processor inspired by the documented architecture of Crane Song HEDD. This is an experimental first release, not an affiliated product, a reverse-engineered implementation, or a measured 1:1 emulation. No proprietary code or branding is included.

## Installation (macOS 11 or newer)

Quit your DAW. In the binary download, double-click `Install HEADJOB.command`. It copies the AU and VST3 to your user Library, without administrator rights. Alternatively, copy HEADJOB.component to `~/Library/Audio/Plug-Ins/Components/` and HEADJOB.vst3 to `~/Library/Audio/Plug-Ins/VST3/`. Restart the DAW and rescan plug-ins. Logic uses AU; other hosts may use VST3. The standalone app is optional and requires an audio device configured in its settings.

This build is ad-hoc signed, not Developer ID signed or notarized. macOS may require explicit permission in Privacy & Security after downloading. Intel code is included, but runtime testing was performed on Apple Silicon. To uninstall, delete the two bundles from those folders.

## Controls and workflow

Three harmonic knobs: Triode, Pentode, Tape, each 0–10, with continuous DAW automation. Double-click resets a knob. The photographed ON switch toggles bypass; its parameter is also exposed to the DAW for automation. The two-row LED meter shows stereo output sample peaks, with approximately 300 ms fall time; red begins near 0 dBFS. It is not a true-peak meter or limiter.

Start at 2–3 and compare at matched listening levels. All harmonic controls at zero, manual Output Gain at 0 dB and Auto Gain off give an exactly delayed clean path. Bypass crossfades over 15 ms, with latency matched dry audio; some DAWs independently implement bypass or suspend processing, outside the plug-in's control. Latency is reported to the host. Internal parameters smooth over 25 ms. DAW sessions and host preset files retain all six parameters. No additional preset browser is included.

Version 0.2 changes only the interface and embeds the supplied art; the DSP and parameter IDs remain compatible with 0.1 sessions.

## Developer ID distribution

The provided build remains ad-hoc signed. Xcode displayed Developer ID certificates, but this agent execution environment did not expose a usable signing identity, even with keychain file read permission. No certificate was created or revoked. Run `Sign-and-Notarize HEADJOB.command` locally from the extracted binary folder to sign a staging copy and produce a notarized DMG. It uses your existing Developer ID Application certificate and a local notarytool Keychain profile. Credential setup happens interactively in your Terminal; no secrets belong in chat. The script is syntax checked but could not be executed end to end here. Apple documentation: https://developer.apple.com/developer-id/ and https://developer.apple.com/documentation/security/customizing-the-notarization-workflow .

## DSP

Stereo input → asymmetric biased tanh Triode → symmetric tanh Pentode → Tape pre-emphasis / soft saturation / inverse de-emphasis → DC removal → fixed compensation → stereo meter.

Nonlinearity runs at 4× sample rate using JUCE's high-quality polyphase IIR oversampler, with integer host latency. Internal DSP uses double precision, host audio uses float. Tape boosts the high-frequency component before saturation and reverses that boost afterward, causing frequency-dependent compression. No wow, flutter, noise, or hysteresis model is included. Stage compensation is fixed and approximate: it prevents gross small-signal gain changes but is not automatic loudness normalization. Strong settings intentionally compress peaks and may change perceived loudness. Output is not brickwall limited; allow headroom. Oversampling reduces, but does not eliminate, aliasing.

The transfer curves and coefficients are original engineering choices. Previously cited hardware THD figures have not been used as universal calibration targets because input level and measurement conditions must be replicated. Hardware reference captures are needed before claiming sound matching.

## Build

Install Xcode with command-line tools, CMake 3.22+, and internet access (or provide a local JUCE checkout).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
cmake --build build --config Release -j 6
ctest --test-dir build --output-on-failure
```

JUCE 8.0.10 is fetched at its version tag. For offline builds add `-DJUCE_PATH=/absolute/path/to/JUCE`. For an Xcode project add `-G Xcode` and open `build/HEADJOB.xcodeproj`. Build products are under `build/HEADJOB_artefacts/Release/`. Source project does not embed absolute machine paths. Review JUCE's AGPL/commercial licensing before distribution or commercial release: https://juce.com/legal/juce-8-licence/ . Original HEADJOB code is offered under MIT; third-party components retain their own licenses.

## Research and reference boundaries

- Manufacturer-authored HEDD-192 manual, p. 4–6, mirrored at https://www.barryrudolph.com/recall/manuals/cranesonghedd.pdf : serial ordering, even-rich Triode, odd-rich Pentode, Tape EQ/compression, smooth bypass. Rechecked for this build.
- Prior research reference: https://www.hispasonic.com/blogs/analisis-profundidad-cranesong-hedd-192/37188 . Published hardware measurements are contextual references, not proof that this build matches hardware.
- Prior research reference: https://repforums.prosoundweb.com/index.php/topic,3410.75.html . Secondary discussion, not a published proprietary algorithm.

The supplied photographic panel is embedded as the interface background. Knobs now use a geometric height-field renderer with normals lit in fixed panel coordinates, removing rotating photographic highlights. The grip and pointer rotate while the circular rim stays fixed; individual original LED cells dim according to output levels, and the ON switch controls bypass. The viewport is cropped to the silver faceplate only; the surrounding scene, green chassis and reflection are excluded. The supplied logo, metal texture and lighting are preserved.

## Output controls (0.3.0)

The small footer knob provides manual Output Gain from -24 to +12 dB (25 ms ramp). Double-click resets to 0 dB. Auto Gain defaults off. It compares stereo-linked input and processed-output mean-square power with a 400 ms exponential window, then smoothly adjusts attenuation over approximately 250 ms. It never boosts; attenuation is limited to 24 dB. Below approximately -100 dBFS input power it holds the correction to avoid chasing silence. It requires settling time and is RMS matching, not LUFS or guaranteed perceptual loudness equality. It cannot raise processed signals that are quieter than the input.

Manual gain is applied AFTER automatic correction, so use 0 dB for level-matched comparisons. Both controls are smoothed, automatable and saved in DAW state. Metering follows both controls. Host bypass crossfades back to the latency-aligned dry signal, bypassing manual and automatic gain. These controls do not provide limiting.
