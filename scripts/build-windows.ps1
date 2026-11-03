$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root
New-Item -ItemType Directory -Force outputs | Out-Null
cmake -S . -B build-windows -G 'Visual Studio 17 2022' -A x64
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed' }
cmake --build build-windows --config Release --target HEADJOB_VST3 HEADJOB_Standalone HEADJOBTests --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Compilation failed' }
ctest --test-dir build-windows -C Release --output-on-failure 2>&1 | Tee-Object outputs/tests.txt
if ($LASTEXITCODE -ne 0) { throw 'DSP, gain or UI tests failed' }
& './build-windows/HEADJOBTests_artefacts/Release/HEADJOBTests.exe' --preview-demo "$root/outputs/HEADJOB-Windows-Preview.png"
if ($LASTEXITCODE -ne 0) { throw 'Editor preview failed' }
$bundle = "$root/build-windows/HEADJOB_artefacts/Release/VST3/HEADJOB.vst3"
$iscc = "${env:ProgramFiles(x86)}/Inno Setup 6/ISCC.exe"
if (-not (Test-Path $iscc)) { throw 'Inno Setup 6 not found' }
& $iscc "/DSourceBundle=$bundle" "$root/installer/windows/HEADJOB.iss"
if ($LASTEXITCODE -ne 0) { throw 'Installer build failed' }
