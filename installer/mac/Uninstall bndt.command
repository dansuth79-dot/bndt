#!/bin/bash
# Removes bndt by Switchkicker. Your saved presets in ~/Documents/Switchkicker/bndt are kept.
echo
echo "  Uninstall bndt by Switchkicker"
echo "  ------------------------------"
echo "  This removes:"
echo "    /Library/Audio/Plug-Ins/VST3/bndt.vst3"
echo "    /Library/Audio/Plug-Ins/Components/bndt.component"
echo "    /Applications/bndt.app"
echo "  Your saved presets are kept."
echo
read -r -p "  Continue? (y/n) " answer
case "$answer" in [yY]*) ;; *) echo "  Nothing removed."; exit 0 ;; esac

sudo rm -rf "/Library/Audio/Plug-Ins/VST3/bndt.vst3" \
            "/Library/Audio/Plug-Ins/Components/bndt.component" \
            "/Applications/bndt.app"
for id in com.switchkicker.bndt.vst3 com.switchkicker.bndt.au com.switchkicker.bndt.app; do
    sudo pkgutil --forget "$id" >/dev/null 2>&1 || true
done
killall -9 AudioComponentRegistrar >/dev/null 2>&1 || true
echo
echo "  bndt has been removed. You can close this window."
