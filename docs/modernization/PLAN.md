# Modernization plan

## Fast local verification and remote checkpoints

**Updated 2026-08-10.** Behavioral contracts and their required coverage are
unchanged. Local fast verification has a total budget of at most five minutes;
already proven unchanged results are not rerun.

| Scope | Required local evidence |
| --- | --- |
| Documentation-only change | Scoped `git diff --check` only; no builds, tests, coverage or analysis. |
| Small behavior-neutral production refactoring | Affected test target; directly dependent contract suites; incremental Release compile check; scoped `git diff --check`; public API/IID check only at a changed boundary. No full runner, Debug build, coverage or static analysis per slice. |
| Subsystem checkpoint | Gather focused evidence from several related slices and push the batch; do not automatically repeat local heavyweight checks. |
| Full local verification | Only at milestone completion, after a build-system/toolchain change, for a CI-specific failure, or when focused tests indicate a possible system-wide regression. |

Do not use `-Clean` unless toolchain/build configuration changed or there is a
concrete stale-build suspicion. If a command is likely to exceed the local
budget, do not start it: use focused verification or CI. Multiple small commits
are pushed together. GitHub CI remains the mandatory complete remote
verification and runs the full Release build, central runner and coverage; no
local batching weakens those remote platform/workflow checks.

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
   compatible plugin binaries and deterministic permission failures remain
   pending. A small anonymized externally supplied experiment corpus is now
   characterized, but full historical UI/plugin/custom-data compatibility is
   still intentionally unverified.
4. Isolate identified pure/core logic from GUI and infrastructure — completed.
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

## Milestone 4 completion

**Completed 2026-08-11.** The identified behavior-preserving internal
isolation slices for DataManagement, project I/O and remote control are
complete. Public facades and their Qt/API, plugin IID and `InterfaceData` ABI
boundaries remain unchanged. The evidence, scope limits and Milestone-5
handover are consolidated in `MILESTONE_4_ISOLATION_REPORT.md`.

This does not claim full GUI independence. `LoadForms()` has no repository
implementation, and no speculative core/GUI abstraction was introduced. The
remaining ownership, safety and behavior decisions are Milestone-5 work; build
system and dependency modernization remain Milestone-6 work.

## Milestone 5A.1 remote-control socket characterization

**Completed 2026-08-11.** 5A.1 characterized the Qt-6 `errorOccurred`
meta-signal and captured the invalid legacy `error(...)` connection warning.
The explicitly approved 5A.2a correction replaces only that connection with a
typed `errorOccurred(QAbstractSocket::SocketError)` connection to the existing
no-op `displayError`; `TCP_013` confirms the warning no longer occurs.
The explicitly approved 5A.2b correction resets current state only for the
disconnecting current socket and schedules every disconnected accepted socket
with `deleteLater()`. `TCP_014..TCP_016` confirm cleared current frame state,
no disconnected-child accumulation, safe server destruction and preservation
of current B when older A disconnects. Real socket-error delivery remains
unverified because it is not deterministic under the current connection.

The two separate approval-required decisions are a typed Qt-6 error-signal
repair with an explicit observable error policy beyond the existing no-op. The
approved disconnected-socket `deleteLater()` policy is now implemented; see
`REMOTE_CONTROL_HARDENING_5A.md` for its boundaries.

## Milestone 5A.3a TCP_007 fixture teardown

**Completed 2026-08-11 without production changes.** An isolated 25-process
diagnosis reproduced the Qt direct-shared-QObject warning and access violation
on process 21 while TCP_007 waited only for client disconnects. TCP_007 now
observes all accepted server sockets through its test-only seam and waits for
their `QPointer` null state plus no remaining `QTcpServer` socket children
before fixture server destruction. Fifty separate post-change runs and the
full focused suite passed. This is a testfixture stabilization, not proof that
all production server-shutdown interleavings are safe; the production-lifetime
risk remains open and 5C stays paused pending a separate decision.

## Milestone 5B.2 remote-control frame validation

