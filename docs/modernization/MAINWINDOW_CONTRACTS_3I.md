# MainWindow contracts 3I

## Phase 3I.4a coverage and contract-gap analysis

This is an analysis-only phase.  The existing real `MainWindowIntegrationTests`
target was rebuilt with MSYS2/GCC `--coverage`, run once offscreen, and passed
all 13 checks (`GUI_001` through `GUI_011`, `initTestCase` and
`cleanupTestCase`; exit code 0).  The first full instrumented build exceeded
the 120-second tool limit while compiling the same build tree, without a
compiler error; the immediately resumed `mingw32-make -j4` completed it and
the instrumented test then passed.  No production or test source changed in
this phase.

The following are **per-file gcov measurements from that MainWindow suite**,
not project coverage and not a quality gate. “Branches executed” is the share
of branch sites reached; “branches taken” is the share taken at least once.

| Production file | Lines | Branches executed | Branches taken | Calls |
| --- | ---: | ---: | ---: | ---: |
| `mainwindow.cpp` | 54.41% (475/873) | 53.48% (938/1754) | 28.68% (503/1754) | 46.70% (510/1092) |
| `DataManagement/UIDataManagementSetClass.cpp` | 29.55% (26/88) | 25.81% (48/186) | 12.90% (24/186) | 24.16% (36/149) |
| `UIFunctions/SubPlotMainWindow.cpp` | 92.00% (23/25) | 100.00% (18/18) | 61.11% (11/18) | 77.78% (14/18) |
| `DataManagement/DataManagementClass.cpp` | 56.22% (122/217) | 57.58% (152/264) | 34.47% (91/264) | 45.51% (71/156) |
| `DataManagement/DataManagementSetClass.cpp` | 79.41% (27/34) | 85.71% (72/84) | 44.05% (37/84) | 64.81% (35/54) |
| `DataManagement/DataMessengerClass.cpp` | 53.57% (30/56) | 51.11% (46/90) | 27.78% (25/90) | 45.28% (48/106) |
| `DropWidgets/DropWidgetsUiLoader.cpp` | 47.17% (25/53) | 52.78% (38/72) | 30.56% (22/72) | 15.71% (11/70) |
| `DropWidgets/QLineEdit.cpp` | 25.88% (22/85) | 8.64% (14/162) | 4.32% (7/162) | 18.18% (18/99) |
| `RemoteControl/RemoteControlServer.cpp` | 4.76% (6/126) | 4.55% (12/264) | 2.65% (7/264) | 6.78% (8/118) |

`qcustomplot` is vendored and excluded from this analysis. `PlotWidget` is
linked only because the real MainWindow graph requires it; 3I does not treat
its metrics as Plot contract coverage (3J).

### MainWindow function checklist

“Direct” means a GUI test calls the function or QAction. “Indirect” means Qt
dispatch or an exercised caller reaches it. “Not run” means no meaningful path
is characterized. The risk rating is for a behavior-preserving refactor made
without a further contract test.

