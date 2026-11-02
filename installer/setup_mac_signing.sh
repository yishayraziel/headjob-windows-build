#!/bin/bash
# HEADJOB: send your Mac signing credentials to GitHub as encrypted repository secrets,
# then start a signed + notarized build. Run once on your Mac, in Terminal:
#     bash setup_mac_signing.sh
# Nothing is sent anywhere except GitHub's encrypted secrets store for the repo below.
set -euo pipefail
REPO="yishayraziel/headjob-windows-build"
BRANCH="refined-preview"
APP_P12="${1:-$HOME/Desktop/DeveloperIDApplication.p12}"
INST_P12="${2:-$HOME/Desktop/DeveloperIDInstaller.p12}"

say() { printf "\n\033[1;31m▶\033[0m %s\n" "$*"; }

# 1. GitHub CLI (reuses the copy from the WADAHEX setup, otherwise downloads it privately)
GHDIR="$HOME/.wadahex-gh"
[ -x "$GHDIR/bin/gh" ] && export PATH="$GHDIR/bin:$PATH"
if ! command -v gh >/dev/null 2>&1; then
  say "Downloading GitHub CLI (one time)"
  ARCH=$(uname -m); [ "$ARCH" = "x86_64" ] && ARCH=amd64
  TAG=$(curl -fsSI https://github.com/cli/cli/releases/latest | tr -d '\r' | sed -nE 's#^[Ll]ocation: .*/tag/v(.*)$#\1#p')
  [ -n "$TAG" ] || { echo "Could not find the latest GitHub CLI version - check your internet connection."; exit 1; }
  TMPD=$(mktemp -d)
  curl -fsSL -o "$TMPD/gh.zip" "https://github.com/cli/cli/releases/download/v$TAG/gh_${TAG}_macOS_${ARCH}.zip"
  unzip -q "$TMPD/gh.zip" -d "$TMPD"
  mkdir -p "$GHDIR"; cp -R "$TMPD"/gh_*/bin "$GHDIR/"
  xattr -dr com.apple.quarantine "$GHDIR" 2>/dev/null || true
  export PATH="$GHDIR/bin:$PATH"
fi
gh auth status >/dev/null 2>&1 || { say "Log in to GitHub (a browser window will open)"; gh auth login -h github.com -p https -w; }

# 2. Certificates exported from Keychain Access
for f in "$APP_P12" "$INST_P12"; do
  [ -f "$f" ] || { echo "Missing $f"; echo "Export it from Keychain Access > My Certificates > right click > Export, save to the Desktop under that name, and run this again."; exit 1; }
done
# Check the files with macOS's own keychain tool, the same way the build machine opens them.
CHECK_DIR=$(mktemp -d); CHECK_KC="$CHECK_DIR/check.keychain-db"
trap 'security delete-keychain "$CHECK_KC" 2>/dev/null || true; rm -rf "$CHECK_DIR"' EXIT
security create-keychain -p headjob-check "$CHECK_KC"
security set-keychain-settings "$CHECK_KC"
security unlock-keychain -p headjob-check "$CHECK_KC"
opens() { security import "$1" -k "$CHECK_KC" -P "$2" -T /usr/bin/codesign >/dev/null 2>&1; }

P12PW=""; OK=0
for attempt in 1 2 3; do
  say "Password you gave the .p12 files when exporting (attempt $attempt of 3)"
  read -r -s -p "p12 password: " P12PW; echo
  if opens "$APP_P12" "$P12PW"; then OK=1; break; fi
  echo "That password does not open DeveloperIDApplication.p12."
done
if [ $OK != 1 ]; then
  echo
  echo "Forgot the password? Export both certificates again from Keychain Access,"
  echo "give both the same new password, save them to the Desktop, and run this again."
  exit 1
fi
if ! opens "$INST_P12" "$P12PW"; then
  echo "That password opens the Application file but not DeveloperIDInstaller.p12."
  echo "Both files must share one password. Export the Installer certificate again with it, and run this again."
  exit 1
fi

IDENTITIES=$(security find-identity "$CHECK_KC")
echo "$IDENTITIES" | grep -q '"Developer ID Application' || { echo "$APP_P12 has no 'Developer ID Application' certificate with its private key."; exit 1; }
echo "$IDENTITIES" | grep -q '"Developer ID Installer'   || { echo "$INST_P12 has no 'Developer ID Installer' certificate with its private key."; exit 1; }
TEAM_ID=$(echo "$IDENTITIES" | sed -nE 's/.*"Developer ID Application.*\(([A-Z0-9]{10})\)".*/\1/p' | head -1)
[ -n "$TEAM_ID" ] || read -r -p "Team ID (developer.apple.com > Membership): " TEAM_ID
echo "Certificates OK. Team ID: $TEAM_ID"

# 3. Notarization login
say "Apple ID email of your developer account"
read -r -p "Apple ID: " APPLE_ID
say "App-specific password (appleid.apple.com > Sign-In and Security > App-Specific Passwords)"
read -r -s -p "App-specific password: " APPLE_APP_PASSWORD; echo

# 4. Upload as encrypted GitHub secrets
say "Saving secrets to $REPO"
base64 -i "$APP_P12"  | tr -d '\n' | gh secret set MACOS_CERT_APP_P12       --repo "$REPO"
base64 -i "$INST_P12" | tr -d '\n' | gh secret set MACOS_CERT_INSTALLER_P12 --repo "$REPO"
printf %s "$P12PW"              | gh secret set MACOS_CERT_PASSWORD --repo "$REPO"
printf %s "$APPLE_ID"           | gh secret set APPLE_ID            --repo "$REPO"
printf %s "$APPLE_APP_PASSWORD" | gh secret set APPLE_APP_PASSWORD  --repo "$REPO"
printf %s "$TEAM_ID"            | gh secret set APPLE_TEAM_ID       --repo "$REPO"
gh secret list --repo "$REPO"

# 5. Run the latest Mac build again, this time signed
say "Starting a signed + notarized build"
RUN=$(gh run list --repo "$REPO" --workflow macos.yml --branch "$BRANCH" --limit 1 --json databaseId --jq '.[0].databaseId')
if [ -n "$RUN" ] && gh run rerun "$RUN" --repo "$REPO"; then
  echo "Build started."
else
  echo "Could not restart the build automatically - the secrets are saved, just tell Claude."
fi
echo
echo "Done. Follow it at https://github.com/$REPO/actions"
echo "You can now delete the two .p12 files from your Desktop."
