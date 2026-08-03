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
3. Characterize external contracts and critical paths — pending.
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
