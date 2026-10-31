#!/usr/bin/env bash
# Builds the macOS installer for bndt by Switchkicker: a .pkg (VST3 / AU / Standalone choices) inside a .dmg.
# Runs on macOS (GitHub Actions does this for you).
#
#   installer/mac/build_pkg.sh <artefacts dir> <version> <output dir>
#   e.g. installer/mac/build_pkg.sh build/bndt_artefacts/Release 1.0.0 dist
#
# Optional signing (leave unset for an ad-hoc signed build, which is fine for a gift):
#   SIGN_APP_IDENTITY        "Developer ID Application: Your Name (TEAMID)"
#   SIGN_INSTALLER_IDENTITY  "Developer ID Installer: Your Name (TEAMID)"
#   NOTARY_APPLE_ID, NOTARY_TEAM_ID, NOTARY_PASSWORD (app-specific password) to notarise
set -euo pipefail

ART="${1:?usage: build_pkg.sh <artefacts dir> <version> <output dir>}"
VERSION="${2:?version}"
OUT="${3:?output dir}"
HERE="$(cd "$(dirname "$0")" && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$OUT" "$WORK/pkgs" "$WORK/resources"
OUT="$(cd "$OUT" && pwd)"

APP_SIGN="${SIGN_APP_IDENTITY:-}"
PKG_SIGN="${SIGN_INSTALLER_IDENTITY:-}"

echo "== Staging bundles"
mkdir -p "$WORK/root-vst3" "$WORK/root-au" "$WORK/root-app"
cp -R "$ART/VST3/bndt.vst3" "$WORK/root-vst3/"
cp -R "$ART/AU/bndt.component" "$WORK/root-au/"
cp -R "$ART/Standalone/bndt.app" "$WORK/root-app/"
xattr -cr "$WORK"/root-* || true

echo "== Code signing (${APP_SIGN:-ad-hoc})"
for b in "$WORK/root-vst3/bndt.vst3" "$WORK/root-au/bndt.component" "$WORK/root-app/bndt.app"; do
    if [ -n "$APP_SIGN" ]; then
        ent=()
        case "$b" in *.app) ent=(--entitlements "$HERE/app.entitlements") ;; esac
        codesign --force --deep --options runtime --timestamp "${ent[@]+"${ent[@]}"}" --sign "$APP_SIGN" "$b"
    else
        codesign --force --deep --sign - "$b"
    fi
    codesign --verify --deep --strict "$b"
done

component() {   # name  identifier  install-location  [scripts dir]
    local name="$1" ident="$2" loc="$3" scripts="${4:-}"
    local plist="$WORK/$name.plist"
    pkgbuild --analyze --root "$WORK/root-$name" "$plist" >/dev/null
    # always install to the standard folder, even if a copy was moved elsewhere
    local i=0
    while /usr/libexec/PlistBuddy -c "Print :$i" "$plist" >/dev/null 2>&1; do
        /usr/libexec/PlistBuddy -c "Set :$i:BundleIsRelocatable false" "$plist"
        i=$((i + 1))
    done
    local args=(--root "$WORK/root-$name" --component-plist "$plist" --identifier "$ident"
                --version "$VERSION" --install-location "$loc" --ownership recommended)
    if [ -n "$scripts" ]; then args+=(--scripts "$scripts"); fi
    pkgbuild "${args[@]}" "$WORK/pkgs/$name.pkg"
}

echo "== Component packages"
component vst3 com.switchkicker.bndt.vst3 /Library/Audio/Plug-Ins/VST3
component au   com.switchkicker.bndt.au   /Library/Audio/Plug-Ins/Components "$HERE/scripts-au"
component app  com.switchkicker.bndt.app  /Applications

echo "== Installer resources"
STYLE="$(tr '\n' ' ' < "$HERE/resources/_style.css")"
for page in welcome readme conclusion; do
    python3 - "$HERE/resources/$page.html" "$WORK/resources/$page.html" "$VERSION" "$STYLE" <<'PY'
import sys
src, dst, version, style = sys.argv[1:5]
html = open(src, encoding="utf-8").read().replace("/*STYLE*/", style).replace("__VERSION__", version)
open(dst, "w", encoding="utf-8").write(html)
PY
done
# 1x + 2x side art in one TIFF so it's sharp on Retina screens
tiffutil -cathidpicheck "$HERE/art/background.png" "$HERE/art/background@2x.png" -out "$WORK/resources/background.tiff"
sed "s/__VERSION__/$VERSION/g" "$HERE/distribution.xml" > "$WORK/distribution.xml"

echo "== Product archive"
PKG="$WORK/Install bndt.pkg"
PB_ARGS=(--distribution "$WORK/distribution.xml" --package-path "$WORK/pkgs" --resources "$WORK/resources")
if [ -n "$PKG_SIGN" ]; then PB_ARGS+=(--sign "$PKG_SIGN" --timestamp); fi
productbuild "${PB_ARGS[@]}" "$PKG"

if [ -n "$PKG_SIGN" ] && [ -n "${NOTARY_APPLE_ID:-}" ]; then
    echo "== Notarising"
    xcrun notarytool submit "$PKG" --apple-id "$NOTARY_APPLE_ID" --team-id "$NOTARY_TEAM_ID" \
                                    --password "$NOTARY_PASSWORD" --wait
    xcrun stapler staple "$PKG"
    SIGNED=1
else
    SIGNED=0
fi

echo "== Disk image"
DMGROOT="$WORK/dmg"
mkdir -p "$DMGROOT"
cp "$PKG" "$DMGROOT/"
cp "$HERE/Uninstall bndt.command" "$DMGROOT/"
chmod +x "$DMGROOT/Uninstall bndt.command"
if [ "$SIGNED" = "1" ]; then
    printf 'Double-click "Install bndt.pkg" and follow the steps.\n' > "$DMGROOT/Read Me First.txt"
else
    cat > "$DMGROOT/Read Me First.txt" <<'TXT'
Installing bndt by Switchkicker
===============================

1. Double-click "Install bndt.pkg".

2. This installer isn't registered with Apple, so macOS may say it
   "can't be opened" or "cannot verify the developer". That's expected.
   - macOS 15 (Sequoia) and later: click Done, open System Settings >
     Privacy & Security, scroll down and click "Open Anyway" next to
     "Install bndt.pkg", then confirm.
   - macOS 14 and earlier: right-click (or Control-click)
     "Install bndt.pkg", choose Open, then Open again.

3. Follow the steps. Click Customize on the Installation Type step if
   you only want some of VST3 / Audio Unit / Standalone app.

4. Open your DAW and rescan plug-ins. In Logic it's under
   AU Instruments > Switchkicker > bndt.

To remove bndt later, double-click "Uninstall bndt.command".
(Same "Open Anyway" step applies the first time.)
TXT
fi

DMG="$OUT/bndt-$VERSION-macOS.dmg"
rm -f "$DMG"
hdiutil create -volname "bndt by Switchkicker" -srcfolder "$DMGROOT" -fs HFS+ -format UDZO -ov "$DMG" >/dev/null
if [ -n "$APP_SIGN" ]; then codesign --force --sign "$APP_SIGN" --timestamp "$DMG"; fi
cp "$PKG" "$OUT/Install bndt $VERSION.pkg"

echo
echo "Done:"
echo "  $DMG"
echo "  $OUT/Install bndt $VERSION.pkg"
pkgutil --check-signature "$OUT/Install bndt $VERSION.pkg" || true
