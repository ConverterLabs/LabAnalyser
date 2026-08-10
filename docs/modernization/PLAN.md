# Modernization plan

## Milestone 2C.1 status

**Reproducible Windows build/test CI:** locally validated on 2026-08-10; remote
GitHub execution is unconfirmed. The workflow uses the existing qmake
Release-build and central test-runner commands with an MSYS2-prioritized PATH
and offscreen Qt GUI tests. A separate fresh-environment job publishes the
combined local coverage report and step summary with no coverage gate. It adds
neither production code nor a coverage gate.

## Sanitizer pilot status

**Blocked by the installed MSYS2 MINGW64 sanitizer runtime.** GCC 15.2.0
accepts `-fsanitize=address,undefined`, but the linker cannot resolve `-lasan`
or `-lubsan`. The focused pilot script fails explicitly before building an
uninstrumented fallback. No project sanitizer result is claimed, no package was
installed or updated, and no sanitizer CI gate is enabled. See `SANITIZERS.md`.

## Static-analysis pilot status

**Baseline recorded on 2026-08-10; focused QSlider correction verified.** The dedicated GCC 15.2 warning build with
`-Wall -Wextra -Wpedantic -Wformat=2 -Wshadow` completed successfully without
`-Werror`. clang-tidy and cppcheck are not installed; no package was changed.
The original filtered baseline had 163 diagnostic lines, including one possible
uninitialized QSlider value. The explicitly approved minimal QSlider fix and
its `DW_016` regression now produce 162 lines with zero
`-Wmaybe-uninitialized` diagnostics; 23 signed/unsigned comparisons, 35 Qt
deprecations and the existing style categories remain visible. No warning was
suppressed and no mandatory analysis gate was added. A separate numeric
equal-Min/Max division risk remains documented as an open QSlider defect
candidate. See `STATIC_ANALYSIS.md`.

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

## Phase 4A DataManagement refactoring analysis

**Planning complete; implementation pending.**
`DATAMANAGEMENT_REFACTOR_PLAN_4A.md` defines compatible, rollbackable
DataManagement slices. The recommended first slice extracts only value-only
form/skip-form, alias and plot/window registry bookkeeping behind
`DataManagementClass`. Container/device ownership, widget signal routing and
UI/IO orchestration remain separate later slices and must retain their existing
facades, Qt signal/slot contracts and plugin/data ABI.

## Phase 4B.1 DataManagement registry characterization

**Completed 2026-08-10.** `DM_REG_001..DM_REG_005` recorded the public-facade
semantics for form/skip-form, alias and plot/window number state before moving
it. The tests capture duplicate/order/removal, alias fallback and numbering
history without coupling to an internal registry.

## Phase 4B.2 DataManagement registry extraction

**Completed 2026-08-10.** The first rollbackable implementation slice extracted
only form files, skip-form flags, aliases, plot pointers and plot/window number
state into a private RAII-owned `DataRegistry` behind the unchanged
`DataManagementClass` facade. `DM_REG_001..DM_REG_005` retained their observed
duplicate/order/removal, alias-fallback and numbering semantics without test
changes. The focused 17/17 DataManagement suite, all 11 central runner targets,
fresh Release and Debug builds, and scoped static-analysis build passed (the
unchanged 162 filtered diagnostics include three signed/unsigned diagnostics
relocated with the extracted loops). The
focused file metrics are recorded in `BEHAVIOR_INVENTORY.md`; they are not a
project coverage gate. No public API, Qt signal/slot, plugin/data ABI or
persistence contract changed. Container/device ownership, widget mappings and
Messenger/UI orchestration remain explicitly deferred to later 4A slices.

## Phase 4C.1 DataManagement container-ownership characterization

**Completed 2026-08-10.** `DM_CONT_001..DM_CONT_005` define the public-facade
baseline for container map identity, known/missing lookup behavior, the empty-ID
QObject lookup insertion side effect, mapper replacement, metadata/binding
preservation, lexical map enumeration, cleanup, foreign-QObject survival and
manager-instance isolation. Released mapper pointers are documented as invalid
and are never dereferenced by tests.

## Phase 4C.2 DataManagement container-owner extraction

**Completed 2026-08-10.** `ContainerStore` now privately owns the existing raw
mapper map behind the unchanged `DataManagementClass` facade. `DM_CONT_001..005`
passed unchanged (22/22 focused DataManagement checks); the central runner's 11
registered targets, fresh Release/Debug builds, and scoped static-analysis
build passed. The actual exposed map address remains stable, and legacy lookup,
replacement, cleanup and foreign-QObject behavior remains preserved.

The mutable raw map returned by `GetContainerPointer()` remains an explicit
ownership/API boundary. No parallel smart-pointer map or copy is introduced;
the Store deletes only mappers and never form QObjects. File-specific coverage
is recorded in `BEHAVIOR_INVENTORY.md` and is not a project gate. Widget ID
mapping (`ElementsToContainerID`), devices, Messenger and GUI/IO remain out of
scope for the next slice.

## Phase 4D.1 DataManagement widget-binding characterization

**Completed 2026-08-10.** `DM_BIND_001..DM_BIND_006` record the current public
facade behavior of `ElementsToContainerID` and bound mapper-object lists before
any extraction. They cover valid/unknown lookup insertion, ordinary repeat and
rebind behavior, both removal overloads, project cleanup, QObject lifetime and
non-ownership, Messenger feedback from a bound widget, the PlotWidget duplicate
registration exception, two live QObjects with the same name, renaming and
return-to-name routing, empty object names, and instance isolation.