**Completed 2026-08-11.** 5B.2a added the private QObject-free
`RemoteControlFrameSplitter::TakeFrame()` now distinguishes `Incomplete`,
`Complete` and `InvalidPrefix` without unaligned reads. It bounds native
`totalSize` to 16 bytes through 1 MiB and clears its direct-use buffer once on
an invalid prefix. `RemoteControlProtocol::DecodeValidatedFrame()` separately
validates complete-frame structure with bounded `memcpy`, exact size sums and
the required ID NUL; valid unknown commands remain distinguishable and ignored
by the existing contract. `TCP_017..TCP_021` passed directly against these
primitives. 5B.2b activates those primitives in `RemoteControlServer`:
incomplete bytes remain buffered; invalid prefixes clear the splitter, abort
the current socket and rely on the existing disconnect/deleteLater path; and
complete structural-invalid or short numeric-set frames are dropped with no
signal/reply while later complete frames can recover on the same connection.
`TCP_022..TCP_026` cover these server outcomes and exact recovery replies.

No public RemoteControl API, signal or slot changed. The native host byte order
remains the compatibility contract. Reply-size limits and payload-semantic
defects remain explicitly open; see `REMOTE_CONTROL_FRAME_HARDENING_5B.md`.

## Milestone 5C.2 String/QStringList text-payload correction

**Completed 2026-08-11 without production changes.** `TCP_027` establishes
the current byte-for-byte final-payload-byte loss for QString and the existing
single-element QStringList mapping, including empty, embedded-NUL, two-NUL and
Latin-1 vectors. `TCP_028` separately establishes that the same shortening
interacts with GuiSelection membership: bare `b` remains unchanged, while
`b\0` selects `b`. The 2021 and 2025 history is inconsistent and supplies no
versioned evidence that `payloadLength` always includes a terminator.

**5C.2b is explicitly approved and implemented.** QString and QStringList set
payloads now preserve every declared byte except one actual final NUL. Latin-1
conversion and the one-element QStringList mapping remain unchanged. TCP_027
uses the baseline vectors from `3244b4f` with the approved results and TCP_029
directly covers helper purity. GuiSelection deliberately remains on the legacy
`left(size - 1)` and membership path documented by TCP_028; bare `b` therefore
still does not select `b`. Get remains asymmetric and unchanged. See
`REMOTE_CONTROL_TEXT_PAYLOADS_5C.md`.

## Milestone 5A.3c RemoteControl fixture teardown

**Completed 2026-08-11 without production changes.** A shared test-only
fixture cleanup now waits for disconnected loopback clients, deferred deletion
of every observed accepted socket and an empty `QTcpServer` socket-child list
before local server destruction. It consolidates the prior intermittent
TCP_007/TCP_013 teardown findings. The process-isolated shutdown harness was
50/50 clean for immediate and drained variants, and 20 complete focused suites
passed without warning, crash or timeout. This is deterministic test cleanup,
not a production-lifetime proof or a replacement for a separately approved
shutdown hardening decision.

## Milestone 5C.3 GuiSelection text-payload correction

**Explicitly approved and implemented.** GuiSelection `set` now uses the same
optional-final-NUL normalization as QString and QStringList, while preserving
its existing membership condition, Latin-1 conversion, one `MessageSender`
emission and container nonmutation. TCP_028 covers bare/trailing/double and
embedded NUL payloads plus a Latin-1 list entry. Get/set asymmetry and any
other InterfaceData conversion remain outside this correction.

## Milestone 5D.2 TCP-set manager integration

**Completed 2026-08-11 without production changes.** `TCP_DM_001` now covers
the missing production-equivalent boundary from a loopback TCP `set` through
`RemoteControlServer::MessageSender`, `MessengerClass::MessageTransmitter`,
`SetData`, `NewDataReceived` and downstream Messenger forwarding. The
existing mapper is updated exactly once observably and the following TCP `get`
returns the new value. The isolated server intentionally remains a transport
and signal source; it does not directly mutate the supplied map. Adding such a
mutation would create a double-mutation and ordering risk, so it is not a
pending product fix.

## Milestone 3A completion

DataManagement characterization is complete for the safely isolated manager,
set and messenger boundaries. Ten stable component test IDs cover normal,
empty, repeat, signal, forwarding and ownership behavior. UIDataManagement
integration remains pending real GUI/IO/plugin/export harnesses; no production
refactoring or coverage gate was introduced.

