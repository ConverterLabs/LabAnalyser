# Decisions

## ADR-001: Preserve qmake as the baseline build

**Date:** 2026-08-03  
**Status:** accepted for baseline

`LabAnalyser.pro` is the only application build definition.  The audit therefore
uses qmake/MSYS2 and does not introduce CMake.  A target-based CMake build may
only be added in milestone 6 after qmake behavior/build parity is tested.

## ADR-002: libmatio is the MAT export dependency

**Date:** 2026-08-03  
**Status:** accepted for baseline

The project file links `-lmatio` and the detected installed package is matio
1.5.28-1.  matOut is not reintroduced.  Any future libmatio change requires
pre/post export compatibility tests and recorded version evidence.

## ADR-003: Unverified compatibility is not reported as passing

**Date:** 2026-08-03  
**Status:** accepted

Only the executed qmake builds and PlotMeasurements test are passing evidence.
Plugin, XML, export, remote-control, parameter, and GUI workflows remain
unverified until deterministic fixtures/harnesses are added.

## ADR-004: Keep the local test entry point qmake-native during milestone 2A

**Date:** 2026-08-03
**Status:** accepted

`tests/run-tests-msys2.ps1` is the single local test entry point.  It uses the
existing MSYS2 qmake/mingw32-make toolchain and an explicit test-project
manifest, rather than discovering `.pro` files or adding CTest/CMake.  Explicit
registration makes the normal local command deterministic and prevents the
application project from being mistaken for a test.

## ADR-005: Use GCC gcov instrumentation as the first coverage mechanism

**Date:** 2026-08-03
**Status:** accepted for characterization

The observed MINGW64 GCC supports `--coverage`, and the accompanying `gcov`
generated line/branch/call results for the existing PlotMeasurements test.  Use
qmake coverage-flag overrides and the same test executable as the initial
mechanism.  Aggregate reporting requires installing/reviewing a reporter such
as the available MSYS2 `mingw-w64-x86_64-lcov`; no coverage gate is permitted
until broader behavior is characterized.

## ADR-006: Reject incompatible loaded plugins safely

**Date:** 2026-08-04
**Status:** accepted security bugfix

After a successful `QPluginLoader::instance()`, `LoadPlugin` must reject a
null `qobject_cast<Platform_Fabric *>`, send a Messenger error and return
without calling `GetInterface` or registering a device. This is an approved
behavior change for the prior null-dereference defect; IID and ABI remain
unchanged.

The defect baseline was isolated in a separate process. Regression contracts
`PLUGIN_001` through `PLUGIN_007` verify preserved compatible-plugin behavior
and safe rejection of wrong-IID and QObject-only plugins. The source-level
public header/IID comparison is unchanged; it is not a claim of binary
compatibility for every independently built third-party plugin.
