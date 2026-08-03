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
