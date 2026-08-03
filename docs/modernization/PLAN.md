# Modernization plan

## Current milestone

**Milestone 2A — local qmake test infrastructure:** completed on 2026-08-03.
The existing PlotMeasurements test is registered in a single executable local
test flow; test and fixture directory structures and coverage evidence are in
place. No production code, CMake build, new behavioral tests, or coverage
threshold was added.

Acceptance evidence: the documented `tests/run-tests-msys2.ps1` command was
run successfully against the unchanged PlotMeasurements test. GCC/gcov coverage
instrumentation was also built and run successfully as a source-level pilot.

## Ordered milestones

1. Repository audit, reproducible baseline, and risk register — completed.
2. Test harness, CTest integration, fixtures, coverage, and CI — in progress.
   Milestone 2A (local qmake test flow) is complete; CTest, fixture content,
   aggregate coverage reporting, and CI remain pending.
3. Characterize external contracts and critical paths — in progress.
   Milestone 3A characterized the DataManagement manager/set/messenger
   subsystem. Phase 3B characterizes LoadSave experiment XML with real
   MainWindow integration, deterministic fixtures and per-file coverage;
   compatible plugin binaries, historic external experiment samples and
   deterministic permission failures remain pending.
4. Isolate pure/core logic from GUI and infrastructure — pending.
5. Ownership/lifetime, undefined behavior, error handling, and security hardening — pending.
6. Build-system and dependency modernization — pending.
7. Subsystem-by-subsystem refactoring with differential verification — pending.
8. Naming/layout cleanup as separate mechanical changes — pending.
9. Documentation, packaging, performance benchmarks, and final compatibility report — pending.

## Current risks

- Only Windows/MSYS2 was runnable; Linux and all cross-platform claims are unverified.
- The test baseline is one PlotMeasurements executable; integration contracts have no fixture coverage.
- qmake has a local test runner but no CTest, aggregate coverage reporting, sanitizer, static-analysis, or CI integration.
- Plugin ABI, XML, binary TCP framing, libmatio/HDF5 output, and UI object names are compatibility-critical.
- Build dependencies are machine-installed rather than locked/reproduced by the repository.

## Milestone 3A completion

DataManagement characterization is complete for the safely isolated manager,
set and messenger boundaries. Ten stable component test IDs cover normal,
empty, repeat, signal, forwarding and ownership behavior. UIDataManagement
integration remains pending real GUI/IO/plugin/export harnesses; no production
refactoring or coverage gate was introduced.

## Phase 3B completion

LoadSave experiment XML is characterized by the real-application contract suite
`XML_001` through `XML_008`. The suite is part of the documented qmake runner
and covers small valid/legacy-shaped fixtures, semantic read/write behavior,
UTF-8, unknown and optional content, malformed/missing files, UI caller return
conventions and figure-window serialization. No production code was changed and
no coverage threshold was enabled. Binary plugin success, a safe corpus of
historic user files, deterministic permission denial, and malformed figure
window widget-count handling remain explicit follow-up risks.

## Phase 3C status

Parameter XML characterization tests `PARAM_001` through `PARAM_009` are
implemented against unchanged production code. On 2026-08-03 the clean
four-project runner, fresh Release build, and fresh Debug build all passed with
MSYS2 runtime directories first on `PATH`. The former `uic.exe`/`rcc.exe`
`-1073741511` issue was an ambient MiKTeX Qt-DLL path conflict and is no longer
a blocker for the documented local workflow. Fresh file-level gcov evidence is
recorded in `PARAMETER_CONTRACTS_3C.md`; no coverage threshold is enabled.
