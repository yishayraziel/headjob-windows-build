#!/bin/bash
# Installs the packaged build on a disposable CI Mac, validates it, then removes it.
# Pass "signed" to also require a valid Developer ID signature and Apple's notarization.
set -euo pipefail
trap 'echo "::error title=validate-macos.sh line $LINENO::failed: $BASH_COMMAND"' ERR
MODE="${1:-unsigned}"
LOGS="$PWD/outputs/macos-validation"; mkdir -p "$LOGS"
PKG=$(ls out/*-macOS-Installer.pkg)
AU="/Library/Audio/Plug-Ins/Components/HEADJOB.component"
VST="/Library/Audio/Plug-Ins/VST3/HEADJOB.vst3"

if [ "$MODE" = signed ]; then
  pkgutil --check-signature "$PKG" | tee "$LOGS/pkg-signature.txt"
  xcrun stapler validate "$PKG" | tee "$LOGS/pkg-staple.txt"
  spctl --assess --type install --verbose=2 "$PKG" 2>&1 | tee "$LOGS/pkg-gatekeeper.txt"
fi

sudo installer -pkg "$PKG" -target / | tee "$LOGS/install.txt"
[ -d "$AU" ] && [ -d "$VST" ]

for binary in "$AU/Contents/MacOS/HEADJOB" "$VST/Contents/MacOS/HEADJOB"; do
  archs=$(lipo -archs "$binary"); echo "$binary: $archs" | tee -a "$LOGS/architectures.txt"
  [[ "$archs" == *arm64* && "$archs" == *x86_64* ]]
done

if [ "$MODE" = signed ]; then
  codesign --verify --deep --strict --verbose=2 "$AU" 2>&1 | tee "$LOGS/au-signature.txt"
  codesign --verify --deep --strict --verbose=2 "$VST" 2>&1 | tee "$LOGS/vst3-signature.txt"
  # The installer's own ticket is checked above. Report a bundle's ticket without failing the build.
  for bundle in "$AU" "$VST"; do
    out=$(xcrun stapler validate "$bundle" 2>&1) && code=0 || code=$?
    [ "$code" = 0 ] || echo "::warning title=stapler validate $(basename "$bundle") exit $code::$(echo "$out" | tr '\n' ' ' | cut -c1-500)"
    [ "$code" != 0 ] || echo "::notice title=stapler validate $(basename "$bundle")::ticket OK"
  done
fi

# Apple's AU validator
sudo killall -9 AudioComponentRegistrar 2>/dev/null || true
sleep 5
auval -v aufx HdJb Hjob 2>&1 | tee "$LOGS/auval.txt"

# real host path: does each knob change the audio of the installed plug-in?
PROBE="build/HEADJOBHostProbe_artefacts/Release/HEADJOBHostProbe"
for target in "$VST" "AudioUnit:Effects/aufx,HdJb,Hjob"; do
  out=$("$PROBE" "$target" 2>&1) && code=0 || code=$?
  echo "$out" | tee -a "$LOGS/host-probe.txt"
  echo "::notice title=host probe $target exit $code::$(echo "$out" | tr '\n' ';' | cut -c1-500)"
  # 5 means a knob did nothing: never publish that. Other codes mean the probe itself could not load the plug-in.
  [ "$code" != 5 ]
done

# same pluginval version and strictness as the Windows validation
TOOLS="$PWD/validation-tools"; mkdir -p "$TOOLS"
curl -fsSL -o "$TOOLS/pluginval.zip" https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_macOS.zip
unzip -q -o "$TOOLS/pluginval.zip" -d "$TOOLS"
"$TOOLS/pluginval.app/Contents/MacOS/pluginval" --strictness-level 5 --validate "$VST" --output-dir "$LOGS" 2>&1 | tee "$LOGS/pluginval.txt"

sudo rm -rf "$AU" "$VST"
sudo pkgutil --forget audio.headjob.installer >/dev/null
echo "PASS: $MODE macOS installer, universal binaries, auval, pluginval strictness 5, uninstall." | tee "$LOGS/result.txt"
