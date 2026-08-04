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

## Milestone 2A local test flow

The existing `tests/PlotMeasurementsTests.cpp` is now reproducibly integrated
into the local qmake test flow:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-tests-msys2.ps1 -Clean -Jobs 4
```

On 2026-08-03 this command configured, built, and ran all 1 registered test
projects successfully (exit code 0).  The runner builds only its explicit test
manifest in `build\tests-msys2-mingw64`, invokes each executable, and propagates
configuration, build, and test failures as a non-zero process exit code.

There is still no CTest registration, sanitizer, static-analysis configuration,
or CI workflow.  The fixture directory structure and fixture rules now exist
under `tests/fixtures/`, but no golden data has been created or inferred.

## Coverage feasibility (MSYS2/GCC)

The installed MINGW64 GCC/gcov 15.2 toolchain supports source instrumentation
through `--coverage`.  A pilot built `PlotMeasurementsTests.pro` with qmake
overrides `QMAKE_CXXFLAGS+=--coverage` and `QMAKE_LFLAGS+=--coverage`, ran the
test, and generated a gcov report with:

| Production source | Lines | Branches | Calls |
| --- | ---: | ---: | ---: |
| `DropWidgets/Plots/PlotMeasurements.cpp` | 92.73% (102/110) | 88.64% (156/176) executed | 72.41% (21/29) executed |

`gcov.exe` is installed.  `gcovr` and `lcov` are not installed, although the
MSYS2 package search exposes `mingw-w64-x86_64-lcov` as an available reporting
tool.  The practical next step is a dedicated qmake coverage configuration that
uses `--coverage`, runs the same test runner, filters project sources, and uses
lcov (or an equivalent reviewed reporter) for aggregate HTML/JSON reports.
No coverage gate is enabled: one testable source file cannot honestly establish
a repository-wide threshold.

## DataManagement characterization 3A

On 2026-08-03 the normal local command built and passed both registered qmake
test projects: the existing PlotMeasurements suite and
`component/DataManagementCharacterizationTests` (12 Qt Test checks, 0 failures;
10 stable DataManagement test IDs). Separate per-file GCC/gcov evidence is in
`DATAMANAGEMENT_3A.md`; it is not total-project coverage. UIDataManagement
MainWindow/XML/plugin/export paths remain unverified.

## LoadSave/XML characterization 3B

The local runner now registers `contract/XmlExperimentContractTests`, which
builds the unchanged application graph without `main.cpp` and exercises the
real `MainWindow`/`UIDataManagementSetClass` hierarchy required by the XML
reader and writer. XML fixtures are under `tests/fixtures/xml/`; they are small
UTF-8 test vectors with no absolute paths, user data or plugin binaries.

The phase records the legacy inverted boolean result convention (`false` is
success), ignored unknown content, relative form resolution, malformed and
missing-file failures, semantic write/read behavior, UI caller forwarding and
subplot serialization. Release, Debug and the full three-project runner passed;
the XML suite has 10 Qt Test checks and no failures. Per-production-file gcov
evidence is in `XML_CONTRACTS_3B.md` (reader 71.90%, writer 94.64%, plugin
loader 0.00%, XML caller 38.64% line coverage); these are not a project total
or a quality gate. Permission-denied behavior and successful binary-plugin
loads remain unverified because they are not deterministic or safely available
in this repository.

## Parameter XML characterization 3C

`ParameterContractTests` adds characterization for ParameterLoader,
ExportInputs2Xml and their UI callers with UTF-8 fixtures. On 2026-08-03 the
clean four-project runner passed (7 + 12 + 10 + 11 Qt Test checks, all with zero
failures), and fresh Release and Debug production builds both succeeded. The
earlier `uic.exe`/`rcc.exe` `-1073741511` condition was traced to an ambient
MiKTeX Qt DLL preceding MSYS2 on `PATH`; the runner/build scripts prepend the
MSYS2 runtime directories and now reproduce successfully. Per-file gcov
evidence is: `parameterloader.cpp` 100.00% lines / 100.00% branches executed;
`exportinputs2xml.cpp` 100.00% / 100.00%; and the broader UI caller file
`UIDataManagementSetClass.cpp` 34.09% / 26.88%. These are not project-wide
coverage values or a threshold.

## MAT export characterization 3D

`MatExportContractTests` characterizes libmatio 1.5.28 MAT v5 export through
public libmatio readback. It covers `MAT_001` through `MAT_008`; only temporary
MAT files are used and no libmatio version or production source was changed.
The 2026-08-04 clean five-project runner, fresh Release build, fresh Debug build
and instrumented MAT contract test all passed. `Export/Export2Mat.cpp` gcov
evidence is 98.44% lines and 97.50% executed branches; this is per-file evidence,
not a project-wide metric or threshold.

## HDF5 export characterization 3E

`Hdf5ExportContractTests` characterizes the unchanged HighFive/HDF5 export
through public HighFive readback.  It covers `HDF5_001` through `HDF5_006` with
temporary files only: timestamp, scalar and vector data, nested ID paths,
string/empty data, overwrite/unknown-ID/invalid-path behavior, and the UI
caller's observed catch-and-return convention.  On 2026-08-04 the clean
six-project runner, fresh Release and Debug builds, and the instrumented HDF5
test all passed (exit code 0).  `Export/export2highfive.cpp` gcov evidence is
100.00% lines, 92.59% branches executed and 50.93% branches taken at least
once; it is per-file evidence, not a project coverage threshold.  The focused
test project has a documented test-only UI seam and is not part of the
production qmake target.  Details, defect candidates and limitations are in
`HDF5_CONTRACTS_3E.md`.

## Remote-control characterization 3F

`RemoteControlContractTests` characterizes `TCP_001`–`TCP_007` over loopback.
The initial clean runner was stopped only by the 300-second tool limit after
cleaning its build tree; the immediately following full runner completed that
tree successfully. Fresh Release/Debug builds and the instrumented suite
passed. `RemoteControlServer.cpp` gcov evidence is 87.41% lines, 87.30%
executed branches, 52.38% taken branches and 76.30% calls. See
`REMOTE_CONTROL_CONTRACTS_3F.md`.

## Plugin characterization 3G

Phase 3G completed under an explicitly approved security behavior change. The
unchanged code had a deterministic null-pointer crash for a successfully
loaded incompatible plugin; the defect was demonstrated only in an isolated
process. `LoadPlugin::readDevice()` now has the minimal null-cast guard,
emitting an existing-Messenger error and returning before `GetInterface()` or
registration. Compatible plugins retain the existing IID
`org.qt-project.Qt.Examples.EchoInterface` and successful load/registration
behavior. Runtime-built compatible, wrong-IID and QObject-only fixtures passed
`PLUGIN_001` through `PLUGIN_007`; the complete post-clean runner and fresh
Release/Debug builds passed. The combined `-Clean` command itself reached only
the 300-second tool limit after cleaning, with no observed compiler/test error;
the immediately following full run completed that cleaned tree. File-specific
`loadplugin.cpp` gcov evidence is 87.50% lines, 90.62% branches executed,
50.00% branches taken at least once and 76.79% calls. Public plugin headers and
IID were unchanged; no generated plugin DLL, executable, build or coverage
artifact is versioned. Remaining limits are documented in
`PLUGIN_CONTRACTS_3G.md`.

## DropWidget characterization 3H

`DropWidgetAdapterTests` is registered in the normal qmake runner and runs
offscreen with its caller environment restored afterward. The suite covers
`DW_001` through `DW_013`, portable `.ui` fixtures and narrow test-only seams
for the otherwise excluded MainWindow/PlotWidget graph. On 2026-08-04 the
combined `-Clean` runner hit only the hard 300-second execution limit after
clearing and rebuilding the test tree, without an observed compiler or test
failure; the immediately following runner completed the same tree successfully
(exit code 0). Fresh Release and Debug production builds and the instrumented
DropWidget suite also passed. File-level gcov evidence for adapters, loader,
tree source, table mapping and exercised DataManagement dependencies is in
`DROPWIDGET_CONTRACTS_3H.md`; it is not project-wide coverage or a threshold.

## MainWindow characterization 3I

`MainWindowIntegrationTests` is registered in `tests/run-tests-msys2.ps1` and
executes the real production MainWindow graph offscreen. The runner applies
`QT_QPA_PLATFORM=offscreen` and its portable repository-fixture root only to
that executable and restores the process environment afterward. `GUI_001`
through `GUI_021` passed (23 Qt Test entries including setup/cleanup) on
2026-08-04. The first `-Clean -Jobs 4` runner reached only the hard 300-second
command limit after cleaning/rebuilding, without an observed compiler or test
error; one resumed run also reached that external limit while continuing the
same tree. The next non-clean full runner completed all ten registered suites
with exit code 0. Fresh Release and Debug production builds and a separate
instrumented MainWindow suite also passed. Current per-file gcov evidence is
in `MAINWINDOW_CONTRACTS_3I.md`; it is not project coverage or a threshold.
