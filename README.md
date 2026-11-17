# Paisley

Psychedelic synthesiser plugin (VST3 + Standalone), JUCE 8. See [DESIGN.md](DESIGN.md).

## Build

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release      # fetches JUCE 8.0.9 if ./JUCE is absent
cmake --build build --config Release
```

Output: `build/Paisley_artefacts/Release/VST3/Paisley.vst3` and `.../Standalone/`.
On Linux you need the usual JUCE packages (libasound2-dev, libx11-dev, libxrandr-dev,
libxinerama-dev, libxcursor-dev, libfreetype-dev, libfontconfig1-dev, libgl1-mesa-dev).

## Render / test tool

```sh
cmake -B build -DPAISLEY_BUILD_TOOLS=ON && cmake --build build --target PaisleyRender
./build/PaisleyRender_artefacts/Release/PaisleyRender out/                       # WAVs + theme PNGs
./build/PaisleyRender_artefacts/Release/PaisleyRender out/ --calibrate Source/FactoryPresetLevels.h
```

## Installers (built by GitHub Actions)

Every push to `main` builds:
- **macOS**: `Paisley-<ver>-macOS.dmg` with a one-click `Install Paisley.pkg` (VST3 + AU + app,
  universal Intel/Apple Silicon, macOS 10.13+).
- **Windows**: `Paisley-<ver>-Windows-Setup.exe` (VST3 into `C:\Program Files\Common Files\VST3`
  + standalone app; static runtime, nothing else to install).

Download them from the run's *Artifacts*. Pushing a tag like `v0.1.0` also publishes both to a
GitHub Release page, which is the easiest link to send to friends.
Neither installer is signed yet; see "READ ME FIRST.txt" in the DMG. Windows SmartScreen shows
"More info > Run anyway" for unsigned installers.