| Function / visibility | Evidence | Status | Open path / phase | Risk |
| --- | --- | --- | --- | --- |
| `MainWindow(QWidget*)` public | GUI_001–011 | direct | CLI `-load`/`-tray`, tray availability | high |
| `~MainWindow()` public | GUI_001; test teardown | direct | destruction after live dialogs/forms | high |
| `GetStatusBar()` public | GUI_002; constructor graph | direct | none beyond trivial accessor | low |
| `GetLogic()` / `UI()` public inline | GUI_001–011 / none for `UI()` | direct / not run | `UI()` accessor only | low |
| `CreateSubPlotWindow()` public | GUI_005, GUI_012 | direct | 2x3 and 1x2 grids, repeat close/recreate; FFT/data/rendering/failures → 3J | high |
| `CreateFFTPlotWindow()` public | GUI_017 | indirect | QAction routing/window and registered target; FFT plot semantics → 3J | high |
| `DeleteFigure()` public | GUI_005 via `SubPlotMainWindow::closeEvent` | indirect | unknown/duplicate figure identifiers | medium |
| `Info(QString)` public slot | none | not run | status/message formatting | medium |
| `Error(QString)` public slot | GUI_006 malformed/missing form | indirect | output formatting and loading delay | medium |
| `closeEvent(QCloseEvent*)` public slot | none | not run | unsaved window-exit Save/Discard/Cancel | high |
| `CloseProject()` public slot | GUI_003, GUI_007 | direct | `isloading` timer branch; plots/forms mixed | high |
| `resizeEvent(QResizeEvent*)` public slot | show/float tests | indirect | exact column ratios and tiny sizes | medium |
| `changeEvent(QEvent*)` public slot | GUI_003 minimize/restore | indirect | activation/window-state variants | medium |
| `dockWidget_topLevelChanged(bool)` public slot | GUI_004 | indirect | dynamic left dock/tab restoration | high |
| `dockWidget_destroyed(QObject*)` public slot | GUI_006, GUI_007 | indirect | dock with PlotWidget, repeated destruction | high |
| `HighLightConnection(QString)` public slot | GUI_014 | direct | valid nested Parameter/Data/State leaf selection; missing IDs remain untested | high |
| `RemoveConnection(QString)` public slot | none | not run | currently empty implementation | low |
| `TrayIconActivated(...)` public slot | GUI_003 | direct | non-double-click reasons and system tray | medium |
| `AddElementToWidget(QString, InterfaceData)` public slot | GUI_009, GUI_014 | direct | nested Parameter/Data/State leaves, manager update and repeat-ID deduplication; removal paths remain open | high |
| `LoadFormFromXML(path, name, skip)` public slot | GUI_006–008, GUI_011 | direct | duplicate same name; path without extension; `QDir` side effect | high |
| `PublishFinished()` / `PublishStart()` public slots | none | not run | tree sorting/update state | medium |
| `OutputTextMenu(QPoint)` public slot | GUI_013 | direct | standard context menu and Clear Output action; other standard actions remain Qt-owned | medium |
| `ErrorWriter(...)` public slot | GUI_006 errors, GUI_013 | direct | long-message trim; loading timer/scroll remains open | medium |
| `InfoWriter(...)` / `NotificationWriter(...)` public slots | GUI_013, GUI_018 / GUI_018 | direct / indirect | `InfoWriter` block-limit and conditional autoscroll; notification append/order; output-dock raise remains open | medium |
| `eventFilter(QObject*, QEvent*)` protected | GUI_004, GUI_006–008 | indirect | dynamic-dock leave/resize, close double invocation | high |
| `AppendWidgetNames(...)` private slot | GUI_006–008 | indirect | PlotWidget registration and duplicate objects | high |
| `ChangeMinMaxValue()` private slot | GUI_016, GUI_019 | direct | valid cancel/commit/inverted bounds; empty selection indexes `selectedItems()[0]` (**unsafe**) | high |
| `RemoveDevice()` private slot | GUI_021 | direct | valid test-device removal; no confirmation/cancel path exists; empty/invalid sender remains open | high |
| `SetAlias(QString)` / `RemoveAlias(QString)` private slots | GUI_016, GUI_020 / GUI_020 | direct / direct | valid cancel, Unicode, empty and removal; multi-ID concatenation is a defect candidate | high |
| `contextMenuTreeWidget(...)` private slot | GUI_016 | direct | valid parameter leaf exposes Min/Max; empty selection is **unsafe** | high |
| `contextMenuTreeWidgetState(...)` / `contextMenuTreeWidgetData(...)` private slots | GUI_016 | direct | State leaf exposes no action; Data single/multi leaf Alias actions; remove-device path remains open | high |
| `on_actionLoad_Form_triggered()` | GUI_009 abort | direct | accepted form picker → GUI_006 already direct loader, action success not covered | medium |
| `on_actionBeenden_triggered()` | none | not run | full application close and unsaved prompt | high |
| `on_actionCreatePlot_triggered()` | GUI_017 | direct | repeated 1x1 figure and Plot target registration; plot semantics → 3J | medium |
| `on_actionCreate_Subplot_triggered()` | GUI_012 | direct | cancel, 2x3 and 1x2 selection, repeated creation; plot semantics → 3J | high |
| `on_actionFFT_triggered()` | GUI_017 | direct | FFT QAction creates/registers a 1x1 target; FFT data semantics → 3J | high |
| `on_actionLoadPlugin_triggered()` | GUI_009 abort | direct | accepted descriptor is PLUGIN_3G contract | medium |
| `on_actionSave_Experiment_triggered()` | GUI_009–010 abort | direct | successful write is XML_3B; empty-path emission is characterized | high |
| `on_actionLoadExperiment_triggered()` | GUI_009 abort | direct | dirty Save/Discard and accepted experiment are XML_3B | high |
| `on_Close_Project_triggered()` | GUI_003, GUI_011 | direct | Save branch, window-exit parity | high |
| `on_actionDaten_Exportieren_mat_triggered()` | GUI_009 abort | direct | successful export is MAT_3D | medium |
| `on_actionSave_triggered()` | GUI_009–010 abort | direct | nonempty `SavePath` write branch → XML_3B | high |
| `on_actionMinimize_to_Tray_triggered()` | GUI_003, GUI_013 | direct | repeated minimize/restore QAction and DoubleClick cycles; real system-tray availability remains platform-dependent | medium |
| `on_actionLoad_Parameter_File_triggered()` | GUI_009–010 abort | direct | successful import is PARAM_3C | medium |
| `on_actionSave_Parameter_Set_triggered()` | GUI_009 abort | direct | successful export is PARAM_3C | medium |
| `on_actionAbout_LabAnalyzer_triggered()` | GUI_009 | direct | current no-op only | low |
| `on_actionAbout_triggered()` | GUI_009 | direct | parentless allocation/title overwrite | medium |
| `on_pushButton_clicked()` | none | not run | no matching UI signal; stale auto-connect warning | low |
| `on_actionExport_Data_h5_triggered()` | GUI_009 abort | direct | successful export is HDF5_3E | medium |
| `on_actionRemote_Connection_Port_2_triggered()` | GUI_009 | direct | displayed bound port, repeated calls | medium |
| `ParseInputArguments()` private | constructor only | indirect | `-load`, `-tray`, malformed argument sequence | high |
| `SelectedItems(...)`, `AddSelectedItems(...)` private | GUI_009 export selection | indirect | nested/select-parent/dedup/error paths | high |
| `RemoveElementFromWidget(QString)` private | none | not run | nested tree deletion/missing paths | high |

