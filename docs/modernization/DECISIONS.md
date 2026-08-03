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
