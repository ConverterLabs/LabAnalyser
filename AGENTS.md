# LabAnalyser modernization and behavior-preservation contract

This file is binding for every coding agent working in this repository. Read it completely before making changes. If a nested `AGENTS.md` exists, its more specific rules also apply.

## Mission

Modernize, restructure, document, and comprehensively test LabAnalyser while preserving all observable behavior unless a behavior change is explicitly approved.

The order of priorities is:

1. Preserve externally observable behavior and data compatibility.
2. Establish reproducible builds and a trustworthy test safety net.
3. Improve correctness, security, ownership, architecture, and maintainability.
4. Modernize the toolchain, C++ usage, dependencies, and CI.
5. Improve performance only when measured and without changing semantics.

Do not perform a big-bang rewrite. Work in small, reviewable, independently testable increments.

## Product constraints that must remain compatible

Treat these as public contracts until proven otherwise:

- Qt plugin loading, the existing plugin interface types, interface IID, binary boundary, and supported plugin workflows.
- Existing experiment/project XML files, UI files, import/export formats, HDF5 and MAT output through **libmatio**, settings, paths, defaults, and error behavior.
- Remote-control protocol, TCP behavior, message framing, ordering, timeouts, and responses.
- Qt signals, slots, properties, object names used by `.ui` files, drag-and-drop behavior, and user-visible GUI behavior.
- Numerical results, including FFT/statistics behavior, ordering, precision, rounding, NaN/Inf handling, and accepted tolerances.
- Supported operating systems and the documented MSYS2 MINGW64 Qt toolchain.

Never silently change a file format, plugin ABI/API, protocol, numerical convention, default, or user workflow. If modernization requires such a change, first document the proposed migration, compatibility layer, risks, and tests, then stop and ask for approval.

## Non-negotiable rule: characterize before changing

For every production function or cohesive behavior being modified:

1. Identify its inputs, outputs, side effects, dependencies, error paths, boundary cases, and callers.
2. Add tests against the current implementation before refactoring it. These are characterization tests: they record what the software actually does, including surprising behavior.
3. Run and record the tests against the unchanged baseline.
4. Make one focused refactoring step.
5. Run the same tests against the refactored implementation.
6. Compare old and new observable results. They must match exactly unless the behavior contract defines a justified numeric tolerance or nondeterministic field normalization.
7. Commit or checkpoint only with a green build and green tests.

A refactoring is not complete merely because it compiles or because new code has unit tests. The same behavioral test vectors must exercise both the baseline and refactored behavior.

When old behavior is clearly a defect or unsafe, first write a test that demonstrates it and classify it in the behavior inventory. Do not encode dangerous undefined behavior as a required contract. Propose the corrected behavior separately and require explicit approval before changing it.

## Required first phase: audit and baseline

Before broad source changes, create and maintain:

- `docs/modernization/BASELINE.md`: exact build environment, dependency versions, commands, known failures, and baseline test results.
- `docs/modernization/BEHAVIOR_INVENTORY.md`: every production class/function grouped by subsystem, its observable contract, current test IDs, risks, and migration status.
- `docs/modernization/ARCHITECTURE.md`: current dependency map, data flow, plugin boundary, persistence formats, networking, GUI boundaries, and target architecture.
- `docs/modernization/PLAN.md`: ordered milestones, acceptance criteria, risks, and current status.
- `docs/modernization/DECISIONS.md`: short architecture decision records for consequential choices.

Inventory all `.cpp`, `.h`, `.ui`, `.qrc`, `.pro`, scripts, external libraries, generated artifacts, protocols, and file formats. Locate existing tests before claiming the repository has none. The existing `tests/PlotMeasurementsTests.cpp` is part of the baseline and must remain passing.

Treat `LabAnalyser.pro` and the compiling source as authoritative for the current dependency baseline: MAT export uses **libmatio** (`-lmatio`), not matOut. The README's old matOut clone/patch instructions are stale and must be corrected after the build has been reproduced. Do not reintroduce matOut. Record the detected libmatio version and test compatibility before and after any libmatio upgrade.

Build the unchanged repository on every environment that is currently available. If a documented target cannot be run, record it as unverified; never report it as passing.

Capture representative golden fixtures from the baseline for XML read/write, plugin discovery/loading, remote control, MAT/HDF5 export, parameter import, and important GUI workflows. Fixtures must be small, deterministic, free of secrets and personal data, and checked into `tests/fixtures/` when licensing permits.