`SubPlotMainWindow` is immediately exercised but does not expand this table:
its constructor, `GetStatusBar()` and `closeEvent()` are covered through
`GUI_005`; its close event assumes a non-null central widget and a real
MainWindow owner, so invalid-owner/empty-central-widget paths remain unsafe and
are not in-process contracts.

### Prioritized next tests

1. **Guarded context actions:** first use nonempty leaf selections only for
   Alias/MinMax/remove-device actions. Empty-selection/index paths must remain
   isolated or child-process defect evidence, never normal in-process tests.
2. **MainWindow/3J boundary:** test QAction routing to standard/FFT/subplot
   windows and rows/columns without asserting plot calculations; put PlotWidget
   data, rendering and FFT semantics in 3J.
3. **Later fault isolation:** exercise command-line `-load`/`-tray`, window
   close Save branches, valid experiment loading, and output-writer trimming
   only after their persistence/process side effects have dedicated fixtures.

## MainWindow safe action, tree and recovery expansion 3I.4b

`GUI_012` through `GUI_015` extend the same real offscreen MainWindow target.
The suite uses a temporary settings root and working directory, and controls
the two modal code paths with a zero-delay `QTimer` on Qt's event loop; it uses
no sleeps, native dialogs, user files or external services. The rebuilt suite
exited with code 0: all 17 Qt Test entries passed (the 15 `GUI_001`–`GUI_015`
tests plus `initTestCase` and `cleanupTestCase`).

