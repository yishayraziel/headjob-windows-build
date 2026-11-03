#!/bin/bash
# Signs (Developer ID), notarizes and packages HEADJOB for macOS.
# Without the signing secrets it falls back to an ad-hoc signed, un-notarized build.
set -euo pipefail
# surface failures as GitHub annotations (readable without the raw logs)
trap 'echo "::error title=mac_package.sh line $LINENO::failed: $BASH_COMMAND"' ERR

staple() {     # Apple's ticket can take a minute to propagate after "Accepted": retry
  for i in 1 2 3 4 5 6 7 8; do
    xcrun stapler staple "$1" && return 0
    echo "stapler not ready yet ($i), retrying in 20 s"; sleep 20
  done
  echo "::error title=stapler::could not staple $1"; return 1
}
notarize() {   # notarize "$1" and fail with Apple's log if it isn't accepted
  local out id status
  out=$(xcrun notarytool submit "$1" "${NOTARY[@]}" --output-format json 2>&1) || { echo "::error title=notarytool::$(echo "$out" | tr '\n' ' ' | cut -c1-900)"; return 1; }
  id=$(echo "$out" | sed -nE 's/.*"id" *: *"([^"]+)".*/\1/p' | head -1)
  status=$(echo "$out" | sed -nE 's/.*"status" *: *"([^"]+)".*/\1/p' | head -1)
  echo "::notice title=notarize $(basename "$1")::status=$status id=$id"
  if [ "$status" != "Accepted" ]; then
    echo "::error title=notary log::$(xcrun notarytool log "$id" "${NOTARY_AUTH[@]}" 2>&1 | tr '\n' ' ' | cut -c1-1500)"
    return 1
  fi
}

VERSION="${VERSION:?VERSION is required}"
RELEASE="build/HEADJOB_artefacts/Release"
AU="$RELEASE/AU/HEADJOB.component"
VST="$RELEASE/VST3/HEADJOB.vst3"
OUT="$PWD/out"
PKG="$OUT/HEADJOB-$VERSION-macOS-Installer.pkg"
ZIP="$OUT/HEADJOB-$VERSION-macOS-Plugins.zip"
TMP="${RUNNER_TEMP:-$(mktemp -d)}"
mkdir -p "$OUT"

SIGNED=0
if [ -n "${MACOS_CERT_APP_P12:-}" ] && [ -n "${MACOS_CERT_INSTALLER_P12:-}" ]; then
  SIGNED=1
  KC="$TMP/headjob.keychain-db"; KCP="$(uuidgen)"
  security create-keychain -p "$KCP" "$KC"
  security set-keychain-settings -lut 21600 "$KC"
  security unlock-keychain -p "$KCP" "$KC"
  echo "$MACOS_CERT_APP_P12"       | base64 --decode > "$TMP/app.p12"
  echo "$MACOS_CERT_INSTALLER_P12" | base64 --decode > "$TMP/inst.p12"
  for p in app inst; do
    imp=$(security import "$TMP/$p.p12" -k "$KC" -P "$MACOS_CERT_PASSWORD" -T /usr/bin/codesign -T /usr/bin/productbuild -T /usr/bin/productsign 2>&1) \
      || { echo "::error title=import $p.p12::$imp"; exit 1; }
  done
  rm -f "$TMP/app.p12" "$TMP/inst.p12"
  security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$KCP" "$KC" >/dev/null
  security list-keychains -d user -s "$KC" $(security list-keychains -d user | tr -d '"')
  APP_ID=$(security find-identity -v -p codesigning "$KC" | grep "Developer ID Application" | head -1 | sed -E 's/.*"(.*)"/\1/')
  INST_ID=$(security find-identity -v "$KC" | grep "Developer ID Installer" | head -1 | sed -E 's/.*"(.*)"/\1/')
  echo "::notice title=identities::app='${APP_ID:-NONE}' installer='${INST_ID:-NONE}'"
  [ -n "$APP_ID" ] && [ -n "$INST_ID" ] || { echo "::error title=certificates::Developer ID Application/Installer identity not found in the imported p12 files"; exit 1; }
  NOTARY_AUTH=(--apple-id "$APPLE_ID" --password "$APPLE_APP_PASSWORD" --team-id "$APPLE_TEAM_ID")
  NOTARY=("${NOTARY_AUTH[@]}" --wait)

  codesign --force --options runtime --timestamp --sign "$APP_ID" "$AU"
  codesign --force --options runtime --timestamp --sign "$APP_ID" "$VST"
  codesign --verify --deep --strict --verbose=2 "$AU"
  codesign --verify --deep --strict --verbose=2 "$VST"

  # notarize the bare bundles (for the manual-install zip) and staple the tickets to them
  mkdir -p "$TMP/notarize"
  ditto "$AU"  "$TMP/notarize/$(basename "$AU")"
  ditto "$VST" "$TMP/notarize/$(basename "$VST")"
  ditto -c -k "$TMP/notarize" "$TMP/plugins.zip"
  notarize "$TMP/plugins.zip"
  staple "$AU"
  staple "$VST"
else
  echo "::warning title=unsigned build::No signing secrets: ad-hoc signing, macOS will warn on install"
  codesign --force --deep -s - "$AU"
  codesign --force --deep -s - "$VST"
fi

# manual-install zip
mkdir -p "$TMP/zip"
ditto "$AU"  "$TMP/zip/$(basename "$AU")"
ditto "$VST" "$TMP/zip/$(basename "$VST")"
ditto -c -k "$TMP/zip" "$ZIP"

# installer: same layout and identifier as the earlier local signed installers, so it upgrades them
ROOT="$TMP/pkgroot/Library/Audio/Plug-Ins"
mkdir -p "$ROOT/Components" "$ROOT/VST3"
ditto "$AU"  "$ROOT/Components/$(basename "$AU")"
ditto "$VST" "$ROOT/VST3/$(basename "$VST")"
cat > "$TMP/components.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><array>
<dict><key>RootRelativeBundlePath</key><string>Library/Audio/Plug-Ins/Components/HEADJOB.component</string><key>BundleIsRelocatable</key><false/><key>BundleOverwriteAction</key><string>upgrade</string></dict>
<dict><key>RootRelativeBundlePath</key><string>Library/Audio/Plug-Ins/VST3/HEADJOB.vst3</string><key>BundleIsRelocatable</key><false/><key>BundleOverwriteAction</key><string>upgrade</string></dict>
</array></plist>
PLIST
pkgbuild --root "$TMP/pkgroot" --component-plist "$TMP/components.plist" \
  --identifier audio.headjob.installer --version "$VERSION" \
  --install-location / --ownership recommended --scripts installer/mac-scripts "$TMP/unsigned.pkg"

if [ $SIGNED = 1 ]; then
  productsign --sign "$INST_ID" --timestamp "$TMP/unsigned.pkg" "$PKG"
  pkgutil --check-signature "$PKG"
  notarize "$PKG"
  staple "$PKG"
else
  cp "$TMP/unsigned.pkg" "$PKG"
fi

(cd "$OUT" && shasum -a 256 "$(basename "$PKG")" "$(basename "$ZIP")" > "HEADJOB-$VERSION-macOS-SHA256.txt")
ls -la "$OUT"
[ -z "${GITHUB_OUTPUT:-}" ] || echo "signed=$SIGNED" >> "$GITHUB_OUTPUT"