## Test strategy

Use the lowest-level test that fully verifies the behavior, plus integration tests at important boundaries.

### Unit and component tests

- Prefer Qt Test for Qt/QObject code and existing project consistency.
- Use `QSignalSpy` for signal count, order, and payloads.
- Use `QTemporaryDir`, temporary files, fake clocks, deterministic random seeds, and local loopback servers.
- Separate pure domain/numerical logic from widgets, files, sockets, and global state so it can be tested directly.
- Cover normal cases, empty/min/max inputs, boundaries, malformed input, failures, cancellation, repeated calls, ownership/lifetime, and concurrency where relevant.
- For floating-point code, state the tolerance and why it is valid; do not use an arbitrary broad epsilon.

### Characterization, golden-master, and differential tests

For behavior that cannot initially be isolated safely, test through a stable seam or executable-level harness. Run identical fixtures against the baseline and candidate implementations and compare:

- return values and exceptions/errors;
- emitted signals and their order;
- created/modified files after canonicalizing only documented nondeterministic metadata;
- network bytes and protocol sequences;
- plugin discovery and calls;
- numerical arrays and plots' underlying data;
- persistent state and round trips (`read -> write -> read`);
- GUI state and critical workflows using Qt's offscreen platform where feasible.

Snapshot tests must compare semantic content when incidental formatting is not contractual. Never update golden files merely to make a failure disappear; explain and approve the difference first.

### Required integration suites

Create automated coverage for at least:

- application startup/shutdown and main-window construction;
- plugin success, missing plugin, invalid plugin, ABI/interface rejection, and unloading/lifetime behavior;
- experiment XML round trips and backward-compatible fixture loading;
- data management and messenger signal/data flow;
- parameter import and malformed inputs;
- HDF5 and libmatio-based MAT export content, shapes, MATLAB types/classes, dimensions, names, numeric values, complex values where supported, empty datasets, and error handling;
- remote-control request/response, malformed frames, disconnects, timeouts, and repeated connections;
- drag/drop widget mapping and important UI state transitions;
- plot measurements, FFT, normalization, interpolation, and degenerate numeric inputs.

## Coverage and quality gates

Coverage is evidence, not proof, and never replaces behavioral assertions.

- Generate line, branch, and function coverage in CI for testable production code.
- During characterization, establish the honest baseline; coverage must never decrease.
- Every changed nontrivial production function must have direct or clearly mapped behavioral tests, including its relevant failure and boundary paths.
- Every public/protected production function must appear in `BEHAVIOR_INVENTORY.md` with one or more test IDs, or a documented justification for exclusion (for example trivial Qt-generated forwarding).
- Milestone target: at least 90% line and 80% branch coverage overall, with 100% function coverage for changed code and critical parsers, persistence, protocol, numerical, and plugin-boundary code.
- Do not game metrics with empty assertions, tests of mocks only, ignored code, or broad coverage exclusions.

Run warnings at a strict practical level and introduce formatting/static analysis without reformatting unrelated code in the same change. Add and keep green where supported:

- debug and release builds;
- Clang/GCC warnings and MSVC/MinGW equivalents where applicable;
- AddressSanitizer and UndefinedBehaviorSanitizer on Linux;
- ThreadSanitizer for relevant concurrency tests when the toolchain supports it;
- static analysis (`clang-tidy`, and optionally CodeQL/cppcheck);
- dependency and secret scanning.

## Build-system and language modernization

Do not mix toolchain migration with behavioral refactoring in one step.

1. Make the existing qmake build and tests reproducible first.
2. Add a modern CMake build using target-based CMake, `AUTOMOC/AUTOUIC/AUTORCC`, CTest, and explicit imported dependencies.
3. Keep qmake working until CMake builds the same application/tests and packaging behavior on supported platforms.
4. Remove qmake only after parity is demonstrated and the migration is documented.
5. Select the newest C++ standard supported by all agreed deployment toolchains. Do not simply set C++20/23 without verifying compiler, Qt, plugin, and dependency compatibility.
6. Pin or constrain dependency versions reproducibly and document update policy. Update dependencies one at a time with tests before and after.

Prefer RAII, value semantics, `std::unique_ptr`, scoped connections, `std::span`, `std::optional`, `std::chrono`, range-based loops, `const`, `override`, and explicit ownership where they improve safety. Avoid raw owning pointers, manual `new/delete`, unchecked indexing, C casts, global mutable state, and macros for typed constants. Respect QObject parent ownership and do not wrap parent-owned QObjects in conflicting smart pointers.