`GUI_012` triggers the actual `actionCreate_Subplot` connection. Rejecting its
dialog creates no `SubPlotMainWindow`; accepting the existing 2x3 and 1x2
choices creates distinct figure windows which close cleanly, including a
repeat cycle. This covers form-dialog routing and figure lifecycle only:
PlotWidget data, FFT, rendering and numerical behavior remain 3J contracts.

`GUI_013` characterizes two repeated tray cycles through the real minimize
action, the private restore QAction and `TrayIconActivated(DoubleClick)`. It
also opens the OutputText standard context menu deterministically, triggers
its production `Clear Output` action, and verifies clearing. `ErrorWriter`
marks a message longer than the current 10,000-character threshold as trimmed;
111 `InfoWriter` calls retain no more than the existing 100 document blocks.
The offscreen backend warns that it cannot raise windows or grab a keyboard;
these are headless-platform limitations, not desktop behavior claims.

`GUI_014` publishes real nested `Parameter::Group::Gain`,
`Data::Group::Samples` and `State::Group::Mode` values through the real
Messenger/manager. It verifies the category/group/leaf hierarchy, displayed
IDs and typed payload values, a manager update, repeat insertion without a
second leaf, and valid-leaf `HighLightConnection` selection for every tree.
Missing IDs and empty selections are deliberately not invoked because the
selection/index paths remain unsafe candidates.

`GUI_015` loads the portable `mainwindow-bindings.ui` fixture twice with
temporary form names. It verifies the form/dock record, central-widget
visibility transition, close/removal, and independent recreation with the
second name. This is the observable form/dock recovery behavior presently
available through `LoadFormFromXML`; there is no separate compiled production
`LoadForms` recovery routine to characterize here.

Newly recorded risks are intentionally unchanged: direct drag events still
cannot supply Qt's private `QDrag` source; context actions retain unsafe empty
selection/index paths; output notification and scroll/loading-timer behavior
remain untested; and real system-tray presence cannot be demonstrated by the
offscreen platform. The existing malformed/unsupported UI partial-load and
global-current-directory side-effect candidates also remain open.

## MainWindow context actions, routing and timers 3I.4c

`GUI_016` through `GUI_018` retain the real MainWindow graph and execute only
valid leaf selections. The target rebuilt successfully and the offscreen suite
completed with exit code 0: all 20 Qt Test entries passed (`GUI_001` through
`GUI_018`, initialization and cleanup). No production file changed.

`GUI_016` publishes valid nested Parameter, Data and State leaves and calls
the existing context-menu slots through Qt's meta-object system. A Parameter
leaf exposes exactly the enabled, visible `Change Min/Max Values` action. A
single Data leaf exposes `Set Alias`; invoking it through a zero-delay event
loop driver commits `First Alias` to `Data::Device::First`, proving its ID
routing. The next Data menu exposes both `Set Alias` and `Remove Alias`. Two
valid selected Data leaves still expose `Set Alias`. A State leaf exposes no
menu because the production State custom-context connection is commented out
and the direct State slot only handles top-level items. No empty selection,
top-level removal or invalid index is invoked.

`GUI_017` triggers `actionCreatePlot` twice and `actionFFT` once. It records
the three QAction emissions, `Figure#0` through `Figure#2`, each registered
as a 1x1 window, and `Plot#1` through `Plot#3` as targets parented to the
matching figure central widgets. All figures close cleanly. These assertions
cover action routing, IDs and target ownership only; no curve, sample, FFT or
rendering contract is inferred.

`GUI_018` verifies ordered Messenger notification payloads and that repeated
notifications append rather than replace prior OutputText content. A
`StatusMessage` displays `GUI18 -> working` and the current one-second status
timer clears it through a bounded `QTRY` event-loop wait. With the scrollbar at
the bottom, repeated `InfoWriter` calls keep it at the bottom; when moved to
the top, a subsequent entry preserves that position. Destroying a heap-owned
MainWindow while its status-message timer is pending completes without a
callback crash. The test does not access a destroyed object.

