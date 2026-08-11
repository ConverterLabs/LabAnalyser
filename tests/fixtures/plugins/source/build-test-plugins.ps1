param([string]$BuildRoot = '', [string]$Msys2Root = 'C:\msys64\mingw64')
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../../../..')).Path
$mingw = Join-Path $Msys2Root 'bin'
$msysUsrBin = Join-Path (Split-Path -Parent $Msys2Root) 'usr\bin'
$env:Path = "$mingw;$msysUsrBin;" + $env:Path
$qmake = @(
  "$mingw\qmake6.exe",
  "$mingw\qmake-qt6.exe",
  (Join-Path $Msys2Root 'lib\qt6\bin\qmake6.exe'),
  (Join-Path $Msys2Root 'lib\qt6\bin\qmake-qt6.exe'),
  (Join-Path $Msys2Root 'share\qt6\bin\qmake6.exe'),
  (Join-Path $Msys2Root 'share\qt6\bin\qmake-qt6.exe'),
  "$mingw\qmake.exe"
) | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf } | Select-Object -First 1
if (-not $qmake) { throw 'qmake executable was not found in known MSYS2 Qt6 locations.' }
if (!$BuildRoot) { $BuildRoot = Join-Path $repo 'build\test-plugins' }
$projects = @('compatible/CompatiblePlugin.pro','wrong_iid/WrongIidPlugin.pro','qobject_only/QObjectOnlyPlugin.pro','member_owned/MemberOwnedInterfacePlugin.pro','heap_owned/HeapOwnedInterfacePlugin.pro')
foreach ($project in $projects) {
  $name = [IO.Path]::GetFileNameWithoutExtension($project)
  $dir = Join-Path $BuildRoot $name; New-Item -ItemType Directory -Force $dir | Out-Null
  Push-Location $dir
  try { & $qmake (Join-Path $PSScriptRoot $project) -spec win32-g++ 'CONFIG+=release' 'CONFIG-=debug'; if($LASTEXITCODE){exit $LASTEXITCODE}; & "$mingw\mingw32-make.exe" -j4; if($LASTEXITCODE){exit $LASTEXITCODE} }
  finally { Pop-Location }
}
$verifier = Join-Path $BuildRoot 'PluginFixtureVerifier'; New-Item -ItemType Directory -Force $verifier | Out-Null
Push-Location $verifier
try { & $qmake (Join-Path $PSScriptRoot 'PluginFixtureVerifier.pro') -spec win32-g++ 'CONFIG+=release' 'CONFIG-=debug'; if($LASTEXITCODE){exit $LASTEXITCODE}; & "$mingw\mingw32-make.exe" -j4; if($LASTEXITCODE){exit $LASTEXITCODE}; & .\release\PluginFixtureVerifier.exe (Join-Path $BuildRoot 'CompatiblePlugin\release\CompatiblePlugin.dll') (Join-Path $BuildRoot 'WrongIidPlugin\release\WrongIidPlugin.dll') (Join-Path $BuildRoot 'QObjectOnlyPlugin\release\QObjectOnlyPlugin.dll'); exit $LASTEXITCODE }
finally { Pop-Location }