## Phase 3B completion

LoadSave experiment XML is characterized by the real-application contract suite
`XML_001` through `XML_008`, the figure-window safety vectors `XML_FIG_001`
through `XML_FIG_007`, and the anonymized external-corpus vectors
`XML_LEGACY_001` through `XML_LEGACY_005`. The suite is part of the documented qmake runner
and covers small valid/legacy-shaped fixtures, semantic read/write behavior,
UTF-8, unknown and optional content, malformed/missing files, UI caller return
conventions and figure-window serialization. No production code was changed and
no coverage threshold was enabled. The legacy vectors preserve three real
experiment structures, expected missing UI/plugin dependency handling and a
temporary compatible-plugin replacement boundary. Proprietary plugin/custom
data behavior, full historical UI restoration, deterministic permission denial,
and oversized/allocation-sensitive figure dimensions remain explicit follow-up
risks. The approved 5F.2 fix now rejects excess `PlotWidgetName` elements with
a parser error before indexing the created plot list; fewer names remain
compatible and existing partial figure state is intentionally not rolled back.

On 2026-08-10 the XML suite passed 15 checks and the full 12-project qmake
runner passed after the three anonymized fixtures were added. No XML production
code, plugin ABI, or coordinator delegation changed.

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

## Phase 4H.1 Remote-control frame splitting

**Completed 2026-08-10.** The first remote-control extraction delegates only
raw complete-frame versus remainder splitting to the private, QObject-free
`RemoteControlFrameSplitter`. `RemoteControlServer` remains the public
transport facade and retains all byte decoding, map access, socket writes and
Messenger emission. `TCP_001`–`TCP_008` preserve the prior vectors and add
safe byte-exact request/reply category and coalesced-`get` evidence. No
command, encoding, string/list, Qt-connection, socket-lifetime or public API
behavior was changed. The subsequent protocol and current-connection slices
are completed as the 4H structural checkpoint below.

## Phase 4H Remote-control isolation checkpoint

**Structurally completed 2026-08-11.** 4H.1 `RemoteControlFrameSplitter`
(`20807a3`), the TCP_008 byte characterization (`29edc00`), 4H.2
`RemoteControlProtocol` (`7e098d2`), TCP_009--TCP_012 connection
characterization (`85e8aa7`) and 4H.3 `RemoteControlConnectionState`
(`56c22c9`) preserve the established single-current-connection behavior. The
focused remote-control suite was green after every extraction; production
slices had green incremental Release compile checks; public API, signals and
slots were unchanged; no generated artifacts were versioned.

**4H.4 remains approval-required hardening, not normal refactoring:**

* Qt-6 error-signal and accepted-socket lifetime handling;
* genuinely independent multi-client sessions;
* the String/StringList `set` payload-byte loss;
* Selection and container-mutation semantics.

These behavior/security decisions remain open. Their current legacy behavior,
unsafe exclusions and test limits are preserved in
`REMOTE_CONTROL_REFACTOR_PLAN_4H.md` and `BEHAVIOR_INVENTORY.md`.

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
## Phase 4G.2a Project I/O export coordination extraction

**Completed 2026-08-10.** A private QObject-free `ProjectIoCoordinator` now
performs only per-call construction/execution of parameter import, parameter
XML export, MAT export and HDF5 export adapters with a non-owning manager
reference. `UIDataManagementSetClass` remains the public QObject facade and
keeps all status/error emissions, signal order and legacy booleans, including
the HDF5 error-emission/`false` return contract. Experiment XML, form loading,
plugin loading, private paths/dirty state, MainWindow routing and process
state remain untouched.

Unchanged UIIO, parameter, MAT and HDF5 contracts; the full 12-target runner;
fresh Release and Debug builds; and the scoped warning build passed. Focused
instrumentation recorded `UIDataManagementSetClass.cpp` at 87.36% lines
(76/87), 82.02% executed branches (146/178), 44.94% branches taken (80/178)
and 72.06% calls (98/136); the new coordinator is 100.00% lines (18/18),
100.00% executed branches (8/8), 62.50% branches taken (5/8) and 77.27% calls
(17/22). These are per-file UIIO-vector measurements, not project coverage;
the earlier parameter-suite facade figure used a different denominator.

