#!/bin/bash
# Builds HEADJOB on this Mac, tests it, signs it and installs it for every DAW. Double-click to run.
cd "$(dirname "$0")"
ROOT="$PWD"
exec > >(tee "$ROOT/build_log.txt") 2>&1
finish(){ echo; echo "HEADJOB_BUILD_FINISHED status=$1"; read -r -p "Press Return to close."; exit "$1"; }
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
echo "=== Xcode"; xcodebuild -version || { echo "Xcode is missing or its licence is not accepted: run  sudo xcodebuild -license accept"; finish 1; }

echo "=== CMake"
CMAKE=""
for c in "$(command -v cmake 2>/dev/null)" /opt/homebrew/bin/cmake /usr/local/bin/cmake /Applications/CMake.app/Contents/bin/cmake "$ROOT/.tools/cmake/CMake.app/Contents/bin/cmake"; do [ -x "$c" ] && { CMAKE="$c"; break; }; done
if [ -z "$CMAKE" ]; then
  echo "Downloading CMake 3.31.6 (one time)"
  mkdir -p "$ROOT/.tools" && curl -fL -o "$ROOT/.tools/cmake.tar.gz" https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-macos-universal.tar.gz || finish 1
  tar xzf "$ROOT/.tools/cmake.tar.gz" -C "$ROOT/.tools" && mv "$ROOT/.tools/cmake-3.31.6-macos-universal" "$ROOT/.tools/cmake"
  CMAKE="$ROOT/.tools/cmake/CMake.app/Contents/bin/cmake"
fi
"$CMAKE" --version | head -1

BUILD="$HOME/Library/Caches/HEADJOB-build"
echo "=== Configure (universal arm64 + x86_64)"
"$CMAKE" -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 || finish 1
echo "=== Build"
"$CMAKE" --build "$BUILD" --config Release --target HEADJOB_AU HEADJOB_VST3 HEADJOBTests --parallel 8 2>&1 | grep -E "error|warning: unused|Built target" ; [ "${PIPESTATUS[0]}" = 0 ] || finish 1
echo "=== Tests"
"$BUILD/HEADJOBTests_artefacts/Release/HEADJOBTests" || { echo "TESTS FAILED"; finish 1; }

AU="$BUILD/HEADJOB_artefacts/Release/AU/HEADJOB.component"
VST="$BUILD/HEADJOB_artefacts/Release/VST3/HEADJOB.vst3"
lipo -info "$AU/Contents/MacOS/HEADJOB" "$VST/Contents/MacOS/HEADJOB"

echo "=== Sign"
ID=$(security find-identity -v -p codesigning | awk '/Developer ID Application/{print $2; exit}')
if [ -n "$ID" ]; then echo "Developer ID certificate: $ID"; else echo "No Developer ID found, signing for this Mac only"; ID="-"; fi
for b in "$AU" "$VST"; do codesign --force --deep --options runtime --timestamp=none --sign "$ID" "$b" && codesign --verify --deep --strict "$b" || finish 1; done

echo "=== Install (your Mac password is needed once)"
sudo rm -rf "/Library/Audio/Plug-Ins/Components/HEADJOB.component" "/Library/Audio/Plug-Ins/VST3/HEADJOB.vst3" \
  "/Library/Audio/Plug-Ins/Components/HEADJOB Refined.component" "/Library/Audio/Plug-Ins/VST3/HEADJOB Refined.vst3"
rm -rf "$HOME/Library/Audio/Plug-Ins/Components/HEADJOB.component" "$HOME/Library/Audio/Plug-Ins/VST3/HEADJOB.vst3" \
  "$HOME/Library/Audio/Plug-Ins/Components/HEADJOB Refined.component" "$HOME/Library/Audio/Plug-Ins/VST3/HEADJOB Refined.vst3"
sudo ditto "$AU" "/Library/Audio/Plug-Ins/Components/HEADJOB.component" && sudo ditto "$VST" "/Library/Audio/Plug-Ins/VST3/HEADJOB.vst3" || finish 1
killall -9 AudioComponentRegistrar 2>/dev/null; sleep 3

echo "=== auval"
auval -v aufx HdJb Hjob | tail -3
VERSION=$(sed -nE 's/^project\(HEADJOB VERSION ([0-9.]+)\)/\1/p' "$ROOT/CMakeLists.txt")
echo "=== Installed HEADJOB $VERSION. Close and reopen Cubase, then load HEADJOB again."
finish 0
