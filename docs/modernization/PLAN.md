# Modernization plan

## Milestone 2C.1 status

**Reproducible Windows build/test CI:** locally validated on 2026-08-10; remote
GitHub execution is unconfirmed. The workflow uses the existing qmake
Release-build and central test-runner commands with an MSYS2-prioritized PATH
and offscreen Qt GUI tests. A separate fresh-environment job publishes the
combined local coverage report and step summary with no coverage gate. It adds
neither production code nor a coverage gate.

## Milestone 2B record

**Milestone 2B — combined local qmake coverage reporting:** completed on 2026-08-10.
The existing PlotMeasurements test is registered in a single executable local
test flow; the dedicated MSYS2/GCC coverage command aggregates all registered
instrumented tests into documented Markdown and JSON reports. No production
code, CMake build, or coverage threshold was added.

Acceptance evidence: the instrumented runner completed all 11 registered tests,
and the subsequent `-CollectOnly` aggregation completed with exit code 0 from
the complete coverage tree. `COVERAGE.md` records the honest 38-source compiled
production baseline and exclusions; it is not a repository-wide gate.

## Ordered milestones

1. Repository audit, reproducible baseline, and risk register — completed.
2. Test harness, CTest integration, fixtures, coverage, and CI — in progress.
   Milestones 2A (local qmake test flow) and 2B (combined gcov reporting) are
   complete. Milestone 2C.1 adds a locally validated Windows build/test
   workflow; CTest, fixture content, coverage gates, wider CI and remote
   workflow evidence remain pending.
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
- qmake has a local test runner, combined gcov reporting and a locally validated
  Windows CI workflow, but no CTest, coverage gate, sanitizer, static-analysis,
  or remote CI confirmation.
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

## Phase 3D status

MAT export is characterized by `MAT_001` through `MAT_008` with public libmatio
readback, all supported scalar variants, vectors, Unicode, error/overwrite
paths and the real UI caller. The clean runner and fresh Release/Debug builds
passed on 2026-08-04. Per-file coverage is in `MAT_CONTRACTS_3D.md`; no
threshold is enabled. Phase 3D is complete; allocation-failure paths remain a
documented fault-injection limitation. No production code or binary fixture was added.

## Phase 3E status

HDF5 export is characterized by `HDF5_001` through `HDF5_006` using public
HighFive readback and temporary files.  On 2026-08-04 the six-project clean
runner, fresh Release build, fresh Debug build and instrumented HDF5 suite all
passed.  `Export/export2highfive.cpp` has per-file gcov evidence of 100.00%
lines, 92.59% branches executed and 50.93% branches taken at least once; no
coverage gate was added.  The documented test-only seam mirrors the UI error
return convention without entering the production build.  Unsafe null-pointer
paths, deterministic permission/allocation faults and real UI construction
remain follow-up limitations.  Phase 3E is complete; no production source,
dependency, committed HDF5 output or binary fixture was added.

## Phase 3F status

Remote-control characterization is complete with loopback-only `TCP_001`–
`TCP_007`, fresh Release/Debug builds and file-level gcov evidence. The split
clean-runner result, unsafe malformed-frame exclusions and legacy behavior are
documented in `REMOTE_CONTROL_CONTRACTS_3F.md`; no production protocol or
dependency changed.

## Phase 3G status

**Completed 2026-08-04.** The owner approved the minimal security behavior
change for the deterministic incompatible-plugin null dereference.
`LoadPlugin::readDevice` now rejects a null `Platform_Fabric` cast through the
existing Messenger before `GetInterface()` or registration; valid plugin
behavior, public plugin headers and IID `org.qt-project.Qt.Examples.EchoInterface`
remain unchanged. Runtime fixture contracts `PLUGIN_001` through `PLUGIN_007`,
the post-clean full runner and fresh Release/Debug builds passed. The combined
`-Clean` invocation was bounded by the 300-second tool limit after cleanup;
the immediately following run completed the same cleaned tree and is the
evidence used here. File-specific `loadplugin.cpp` coverage is 87.50% lines,
90.62% branches executed, 50.00% branches taken and 76.79% calls. No coverage
threshold was enabled. Test-only seam and remaining platform/ABI/error-path
limits are recorded in `PLUGIN_CONTRACTS_3G.md`.

## Phase 3H status

**Isolierbarer DropWidget-/Indicator-Umfang abgeschlossen (2026-08-04).** `DW_001` through `DW_013` characterize the non-plot
DropWidget adapters, portable UI-loader fixtures, tree MIME source, narrow
manager/messenger forwarding and table-row binding. The normal qmake runner
builds and runs the suite in temporary offscreen mode. The combined clean run
was bounded solely by the 300-second execution limit after cleanup/rebuild;
the immediate non-clean run completed that tree successfully, and fresh
Release/Debug production builds plus file-specific gcov evidence passed. No
production DropWidget source, MIME semantics or defect candidate was changed.
The complete contracts, per-file coverage, function-level gaps, prioritized
follow-up tests, test-only seams and remaining exclusions are documented in
`DROPWIDGET_CONTRACTS_3H.md`.

