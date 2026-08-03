# Baseline

Date: 2026-08-03.  Repository revision inspected: `0ac0700` (`git describe` used by
qmake produced `1.3`).  The worktree already contained a modification to
`AGENTS.md`; it was not made or changed by this audit.

## Available environment

The only runnable target environment observed is Windows/MSYS2 MINGW64.

| Component | Observed version |
| --- | --- |
| OS/shell | Windows PowerShell 5.1 |
| qmake | 3.1 |
| Qt | 6.9.2 (`C:\\msys64\\mingw64`) |
| GCC | MSYS2 MinGW-w64 GCC 15.2.0 (package `15.2.0-8`) |
| matio | `mingw-w64-x86_64-matio` 1.5.28-1 |
| HDF5 | `mingw-w64-x86_64-hdf5` 1.14.6-3 |
| FFTW | `mingw-w64-x86_64-fftw` 3.3.10-5 |
| HighFive | `mingw-w64-x86_64-highfive` 2.10.1-1 |
| CMake/CTest | installed, but no `CMakeLists.txt` exists |

The qmake project (`LabAnalyser.pro`) is authoritative for this baseline.  It
links `-lmatio`, `-lhdf5`, and `-lfftw3`; no matOut source or link dependency
was found.  The README already describes matio, so no README correction was
required in this milestone.

## Reproduction commands and results

All commands were run without changing production or test source.  Audit build
directories are ignored by Git under `build/`.

| Scope | Command | Result |
| --- | --- | --- |
| Release application + deployment | `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\\build-msys2.ps1 -Configuration release -BuildDir build\\baseline-20260803-release -DeployDir build\\baseline-20260803-deploy -Deploy -Jobs 4` | PASS; produced `release\\LabAnalyser.exe` (1,793,536 bytes) and standalone deployment |
| Plot measurements test build | qmake `tests/PlotMeasurementsTests.pro` in `build\\baseline-20260803-plot-tests`, then `mingw32-make -j4` | PASS |
| Plot measurements test execution | `build\\baseline-20260803-plot-tests\\release\\PlotMeasurementsTests.exe -txt` | PASS (exit code 0) |
| Debug application | `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\\build-msys2.ps1 -Configuration debug -BuildDir build\\baseline-20260803-debug -Jobs 4` | PASS; produced `debug\\LabAnalyser.exe` (100,335,762 bytes) |

The direct invocation of `build-msys2.ps1` was initially rejected by the local
PowerShell execution policy.  The README-documented process-local
`-ExecutionPolicy Bypass` invocation above resolved that environmental blocker.
Deployment emitted a non-fatal `windeployqt` warning that `dxcompiler.dll` and
`dxil.dll` were not found; the script nevertheless completed successfully.

The application compile emitted existing warnings (not errors), including
unused parameters, signed/unsigned comparisons, and Boost's global bind
placeholder deprecation.  They are baseline observations, not changed here.

## Tests and unavailable verification

`tests/PlotMeasurementsTests.cpp` is the only discovered automated test source.
It covers constant, linear, sine/harmonic, non-uniform, and invalid plot-measurement
inputs.  There is no CTest registration, test runner configuration, coverage,
sanitizer, static-analysis configuration, or CI workflow in the repository.

No Linux host/toolchain, plugin fixture, experiment XML fixture, parameter
fixture, MAT/HDF5 fixture, remote-control harness, or GUI offscreen harness was
available.  These compatibility contracts are therefore **unverified**, not
passing.  Capturing their golden fixtures belongs to milestone 2/3 once a safe
test harness exists.

## Known baseline risks / blockers

- The full application depends on absolute Windows library conventions in the
  qmake project/build script and has no committed dependency lockfile.
- Remote-control framing reads `uint32_t` via casts from received bytes before
  validating a minimum frame size; its protocol must be characterized before
  any safety change.
- Plugin IID/API, XML format, Qt object names, and persistence/export behavior
  have no automated compatibility evidence yet.
