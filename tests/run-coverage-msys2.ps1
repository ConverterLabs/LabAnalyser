#requires -Version 5.1
<#
.SYNOPSIS
Builds every registered qmake test with GCC coverage instrumentation and writes
a combined, production-source-only gcov summary.

.DESCRIPTION
The test manifest stays owned by run-tests-msys2.ps1.  This wrapper invokes it
once in a dedicated build root with -Coverage, then merges every generated
.gcda record belonging to a non-vendored production .cpp file.  Generated Qt
sources, test sources and vendored qcustomplot are excluded by rule and listed
in the report.  Production files which no registered test target compiled are
listed separately; they are not silently counted as zero.

.EXAMPLE
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-coverage-msys2.ps1 -Jobs 4
#>

[CmdletBinding()]
param(
    [string]$Msys2Root = 'C:\msys64\mingw64',
    [string]$BuildRoot = '',
    [int]$Jobs = $env:NUMBER_OF_PROCESSORS,
    [switch]$CollectOnly
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

function Get-Percent {
    param([long]$Covered, [long]$Total)

    if ($Total -eq 0) {
        return 'n/a'
    }

    return ('{0:N2}%' -f (100.0 * $Covered / $Total)).Replace(',', '.')
}

function Add-Count {
    param(
        [Parameter(Mandatory)][hashtable]$Map,
        [Parameter(Mandatory)][string]$Key,
        [Parameter(Mandatory)][long]$Count
    )

    if ($Map.ContainsKey($Key)) {
        $Map[$Key] = [long]$Map[$Key] + $Count
    }
    else {
        $Map[$Key] = $Count
    }
}

function Set-MaxCount {
    param(
        [Parameter(Mandatory)][hashtable]$Map,
        [Parameter(Mandatory)][string]$Key,
        [Parameter(Mandatory)][long]$Count
    )

    if (-not $Map.ContainsKey($Key) -or $Count -gt [long]$Map[$Key]) {
        $Map[$Key] = $Count
    }
}

function New-SourceStats {
    return @{
        Lines = @{}
        Branches = @{}
        Calls = @{}
        Functions = @{}
        Instances = 0
    }
}

function Get-RelativeRepositoryPath {
    param([Parameter(Mandatory)][string]$Path)

    return $Path.Substring($repositoryRoot.Length).TrimStart('\', '/').Replace('\', '/')
}

function Assert-CoverageBuildRootIsSafe {
    param([Parameter(Mandatory)][string]$Path)

    $repositoryPrefix = ([System.IO.Path]::GetFullPath($repositoryRoot)).TrimEnd('\') + '\'
    $target = ([System.IO.Path]::GetFullPath($Path)).TrimEnd('\') + '\'
    if (-not $target.StartsWith($repositoryPrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Coverage build root must remain inside the repository: $Path"
    }
}

function Merge-GcovFile {
    param(
        [Parameter(Mandatory)][string]$GcovFile,
        [Parameter(Mandatory)][hashtable]$Stats
    )

    $currentLine = $null
    foreach ($line in Get-Content -LiteralPath $GcovFile) {
        if ($line -match '^\s*(?<count>[-#=0-9]+):\s*(?<line>\d+):') {
            $lineNumber = [int]$Matches.line
            $countToken = $Matches.count.Trim()
            $currentLine = $lineNumber
            if ($lineNumber -gt 0 -and $countToken -ne '-' -and $countToken -ne '=====') {
                $count = if ($countToken -eq '#####') { 0 } else { [long]$countToken }
                Add-Count -Map $Stats.Lines -Key ([string]$lineNumber) -Count $count
            }
            continue
        }

        if ($null -ne $currentLine -and $line -match '^\s*branch\s+(?<branch>\d+)\s+(?<result>never executed|taken\s+(?<count>\d+))') {
            # -1 means gcov explicitly reported an unevaluated branch; zero
            # means it was evaluated but not taken.  The maximum across test
            # binaries preserves both the executed and taken-at-least-once
            # aggregate meanings without treating an unevaluated record as a
            # zero-taken execution.
            $count = if ($Matches.result -eq 'never executed') { -1 } else { [long]$Matches.count }
            Set-MaxCount -Map $Stats.Branches -Key "${currentLine}:$($Matches.branch)" -Count $count
            continue
        }

        if ($null -ne $currentLine -and $line -match '^\s*call\s+(?<call>\d+)\s+(?<result>never executed|returned(?:\s+\d+%?)?)') {
            # gcov reports a call returning zero as executed.  Store one for
            # that case and zero only for an explicit "never executed" line.
            $count = if ($Matches.result -eq 'never executed') { 0 } else { 1 }
            Add-Count -Map $Stats.Calls -Key "${currentLine}:$($Matches.call)" -Count $count
            continue
        }

        if ($line -match '^\s*function\s+(?<name>.+)\s+called\s+(?<count>\d+)') {
            Add-Count -Map $Stats.Functions -Key $Matches.name.Trim() -Count ([long]$Matches.count)
        }
    }
}

function Merge-GcovJsonFunctions {
    param(
        [Parameter(Mandatory)][string]$JsonGzipFile,
        [Parameter(Mandatory)][string]$SourceFullName,
        [Parameter(Mandatory)][string]$TestBuildDirectory,
        [Parameter(Mandatory)][hashtable]$Stats
    )

    $fileStream = [System.IO.File]::OpenRead($JsonGzipFile)
    try {
        $gzipStream = New-Object System.IO.Compression.GzipStream($fileStream, [System.IO.Compression.CompressionMode]::Decompress)
        try {
            $reader = New-Object System.IO.StreamReader($gzipStream)
            try {
                $json = $reader.ReadToEnd() | ConvertFrom-Json
            }
            finally {
                $reader.Dispose()
            }
        }
        finally {
            $gzipStream.Dispose()
        }
    }
    finally {
        $fileStream.Dispose()
    }

    $matchingSource = @($json.files | Where-Object {
        $reported = [string]$_.file
        $resolved = if ([System.IO.Path]::IsPathRooted($reported)) {
            [System.IO.Path]::GetFullPath($reported)
        }
        else {
            [System.IO.Path]::GetFullPath((Join-Path $TestBuildDirectory $reported))
        }
        $resolved.Replace('\', '/') -eq $SourceFullName.Replace('\', '/')
    })
    if ($matchingSource.Count -ne 1) {
        throw "Expected one JSON source entry for $SourceFullName in $JsonGzipFile, found $($matchingSource.Count)."
    }

    foreach ($function in $matchingSource[0].functions) {
        # A function can occur in several instrumented test binaries.  The
        # merged coverage question is whether it ran at least once, so retain
        # the greatest execution count rather than summing duplicate records.
        Set-MaxCount -Map $Stats.Functions -Key ([string]$function.name) -Count ([long]$function.execution_count)
    }
}

if ([string]::IsNullOrWhiteSpace($BuildRoot)) {
    $BuildRoot = Join-Path $repositoryRoot 'build\coverage-msys2-mingw64'
}
$BuildRoot = Get-FullPath $BuildRoot
$Msys2Root = Get-FullPath $Msys2Root
Assert-CoverageBuildRootIsSafe -Path $BuildRoot
if ($Jobs -lt 1) {
    $Jobs = 1
}

$mingwBin = Join-Path $Msys2Root 'bin'
$msysUsrBin = Join-Path (Split-Path -Parent $Msys2Root) 'usr\bin'
# Resolve gcov: prefer the unversioned 'gcov.exe'; fall back to 'gcov-N.exe'
# produced by MSYS2 when it ships only the versioned binary for a given GCC release.
$gcov = Join-Path $mingwBin 'gcov.exe'
if (-not (Test-Path -LiteralPath $gcov -PathType Leaf)) {
    $versioned = @(Get-ChildItem -LiteralPath $mingwBin -File -Filter 'gcov-*.exe' |
        Sort-Object -Property Name -Descending | Select-Object -First 1)
    if ($versioned.Count -eq 1) {
        $gcov = $versioned[0].FullName
        Write-Host "gcov.exe not found; using versioned binary: $gcov"
    }
}
$testRunner = Join-Path $PSScriptRoot 'run-tests-msys2.ps1'
Assert-File -Path $gcov -Description 'gcov'
Assert-File -Path $testRunner -Description 'test runner'

$reportRoot = Join-Path $BuildRoot 'report'
$gcovWorkRoot = Join-Path $reportRoot 'gcov-work'
$oldPath = $env:Path
try {
    $env:Path = "$mingwBin;$msysUsrBin;" + $env:Path

    if ($CollectOnly) {
        if (-not (Test-Path -LiteralPath $BuildRoot -PathType Container)) {
            throw "Coverage build root not found for -CollectOnly: $BuildRoot"
        }
    }
    else {
        Write-Host '> coverage test runner'
        & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $testRunner -Msys2Root $Msys2Root -BuildRoot $BuildRoot -Jobs $Jobs -Clean -Coverage
        if ($LASTEXITCODE -ne 0) {
            throw "Coverage test runner failed with exit code $LASTEXITCODE."
        }
    }

    if (Test-Path -LiteralPath $reportRoot) {
        # The report directory is always a child of the dedicated coverage
        # build root; clear only its generated contents before recollecting.
        Remove-Item -LiteralPath $reportRoot -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $reportRoot | Out-Null
    New-Item -ItemType Directory -Force -Path $gcovWorkRoot | Out-Null

    $vendoredSource = 'src/DropWidgets/Plots/qcustomplot.cpp'
    $productionSources = @(
        Get-ChildItem -LiteralPath $repositoryRoot -Recurse -File -Filter *.cpp |
            Where-Object {
                $relative = Get-RelativeRepositoryPath -Path $_.FullName
                $pathParts = $relative -split '/'
                -not $relative.StartsWith('tests/', [System.StringComparison]::OrdinalIgnoreCase) -and
                -not ($pathParts -contains 'build') -and
                -not ($pathParts -contains 'tests') -and
                -not $_.Name.StartsWith('moc_', [System.StringComparison]::OrdinalIgnoreCase) -and
                -not $_.Name.StartsWith('qrc_', [System.StringComparison]::OrdinalIgnoreCase) -and
                -not $_.Name.StartsWith('ui_', [System.StringComparison]::OrdinalIgnoreCase) -and
                $relative -ne $vendoredSource
            } |
            Sort-Object FullName
    )
    $sourceByObjectBaseName = @{}
    foreach ($source in $productionSources) {
        $baseName = [System.IO.Path]::GetFileNameWithoutExtension($source.Name)
        if ($sourceByObjectBaseName.ContainsKey($baseName)) {
            throw "Coverage source-name collision for '$baseName'; add an explicit mapping."
        }
        $sourceByObjectBaseName[$baseName] = $source
    }

    $statsBySource = @{}
    $skippedDataFiles = New-Object System.Collections.Generic.List[string]
    $gcdaFiles = @(Get-ChildItem -LiteralPath $BuildRoot -Recurse -File -Filter *.gcda)
    if ($gcdaFiles.Count -eq 0) {
        throw "No .gcda files were produced below $BuildRoot."
    }

    $index = 0
    foreach ($gcdaFile in $gcdaFiles) {
        $objectBaseName = [System.IO.Path]::GetFileNameWithoutExtension($gcdaFile.Name)
        if (-not $sourceByObjectBaseName.ContainsKey($objectBaseName)) {
            $skippedDataFiles.Add((Get-RelativeRepositoryPath -Path $gcdaFile.FullName))
            continue
        }

        $source = $sourceByObjectBaseName[$objectBaseName]
        $relativeSource = Get-RelativeRepositoryPath -Path $source.FullName
        if (-not $statsBySource.ContainsKey($relativeSource)) {
            $statsBySource[$relativeSource] = New-SourceStats
        }

        $workDirectory = Join-Path $gcovWorkRoot ('record-{0:D4}' -f $index)
        $index++
        New-Item -ItemType Directory -Force -Path $workDirectory | Out-Null
        Push-Location $workDirectory
        try {
            # gcov can print non-fatal diagnostics for relative header entries
            # recorded in a .gcno file.  Its process exit code, not PowerShell's
            # native-stderr error record, determines whether the source report
            # is usable.
            $savedErrorActionPreference = $ErrorActionPreference
            try {
                $ErrorActionPreference = 'Continue'
                $gcovOutput = & $gcov -b -c -f -p -o $gcdaFile.DirectoryName $source.FullName 2>&1
                $gcovExitCode = $LASTEXITCODE
            }
            finally {
                $ErrorActionPreference = $savedErrorActionPreference
            }
            if ($gcovExitCode -ne 0) {
                throw "gcov failed for $relativeSource from $($gcdaFile.FullName): $($gcovOutput -join [Environment]::NewLine)"
            }
        }
        finally {
            Pop-Location
        }

        $matchingGcov = @(
            Get-ChildItem -LiteralPath $workDirectory -File -Filter *.gcov |
                Where-Object {
                    $sourceLine = Get-Content -LiteralPath $_.FullName -TotalCount 2 | Where-Object { $_ -like '        -:    0:Source:*' -or $_ -like '*Source:*' } | Select-Object -First 1
                    if ($null -eq $sourceLine) { return $false }
                    $reportedSource = ($sourceLine -replace '^.*Source:', '')
                    if ([System.IO.Path]::IsPathRooted($reportedSource)) {
                        $reportedFullPath = [System.IO.Path]::GetFullPath($reportedSource)
                    }
                    else {
                        # qmake records relative source paths from the test
                        # build directory, while .gcda lives in its release/
                        # child directory.
                        $testBuildDirectory = Split-Path -Parent $gcdaFile.DirectoryName
                        $reportedFullPath = [System.IO.Path]::GetFullPath((Join-Path $testBuildDirectory $reportedSource))
                    }
                    return $reportedFullPath.Replace('\', '/') -eq $source.FullName.Replace('\', '/')
                }
        )
        if ($matchingGcov.Count -ne 1) {
            throw "Expected one gcov output for $relativeSource from $($gcdaFile.FullName), found $($matchingGcov.Count)."
        }

        $statsBySource[$relativeSource].Instances++
        Merge-GcovFile -GcovFile $matchingGcov[0].FullName -Stats $statsBySource[$relativeSource]

        $jsonWorkDirectory = Join-Path $workDirectory 'json'
        New-Item -ItemType Directory -Force -Path $jsonWorkDirectory | Out-Null
        Push-Location $jsonWorkDirectory
        try {
            $savedErrorActionPreference = $ErrorActionPreference
            try {
                $ErrorActionPreference = 'Continue'
                $jsonOutput = & $gcov -j -b -c -p -o $gcdaFile.DirectoryName $source.FullName 2>&1
                $jsonExitCode = $LASTEXITCODE
            }
            finally {
                $ErrorActionPreference = $savedErrorActionPreference
            }
            if ($jsonExitCode -ne 0) {
                throw "gcov JSON aggregation failed for $relativeSource from $($gcdaFile.FullName): $($jsonOutput -join [Environment]::NewLine)"
            }
        }
        finally {
            Pop-Location
        }
        $jsonFiles = @(Get-ChildItem -LiteralPath $jsonWorkDirectory -File -Filter *.gcov.json.gz)
        if ($jsonFiles.Count -ne 1) {
            throw "Expected one gcov JSON output for $relativeSource from $($gcdaFile.FullName), found $($jsonFiles.Count)."
        }
        $testBuildDirectory = Split-Path -Parent $gcdaFile.DirectoryName
        Merge-GcovJsonFunctions -JsonGzipFile $jsonFiles[0].FullName -SourceFullName $source.FullName -TestBuildDirectory $testBuildDirectory -Stats $statsBySource[$relativeSource]
    }

    $rows = New-Object System.Collections.Generic.List[object]
    foreach ($relativeSource in ($statsBySource.Keys | Sort-Object)) {
        $stats = $statsBySource[$relativeSource]
        $lineTotal = [long]$stats.Lines.Count
        $lineCovered = [long](@($stats.Lines.Values | Where-Object { $_ -gt 0 }).Count)
        $branchTotal = [long]$stats.Branches.Count
        $branchExecuted = [long](@($stats.Branches.Values | Where-Object { $_ -ge 0 }).Count)
        $branchTaken = [long](@($stats.Branches.Values | Where-Object { $_ -gt 0 }).Count)
        $callTotal = [long]$stats.Calls.Count
        $callCovered = [long](@($stats.Calls.Values | Where-Object { $_ -gt 0 }).Count)
        $functionTotal = [long]$stats.Functions.Count
        $functionCovered = [long](@($stats.Functions.Values | Where-Object { $_ -gt 0 }).Count)
        $rows.Add([PSCustomObject]@{
            Source = $relativeSource
            Records = [int]$stats.Instances
            LinesCovered = $lineCovered
            LinesTotal = $lineTotal
            BranchesExecuted = $branchExecuted
            BranchesTaken = $branchTaken
            BranchesTotal = $branchTotal
            CallsCovered = $callCovered
            CallsTotal = $callTotal
            FunctionsCovered = $functionCovered
            FunctionsTotal = $functionTotal
        })
    }

    $compiledSources = @($rows.Source)
    $uncompiledSources = @(
        $productionSources |
            ForEach-Object { Get-RelativeRepositoryPath -Path $_.FullName } |
            Where-Object { $compiledSources -notcontains $_ } |
            Sort-Object
    )

    $totals = [PSCustomObject]@{
        LinesCovered = [long](($rows | Measure-Object -Property LinesCovered -Sum).Sum)
        LinesTotal = [long](($rows | Measure-Object -Property LinesTotal -Sum).Sum)
        BranchesExecuted = [long](($rows | Measure-Object -Property BranchesExecuted -Sum).Sum)
        BranchesTaken = [long](($rows | Measure-Object -Property BranchesTaken -Sum).Sum)
        BranchesTotal = [long](($rows | Measure-Object -Property BranchesTotal -Sum).Sum)
        CallsCovered = [long](($rows | Measure-Object -Property CallsCovered -Sum).Sum)
        CallsTotal = [long](($rows | Measure-Object -Property CallsTotal -Sum).Sum)
        FunctionsCovered = [long](($rows | Measure-Object -Property FunctionsCovered -Sum).Sum)
        FunctionsTotal = [long](($rows | Measure-Object -Property FunctionsTotal -Sum).Sum)
    }

    $markdown = New-Object System.Collections.Generic.List[string]
    $markdown.Add('# Combined MSYS2/GCC coverage report')
    $markdown.Add('')
    $markdown.Add("Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss K')")
    $markdown.Add('')
    $markdown.Add('Command: `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\\tests\\run-coverage-msys2.ps1 -Jobs 4`')
    $markdown.Add('')
    $markdown.Add('## Scope and exclusions')
    $markdown.Add('')
    $markdown.Add('This report merges `.gcda` records generated by every test project registered in `run-tests-msys2.ps1`. Its denominator is the non-vendored production `.cpp` files actually compiled by at least one instrumented test target. It is not a repository-wide coverage percentage: production sources not compiled by any registered test are listed separately rather than silently scored as zero.')
    $markdown.Add('')
    $markdown.Add('- Excluded vendored code: `src/DropWidgets/Plots/qcustomplot.cpp`.')
    $markdown.Add('- Excluded generated code: `moc_*`, `qrc_*`, `ui_*` and other build-directory outputs. These are not source entries in the production-source denominator.')
    $markdown.Add('- Excluded test code: every source below `tests/`.')
    $markdown.Add('')
    $markdown.Add('## Aggregate for compiled production sources')
    $markdown.Add('')
    $markdown.Add('| Lines | Branches executed | Branches taken at least once | Calls | Functions |')
    $markdown.Add('| --- | --- | --- | --- | --- |')
    $markdown.Add("| $(Get-Percent $totals.LinesCovered $totals.LinesTotal) ($($totals.LinesCovered)/$($totals.LinesTotal)) | $(Get-Percent $totals.BranchesExecuted $totals.BranchesTotal) ($($totals.BranchesExecuted)/$($totals.BranchesTotal)) | $(Get-Percent $totals.BranchesTaken $totals.BranchesTotal) ($($totals.BranchesTaken)/$($totals.BranchesTotal)) | $(Get-Percent $totals.CallsCovered $totals.CallsTotal) ($($totals.CallsCovered)/$($totals.CallsTotal)) | $(Get-Percent $totals.FunctionsCovered $totals.FunctionsTotal) ($($totals.FunctionsCovered)/$($totals.FunctionsTotal)) |")
    $markdown.Add('')
    $markdown.Add('## Per production source')
    $markdown.Add('')
    $markdown.Add('| Source | gcda records | Lines | Branches executed | Branches taken | Calls | Functions |')
    $markdown.Add('| --- | ---: | --- | --- | --- | --- | --- |')
    foreach ($row in $rows) {
        $markdown.Add(('| `' + $row.Source + '` | {0} | {1} ({2}/{3}) | {4} ({5}/{6}) | {7} ({8}/{9}) | {10} ({11}/{12}) | {13} ({14}/{15}) |' -f $row.Records, (Get-Percent $row.LinesCovered $row.LinesTotal), $row.LinesCovered, $row.LinesTotal, (Get-Percent $row.BranchesExecuted $row.BranchesTotal), $row.BranchesExecuted, $row.BranchesTotal, (Get-Percent $row.BranchesTaken $row.BranchesTotal), $row.BranchesTaken, $row.BranchesTotal, (Get-Percent $row.CallsCovered $row.CallsTotal), $row.CallsCovered, $row.CallsTotal, (Get-Percent $row.FunctionsCovered $row.FunctionsTotal), $row.FunctionsCovered, $row.FunctionsTotal))
    }
    $markdown.Add('')
    $markdown.Add('## Production sources not compiled by a registered instrumented test')
    $markdown.Add('')
    if ($uncompiledSources.Count -eq 0) {
        $markdown.Add('None.')
    }
    else {
        foreach ($source in $uncompiledSources) {
            $markdown.Add(('- `' + $source + '`'))
        }
    }
    $markdown.Add('')
    $markdown.Add('## Ignored gcda records')
    $markdown.Add('')
    $markdown.Add("$($skippedDataFiles.Count) test/generated records were ignored by the production-source filter.")

    $markdownPath = Join-Path $reportRoot 'coverage-summary.md'
    $jsonPath = Join-Path $reportRoot 'coverage-summary.json'
    [System.IO.File]::WriteAllLines($markdownPath, $markdown, (New-Object System.Text.UTF8Encoding($false)))
    [PSCustomObject]@{
        GeneratedAt = (Get-Date).ToString('o')
        Scope = 'Non-vendored production .cpp files compiled by registered instrumented test projects'
        Excluded = @('src/DropWidgets/Plots/qcustomplot.cpp', 'generated Qt/build sources', 'tests/**')
        Totals = $totals
        Files = $rows
        UncompiledProductionSources = $uncompiledSources
        IgnoredGcdaRecords = $skippedDataFiles
    } | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $jsonPath -Encoding UTF8

    Write-Host "Combined coverage report: $markdownPath"
    Write-Host "Combined coverage data: $jsonPath"
}
finally {
    $env:Path = $oldPath
}
