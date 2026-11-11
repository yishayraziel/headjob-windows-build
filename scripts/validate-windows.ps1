$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$logs = Join-Path $root 'outputs\windows-validation'
New-Item -ItemType Directory -Force $logs | Out-Null
$installer = Join-Path $root 'dist\HEADJOB-0.7.0-Windows-x64-Setup.exe'
$installedBundle = Join-Path $env:CommonProgramFiles 'VST3\HEADJOB.vst3'
$binary = Join-Path $installedBundle 'Contents\x86_64-win\HEADJOB.vst3'
if (Test-Path $installedBundle) { throw 'Validation requires a clean disposable Windows machine.' }
$presets = Join-Path $env:APPDATA 'HEADJOB\Presets'
New-Item -ItemType Directory -Force $presets | Out-Null
$marker = Join-Path $presets ('installer-test-' + [guid]::NewGuid() + '.headjob')
Set-Content -LiteralPath $marker -Value 'installer preservation check'
$install = Start-Process -FilePath $installer -ArgumentList @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART',"/LOG=`"$logs\install.log`"") -Wait -PassThru
if ($install.ExitCode -ne 0 -or -not (Test-Path $binary)) { throw 'Installation failed.' }
$bytes = [IO.File]::ReadAllBytes($binary)
$pe = [BitConverter]::ToInt32($bytes, 0x3c)
if ([BitConverter]::ToUInt32($bytes,$pe) -ne 0x4550 -or [BitConverter]::ToUInt16($bytes,$pe+4) -ne 0x8664) { throw 'Installed plugin is not Windows x64 PE.' }
$toolsDir = Join-Path $root 'validation-tools'
New-Item -ItemType Directory -Force $toolsDir | Out-Null
Invoke-WebRequest 'https://github.com/Tracktion/pluginval/releases/download/v1.0.4/pluginval_Windows.zip' -OutFile "$toolsDir\pluginval.zip"
Expand-Archive "$toolsDir\pluginval.zip" "$toolsDir\pluginval" -Force
$validator = Get-ChildItem "$toolsDir\pluginval" -Filter pluginval.exe -Recurse | Select-Object -First 1
if (-not $validator) { throw 'Official pluginval download did not contain pluginval.exe.' }
$validation = Start-Process -FilePath $validator.FullName -ArgumentList @('--strictness-level','5','--validate',"`"$installedBundle`"",'--output-dir',"`"$logs`"") -Wait -PassThru -RedirectStandardOutput "$logs\pluginval-stdout.txt" -RedirectStandardError "$logs\pluginval-stderr.txt"
Get-Content "$logs\pluginval-stdout.txt"
if ($validation.ExitCode -ne 0) { throw "VST3 host validation failed with exit code $($validation.ExitCode)." }
$uninstaller = Join-Path $env:ProgramFiles 'HEADJOB\Uninstall\unins000.exe'
$uninstall = Start-Process -FilePath $uninstaller -ArgumentList @('/VERYSILENT','/SUPPRESSMSGBOXES','/NORESTART',"/LOG=`"$logs\uninstall.log`"") -Wait -PassThru
if ($uninstall.ExitCode -ne 0 -or (Test-Path $binary)) { throw 'Uninstallation failed.' }
if (-not (Test-Path $marker)) { throw 'Uninstallation removed user presets.' }
Remove-Item -LiteralPath $marker
'PASS: Windows x64 binary, silent install, pluginval strictness 5, uninstall, user preset preservation.' | Tee-Object "$logs\result.txt"
Get-FileHash $installer -Algorithm SHA256 | Format-List | Out-File "$logs\installer-sha256.txt"
