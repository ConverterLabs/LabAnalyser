#requires -Version 5.1
<#
.SYNOPSIS
Builds and runs every currently registered LabAnalyser qmake test with MSYS2
MINGW64.  The script returns a non-zero exit code when configuration, build, or
test execution fails.

.EXAMPLE
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-tests-msys2.ps1 -Clean
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

function Assert-CleanTargetIsSafe {
    param([Parameter(Mandatory)][string]$Path)

    $projectRoot = (Get-FullPath $repositoryRoot).TrimEnd('\') + '\'
    $target = (Get-FullPath $Path).TrimEnd('\') + '\'
    if (-not $target.StartsWith($projectRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to clean a directory outside the repository: $Path"
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

$Msys2Root = Get-FullPath $Msys2Root
if ([string]::IsNullOrWhiteSpace($BuildRoot)) {
    $BuildRoot = Join-Path $repositoryRoot 'build\tests-msys2-mingw64'
}
$BuildRoot = Get-FullPath $BuildRoot
if ($Jobs -lt 1) {
    $Jobs = 1
}

$mingwBin = Join-Path $Msys2Root 'bin'
$msysUsrBin = Join-Path (Split-Path -Parent $Msys2Root) 'usr\bin'
$qmake = Join-Path $mingwBin 'qmake6.exe'
if (-not (Test-Path -LiteralPath $qmake -PathType Leaf)) {
    $qmake = Join-Path $mingwBin 'qmake.exe'
}
$make = Join-Path $mingwBin 'mingw32-make.exe'

Assert-File -Path $qmake -Description 'qmake'
Assert-File -Path $make -Description 'mingw32-make'

# Add every test project here.  The order is explicit so failures are stable and
# future suites can be added without accidentally configuring LabAnalyser.pro.
$testProjects = @(
    [PSCustomObject]@{
        Id = 'unit/PlotMeasurementsTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\PlotMeasurementsTests.pro'
        ExecutableRelativePath = 'release\PlotMeasurementsTests.exe'
    },
    [PSCustomObject]@{
        Id = 'component/DataManagementCharacterizationTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\component\datamanagement\DataManagementCharacterizationTests.pro'
        ExecutableRelativePath = 'release\DataManagementCharacterizationTests.exe'
    },
    [PSCustomObject]@{
        Id = 'contract/XmlExperimentContractTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\contract\xml\XmlExperimentContractTests.pro'
        ExecutableRelativePath = 'release\XmlExperimentContractTests.exe'
    },
    [PSCustomObject]@{
        Id = 'contract/ParameterContractTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\contract\parameters\ParameterContractTests.pro'
        ExecutableRelativePath = 'release\ParameterContractTests.exe'
    },
    [PSCustomObject]@{
        Id = 'contract/MatExportContractTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\contract\mat\MatExportContractTests.pro'
        ExecutableRelativePath = 'release\MatExportContractTests.exe'
    },
    [PSCustomObject]@{
        Id = 'contract/Hdf5ExportContractTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\contract\hdf5\Hdf5ExportContractTests.pro'
        ExecutableRelativePath = 'release\Hdf5ExportContractTests.exe'
    },
    [PSCustomObject]@{
        Id = 'contract/RemoteControlContractTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\contract\remotecontrol\RemoteControlContractTests.pro'
        ExecutableRelativePath = 'release\RemoteControlContractTests.exe'
    },
    [PSCustomObject]@{
        Id = 'contract/dropwidgets/DropWidgetAdapterTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\contract\dropwidgets\DropWidgetAdapterTests.pro'
        ExecutableRelativePath = 'release\DropWidgetAdapterTests.exe'
    }
    ,[PSCustomObject]@{
        Id = 'contract/PluginLoaderContractTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\contract\plugins\PluginLoaderContractTests.pro'
        ExecutableRelativePath = 'release\PluginLoaderContractTests.exe'
    }
    ,[PSCustomObject]@{
        Id = 'integration/mainwindow/MainWindowIntegrationTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\integration\mainwindow\MainWindowIntegrationTests.pro'
        ExecutableRelativePath = 'release\MainWindowIntegrationTests.exe'
    }
    ,[PSCustomObject]@{
        Id = 'contract/plotwidget/PlotWidgetContractTests'
        ProjectFile = Join-Path $repositoryRoot 'tests\contract\plotwidget\PlotWidgetContractTests.pro'
        ExecutableRelativePath = 'release\PlotWidgetContractTests.exe'
    }
)

foreach ($testProject in $testProjects) {
    Assert-File -Path $testProject.ProjectFile -Description "test project '$($testProject.Id)'"
}

if ($Clean -and (Test-Path -LiteralPath $BuildRoot)) {
    Assert-CleanTargetIsSafe -Path $BuildRoot
    Remove-Item -LiteralPath $BuildRoot -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $BuildRoot | Out-Null

$oldPath = $env:Path
$oldMSYSTEM = $env:MSYSTEM
$oldCHERE = $env:CHERE_INVOKING
$oldQtQpaPlatform = $env:QT_QPA_PLATFORM
try {
    $pathParts = @($mingwBin)
    if (Test-Path -LiteralPath $msysUsrBin -PathType Container) {
        $pathParts += $msysUsrBin
    }
    $env:Path = ($pathParts + $env:Path) -join ';'
    $env:MSYSTEM = 'MINGW64'
    $env:CHERE_INVOKING = '1'
    $pluginBuildScript = Join-Path $repositoryRoot 'tests\fixtures\plugins\source\build-test-plugins.ps1'
    Assert-File -Path $pluginBuildScript -Description 'test plugin build script'
    $env:Path = "$mingwBin;$msysUsrBin;" + $env:Path
    Invoke-Native -Exe 'powershell.exe' -Arguments @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $pluginBuildScript, '-BuildRoot', (Join-Path $repositoryRoot 'build\test-plugins'))
    $env:LABANALYSER_TEST_PLUGIN_ROOT = Join-Path $repositoryRoot 'build\test-plugins'

    foreach ($testProject in $testProjects) {
        $testBuildDir = Join-Path $BuildRoot ($testProject.Id.Replace('/', '\\'))
        New-Item -ItemType Directory -Force -Path $testBuildDir | Out-Null

        Push-Location $testBuildDir
        try {
            # Reassert the runtime lookup order for every native process.
            $env:Path = "$mingwBin;$msysUsrBin;" + $env:Path
            Invoke-Native -Exe $qmake -Arguments @($testProject.ProjectFile, '-spec', 'win32-g++', 'CONFIG+=release', 'CONFIG-=debug')
            $env:Path = "$mingwBin;$msysUsrBin;" + $env:Path
            Invoke-Native -Exe $make -Arguments @("-j$Jobs")
            $testExecutable = Join-Path $testBuildDir $testProject.ExecutableRelativePath
            Assert-File -Path $testExecutable -Description "test executable '$($testProject.Id)'"
            $env:Path = "$mingwBin;$msysUsrBin;" + $env:Path
            # The real MainWindow and PlotWidget contract targets require an
            # offscreen Qt platform. Keep the caller's setting for every other
            # test and restore it immediately after this executable returns.
            $suiteQtQpaPlatform = $env:QT_QPA_PLATFORM
            $suiteRepositoryRoot = $env:LABANALYSER_TEST_REPOSITORY_ROOT
            try {
                if ($testProject.Id -in @('integration/mainwindow/MainWindowIntegrationTests', 'contract/plotwidget/PlotWidgetContractTests')) {
                    $env:QT_QPA_PLATFORM = 'offscreen'
                    $env:LABANALYSER_TEST_REPOSITORY_ROOT = $repositoryRoot
                }
                Invoke-Native -Exe $testExecutable -Arguments @('-txt')
            }
            finally {
                $env:QT_QPA_PLATFORM = $suiteQtQpaPlatform
                $env:LABANALYSER_TEST_REPOSITORY_ROOT = $suiteRepositoryRoot
            }
        }
        finally {
            Pop-Location
        }
    }
}
finally {
    $env:Path = $oldPath
    $env:MSYSTEM = $oldMSYSTEM
    $env:CHERE_INVOKING = $oldCHERE
    $env:QT_QPA_PLATFORM = $oldQtQpaPlatform
}

Write-Host "All $($testProjects.Count) registered test project(s) passed."