The resulting risks are explicit: bindings are keyed by object name rather than
QObject identity; QObject destruction leaves a stale mapper pointer/name
binding until cleanup; and repeated `PlotWidget` registration causes duplicate
manager-to-widget updates. These remain characterization evidence and are not
repaired. The following 4D.2 extraction, if approved, is limited to a private
WidgetBindingRegistry and must retain all `DM_BIND_*` behavior; ContainerStore,
DataRegistry, devices, Messenger and GUI/IO remain outside its scope.

## Phase 4D.2 DataManagement widget-binding extraction

**Completed 2026-08-10.** `WidgetBindingRegistry` privately owns only the
legacy name-to-container-ID value map. `DataManagementClass` remains the public
facade and still coordinates mapper binding-list changes, `ContainerStore`
cleanup and the PlotWidget duplicate-registration exception. The unchanged
`DM_BIND_001..DM_BIND_006` suite passed (32/32 focused checks), all 11 central
runner targets passed, fresh Release/Debug builds passed, and the scoped
static-analysis build retained the existing 162 diagnostics with none in the
new registry.

The registry deliberately does not own or observe QObjects: empty names,
name collisions, non-migrating renames, stale binding pointers and foreign
QObject non-ownership remain the characterized legacy behavior. File-specific
coverage is documented in `BEHAVIOR_INVENTORY.md`, not as a project gate. The
next slice must not absorb ContainerStore, DataRegistry, Messenger, device or
GUI/IO responsibilities.

## Phase 4E.1 Messenger command-dispatch characterization

**Completed 2026-08-10.** `DM_MSG_001..DM_MSG_003` characterize all known
`MessengerClass::MessageReceiver` commands and the `MessageTransmitter`
forwarding boundary without production changes: signal order/counts,
IDs/payloads, manager container effects, repeats, safe empty/unknown input and
the valid parent/parent-parent CloseProject path. The next possible slice is a
private `MessageDispatchPolicy`; it must retain these vectors exactly and may
not absorb transport, plugins, XML, UI or device logic.

## Phase 4E.2 MessageDispatchPolicy extraction

**Completed 2026-08-10.** A private value-only `MessageDispatchPolicy`
classifies the existing case-sensitive command strings into ordered intents;
`MessengerClass` remains the sole QObject/Signal/Slot executor. `DM_MSG_001`
through `DM_MSG_003` remained unchanged and green. No public API, Qt signal,
slot, plugin IID or InterfaceData ABI changed. The remaining unsafe null-parent,
null-sender and destroyed-QObject paths are unchanged exclusions, not fixed by
this refactoring.

## Phase 4F.1 DataManagement device-ownership characterization

**Completed 2026-08-10.** `DM_DEV_001..DM_DEV_004` characterize the unchanged
public device facade with instrumented test-only `Platform_Interface` probes.
They record first-registration/pointer identity, lexical enumeration, duplicate
name/path handling, known/unknown/repeated close, multi-device removal,
re-registration, `CloseProjectLogic` destruction order, external QObject
non-ownership via QPointer and manager-instance isolation. The existing plugin
contracts remain the source of failed-load/recovery evidence.

The observed legacy boundary is intentionally explicit: `RemoveDevices()`
deletes owned interfaces but leaves their paths in the separate path map; a
different pointer rejected by a duplicate name is not adopted and remains the
caller’s cleanup responsibility. Null interfaces, stale raw-pointer access and
explicit plugin unload remain unsafe exclusions. No production code changed;
the focused DataManagement suite (35 checks), plugin contract suite (9 checks)
and central 11-target runner all passed with exit code 0. The runner preserves
native non-zero exit-code failures while allowing expected HDF5 negative-vector
stderr diagnostics to remain nonfatal.
The next potential 4F.2 slice is limited to a private `DeviceRegistry` that
must retain these vectors before separately approved ownership hardening.

## Phase 4F.2 DataManagement DeviceRegistry extraction

**Completed 2026-08-10.** `DeviceRegistry` extracted only the private legacy
device-pointer and path maps behind the unchanged `DataManagementClass` facade.
The unchanged `DM_DEV_001..DM_DEV_004` tests preserve first registration,
lexical order, duplicate rejection, close/removal/re-registration and project
cleanup semantics, including deliberately retained paths after `RemoveDevices`.
The registry has no QObject/QPluginLoader ownership and does not change raw
pointer lifetime at manager destruction.

Focused DataManagement (35 checks), Plugin contracts (9 checks), and the full
11-target central runner passed. Fresh Release and Debug application builds,
the scoped GCC warning build (161 diagnostics; none in `DeviceRegistry.cpp`)
and file-specific coverage are recorded in the inventory and refactoring plan.
Public method/signature, Qt signal/slot, plugin IID and `InterfaceData` header
checks are unchanged; the only `DataManagementClass.h` diff is private storage
delegation. The remaining raw-pointer destruction boundary is explicitly
deferred to a separate approved ownership-hardening slice.

## Phase 4G.1 Project-IO facade characterization

**Completed 2026-08-10.** The new offscreen
`ProjectIoFacadeContractTests` suite is reproducibly registered in
`tests/run-tests-msys2.ps1`; it sets `QT_QPA_PLATFORM=offscreen` only while
this real-MainWindow target runs. `UIIO_001..UIIO_006` passed with temporary
files, directories and runtime-built test plugins, and the central runner
completed all 12 registered test projects with exit code 0.

The façade evidence covers successful/repeated experiment save/load, missing,
malformed and partial experiment handling, form routing, parameter XML,
MAT/HDF5, valid/incompatible/missing plugins, exact public bool conventions,
Messenger ordering and process CWD/locale observations. No production code was
changed. `LoadForms()` remains a declared-but-undefined blocker; private path
and dirty-state fields have no public observation API. The following 4G.2
implementation slice, if separately approved, is limited to an internal
`ProjectIoCoordinator` behind the unchanged facade.