## Phase 4G.2b Experiment-read delegation

**Completed 2026-08-10.** `LoadExperiment()` remains the unchanged public
QObject facade for `LoadPath`, return/error conventions and `CloseProject`
routing. Its one reader-construction operation now delegates to the private,
non-owning `ProjectIoCoordinator`, which instantiates the unchanged
`XmlExperimentReader` with the exact historic UI-facade/Messenger/parent
hierarchy. Experiment saving, form implementation, plugin loader, private
path/change state, and reader/writer internals remain intentionally outside
this slice.

The central instrumented runner completed all 12 registered projects green,
including XML (`XML_001..XML_008` and `XML_LEGACY_001..XML_LEGACY_005`), UIIO,
Plugin and MainWindow suites. Fresh Release and Debug builds passed. Current
combined-run file measurements are `UIDataManagementSetClass.cpp`: 91.03%
lines (71/78), 82.86% executed branches (145/175), 46.29% branches taken
(81/175), 73.88% calls (99/134), 100.00% functions (10/10); and
`ProjectIoCoordinator.cpp`: 100.00% lines (22/22), 100.00% executed branches
(10/10), 60.00% branches taken (6/10), 76.92% calls (20/26), 100.00% functions
(6/6). These are combined registered-test measurements, not project coverage;
they are not directly comparable with the earlier focused UIIO denominator.

The scoped additional-warning build completed with no own-production warning
in `own-production-warnings.txt`; clang-tidy and cppcheck remain unavailable.
All five committed legacy fixture hashes were unchanged before/after testing.
Rollback is local: restore the direct reader construction in `LoadExperiment()`
and remove `ReadExperiment()`; no public API, Qt metaobject member, plugin IID
or InterfaceData ABI changed.

## Phase 4G.2c Experiment-write delegation

**Completed 2026-08-10.** `SaveExperiment()` remains the public Qt slot and
facade for its inverted error return, status/error text, plugin-save routing
and private path/change state. Only construction/execution of the unchanged
`xmlexperimentwriter` is now delegated to private, non-owning
`ProjectIoCoordinator::WriteExperiment()`, with the exact former UI
facade/Messenger/parent-manager arguments. XML writer implementation, format,
paths, formatting, settings/UI state and overwrite semantics were not altered.

The central instrumented runner completed all 12 registered projects green,
including the full XML/legacy, UIIO and MainWindow suites. Fresh Release and
Debug builds passed. Current combined-run measurements are
`UIDataManagementSetClass.cpp`: 90.91% lines (70/77), 82.35% executed branches
(140/170), 46.47% branches taken (79/170), 74.24% calls (98/132), 100.00%
functions (10/10); `ProjectIoCoordinator.cpp`: 100.00% lines (26/26), 100.00%
executed branches (12/12), 58.33% branches taken (7/12), 76.67% calls (23/30),
100.00% functions (7/7); and unchanged `xmlexperimentwriter.cpp`: 50.89%
lines (57/112), 25.86% executed branches (60/232), 13.79% branches taken
(32/232), 28.00% calls (49/175), 100.00% functions (9/9). These are combined
registered-test measurements, not project coverage; their denominator differs
from earlier focused XML coverage.

The scoped additional-warning build remained at the established 161 filtered
own-production diagnostics, with no diagnostic in `ProjectIoCoordinator.cpp`.
Legacy fixture hashes matched the manifest before/after. Rollback is local:
restore the direct writer construction in `SaveExperiment()` and remove
`WriteExperiment()`; no public API, Qt metaobject member, plugin IID or
InterfaceData ABI changed.

## ProjectIoCoordinator subsystem checkpoint

**Completed 2026-08-10.** The planned ProjectIoCoordinator extraction is
complete for all implemented adapter operations: parameter import/export,
MAT/HDF5 export, experiment read/write and plugin descriptor loading. The
coordinator remains private, non-QObject and non-owning. The public
`UIDataManagementSetClass` remains intentionally responsible for mutable path
and change state, return conversion, signals/status/errors, Messenger follow
ups and MainWindow/UI orchestration. `LoadForms()` remains blocked because the
repository declares it without an implementation; no semantics were invented.