Remaining risks at the end of 3I.4c were Parameter Min/Max commit and context
paths using empty or top-level selections. The multiple-Data-leaf branch
concatenates selected IDs without a delimiter in the current implementation,
so its actual alias commit was pending characterization. Real system-tray
availability, notification dock raising, loading-delay timers, and all
PlotWidget values, curves, rendering and FFT semantics remain outside this
phase (3J or later).

## MainWindow mutable context actions 3I.4d

`GUI_019` through `GUI_021` run only valid, explicitly created Parameter/Data
leaves and one test-only device root in the existing offscreen target. The
rebuilt target and full suite exited with code 0; all 23 Qt Test entries passed
(`GUI_001`–`GUI_021`, initialization and cleanup). The test device implements
only the public `Platform_Interface` contract, is owned by the real manager,
and is removed before the test returns.

`GUI_019` opens `Change Min/Max Values` for `GUI19Device::Parameter::Gain`.
The stored initial pair is `(1.25, 5.5)`, while the current first dialog
displays `1.3` because its default decimal precision rounds the value. Rejecting
both dialogs preserves the stored pair. Accepting `-3` then `9` stores that
pair and emits exactly one manager `MessageSender("get", ID, InterfaceData())`
request after the dialogs. Accepting `10` then `-2` stores `(10, -2)` unchanged:
the current path has no Min-versus-Max validation. This is a defect candidate,
not a desired range contract.

`GUI_020` records Data alias behavior for valid leaves. Cancelling `Set Alias`
preserves the ID as its alias; a Unicode alias (`Mäßwert Ω`) is stored; an
accepted empty alias is rejected by the current `!Alias.isEmpty()` guard; and
`Remove Alias` restores the manager alias to the ID. Tree leaf text remains
the original leaf name throughout. For two valid leaves the dialog receives
the actual concatenated string `GUI20Device::Data::FirstGUI20Device::Data::Second`
without a delimiter; committing stores an alias under that synthetic key and
does not alter either individual alias. This is a confirmed defect candidate.

`GUI_021` observes that `Remove Device` is not a confirmation workflow. Opening
the valid device-root menu preserves manager and tree state, but triggering its
single enabled action deletes the `Platform_Interface` immediately, removes
the matching top-level root from Parameter/Data/State trees and clears their
selections; no manager `MessageSender` is emitted. There is no Cancel dialog or
separate confirmed branch to test. The manager container for
`GUI21Device::Channel` remains after device removal, a further cleanup/semantic
defect candidate. Empty selections, invalid senders and unknown device roots
remain excluded as potentially unsafe paths.

## Phase 3I completion evidence

Phase 3I is complete for the real MainWindow workflows characterized by
`GUI_001` through `GUI_021`. The normal `tests/run-tests-msys2.ps1` runner now
builds and executes `integration/mainwindow/MainWindowIntegrationTests` after
the existing unit/component/contract targets. It sets `QT_QPA_PLATFORM=offscreen`
and `LABANALYSER_TEST_REPOSITORY_ROOT` only while that executable runs, then
restores the caller's prior environment. The repository root makes all GUI
fixtures portable independently of the qmake build directory.

The first combined `-Clean -Jobs 4` invocation was stopped solely by the fixed
300-second command limit after removal and rebuild of its test tree, with no
observed compiler or test failure. Its first resumed non-clean run also reached
that external limit while continuing the same tree. The next resumed non-clean
full runner completed successfully with exit code 0 and all ten registered
test projects, including all 23 MainWindow Qt Test entries (21 GUI IDs plus
initialization and cleanup). Fresh qmake Release and Debug production builds
also completed with exit code 0. Existing compiler warnings were observed but
not changed in this phase.

The following are current **per-file gcov results from the instrumented
MainWindow suite**, not project coverage or a quality gate. “Branches
executed” means reached branch sites; “branches taken” means branches taken at
least once.

