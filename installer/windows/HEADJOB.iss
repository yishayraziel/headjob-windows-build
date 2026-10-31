#ifndef SourceBundle
 #error SourceBundle must point to HEADJOB.vst3
#endif
[Setup]
AppId={{83715AA5-E272-4C9F-A626-85051B62126A}
AppName=HEADJOB
AppVersion=0.3.0
AppPublisher=HEADJOB Audio
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir=..\..\dist
OutputBaseFilename=HEADJOB-0.3.0-Windows-x64-Setup
UninstallFilesDir={commonpf64}\HEADJOB\Uninstall
UninstallDisplayName=HEADJOB VST3
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
SetupLogging=yes
[Files]
Source: "{#SourceBundle}\*"; DestDir: "{app}\HEADJOB.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