The subsystem checkpoint passed: the non-clean central runner completed all 12
registered targets (including XML/legacy, UIIO, parameter, MAT, HDF5 and
plugin contracts); fresh Release and Debug builds passed; and scoped warning
analysis completed with the established 161 own-production diagnostics, none
in `ProjectIoCoordinator.cpp`. Combined test measurements are
`UIDataManagementSetClass.cpp`: 90.67% lines (68/75), 81.71% executed branches
(134/164), 46.34% taken branches (76/164), 75.00% calls (93/124), 100.00%
functions (10/10); `ProjectIoCoordinator.cpp`: 97.22% lines (35/36), 100.00%
executed branches (24/24), 62.50% taken branches (15/24), 69.05% calls
(29/42), 100.00% functions (8/8). These are file-level combined-test results,
not project coverage. Public API/Qt-metaobject/IID checks, scoped diff and
artifact checks are the remaining documentation-commit validation below.

Legacy XML compatibility stays mandatory: committed fixture hashes must remain
unchanged, fixtures must never be overwritten, and legacy read/write/read
contracts continue to write only temporary targets.

## Phase 5E.2a plugin ownership-fixture characterization

`PLUGIN_008..PLUGIN_011` add two runtime-built, test-only ownership models at
the existing plugin boundary: a member-owned interface that must never be host
deleted, and a separately heap-allocated interface that receives one controlled
fixture-only rest cleanup after manager destruction. They characterize only
safe loading, pointer/QObject observation, the absence of implicit manager
device cleanup, and local loader/root persistence. They do not generalize to
third-party plugins and do not alter the IID or ABI.

The explicitly deferred 5E.2b decision is the dangerous boundary itself:
whether and under which versioned contract `DeviceRegistry` may delete a real
plugin-provided `Platform_Interface`. Member-interface deletion, retained
interface unload, null plugin returns, cross-CRT ABI combinations and stale
raw-pointer accesses remain excluded until a separately approved hardening
slice.

The focused evidence is green: portable fixture build exit code 0 and
`PluginLoaderContractTests` exit code 0 with 13 Qt Test checks. The manager
destruction observation is deliberately narrow: it is not proof that explicit
device cleanup or arbitrary plugin unloading is safe.

## Phase 5E.3b device cleanup strategy preparation

**Completed 2026-08-11.** `DeviceRegistry` now internally records each active
device as raw interface pointer, descriptor path and private cleanup strategy.
The unchanged public `AddDevice()` always selects `HostDelete`, as do every
currently reachable production registration and explicit cleanup path.
`RetainLegacyPlugin` and `PluginReleaseV2` are intentionally inactive enum
values only; this slice adds no loader persistence, Messenger connection
tracking or logical legacy removal.

The preceding isolated diagnosis found deterministic `0xC0000374` heap
corruption when a member-owned fixture reaches any existing host-delete path,
while the heap fixture was deleted exactly once by all three paths. The
unchanged focused DataManagement and Plugin suites passed before this structural
preparation is recorded: later activation requires separate legacy-removal
characterization and an approved loader-lease strategy.

## Phase 5E.3c1 successful plugin-loader leases

**Completed 2026-08-11.** Successful existing plugin registrations transfer
their uniquely owned `QPluginLoader` into an internal `PluginLeasePool` owned
by the current application. The pool never explicitly unloads a loader and
does not give loaders QObject parents. Transfer happens only after the existing
successful instance/cast/interface/registration sequence; duplicate names and
failed load/IID paths do not add leases.

`PLUGIN_012` and `PLUGIN_013` join the unchanged plugin contracts. This is not
the approved Legacy retain behavior yet: DeviceRegistry still uses only
`HostDelete`, and no cleanup, Messenger connection, IID or plugin-interface
semantics changed. The following slice must characterize and then activate
logical Legacy removal separately.

## Phase 5E.3c2 Legacy-V1 plugin ownership fix