| Production file | Lines | Branches executed | Branches taken | Calls |
| --- | ---: | ---: | ---: | ---: |
| `mainwindow.cpp` | 76.98% (672/873) | 75.60% (1326/1754) | 41.62% (730/1754) | 66.21% (723/1092) |
| `DataManagement/UIDataManagementSetClass.cpp` | 29.55% (26/88) | 25.81% (48/186) | 12.90% (24/186) | 24.16% (36/149) |
| `UIFunctions/SubPlotMainWindow.cpp` | 92.00% (23/25) | 100.00% (18/18) | 61.11% (11/18) | 77.78% (14/18) |
| `DataManagement/DataManagementClass.cpp` | 73.73% (160/217) | 75.00% (198/264) | 47.73% (126/264) | 67.95% (106/156) |
| `DataManagement/DataManagementSetClass.cpp` | 88.24% (30/34) | 90.48% (76/84) | 46.43% (39/84) | 70.37% (38/54) |
| `DataManagement/DataMessengerClass.cpp` | 67.86% (38/56) | 75.56% (68/90) | 43.33% (39/90) | 66.98% (71/106) |
| `DropWidgets/DropWidgetsUiLoader.cpp` | 47.17% (25/53) | 52.78% (38/72) | 30.56% (22/72) | 15.71% (11/70) |

Excluded and retained for later work: dangerous empty/invalid selection and
sender paths; command-line `-load`/`-tray`; native dialog behavior; accepted
real user-file workflows; real system-tray availability; loading-delay and
notification-dock timing; and external plugin/network success paths. Plot data,
curves, FFT algorithms, plot interaction and rendering remain explicitly 3J,
even where 3I proves figure/action/target lifecycle. Defect candidates retained
without repair are the stale `on_pushButton_clicked` auto-connect warning,
global CWD mutation in `LoadFormFromXML`, partial unknown-widget form loads,
cancelled save/path/dirty-state mutations, unparented About ownership/title,
direct-drag source limitation, Min/Max display rounding and inverted-range
acceptance, delimiter-free multi-ID aliasing, immediate device removal without
confirmation, and retained data containers after device removal.

## MainWindow internal isolation follow-up

The focused follow-up keeps all public MainWindow slots and UI object names in
place while reducing repeated implementation detail. `MainWindowOutputLog`
owns only output formatting; `MainWindowTreePath` owns only the legacy `::`
tree-ID construction; and `MainWindowTreeViewState` owns only batch publish
view state. `GUI_022` covers the latter, while GUI_013/018 cover output and
the existing GUI action tests cover tree-ID consumers. `GUI_SAFE_002..004`
cover null figure/dock and extensionless form-path guards.

`GUI_SAFE_005` covers a registry entry whose named dock no longer exists.
`CloseProject()` removes that orphaned record through the existing manager
facade instead of repeatedly looking up a null dock. Normal live dock close
and registry cleanup routing remain unchanged.

`GUI_SAFE_006` verifies that the temporary `Clear Output` action belongs to
its temporary standard context menu. Repeated menu use no longer retains
otherwise unreachable actions under `MainWindow`; the visible menu action and
clear behavior remain those covered by GUI_013.

`GUI_SAFE_007` verifies that a failed form load retains neither a form record
nor the temporary parentless tab created for loading. The existing corrupt-form
message and valid dynamic-form path remain unchanged.

GUI_009 and GUI_012 additionally verify that the synchronous About and
Subplot-selection dialogs are released after `exec()`. Their titles, actions,
accept/reject behavior and resulting valid subplot creation remain unchanged.

`MainWindowSubplotDialog` now contains only the local row/column selection UI.
`MainWindow` remains responsible for applying an accepted selection through
`CreateSubPlotWindow`; GUI_012 covers the unchanged cancellation and 2×3/1×2
creation paths.

`GUI_SAFE_008` verifies that the temporary Set/Remove Alias actions are owned
by the temporary data context menu. The existing valid alias action, ID and
multi-selection behavior remain covered by GUI_016 and GUI_020.

Not extracted: dynamic form/dock lifecycle, project close/load/save flow,
plot/figure creation, command-line handling and dialogs. They each combine
MainWindow, XML/legacy-fixture, DataManagement or QObject-lifetime semantics;
their existing risks remain documented rather than being moved behind an
untested abstraction.
