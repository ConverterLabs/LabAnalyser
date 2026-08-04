param([string]$BuildRoot = '')
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '../../../..')).Path
$mingw = 'C:\msys64\mingw64\bin'
$env:Path = "$mingw;C:\msys64\usr\bin;" + $env:Path
if (!$BuildRoot) { $BuildRoot = Join-Path $repo 'build\test-plugins' }
$projects = @('compatible/CompatiblePlugin.pro','wrong_iid/WrongIidPlugin.pro','qobject_only/QObjectOnlyPlugin.pro')
foreach ($project in $projects) {
  $name = [IO.Path]::GetFileNameWithoutExtension($project)
  $dir = Join-Path $BuildRoot $name; New-Item -ItemType Directory -Force $dir | Out-Null
  Push-Location $dir
  try { & "$mingw\qmake6.exe" (Join-Path $PSScriptRoot $project) -spec win32-g++ 'CONFIG+=release' 'CONFIG-=debug'; if($LASTEXITCODE){exit $LASTEXITCODE}; & "$mingw\mingw32-make.exe" -j4; if($LASTEXITCODE){exit $LASTEXITCODE} }
  finally { Pop-Location }
}
$verifier = Join-Path $BuildRoot 'PluginFixtureVerifier'; New-Item -ItemType Directory -Force $verifier | Out-Null
Push-Location $verifier
try { & "$mingw\qmake6.exe" (Join-Path $PSScriptRoot 'PluginFixtureVerifier.pro') -spec win32-g++ 'CONFIG+=release' 'CONFIG-=debug'; if($LASTEXITCODE){exit $LASTEXITCODE}; & "$mingw\mingw32-make.exe" -j4; if($LASTEXITCODE){exit $LASTEXITCODE}; & .\release\PluginFixtureVerifier.exe (Join-Path $BuildRoot 'CompatiblePlugin\release\CompatiblePlugin.dll') (Join-Path $BuildRoot 'WrongIidPlugin\release\WrongIidPlugin.dll') (Join-Path $BuildRoot 'QObjectOnlyPlugin\release\QObjectOnlyPlugin.dll'); exit $LASTEXITCODE }
finally { Pop-Location }