**Completed 2026-08-11.** The explicitly approved parallel strategy is active:
only successful Legacy-V1 loads through the private loader path receive
`RetainLegacyPlugin`; direct public `AddDevice()` callers still receive
`HostDelete`. Logical removal drops the active record, preserves the historic
per-operation path semantics, disconnects only the recorded Messenger/plugin
QObject pair, and deliberately neither deletes the interface nor unloads its
application-lifetime lease. `PLUGIN_014..PLUGIN_019` cover the safe member and
heap fixture models, targeted Messenger disconnect, reload and unchanged
HostDelete behavior. `PluginReleaseV2` remains a future versioned IID/API
decision. Legacy plugin resources and threads may remain active until process
end; no real third-party ownership matrix is claimed.

The fast evidence is limited to rebuilt fixtures, Plugin contracts (21 passed),
DataManagement contracts (35 passed), and an incremental Release compile. The
direct UIIO target did not complete its full application-graph regeneration
within the local tool limit and is therefore explicitly unverified for this
commit rather than reported green; it remains a focused follow-up before any
Project-IO change.

## Phase 5E Legacy-V1 plugin ownership hardening

**Completed 2026-08-11.** The approved compatibility strategy is complete for
Legacy-V1: `4fc5aad` selects `RetainLegacyPlugin` only for private successful
loader registrations, while public `AddDevice()` remains `HostDelete`.
Logical removal removes registry access and recorded Messenger connections but
does not host-delete or unload the Legacy interface; loader, root and
interface remain resident until process end. The consequence is deliberate:
legacy resources, threads and hardware state can persist until exit, and no
absolute third-party plugin compatibility claim is made.

GitHub Actions run `31519846064` confirms the normal Windows qmake build and
central test job, including the previously locally unconfirmed UIIO/
ProjectIoFacadeContractTests boundary. Combined coverage is 49.79% lines
(3088/6202), 44.21% executed branches (4777/10805), 24.68% branches taken at
least once (2667/10805), 40.16% calls (3053/7602), and 66.51% functions
(423/636); no coverage gate is active. Lifecycle-V2, with a new IID and a
plugin-side release operation, remains separately approval-gated.

## Milestone 5 QSlider equal-bound hardening

**Explicitly approved and implemented.** `QSliderD::SetVariantData()` now
skips only the scaling/set-value operation when `MinMax.first ==
MinMax.second`; it does not alter non-equal range rounding, slider ranges,
manager bindings, XML or signals outside that unsafe input. `DW_018` covers
floating, signed and unsigned updates, absence of manager/value signals, and
subsequent signal usability. The unsafe baseline division/cast was documented
without executing undefined behavior in-process.

## Phase 5F.2 XML FigureWindow name-bound hardening

**Explicitly approved and implemented.** `XmlExperimentReader::CreateFigureWindow()`
now checks the discovered PlotWidget count before every `PlotWidgetName` index
access. An excess name raises the deterministic parser error `Figure window
contains more PlotWidgetName elements than created PlotWidgets.` and preserves
the established `true == error` result. `XML_FIG_001..XML_FIG_007` cover exact,
fewer and absent names, unknown children, safe empty grids and the retained
partial state of a multiwindow error. This is a narrow OOB prevention change:
no XML format, writer, dimension normalization, allocation limit or rollback
semantics changed. Oversized dimensions and partial-state rollback remain
open hardening risks.

## Milestone 5 package A: crash and null safety

**Completed locally; remote package checkpoint pending.** The small commits
`a3f0bea`, `d62e37e`, `9dc011b`, `0760d11` and `9def039` add only approved
no-op guards for senderless DataManagement slots, unknown/missing manager
inputs, absent Messenger hierarchy, null QObject bindings and invalid
MainWindow action selections. `DM_SAFE_001..003` and `GUI_SAFE_001` preserve
normal sender, Messenger and valid GUI-action contracts while preventing the
former direct dereferences. The scoped details and explicit stale raw-pointer
exclusion are in `CRASH_NULL_SAFETY_5A.md`.

