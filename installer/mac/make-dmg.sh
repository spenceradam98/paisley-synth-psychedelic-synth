#!/bin/bash
# Builds Paisley-<version>-macOS.dmg containing a one-click installer package.
# Usage: installer/mac/make-dmg.sh <artefacts dir (…/Paisley_artefacts/Release)> <version> <out dir>
set -euo pipefail
ART="$1"; VER="$2"; OUT="$3"
WORK="$(mktemp -d)"
mkdir -p "$WORK/vst3" "$WORK/au" "$WORK/app" "$WORK/dmg" "$OUT"

# Ad-hoc sign so the bundles run on Apple Silicon and pass basic checks
for b in "$ART/VST3/Paisley.vst3" "$ART/AU/Paisley.component" "$ART/Standalone/Paisley.app"; do
  codesign --force --deep -s - "$b"
done
cp -R "$ART/VST3/Paisley.vst3"      "$WORK/vst3/"
cp -R "$ART/AU/Paisley.component"   "$WORK/au/"
cp -R "$ART/Standalone/Paisley.app" "$WORK/app/"

pkgbuild --root "$WORK/vst3" --install-location "/Library/Audio/Plug-Ins/VST3" \
         --identifier com.paisleyaudio.paisley.vst3 --version "$VER" "$WORK/vst3.pkg"
pkgbuild --root "$WORK/au"   --install-location "/Library/Audio/Plug-Ins/Components" \
         --identifier com.paisleyaudio.paisley.au --version "$VER" \
         --scripts "$(dirname "$0")/scripts" "$WORK/au.pkg"
pkgbuild --root "$WORK/app"  --install-location "/Applications" \
         --identifier com.paisleyaudio.paisley.app --version "$VER" "$WORK/app.pkg"

sed "s/@VERSION@/$VER/g" "$(dirname "$0")/distribution.xml" > "$WORK/distribution.xml"
productbuild --distribution "$WORK/distribution.xml" --package-path "$WORK" \
             --resources "$(dirname "$0")/resources" "$WORK/dmg/Install Paisley.pkg"
cp "$(dirname "$0")/READ ME FIRST.txt" "$WORK/dmg/"

hdiutil create -volname "Paisley $VER" -srcfolder "$WORK/dmg" -ov -format UDZO "$OUT/Paisley-$VER-macOS.dmg"
echo "Created $OUT/Paisley-$VER-macOS.dmg"
