# Coverage approach (MSYS2/GCC)

## Current evidence

The local MINGW64 GCC 15.2 and `gcov.exe` support instrumentation.  This pilot
was successfully run on 2026-08-03 from the repository root:

```powershell
$coverageBuild = Join-Path $PWD 'build\coverage-pilot-20260803'
New-Item -ItemType Directory -Path $coverageBuild | Out-Null
$env:Path = 'C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:Path
Push-Location $coverageBuild
& 'C:\msys64\mingw64\bin\qmake6.exe' ..\..\tests\PlotMeasurementsTests.pro -spec win32-g++ 'CONFIG+=release' 'CONFIG-=debug' 'QMAKE_CXXFLAGS+=--coverage' 'QMAKE_LFLAGS+=--coverage'
& 'C:\msys64\mingw64\bin\mingw32-make.exe' -j4
& .\release\PlotMeasurementsTests.exe -txt
& 'C:\msys64\mingw64\bin\gcov.exe' -b -c -o release ..\..\DropWidgets\Plots\PlotMeasurements.cpp
Pop-Location
```

The production file report was 92.73% lines, 88.64% branches executed, and
72.41% calls executed.  It is a characterization measurement for one source
file, not an overall project metric.

## Adopt incrementally

1. Add a separate qmake coverage configuration (never mix it into normal
   release artifacts) with `--coverage` on compile and link.
2. Run the unchanged local test manifest.
3. Use gcov source-level reports until a reviewed aggregate reporter is
   available.  MSYS2 offers `mingw-w64-x86_64-lcov`, but it was not installed or
   modified in milestone 2A.
4. Filter generated Qt/MOC and third-party sources only with documented,
   reviewable rules; never hide untested project production files.
5. Establish an honest aggregate baseline before proposing a gate.  The 90%/
   80% milestone targets do not apply to the present one-module test baseline.

Coverage complements behavioral assertions; it does not establish compatibility.

## Combined baseline (Milestone 2B, 2026-08-10)

The reproducible local command is:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-coverage-msys2.ps1 -Jobs 4
```

It creates a dedicated `build/coverage-msys2-mingw64` tree, builds and runs the
registered qmake tests with `--coverage`, and writes the merged report to
`build/coverage-msys2-mingw64/report/coverage-summary.md` and
`coverage-summary.json`.  The aggregation command can be repeated without
rebuilding when that tree still contains matching `.gcda` and `.gcno` files:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-coverage-msys2.ps1 -CollectOnly -Jobs 4
```

The verified combined baseline covers the 38 non-vendored production sources
compiled by at least one registered instrumented test: **41.32% lines**
(2419/5854), **36.44% branches executed** (3855/10579), **20.40% branches
taken at least once** (2158/10579), **32.98% calls** (2406/7295), and
**61.23% functions** (338/552).  This is an aggregate for compiled production
sources, not repository-wide coverage. `main.cpp` is currently the sole
production `.cpp` without coverage data and is listed explicitly in the report.

The reviewed denominator excludes vendored
`DropWidgets/Plots/qcustomplot.cpp`, generated `moc_*`/`qrc_*`/`ui_*` and all
other build-directory outputs, and `tests/**`.  Header-source diagnostics that
`gcov` emits while resolving relative include entries are non-fatal when gcov
returns zero; a missing production record, absent matching report, or non-zero
gcov exit remains fatal.  No threshold or coverage gate is active.