The unresolved nonnull stale `FormP` boundary is not safely detectable under
the historical name-keyed, raw-pointer binding contract. It remains Milestone
5 ownership work; this package deliberately does not introduce automatic
binding cleanup, object-name migration, QObject ownership or new visible
errors. The one non-clean central runner was resumed once in its existing build
tree after the external 120-second limit, but neither invocation completed the
full graph; no compiler or test failure was observed before the second limit.
GitHub CI is the pending complete package validation.

## Milestone 5 package B: numerical and plot hardening

**Completed locally; remote package checkpoint pending.** Commits `9b01417`,
`b0d3dbe`, `ad3aaa8` and `7a9431e` harden only degenerate PlotWidget and
PlotMeasurements inputs: FFT inputs below two samples, test-only FFTW
allocation/plan failure, zero-frequency quality indices, empty XY vectors and
pure normalized-sample boundaries. Valid FFT amplitudes, bins, tolerances,
styles, time/frequency toggling and qcustomplot remain unchanged. Focused
offscreen PlotWidget evidence is 20 passing Qt Test entries; PlotMeasurements
has 8 passing entries, both exit code 0. See `NUMERICAL_PLOT_HARDENING_5B.md`.

The package does not claim rendering, gestures, valid cursor readout/history,
context workflows, non-finite FFT policy, large resource limits or real
allocator failure behavior. Those remain distinct hardening risks; GitHub CI
is the next full package checkpoint.

## Milestone 5 package C: network and persistence hardening

**Completed locally; remote package checkpoint pending.** `c69ffd3` bounds
encoded remote `get` replies to 1 MiB, with checked `quint64` arithmetic before
allocation/conversion and current-socket abort/recovery rather than a partial
or empty success response. `TCP_030..TCP_031` preserve valid reply bytes and
cover the direct limit plus loopback recovery. The XML dimension slice adds
`XmlFigureDimensions` before figure creation: rows/columns are integral
`0..32`, their product is at most 256, and invalid values retain reader
`true == error` with deterministic text and no created figure. `XML_FIG_008`
covers direct and real-reader boundaries; zero-sized legacy grids remain valid.

No arbitrary parameter/XML file-size cap was introduced because no further
narrow, format-neutral integer/allocation guard was evidenced. Existing
PlotWidgetName partial-state behavior, XML format, native TCP byte order,
valid replies and legacy fixtures remain unchanged. GitHub CI is the remaining
package checkpoint; local focused RemoteControl and XML suites passed.

## MATLAB remote connector checkpoint

**Implemented locally; GitHub execution pending.** The connector's exported C
ABI and native TCP frames were characterized against the unchanged Boost-based
implementation, then preserved through a Winsock/RAII refactor. A standalone
CMake build, focused CTest, static MinGW runtime linkage and exact
top-level `+LabAnalyser` deployment is integrated into the Windows build.
The workflow verifies the ten-file MATLAB package before uploading
`LabAnalyser-windows-release`. Local package/build evidence is recorded in
`MATLAB_REMOTE_CONNECTOR.md`; actual MATLAB execution and the next remote
workflow run remain unverified.

The approved follow-up I/O hardening adds an absolute two-second deadline for
each complete connector send/receive. The pre-change silent-peer vector waited
until the fixture closed at 2.6 seconds; post-change silent and partial replies
time out, discard the socket and make subsequent calls return promptly, while
immediate peer close does not wait for the deadline. Existing successful wire
vectors and the MATLAB package contract remain green.

The approved wildcard follow-up is implemented as a compatible extension.
`TCP_032` first characterized `*` and `*Buffer*` as literal, empty queries,
then verifies case-sensitive whole-ID glob matching for all/contains/prefix/
suffix/no-match/repeated-star cases. Exact IDs and the legacy no-star substring
search remain unchanged. MATLAB expands a wildcard list into exact requests
and returns a map of `x`/`y` structs; the additive `IsConnected` DLL export
turns a mid-transfer disconnect into `LabAnalyser:ConnectionLost`. Focused
server and connector contracts are green; execution in MATLAB and the full
Windows GitHub build remain pending.

The release layout was corrected on 2026-08-12: the CMake install prefix is
now the deployment root itself. Consequently `LabAnalyser-windows-release`
contains `./+LabAnalyser` directly instead of
`./LabAnalyser/+LabAnalyser`; CI asserts that exact location and file set.
