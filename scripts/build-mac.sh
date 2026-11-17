#!/bin/bash
# Build Paisley on your own Mac (needs Xcode 13+ and CMake: `brew install cmake`).
set -e
cd "$(dirname "$0")/.."
cmake -B build-mac -G Xcode
cmake --build build-mac --config Release --target Paisley_VST3 Paisley_AU Paisley_Standalone -- -quiet
mkdir -p ~/Library/Audio/Plug-Ins/VST3 ~/Library/Audio/Plug-Ins/Components
cp -R build-mac/Paisley_artefacts/Release/VST3/Paisley.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R build-mac/Paisley_artefacts/Release/AU/Paisley.component ~/Library/Audio/Plug-Ins/Components/
echo "Installed. Standalone app: build-mac/Paisley_artefacts/Release/Standalone/Paisley.app"
