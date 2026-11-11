#!/bin/bash
# Measures the installed Phoenix II from the outside, the way a DAW uses it: test tones in, audio out.
# Nothing inside the plug-in is read. Takes a few minutes. Double-click to run.
cd "$(dirname "$0")"
ROOT="$PWD"; OUT="$ROOT/Probe/PhoenixII"; mkdir -p "$OUT"
exec > >(tee "$OUT/probe_log.txt") 2>&1
finish(){ echo; echo "PHOENIX_PROBE_FINISHED status=$1"; read -r -p "Press Return to close."; exit "$1"; }
export DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer
CMAKE=""
for c in "$(command -v cmake 2>/dev/null)" /opt/homebrew/bin/cmake /usr/local/bin/cmake /Applications/CMake.app/Contents/bin/cmake "$ROOT/.tools/cmake/CMake.app/Contents/bin/cmake"; do [ -x "$c" ] && { CMAKE="$c"; break; }; done
if [ -z "$CMAKE" ]; then
  echo "Downloading CMake 3.31.6 (one time)"
  mkdir -p "$ROOT/.tools" && curl -fL -o "$ROOT/.tools/cmake.tar.gz" https://github.com/Kitware/CMake/releases/download/v3.31.6/cmake-3.31.6-macos-universal.tar.gz || finish 1
  tar xzf "$ROOT/.tools/cmake.tar.gz" -C "$ROOT/.tools" && mv "$ROOT/.tools/cmake-3.31.6-macos-universal" "$ROOT/.tools/cmake"
  CMAKE="$ROOT/.tools/cmake/CMake.app/Contents/bin/cmake"
fi
JUCE_ARG=""; [ -d "$HOME/Library/Caches/HEADJOB-build/_deps/juce-src" ] && JUCE_ARG="-DJUCE_PATH=$HOME/Library/Caches/HEADJOB-build/_deps/juce-src"
BUILD="$HOME/Library/Caches/HEADJOB-probe-build"
echo "=== Build the probe"
"$CMAKE" -S "$ROOT/Tools/PluginProbe" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release $JUCE_ARG || finish 1
"$CMAKE" --build "$BUILD" --config Release --parallel 8 2>&1 | grep -E "error|Built target" ; [ "${PIPESTATUS[0]}" = 0 ] || finish 1
PROBE="$BUILD/PluginProbe_artefacts/Release/PluginProbe"

echo "=== Find Phoenix II"
FOUND=()
for d in /Library/Audio/Plug-Ins/VST3 "$HOME/Library/Audio/Plug-Ins/VST3" /Library/Audio/Plug-Ins/Components "$HOME/Library/Audio/Plug-Ins/Components"; do
  for f in "$d"/*hoenix*; do [ -e "$f" ] && FOUND+=("$f"); done
done
printf '  %s\n' "${FOUND[@]}"
[ ${#FOUND[@]} -gt 0 ] || { echo "Phoenix II not found in the plug-in folders"; finish 1; }
for f in "${FOUND[@]}"; do
  echo "=== Measuring $f"
  "$PROBE" "$f" "$OUT" && finish 0
  echo "That one did not work, trying the next format"
done
finish 1
