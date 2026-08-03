# Modernization plan

## Current milestone

**Milestone 1 — Repository audit, reproducible baseline, and risk register:**
completed on 2026-08-03.  Scope was documentation and executing the unchanged
build/tests only; no production source changes are authorized.

Acceptance evidence: repository/file inventory, exact runnable environment,
qmake commands/results, existing-test result, dependency versions, architectural
map, behavior/test coverage map, and known blockers are recorded in the sibling
documents.

## Ordered milestones

1. Repository audit, reproducible baseline, and risk register — in progress.
2. Test harness, CTest integration, fixtures, coverage, and CI — pending.
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
- qmake has no CTest/coverage/sanitizer/static-analysis/CI integration.
- Plugin ABI, XML, binary TCP framing, libmatio/HDF5 output, and UI object names are compatibility-critical.
- Build dependencies are machine-installed rather than locked/reproduced by the repository.
