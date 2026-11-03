#!/bin/zsh
set -euo pipefail
cd -- "${0:A:h}"
trap 'echo "Stopped before completion. See error above."; read "?Press Return to close."' ERR
appIdentity=$(/usr/bin/security find-identity -v -p codesigning | /usr/bin/awk '/Developer ID Application:/{print $2;exit}')
installerIdentity=$(/usr/bin/security find-identity -v -p basic | /usr/bin/awk '/Developer ID Installer:/{print $2;exit}')
[[ -n "$appIdentity" && -n "$installerIdentity" ]] || { echo 'Developer ID identities unavailable'; exit 1; }
stage=$(/usr/bin/mktemp -d "${TMPDIR:-/tmp}/HEADJOB.XXXXXX")
mkdir -p "$stage/root/Library/Audio/Plug-Ins/Components" "$stage/root/Library/Audio/Plug-Ins/VST3"
/usr/bin/ditto --norsrc --noextattr 'HEADJOB.component' "$stage/root/Library/Audio/Plug-Ins/Components/HEADJOB.component"
/usr/bin/ditto --norsrc --noextattr 'HEADJOB.vst3' "$stage/root/Library/Audio/Plug-Ins/VST3/HEADJOB.vst3"
for plugin in 'Components/HEADJOB.component' 'VST3/HEADJOB.vst3'; do
 /usr/bin/codesign --force --options runtime --timestamp --sign "$appIdentity" "$stage/root/Library/Audio/Plug-Ins/$plugin"
 /usr/bin/codesign --verify --deep --strict "$stage/root/Library/Audio/Plug-Ins/$plugin"
done
cat > "$stage/components.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd"><plist version="1.0"><array><dict><key>RootRelativeBundlePath</key><string>Library/Audio/Plug-Ins/Components/HEADJOB.component</string><key>BundleIsRelocatable</key><false/><key>BundleOverwriteAction</key><string>upgrade</string></dict><dict><key>RootRelativeBundlePath</key><string>Library/Audio/Plug-Ins/VST3/HEADJOB.vst3</string><key>BundleIsRelocatable</key><false/><key>BundleOverwriteAction</key><string>upgrade</string></dict></array></plist>
PLIST
/usr/bin/pkgbuild --root "$stage/root" --component-plist "$stage/components.plist" --identifier audio.headjob.installer --version 0.6.0 --install-location / --ownership recommended "$stage/unsigned.pkg"
output="$PWD/HEADJOB-0.6.0-Signed-Installer-$(date +%Y%m%d-%H%M%S).pkg"
/usr/bin/productsign --sign "$installerIdentity" --timestamp "$stage/unsigned.pkg" "$output"
/usr/sbin/pkgutil --check-signature "$output"
echo "Signed: $output"
read 'profile?Notary Keychain profile (Return for HEADJOB): '
profile=${profile:-HEADJOB}
/usr/bin/xcrun notarytool submit "$output" --keychain-profile "$profile" --wait
/usr/bin/xcrun stapler staple "$output"
/usr/bin/xcrun stapler validate "$output"
/usr/sbin/spctl --assess --type install --verbose=2 "$output"
echo "SIGNED AND NOTARIZED: $output"
read '?Press Return to close.'
