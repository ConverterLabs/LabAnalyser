#requires -Version 5.1
<#
.SYNOPSIS
Builds LabAnalyser.pro with the MSYS2 MINGW64 Qt toolchain.

.EXAMPLE
.\build-msys2.ps1

.EXAMPLE
.\build-msys2.ps1 -Clean -Deploy

.EXAMPLE
.\build-msys2.ps1 -MatOutDir C:\path\to\matOut -Configuration debug
#>

[CmdletBinding()]
param(
    [ValidateSet('release', 'debug')]
    [string]$Configuration = 'release',

    [string]$Msys2Root = 'C:\msys64\mingw64',
    [string]$ProjectFile = '',
    [string]$BuildDir = '',
    [string]$MatOutDir = '',
    [string]$Hdf5LibDir = '',

    [int]$Jobs = $env:NUMBER_OF_PROCESSORS,

    [switch]$Clean,
    [switch]$Deploy,
    [switch]$SkipExternalChecks
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ScriptRootPath = if ([string]::IsNullOrWhiteSpace($PSScriptRoot)) {
    (Get-Location).Path
} else {
    $PSScriptRoot
}

function Get-FullPath {
    param([Parameter(Mandatory)][string]$Path)

    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }

    return [System.IO.Path]::GetFullPath((Join-Path $ScriptRootPath $Path))
}

