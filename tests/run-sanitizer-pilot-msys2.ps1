#requires -Version 5.1
<#
.SYNOPSIS
Builds and runs the focused MSYS2 MINGW64 AddressSanitizer/UBSan pilot.

.DESCRIPTION
The pilot deliberately covers only DataManagement, XML/Parameter, MAT/HDF5,
Remote Control and Plugin Loader contracts. It never modifies production
targets. A missing GCC sanitizer runtime is a hard, explicit preflight failure
rather than a silently uninstrumented test run.
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

    if ([System.IO.Path]::IsPathRooted($Path)) {
        return [System.IO.Path]::GetFullPath($Path)
    }

    return [System.IO.Path]::GetFullPath((Join-Path $repositoryRoot $Path))
}

function Assert-File {
    param([Parameter(Mandatory)][string]$Path, [Parameter(Mandatory)][string]$Description)

    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "$Description not found: $Path"
    }
}

function Invoke-Native {
    param([Parameter(Mandatory)][string]$Exe, [string[]]$Arguments = @())

    Write-Host "> $Exe $($Arguments -join ' ')"
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "Command failed with exit code $LASTEXITCODE`: $Exe"
    }
}

function Find-SanitizerRuntime {
    param([Parameter(Mandatory)][string]$RuntimeName)

    $candidate = (& $gpp "-print-file-name=$RuntimeName").Trim()
    if ([System.IO.Path]::IsPathRooted($candidate) -and (Test-Path -LiteralPath $candidate -PathType Leaf)) {
        return $candidate
    }

    return $null
}

$Msys2Root = Get-FullPath $Msys2Root
if ([string]::IsNullOrWhiteSpace($BuildRoot)) {
    $BuildRoot = Join-Path $repositoryRoot 'build\sanitizer-pilot-msys2-mingw64'
}
$BuildRoot = Get-FullPath $BuildRoot
if ($Jobs -lt 1) { $Jobs = 1 }

$mingwBin = Join-Path $Msys2Root 'bin'
$msysUsrBin = Join-Path (Split-Path -Parent $Msys2Root) 'usr\bin'
$qmake = Join-Path $mingwBin 'qmake6.exe'
if (-not (Test-Path -LiteralPath $qmake -PathType Leaf)) { $qmake = Join-Path $mingwBin 'qmake.exe' }
$make = Join-Path $mingwBin 'mingw32-make.exe'
$gpp = Join-Path $mingwBin 'g++.exe'
Assert-File -Path $qmake -Description 'qmake'
Assert-File -Path $make -Description 'mingw32-make'
Assert-File -Path $gpp -Description 'g++'

$oldPath = $env:Path
$oldAsanOptions = $env:ASAN_OPTIONS
$oldUbsanOptions = $env:UBSAN_OPTIONS
try {
    $env:Path = "$mingwBin;$msysUsrBin;" + $env:Path
    $asanRuntime = Find-SanitizerRuntime -RuntimeName 'libasan.dll'
    $ubsanRuntime = Find-SanitizerRuntime -RuntimeName 'libubsan.dll'
    if ($null -eq $asanRuntime -or $null -eq $ubsanRuntime) {
        throw "MSYS2 MINGW64 GCC sanitizer runtimes are unavailable (libasan.dll=$asanRuntime; libubsan.dll=$ubsanRuntime). The pilot refuses to build an uninstrumented fallback."
    }

    if ($Clean -and (Test-Path -LiteralPath $BuildRoot)) {
        Remove-Item -LiteralPath $BuildRoot -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $BuildRoot | Out-Null

    $sanitizerFlags = @(
        'QMAKE_CXXFLAGS+=-fsanitize=address,undefined',
        'QMAKE_CXXFLAGS+=-fno-omit-frame-pointer',
        'QMAKE_CXXFLAGS+=-g',
        'QMAKE_LFLAGS+=-fsanitize=address,undefined'
    )
    $pluginBuildScript = Join-Path $repositoryRoot 'tests\fixtures\plugins\source\build-test-plugins.ps1'
    Assert-File -Path $pluginBuildScript -Description 'test plugin build script'
    Invoke-Native -Exe 'powershell.exe' -Arguments @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $pluginBuildScript, '-BuildRoot', (Join-Path $repositoryRoot 'build\test-plugins'))
    $env:LABANALYSER_TEST_PLUGIN_ROOT = Join-Path $repositoryRoot 'build\test-plugins'
    $testProjects = @(
        [PSCustomObject]@{ Id = 'DataManagement'; Project = 'tests\component\datamanagement\DataManagementCharacterizationTests.pro'; Exe = 'release\DataManagementCharacterizationTests.exe' },
        [PSCustomObject]@{ Id = 'Xml'; Project = 'tests\contract\xml\XmlExperimentContractTests.pro'; Exe = 'release\XmlExperimentContractTests.exe' },
        [PSCustomObject]@{ Id = 'Parameter'; Project = 'tests\contract\parameters\ParameterContractTests.pro'; Exe = 'release\ParameterContractTests.exe' },
        [PSCustomObject]@{ Id = 'Mat'; Project = 'tests\contract\mat\MatExportContractTests.pro'; Exe = 'release\MatExportContractTests.exe' },
        [PSCustomObject]@{ Id = 'Hdf5'; Project = 'tests\contract\hdf5\Hdf5ExportContractTests.pro'; Exe = 'release\Hdf5ExportContractTests.exe' },
        [PSCustomObject]@{ Id = 'RemoteControl'; Project = 'tests\contract\remotecontrol\RemoteControlContractTests.pro'; Exe = 'release\RemoteControlContractTests.exe' },
        [PSCustomObject]@{ Id = 'PluginLoader'; Project = 'tests\contract\plugins\PluginLoaderContractTests.pro'; Exe = 'release\PluginLoaderContractTests.exe' }
    )
    $env:ASAN_OPTIONS = 'detect_leaks=1:halt_on_error=1:abort_on_error=1'
    $env:UBSAN_OPTIONS = 'print_stacktrace=1:halt_on_error=1'

    foreach ($testProject in $testProjects) {
        $projectFile = Get-FullPath $testProject.Project
        Assert-File -Path $projectFile -Description "test project '$($testProject.Id)'"
        $testBuildDir = Join-Path $BuildRoot $testProject.Id
        New-Item -ItemType Directory -Force -Path $testBuildDir | Out-Null
        Push-Location $testBuildDir
        try {
            Invoke-Native -Exe $qmake -Arguments (@($projectFile, '-spec', 'win32-g++', 'CONFIG+=release', 'CONFIG-=debug') + $sanitizerFlags)
            Invoke-Native -Exe $make -Arguments @("-j$Jobs")
            $testExecutable = Join-Path $testBuildDir $testProject.Exe
            Assert-File -Path $testExecutable -Description "sanitized test executable '$($testProject.Id)'"
            Invoke-Native -Exe $testExecutable -Arguments @('-txt')
        }
        finally {
            Pop-Location
        }
    }
}
finally {
    $env:Path = $oldPath
    $env:ASAN_OPTIONS = $oldAsanOptions
    $env:UBSAN_OPTIONS = $oldUbsanOptions
}

Write-Host 'Focused AddressSanitizer/UBSan pilot passed.'