Preserve existing source/file names during behavioral work. Large naming or directory migrations must be separate mechanical changes after test coverage exists, with include paths and history kept reviewable.

## Target architecture

Move incrementally toward clear boundaries:

- `app/gui`: widgets, view state, actions, and Qt presentation only;
- `core`: domain/data-processing logic independent of GUI;
- `io`: XML, HDF5, MAT, settings, and parameter adapters;
- `network`: remote-control transport and protocol;
- `plugins`: stable public interfaces and loading adapter;
- `tests`: unit, integration, contract, and fixtures.

Use dependency injection or narrow interfaces for files, clocks, network, plugin loading, and environment access. Do not create abstractions without an observed testing or dependency-boundary need.

## CI requirements

Add GitHub Actions that, as available, build and test on Linux and Windows, cache only safe dependency/build inputs, publish test/coverage reports, and fail on regressions. CI commands must also work locally and be documented in `CONTRIBUTING.md`.

At minimum, each pull request/change set must run:

1. configure and compile from a clean directory;
2. all unit and integration tests via CTest (and the legacy test command during migration);
3. coverage gate on the designated coverage job;
4. sanitizer jobs on supported Linux builds;
5. static analysis/format checks;
6. fixture compatibility and differential tests.

## Work sequence

Execute these milestones in order and keep `PLAN.md` current:

1. Repository audit, reproducible baseline, and risk register.
2. Test harness, CTest integration, fixtures, coverage, and CI.
3. Characterization of external contracts and critical paths.
4. Isolation of pure/core logic from GUI and infrastructure.
5. Ownership/lifetime, undefined behavior, error handling, and security hardening.
6. Build-system and dependency modernization.
7. Subsystem-by-subsystem refactoring with differential verification.
8. Naming/layout cleanup in separate mechanical changes.
9. Documentation, packaging, performance benchmarks, and final compatibility report.

Within a milestone, take one subsystem at a time. Do not modify unrelated subsystems merely because cleanup is possible.

## Agent operating rules

- Begin each work session by reading `PLAN.md`, `BASELINE.md`, recent git diff/status, and applicable instructions.
- Present a short plan before a substantial change, then execute autonomously while the next step is safe and in scope.
- Never hide a failing baseline. Distinguish pre-existing failures from regressions with evidence.
- Never delete or weaken a test solely because refactored code fails it.
- Never claim equivalence based only on code inspection, compilation, or coverage percentage.
- Avoid changing production behavior and tests in the same commit unless the commit is an explicitly approved behavior change.
- Keep commits small and single-purpose. Suggested prefixes: `test:`, `refactor:`, `build:`, `ci:`, `docs:`, `fix:`.
- Do not commit binaries, generated build output, credentials, machine-specific paths, or large real-world datasets.
- Do not modify vendored third-party code such as qcustomplot as if it were project code; isolate, update, or patch it explicitly with provenance and license review.
- Research current official documentation before selecting versions or relying on changed Qt/CMake/compiler behavior. Record version and source in the relevant decision.
- If dependencies or target environments are unavailable, continue with work that can be verified and record the exact blocker. Do not fabricate results.

Stop and ask for a decision only when there is an actual product choice: observable behavior is ambiguous, compatibility must be broken, a dependency/license change is consequential, or required credentials/hardware are unavailable. Otherwise proceed through the plan.

## Required completion report

The modernization is complete only when all of the following are delivered and verified:

- clean documented builds on every agreed supported platform;
- all tests green, including baseline characterization and differential suites;
- coverage gates met with reports;
- sanitizers and static analysis green or every exception explicitly documented;
- all production functions mapped to behavioral tests or justified exclusions;
- backward-compatible plugin, XML, export, network, and GUI contracts demonstrated;
- updated architecture, build, contribution, test, packaging, and migration documentation;
- a final `docs/modernization/COMPATIBILITY_REPORT.md` listing test evidence, coverage, benchmarks, intentional changes, remaining risks, and anything not verified.

The phrase “same behavior before and after” means demonstrated equivalence of observable contracts on documented test vectors and environments. It is not an absolute mathematical proof for all possible inputs. Report that limitation honestly.


## General

Führe lang laufende Befehle im Vordergrund aus oder behalte ihre Prozess-Session. Bei einem Tool-Timeout pollst du dieselbe Session weiter. Gib laufende Prozesse nicht als Endergebnis an.