#ifndef SourceBundle
 #error SourceBundle must point to HEADJOB.vst3
#endif
[Setup]
AppId={{DC16BE74-B6FB-4C5B-952E-853B5C562F23}
AppName=HEADJOB Refined
AppVersion=0.5.0
AppPublisher=HEADJOB Audio
DefaultDirName={commoncf64}\VST3
DisableDirPage=yes
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0
OutputDir=..\..\dist
OutputBaseFilename=HEADJOB-Refined-0.5.0-Windows-x64-Setup
UninstallFilesDir={commonpf64}\HEADJOB Refined\Uninstall
UninstallDisplayName=HEADJOB Refined VST3
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
CloseApplications=yes
RestartApplications=no
SetupLogging=yes
[Files]
Source: "{#SourceBundle}\*"; DestDir: "{app}\HEADJOB Refined.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