function Convert-ToQMakePath {
    param([Parameter(Mandatory)][string]$Path)

    return (Get-FullPath $Path).Replace('\', '/')
}

function Resolve-Tool {
    param(
        [Parameter(Mandatory)][string]$Name,
        [Parameter(Mandatory)][string[]]$Candidates
    )

    foreach ($candidate in $Candidates) {
        if (Test-Path -LiteralPath $candidate -PathType Leaf) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw "$Name not found. Checked: $($Candidates -join ', ')"
}

function Assert-Directory {
    param(
        [Parameter(Mandatory)][string]$Path,
        [Parameter(Mandatory)][string]$Description
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
        throw "$Description not found: $Path"
    }
}

function Assert-File {
    param(
        [Parameter(Mandatory)][string]$Path,
        [Parameter(Mandatory)][string]$Description
    )

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Description not found: $Path"
    }
}

function Test-MatOutDirectory {
    param([Parameter(Mandatory)][string]$Path)

    $fullPath = Get-FullPath $Path
    foreach ($file in @('matOut.h', 'toolsMatlab.c', 'toolsMisc.c')) {
        if (-not (Test-Path -LiteralPath (Join-Path $fullPath $file) -PathType Leaf)) {
            return $false
        }
    }

    return $true
}

function Find-DefaultMatOutDir {
    $candidates = New-Object System.Collections.Generic.List[string]
    $candidates.Add((Join-Path $ScriptRootPath '..\matOut'))

    $hintFiles = @(
        (Join-Path $ScriptRootPath 'LabAnalyser.pro.user'),
        (Join-Path $ScriptRootPath 'LabAnalyser\build\Desktop_Qt_6_9_2_shared_MinGW_w64_MINGW64_MSYS2-Release\Makefile.Release'),
        (Join-Path $ScriptRootPath 'LabAnalyser\build\Desktop_Qt_6_9_2_shared_MinGW_w64_MINGW64_MSYS2-Release\Makefile.Debug')
    )

    foreach ($hintFile in $hintFiles) {
        if (-not (Test-Path -LiteralPath $hintFile -PathType Leaf)) {
            continue
        }

        $matches = Select-String -Path $hintFile -Pattern '(?i)[A-Z]:[/\\](?:[^\s/\\]+[/\\])*matOut(?=\s|$)' -AllMatches
        foreach ($result in $matches) {
            foreach ($match in $result.Matches) {
                $candidates.Add($match.Value.Replace('/', '\'))
            }
        }
    }

    $seen = @{}
    foreach ($candidate in $candidates) {
        $fullPath = Get-FullPath $candidate
        $key = $fullPath.ToLowerInvariant()
        if ($seen.ContainsKey($key)) {
            continue
        }

        $seen[$key] = $true
        if (Test-MatOutDirectory -Path $fullPath) {
            return $fullPath
        }
    }

    return (Get-FullPath $candidates[0])
}

function Assert-CleanTargetIsSafe {
    param([Parameter(Mandatory)][string]$Path)

    $projectRoot = (Get-FullPath $ScriptRootPath).TrimEnd('\') + '\'
    $target = (Get-FullPath $Path).TrimEnd('\') + '\'

    if (-not $target.StartsWith($projectRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to clean a directory outside the project root: $Path"
    }
}

function Invoke-Native {
    param(
        [Parameter(Mandatory)][string]$Exe,
        [string[]]$Arguments = @()
    )

    Write-Host "> $Exe $($Arguments -join ' ')"
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE`: $Exe"
    }
}

$Msys2Root = Get-FullPath $Msys2Root
if ([string]::IsNullOrWhiteSpace($ProjectFile)) {
    $ProjectFile = Join-Path $ScriptRootPath 'LabAnalyser.pro'
}
$ProjectFile = Get-FullPath $ProjectFile
if ([string]::IsNullOrWhiteSpace($MatOutDir)) {
    $MatOutDir = Find-DefaultMatOutDir
}
$MatOutDir = Get-FullPath $MatOutDir
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $ScriptRootPath "build\msys2-mingw64-$Configuration"
}
$BuildDir = Get-FullPath $BuildDir
if ([string]::IsNullOrWhiteSpace($Hdf5LibDir)) {
    $Hdf5LibDir = Join-Path $Msys2Root 'lib'
}
$Hdf5LibDir = Get-FullPath $Hdf5LibDir

if ($Jobs -lt 1) {
    $Jobs = 1
}

$mingwBin = Join-Path $Msys2Root 'bin'
$msysUsrBin = Join-Path (Split-Path -Parent $Msys2Root) 'usr\bin'

Assert-File -Path $ProjectFile -Description 'Qt project file'
Assert-Directory -Path $Msys2Root -Description 'MSYS2 MINGW64 root'
Assert-Directory -Path $mingwBin -Description 'MSYS2 MINGW64 bin directory'

$qmake = Resolve-Tool -Name 'qmake' -Candidates @(
    (Join-Path $mingwBin 'qmake6.exe'),
    (Join-Path $mingwBin 'qmake-qt6.exe'),
    (Join-Path $mingwBin 'qmake.exe')
)
$make = Resolve-Tool -Name 'mingw32-make' -Candidates @(
    (Join-Path $mingwBin 'mingw32-make.exe')
)
$windeployqt = $null
if ($Deploy) {
    $windeployqt = Resolve-Tool -Name 'windeployqt' -Candidates @(
        (Join-Path $mingwBin 'windeployqt6.exe'),
        (Join-Path $mingwBin 'windeployqt-qt6.exe'),
        (Join-Path $mingwBin 'windeployqt.exe')
    )
}

if (-not $SkipExternalChecks) {
    Assert-Directory -Path $MatOutDir -Description 'matOut include directory'
    foreach ($file in @('matOut.h', 'toolsMatlab.c', 'toolsMisc.c')) {
        Assert-File -Path (Join-Path $MatOutDir $file) -Description "matOut file $file"
    }

    Assert-Directory -Path $Hdf5LibDir -Description 'HDF5/FFTW library directory'
    foreach ($file in @('libhdf5.dll.a', 'libfftw3.dll.a')) {
        Assert-File -Path (Join-Path $Hdf5LibDir $file) -Description "import library $file"
    }

    Assert-Directory -Path (Join-Path $Msys2Root 'include\highfive') -Description 'HighFive include directory'
}

if ($Clean -and (Test-Path -LiteralPath $BuildDir)) {
    Assert-CleanTargetIsSafe -Path $BuildDir
    Remove-Item -LiteralPath $BuildDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

$oldPath = $env:Path
$oldMSYSTEM = $env:MSYSTEM
$oldCHERE = $env:CHERE_INVOKING

try {
    $pathParts = @($mingwBin)
    if (Test-Path -LiteralPath $msysUsrBin -PathType Container) {
        $pathParts += $msysUsrBin
    }
    $env:Path = ($pathParts + $env:Path) -join ';'
    $env:MSYSTEM = 'MINGW64'
    $env:CHERE_INVOKING = '1'

    $configToRemove = if ($Configuration -eq 'release') { 'debug' } else { 'release' }
    $qmakeArgs = @(
        (Convert-ToQMakePath $ProjectFile),
        '-spec',
        'win32-g++',
        "CONFIG+=$Configuration",
        "CONFIG-=$configToRemove",
        "INCLUDEPATH+=$(Convert-ToQMakePath $MatOutDir)",
        "QMAKE_LIBDIR+=$(Convert-ToQMakePath $Hdf5LibDir)"
    )

    Push-Location $BuildDir
    try {
        Invoke-Native -Exe $qmake -Arguments $qmakeArgs
        Invoke-Native -Exe $make -Arguments @("-j$Jobs")
    }
    finally {
        Pop-Location
    }

    $exeDir = Join-Path $BuildDir $Configuration
    $exePath = Join-Path $exeDir 'LabAnalyser.exe'
    Assert-File -Path $exePath -Description 'build output'

    if ($Deploy) {
        Invoke-Native -Exe $windeployqt -Arguments @("--$Configuration", '--compiler-runtime', (Convert-ToQMakePath $exePath))

        foreach ($pattern in @('libhdf5*.dll', 'libfftw3*.dll')) {
            Get-ChildItem -Path $mingwBin -Filter $pattern | ForEach-Object {
                Copy-Item -LiteralPath $_.FullName -Destination $exeDir -Force
            }
        }
    }

    Write-Host ''
    Write-Host "Build succeeded: $exePath"
}
finally {
    $env:Path = $oldPath
    $env:MSYSTEM = $oldMSYSTEM
    $env:CHERE_INVOKING = $oldCHERE
}
