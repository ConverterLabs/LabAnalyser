#requires -Version 5.1
<#
.SYNOPSIS
Builds LabAnalyser.pro with the MSYS2 MINGW64 Qt toolchain.

.EXAMPLE
.\build-msys2.ps1

.EXAMPLE
.\build-msys2.ps1 -Clean -Deploy

.EXAMPLE
.\build-msys2.ps1 -Configuration debug
#>

[CmdletBinding()]
param(
    [ValidateSet('release', 'debug')]
    [string]$Configuration = 'release',

    [string]$Msys2Root = 'C:\msys64\mingw64',
    [string]$ProjectFile = '',
    [string]$BuildDir = '',
    [string]$DeployDir = '',
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
if ([string]::IsNullOrWhiteSpace($BuildDir)) {
    $BuildDir = Join-Path $ScriptRootPath "build\msys2-mingw64-$Configuration"
}
$BuildDir = Get-FullPath $BuildDir
if ([string]::IsNullOrWhiteSpace($DeployDir)) {
    $DeployDir = Join-Path $ScriptRootPath "dist\LabAnalyser-$Configuration"
}
$DeployDir = Get-FullPath $DeployDir
if ([string]::IsNullOrWhiteSpace($Hdf5LibDir)) {
    $Hdf5LibDir = Join-Path $Msys2Root 'lib'
}
$Hdf5LibDir = Get-FullPath $Hdf5LibDir

if ($Jobs -lt 1) {
    $Jobs = 1
}

$mingwBin = Join-Path $Msys2Root 'bin'
$msysUsrBin = Join-Path (Split-Path -Parent $Msys2Root) 'usr\bin'
$qmakeSearchDirs = @(
    $mingwBin,
    (Join-Path $Msys2Root 'lib\qt6\bin'),
    (Join-Path $Msys2Root 'share\qt6\bin')
) | Select-Object -Unique

Assert-File -Path $ProjectFile -Description 'Qt project file'
Assert-Directory -Path $Msys2Root -Description 'MSYS2 MINGW64 root'
Assert-Directory -Path $mingwBin -Description 'MSYS2 MINGW64 bin directory'

$qmakeCandidates = foreach ($searchDir in $qmakeSearchDirs) {
    foreach ($qmakeName in @('qmake6.exe', 'qmake-qt6.exe', 'qmake.exe', 'qmake6.bat', 'qmake-qt6.bat', 'qmake.bat')) {
        Join-Path $searchDir $qmakeName
    }
}
$qmake = Resolve-Tool -Name 'qmake' -Candidates $qmakeCandidates
$make = Resolve-Tool -Name 'mingw32-make' -Candidates @(
    (Join-Path $mingwBin 'mingw32-make.exe')
)
$cmake = (Get-Command cmake.exe -ErrorAction Stop).Source
$ctest = (Get-Command ctest.exe -ErrorAction Stop).Source
$windeployqt = $null
if ($Deploy) {
    $windeployqt = Resolve-Tool -Name 'windeployqt' -Candidates @(
        (Join-Path $mingwBin 'windeployqt6.exe'),
        (Join-Path $mingwBin 'windeployqt-qt6.exe'),
        (Join-Path $mingwBin 'windeployqt.exe')
    )
}

if (-not $SkipExternalChecks) {
    Assert-Directory -Path $Hdf5LibDir -Description 'HDF5/FFTW library directory'
    foreach ($file in @('libhdf5.dll.a', 'libfftw3.dll.a', 'libmatio.dll.a')) {
        Assert-File -Path (Join-Path $Hdf5LibDir $file) -Description "import library $file"
    }
    Assert-File -Path (Join-Path $Msys2Root 'include\matio.h') -Description 'matio header'

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
    $pathParts = @($mingwBin) + $qmakeSearchDirs
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

    $connectorSourceDir = Join-Path $ScriptRootPath 'MatlabRemoteConnector'
    $connectorBuildDir = Join-Path $BuildDir 'matlab-connector'
    $connectorConfiguration = if ($Configuration -eq 'release') { 'Release' } else { 'Debug' }
    Invoke-Native -Exe $cmake -Arguments @(
        '-S', $connectorSourceDir,
        '-B', $connectorBuildDir,
        '-G', 'MinGW Makefiles',
        "-DCMAKE_BUILD_TYPE=$connectorConfiguration",
        '-DBUILD_TESTING=ON',
        "-DCMAKE_CXX_COMPILER=$(Join-Path $mingwBin 'g++.exe')"
    )
    Invoke-Native -Exe $cmake -Arguments @(
        '--build', $connectorBuildDir,
        '--parallel', $Jobs
    )
    Invoke-Native -Exe $ctest -Arguments @(
        '--test-dir', $connectorBuildDir,
        '--output-on-failure'
    )
    $connectorDll = Join-Path $connectorBuildDir 'TCPClient.dll'
    Assert-File -Path $connectorDll -Description 'MATLAB TCP connector DLL'

    if ($Deploy) {
        Assert-CleanTargetIsSafe -Path $DeployDir
        if ([String]::Equals($DeployDir, $BuildDir, [StringComparison]::OrdinalIgnoreCase)) {
            throw "DeployDir must not be the build directory: $DeployDir"
        }

        if (Test-Path -LiteralPath $DeployDir) {
            Remove-Item -LiteralPath $DeployDir -Recurse -Force
        }

        New-Item -ItemType Directory -Force -Path $DeployDir | Out-Null
        $deployExePath = Join-Path $DeployDir 'LabAnalyser.exe'
        Copy-Item -LiteralPath $exePath -Destination $deployExePath -Force

        $matlabPackageRoot = Join-Path $DeployDir 'LabAnalyser'
        Invoke-Native -Exe $cmake -Arguments @(
            '--install', $connectorBuildDir,
            '--prefix', $matlabPackageRoot
        )
        $deployedConnector = Join-Path $matlabPackageRoot '+LabAnalyser\TCPClient.dll'
        Assert-File -Path $deployedConnector -Description 'deployed MATLAB TCP connector DLL'

        Invoke-Native -Exe $windeployqt -Arguments @(
            "--$Configuration",
            '--compiler-runtime',
            (Convert-ToQMakePath $deployExePath)
        )

        $objdump = Resolve-Tool -Name 'objdump' -Candidates @(
            (Join-Path $mingwBin 'objdump.exe')
        )

        $runtimeDllSearchDirs = @(
            (Join-Path (Split-Path -Parent $Hdf5LibDir) 'bin'),
            $mingwBin,
            $Hdf5LibDir
        ) |
            Where-Object { Test-Path -LiteralPath $_ -PathType Container } |
            Select-Object -Unique

        function Find-RuntimeDll {
            param(
                [Parameter(Mandatory = $true)]
                [string]$DllName
            )

            foreach ($searchDir in $runtimeDllSearchDirs) {
                $candidate = Join-Path $searchDir $DllName
                if (Test-Path -LiteralPath $candidate -PathType Leaf) {
                    return $candidate
                }
            }

            return $null
        }

        function Get-ImportedDllNames {
            param(
                [Parameter(Mandatory = $true)]
                [string]$DllPath
            )

            $dependencyLines = & $objdump -p $DllPath 2>$null
            if ($LASTEXITCODE -ne 0) {
                throw "Could not inspect native dependencies of '$DllPath'."
            }

            foreach ($line in $dependencyLines) {
                if ($line -match '^\s*DLL Name:\s*(?<name>[^\s]+)\s*$') {
                    $Matches['name']
                }
            }
        }

        function Copy-DllWithDependencies {
            param(
                [Parameter(Mandatory = $true)]
                [string]$DllPath,

                [Parameter(Mandatory = $true)]
                [hashtable]$CopiedDlls
            )

            $dllName = [IO.Path]::GetFileName($DllPath)
            $dllKey = $dllName.ToLowerInvariant()
            if ($CopiedDlls.ContainsKey($dllKey)) {
                return
            }

            $CopiedDlls[$dllKey] = $true
            Copy-Item -LiteralPath $DllPath -Destination (Join-Path $DeployDir $dllName) -Force

            foreach ($dependencyName in @(Get-ImportedDllNames -DllPath $DllPath)) {
                $dependencyPath = Find-RuntimeDll -DllName $dependencyName
                if ($null -ne $dependencyPath) {
                    Copy-DllWithDependencies -DllPath $dependencyPath -CopiedDlls $CopiedDlls
                }
            }
        }

        function Test-DeployedDll {
            param(
                [Parameter(Mandatory = $true)]
                [string]$DllName
            )

            return $null -ne (
                Get-ChildItem -LiteralPath $DeployDir -Recurse -Filter $DllName -File -ErrorAction SilentlyContinue |
                    Select-Object -First 1
            )
        }

        $copiedDlls = @{}
        foreach ($pattern in @('libhdf5-*.dll', 'libfftw3-*.dll', 'libmatio-*.dll')) {
            $runtimeDlls = @(
                @(
                    foreach ($searchDir in $runtimeDllSearchDirs) {
                        Get-ChildItem -LiteralPath $searchDir -Filter $pattern -File -ErrorAction SilentlyContinue
                    }
                ) | Select-Object -Property FullName -Unique
            )

            if ($runtimeDlls.Count -eq 0) {
                throw "Could not find a runtime DLL matching '$pattern' in the configured runtime directories."
            }

            foreach ($runtimeDll in $runtimeDlls) {
                Copy-DllWithDependencies -DllPath $runtimeDll.FullName -CopiedDlls $copiedDlls
            }
        }

        # windeployqt does not always copy the MinGW runtime dependencies of
        # Qt plugins. Inspect every deployed binary until the package is closed.
        $inspectedDeploymentFiles = @{}
        do {
            $copiedFileCountBeforeScan = $copiedDlls.Count
            $deploymentFiles = Get-ChildItem -LiteralPath $DeployDir -Recurse -File |
                Where-Object { $_.Extension -in @('.dll', '.exe') }

            foreach ($deploymentFile in $deploymentFiles) {
                if ($inspectedDeploymentFiles.ContainsKey($deploymentFile.FullName)) {
                    continue
                }

                $inspectedDeploymentFiles[$deploymentFile.FullName] = $true
                foreach ($dependencyName in @(Get-ImportedDllNames -DllPath $deploymentFile.FullName)) {
                    if (Test-DeployedDll -DllName $dependencyName) {
                        continue
                    }

                    $dependencyPath = Find-RuntimeDll -DllName $dependencyName
                    if ($null -ne $dependencyPath) {
                        Copy-DllWithDependencies -DllPath $dependencyPath -CopiedDlls $copiedDlls
                    }
                }
            }
        } while ($copiedDlls.Count -gt $copiedFileCountBeforeScan)

        Assert-File -Path $deployExePath -Description 'deployed LabAnalyser executable'
        Write-Host "Standalone deployment created: $DeployDir"
    }

    Write-Host ''
    Write-Host "Build succeeded: $exePath"
}
finally {
    $env:Path = $oldPath
    $env:MSYSTEM = $oldMSYSTEM
    $env:CHERE_INVOKING = $oldCHERE
}
