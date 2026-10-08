# CMake and qmake parity

## Supported initial platform

The additive CMake build is verified locally with MSYS2 MINGW64, GCC 15.2,
Qt 6.9.2, CMake 4.0.2, libmatio 1.5.28, HDF5, HighFive, FFTW3 and Boost.
No dependency was installed, upgraded or fetched for this work. qmake remains
the parallel compatibility build until CMake parity is demonstrated on further
platforms.

```powershell
$env:Path = 'C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:Path
cmake -S . -B build\cmake-msys2 -G 'MinGW Makefiles' -DCMAKE_BUILD_TYPE=Release -DLABANALYSER_BUILD_TESTS=ON
cmake --build build\cmake-msys2 --parallel 2
ctest --test-dir build\cmake-msys2 --output-on-failure --parallel 2
```

`CMakeLists.txt` uses target imports for Qt, pkg-config-backed matio/HDF5/FFTW3,
Boost and the MSYS2 HighFive header location. `AUTOMOC`, `AUTOUIC` and
`AUTORCC` are enabled. Vendored qcustomplot is the internal
`LabAnalyserQCustomPlot` target, not a dependency update or refactoring scope.

## CTest registration

The current twelve qmake-runner projects are registered with stable target
names: `PlotMeasurementsTests`, `DataManagementCharacterizationTests`,
`XmlExperimentContractTests`, `ParameterContractTests`,
`MatExportContractTests`, `Hdf5ExportContractTests`,
`RemoteControlContractTests`, `DropWidgetAdapterTests`,
`PluginLoaderContractTests`, `ProjectIoFacadeContractTests`,
`MainWindowIntegrationTests` and `PlotWidgetContractTests`.

The CMake fixture targets build CompatiblePlugin, WrongIidPlugin,
QObjectOnlyPlugin, MemberOwnedInterfacePlugin and HeapOwnedInterfacePlugin
below `build/test-plugins/`, matching the existing portable contract path.
CTest sets the repository root and plugin-root variables. GUI targets alone
receive `QT_QPA_PLATFORM=offscreen` through CTest properties.

Local evidence: a fresh CMake Release build completed, then all 12 CTest
targets passed (0 failures). The legacy qmake runner was started once and
resumed once in the same non-clean build tree; both invocations reached the
external 120-second command limit without a compiler or test failure before
the limit. It is therefore not claimed locally green for this checkpoint.

## Known parity limits

2026-10-08 deployment regression: the user reported missing toolbar icons in
the CMake application. `resources.qrc` is currently in the static core archive,
without an explicit application resource-initialization reference. The archive
resource object can therefore be omitted at link time. The user requested the
established qmake/MSYS2 build for deployment; CMake resource/visual parity is
not demonstrated, and the deployed application is being replaced through
`scripts/build-msys2.ps1`. No production behavior change is needed for that
build selection.

2026-10-08 Windows application subsystem correction: the first CMake-deployed
EXE had PE subsystem 3 (console), while the existing qmake deployment had
subsystem 2 (GUI). The user reported the extra console window. The application
target now sets `WIN32_EXECUTABLE` on Windows, restoring the existing launch
behavior; test targets retain their console output. The incremental Release
application build passed and `objdump -p` confirmed subsystem 2 before copying
the replacement EXE. Deployment SHA-256 matched the build artifact. Existing
unchanged plot tests were not repeated. Interactive launch after replacement
has not been verified by the agent.

- Debug, Linux, install/package and deployment parity are not yet demonstrated.
- CMake has no install or packaging target yet.
- GitHub Actions keeps the established qmake job unchanged and adds a separate
  Windows MSYS2 CMake/CTest job with the same package basis and failure logs.
