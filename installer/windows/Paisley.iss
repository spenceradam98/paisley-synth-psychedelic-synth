; Inno Setup script: one-click Windows installer for Paisley (VST3 + standalone).
; Built by CI:  iscc /DMyVersion=0.1.0 /DArtefacts=<...\Paisley_artefacts\Release> /O<outdir> Paisley.iss

#ifndef MyVersion
  #define MyVersion "0.1.0"
#endif
#ifndef Artefacts
  #define Artefacts "..\..\build\Paisley_artefacts\Release"
#endif

[Setup]
AppId={{6C1E2A5B-8F0D-4B7E-9A3C-5D2F1E0B7A41}
AppName=Paisley
AppVersion={#MyVersion}
AppPublisher=Paisley Audio
DefaultDirName={autopf}\Paisley Audio\Paisley
DefaultGroupName=Paisley
DisableProgramGroupPage=yes
OutputBaseFilename=Paisley-{#MyVersion}-Windows-Setup
Compression=lzma2
SolidCompression=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
UninstallDisplayName=Paisley

[Types]
Name: "full"; Description: "Full installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 plug-in (Ableton, FL Studio, Reaper, Cubase, Bitwig, Studio One…)"; Types: full custom; Flags: fixed
Name: "app";  Description: "Standalone app"; Types: full

[Files]
Source: "{#Artefacts}\VST3\Paisley.vst3\*"; DestDir: "{commoncf64}\VST3\Paisley.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Artefacts}\Standalone\Paisley.exe"; DestDir: "{app}"; Components: app; Flags: ignoreversion

[Icons]
Name: "{group}\Paisley"; Filename: "{app}\Paisley.exe"; Components: app
Name: "{autodesktop}\Paisley"; Filename: "{app}\Paisley.exe"; Components: app; Tasks: desktopicon

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut for the standalone app"; Components: app; Flags: unchecked

[Run]
Filename: "{app}\Paisley.exe"; Description: "Launch Paisley now"; Components: app; Flags: nowait postinstall skipifsilent unchecked

[UninstallDelete]
Type: filesandordirs; Name: "{commoncf64}\VST3\Paisley.vst3"