Phase 3H.5b subsequently closed the isolated nonvisual indicator, timer,
adapter-signal, range, XML and list-mutation gaps with `DW_014` through
`DW_017` (19/19 suite checks passing). The Indicator source files now have
100% line coverage in this focused run; remaining coverage is intentionally
limited to 3I manager/drag/MainWindow, 3J plot, pixel-only rendering and
dangerous null/index paths. Die verbleibenden Manager-, Tree-Drag- und
MainWindow-Pfade sind verbindlich Phase 3I zugeordnet; PlotWidget-/Plotpfade
sind verbindlich Phase 3J zugeordnet. Null-/Indexpfade bleiben bis zu einem
isolierten Defektnachweis ausdrücklich ausgeschlossen. Phase 3H ist damit für
den isolierbaren Umfang abgeschlossen.

## Phase 3I status

**Technisch charakterisiert; Lückenanalyse ausgewertet (3I.4a, 2026-08-04).**
The real offscreen MainWindow suite currently covers `GUI_001` through
`GUI_011`, including construction, standard docks, dynamic UI forms,
manager/messenger forwarding, deterministic modal cancellation, and the safe
Close-Project discard path. Its instrumented per-file gcov evidence and the
complete public/protected/Qt-slot checklist are in
`MAINWINDOW_CONTRACTS_3I.md`; these are not project-wide coverage values.
Subplot action routing, context/tree actions, CLI/tray variants, output menus,
safe nested tree/dock behavior and all dangerous empty-selection/index paths
remain deliberately open. Phase 3I is therefore not marked fully complete.

## Phase 3I completion

**Completed 2026-08-04.** `GUI_001` through `GUI_021` now cover real
MainWindow construction/destruction, docks/forms and recovery,
manager/messenger/tree forwarding, modal cancellation, close-project paths,
subplot/standard/FFT action routing, tray/output/status behavior and safe
valid-selection context actions. The suite is registered in the normal qmake
runner, which sets offscreen and its portable fixture root only for this suite
and restores the caller environment afterward. The combined `-Clean` command
reached the fixed 300-second external limit after cleaning/rebuilding; a first
resumed run also timed out while continuing that tree, and the next non-clean
full runner completed all ten registered projects with exit code 0. Fresh
Release/Debug builds and current per-file gcov evidence are in
`MAINWINDOW_CONTRACTS_3I.md`; they are not project-wide metrics or a gate.

No production source changed. Dangerous empty/invalid selection, null
sender/index, CLI/native-dialog/real-tray and external-file paths stay
documented exclusions. PlotWidget data, curves, FFT calculations, interaction
and rendering are explicitly Phase 3J, not completion criteria for 3I.

## Phase 3J completion

**Completed 2026-08-04 for the characterized PlotWidget/FFT contract scope.**
The normal qmake runner builds and runs offscreen `PlotWidgetContractTests` for
`PLOT_001`â€“`PLOT_006` and `FFT_001`â€“`FFT_008`. The full runner and fresh
Release/Debug builds passed; per-file coverage evidence and numerical gaps are
in `PLOT_CONTRACTS_3J.md`. This is not authorization for a broad PlotWidget
refactor: safe cursor, context-menu, history and quality-criteria paths remain
uncovered, while rendering, gestures and dangerous FFT/index/error paths are
explicit exclusions.

## Milestone 3 characterization closure audit

**Charakterisierung der identifizierten kritischen externen Verträge
abgeschlossen; vollständiges Funktionsregister und Assurance-Gates offen.**
Phases 3A through 3J have reproducible test evidence recorded in their
respective contract documents and are summarized in
`MILESTONE_3_CHARACTERIZATION_REPORT.md`. The one intentional production
behavior change is the separately approved phase-3G incompatible-plugin safety
fix; the other phase scopes did not change production behavior.

This status does not clear the repository for broad refactoring. The
function-level inventory remains incomplete for a number of public/protected
surfaces, and documented safe-to-test plot cursor/context/history/quality
criteria paths are still open. MainWindow native-desktop/CLI paths, third-party
plugin ABI combinations, historic user-data corpus compatibility, OS-level
permission/error injection, visual rendering and deliberately dangerous
null/index/allocation paths require separate treatment. Aggregate coverage,
coverage gates, CTest/CI, sanitizer/static-analysis and multi-platform
verification also remain later milestones.
