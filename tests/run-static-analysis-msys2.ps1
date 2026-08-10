#requires -Version 5.1
<#
.SYNOPSIS
Creates the local MSYS2 MINGW64 compiler-warning/static-analysis baseline.

.DESCRIPTION
Builds the existing qmake application in a dedicated directory with additional
GCC warnings, without -Werror. The report contains only own production source
diagnostics; qcustomplot, generated Qt sources, tests and build directories are
excluded from the reviewed result. clang-tidy/cppcheck availability is recorded
but no package is installed or updated.
#>

[CmdletBinding()]
param(
    [string]$Msys2Root = 'C:\msys64\mingw64',
    [string]$BuildRoot = '',
    [int]$Jobs = $env:NUMBER_OF_PROCESSORS,
    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path -Parent $PSScriptRoot

function Get-FullPath {
    param([Parameter(Mandatory)][string]$Path)

    if ([System.IO.Path]::IsPathRooted($Path)) { return [System.IO.Path]::GetFullPath($Path) }
    return [System.IO.Path]::GetFullPath((Join-Path $repositoryRoot $Path))
}

function Assert-File {
    param([Parameter(Mandatory)][string]$Path, [Parameter(Mandatory)][string]$Description)
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) { throw "$Description not found: $Path" }
}

function Invoke-CapturedNative {
    param([Parameter(Mandatory)][string]$Exe, [string[]]$Arguments = @())

    Write-Host "> $Exe $($Arguments -join ' ')"
    $savedErrorActionPreference = $ErrorActionPreference
    try {
        $ErrorActionPreference = 'Continue'
        $output = & $Exe @Arguments 2>&1
        $exitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $savedErrorActionPreference
    }
    return [PSCustomObject]@{ Output = @($output | ForEach-Object { $_.ToString() }); ExitCode = $exitCode }
}

function Is-OwnProductionDiagnostic {
    param([Parameter(Mandatory)][string]$Line)

    if ($Line -notmatch '(?<path>[^:\r\n]+\.(?:cpp|h)):\d+(?::\d+)?:\s+warning:') { return $false }
    $reportedPath = $Matches.path
    $path = if ([System.IO.Path]::IsPathRooted($reportedPath)) {
        [System.IO.Path]::GetFullPath($reportedPath)
    }
    else {
        [System.IO.Path]::GetFullPath((Join-Path $BuildRoot $reportedPath))
    }
    $repositoryPrefix = $repositoryRoot.TrimEnd('\') + '\'
    if (-not $path.StartsWith($repositoryPrefix, [System.StringComparison]::OrdinalIgnoreCase)) { return $false }
    $path = $path.Substring($repositoryPrefix.Length).Replace('\', '/')
    return -not (
        $path -match '(^|/)tests/' -or
        $path -match '(^|/)build/' -or
        $path -match 'qcustomplot\.(cpp|h)$' -or
        $path -match '(^|/)(moc_|qrc_|ui_)'
    )
}

$Msys2Root = Get-FullPath $Msys2Root
if ([string]::IsNullOrWhiteSpace($BuildRoot)) { $BuildRoot = Join-Path $repositoryRoot 'build\static-analysis-msys2-mingw64' }
$BuildRoot = Get-FullPath $BuildRoot
if ($Jobs -lt 1) { $Jobs = 1 }

$mingwBin = Join-Path $Msys2Root 'bin'
$msysUsrBin = Join-Path (Split-Path -Parent $Msys2Root) 'usr\bin'
$qmake = Join-Path $mingwBin 'qmake6.exe'
if (-not (Test-Path -LiteralPath $qmake -PathType Leaf)) { $qmake = Join-Path $mingwBin 'qmake.exe' }
$make = Join-Path $mingwBin 'mingw32-make.exe'
$projectFile = Join-Path $repositoryRoot 'StaticAnalysis.pro'
Assert-File -Path $qmake -Description 'qmake'
Assert-File -Path $make -Description 'mingw32-make'
Assert-File -Path $projectFile -Description 'application project file'

if ($Clean -and (Test-Path -LiteralPath $BuildRoot)) { Remove-Item -LiteralPath $BuildRoot -Recurse -Force }
New-Item -ItemType Directory -Force -Path $BuildRoot | Out-Null

$oldPath = $env:Path
try {
    $env:Path = "$mingwBin;$msysUsrBin;" + $env:Path
    $toolInventory = New-Object System.Collections.Generic.List[string]
    foreach ($tool in @('clang-tidy.exe', 'cppcheck.exe', 'clang++.exe', 'g++.exe')) {
        $command = Get-Command $tool -ErrorAction SilentlyContinue
        if ($null -eq $command) {
            $toolInventory.Add("${tool}: not found")
            continue
        }
        $version = (& $command.Source --version 2>&1 | Select-Object -First 1).ToString()
        $toolInventory.Add("${tool}: $($command.Source) | $version")
    }
    [System.IO.File]::WriteAllLines((Join-Path $BuildRoot 'tool-inventory.txt'), $toolInventory, (New-Object System.Text.UTF8Encoding($false)))

    Push-Location $BuildRoot
    try {
        $warningFlags = @(
            'QMAKE_CXXFLAGS+=-Wall',
            'QMAKE_CXXFLAGS+=-Wextra',
            'QMAKE_CXXFLAGS+=-Wpedantic',
            'QMAKE_CXXFLAGS+=-Wformat=2',
            'QMAKE_CXXFLAGS+=-Wshadow',
            "QMAKE_LIBDIR+=$($Msys2Root.Replace('\', '/'))/lib"
        )
        $qmakeResult = Invoke-CapturedNative -Exe $qmake -Arguments (@($projectFile, '-spec', 'win32-g++', 'CONFIG+=release', 'CONFIG-=debug') + $warningFlags)
        $makeResult = if ($qmakeResult.ExitCode -eq 0) { Invoke-CapturedNative -Exe $make -Arguments @("-j$Jobs") } else { [PSCustomObject]@{ Output = @(); ExitCode = -1 } }
        $allOutput = @($qmakeResult.Output + $makeResult.Output)
        [System.IO.File]::WriteAllLines((Join-Path $BuildRoot 'compiler-build.log'), $allOutput, (New-Object System.Text.UTF8Encoding($false)))
        $ownWarnings = @($allOutput | Where-Object { Is-OwnProductionDiagnostic -Line $_ } | Sort-Object -Unique)
        [System.IO.File]::WriteAllLines((Join-Path $BuildRoot 'own-production-warnings.txt'), $ownWarnings, (New-Object System.Text.UTF8Encoding($false)))

        if ($qmakeResult.ExitCode -ne 0) { throw "qmake warning baseline failed with exit code $($qmakeResult.ExitCode)." }
        if ($makeResult.ExitCode -ne 0) { throw "warning baseline build failed with exit code $($makeResult.ExitCode)." }
    }
    finally {
        Pop-Location
    }
}
finally {
    $env:Path = $oldPath
}

Write-Host "Static-analysis baseline completed: $BuildRoot"
