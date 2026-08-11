# Behavior inventory

Status legend: **baseline tested** = current automated test executed; **mapped,
unverified** = interface/source identified but no behavioral test exists; **third
party** = do not refactor as project code.  This is the milestone-1 source map;
each later change must replace its applicable unverified entry with concrete test
IDs and characterization vectors before modifying it.

| Subsystem | Production files / public surface | Observable contract and risks | Tests | Status |
| --- | --- | --- | --- | --- |
| Startup/UI | `main.cpp`; `mainwindow.*`; `About.ui`; `mainwindow.ui` | Application startup, Qt object names, menus/actions, docks, dynamic forms, project lifecycle. | — | mapped, unverified |
| Data management | `DataManagementClass.*`, `DataManagementSetClass.*`, `DataMessengerClass.*`, `mapper.h` | ID/container registration, alias/min-max, plot/form/device registries, `SetData` and signal ordering. Pointer ownership and lifetime require characterization. | — | mapped, unverified |
| Plugin API | `plugins/platforminterface.h`, `InterfaceDataType.*`, `LoadSave/loadplugin.*` | Binary plugin ABI, `EchoInterface_iid`, `GetInterface`, `GetSymbol`, messages, XML plugin path/error behavior. | — | mapped, unverified |
| Experiment XML | `LoadSave/xmlexperimentreader.*`, `xmlexperimentwriter.*`; `UIDataManagementSetClass::{LoadExperiment,SaveExperiment}` | Read/write `Experiment` elements (Tabs, Devices, Widgets, State, FigureWindows, Connections), relative paths and Qt dock state. Reader/writer use `false` for success; unknown XML is skipped. `errorString()` is empty after parse errors; malformed figure widget counts risk unchecked indexing. | `contract/XmlExperimentContractTests::{XML_001..XML_008}` | baseline characterized; plugin-binary and historic-file corpus unverified |
| Parameter XML | `Import/parameterloader.*`, `Export/exportinputs2xml.*`; `UIDataManagementSetClass::{ImportFromXml,Export2Xml}` | Inverted error booleans; ID/value-only persistence; malformed-file, conversion, duplicate and repeated-load behavior; XML forwarding. | `contract/ParameterContractTests::{PARAM_001..PARAM_009}` | baseline characterized; metadata persistence absent; broader UI export paths unverified |
| MAT export | `Export/Export2Mat.*`; `UIDataManagementSetClass::Export2Mat` | MAT v5 schema, scalar/vector double data, UTF-8 characters, overwrite/error behavior. | `contract/MatExportContractTests::{MAT_001..MAT_008}` | baseline characterized; allocation-failure, HDF5 and matrix/rank-3 sources unverified |
| HDF5 export | `Export/export2highfive.*`; `UIDataManagementSetClass::Export2Hdf5` | HighFive file truncation; `Timestamp`; `::` to `/` paths; numeric/scalar, vector and string data; direct file errors throw while UI catches and returns `false`. Null manager/second vector pointer are unsafe paths. | `contract/Hdf5ExportContractTests::{HDF5_001..HDF5_006}` | baseline characterized; test-only UI return seam; permission/allocation faults and real UI construction unverified |
| TCP remote control | `RemoteControl/RemoteControlServer.*`, private `RemoteControlConnectionState.*`, `RemoteControlFrameSplitter.*` and `RemoteControlProtocol.*` | Loopback port fallback; native binary framing; `set`/`get` bytes, signals, fragmentation, disconnect, current-socket behavior and safe decode/response byte vectors. `TCP_001..TCP_008` cover lifecycle, command/response bytes, safe fragmentation and reconnect; `TCP_009..TCP_012` cover the last-accepted-client transition, discarded old remainder, reply routing and safe client/server teardown. With two live clients, only the last accepted socket is processed; accepting B clears A's fragment state and later A input is not processed. ConnectionState observes but never owns the current socket; protocol/splitter remain QObject-free. | `contract/RemoteControlContractTests::{TCP_001..TCP_012}` | 4H structural isolation complete; 4H.4 hardening requires approval. Accepted server-socket lifetime, unsafe frames and platform faults remain unverified. |
| Plot widgets | `DropWidgets/Plots/PlotWidget.*`, `FFTPlotWidget.*` | Plot configuration, data mapping, legend/cursor/FFT presentation and event behavior. | `contract/plotwidget/PlotWidgetContractTests::{PLOT_001..PLOT_006, FFT_001..FFT_008}` | Time-domain and FFT data contracts characterized; rendering and unsafe paths remain unverified |
| Plot measurements | `DropWidgets/Plots/PlotMeasurements.*` | Normalize sample order; interpolation; interval count/min/max/mean/RMS; THD and NaN invalid cases. | `unit/PlotMeasurementsTests::{constantSignal,linearSignal,sineAndHarmonics,nonUniformSamples,invalidIntervals}`; run by `tests/run-tests-msys2.ps1` | baseline tested; local runner integrated |
| Drag/drop widgets | `DropWidgets/DropWidgetsUiLoader.*`, `DropWidget.h`, `QBLed/QCheckBox/QComboBox/QDoubleSpinBox/QLCDNumber/QLabel/QLed/QLineEdit/QListView/QProgressBar/QPushButton/QSlider/QSpinBox/QTSLed/QTableWidgeD.*`, `CreateID.*` | UI loading, widget-to-ID mapping, drag/drop acceptance, value conversion, XML save/load, signal/UI state. | — | mapped, unverified |
| Indicators | `CustomWidgets/QBLedIndicator.*`, `QLedIndicator.*`, `QTSLedIndicator.*` | LED rendering/state input. | — | mapped, unverified |
| Tree/subplots | `TreeWidgetCustomDrop.*`, `UIFunctions/SubPlotMainWindow.*` | Drop behavior, subplot grid/window identity and lifecycle. | — | mapped, unverified |
| Resources | `resources.qrc`, `icons/*`, `.ui` | Resource paths and Designer object names are externally visible UI contracts. | — | mapped, unverified |
| Plot library | `DropWidgets/Plots/qcustomplot.*` | Vendored chart implementation; preserve provenance and do not modify as project code. | — | third party |

## Complete production source manifest

The qmake application lists 42 `.cpp` translation units and 44 `.h` headers.
The source groups above cover all listed application files: `main`, `mainwindow`,
`TreeWidgetCustomDrop`, `UIFunctions`, `DataManagement`, `LoadSave`, `Import`,
`Export`, `RemoteControl`, `plugins`, `CustomWidgets`, `DropWidgets` and
`DropWidgets/Plots`.  The 2 `.ui` files, 1 `.qrc`, root `LabAnalyser.pro`, test
`.pro`, and `build-msys2.ps1` were also inventoried.  No `.pri`, CMake, Tcl,
batch, shell, or Python project scripts were found.

Function-level acceptance mapping has intentionally not been invented: apart
from the five named PlotMeasurements test slots, no current tests identify
production functions.  Milestones 2 and 3 must add test IDs per public/protected
function and fixtures for each persistence, plugin, export, network, and GUI
contract before refactoring those functions.

## Test-infrastructure status

`tests/run-tests-msys2.ps1` contains the explicit current test manifest and
returns a failing process exit code for qmake, make, or Qt Test failures.
`tests/{unit,component,integration,contract}` establishes placement for future
suites without moving the baseline test.  `tests/fixtures/` contains the
approved empty category structure and rules only; none of its directories is
evidence of a characterized format yet.

## DataManagement 3A characterization

`component/DataManagementCharacterizationTests` adds stable IDs `DM_001` through
`DM_010` for `DataManagementClass`, `DataManagementSetClass`, and
`MessengerClass`; it uses QSignalSpy for message counts, order and payloads.
`UIDataManagementSetClass` was subsequently characterized through its real
`MainWindow` graph by `UIIO_001..UIIO_006`; the focused facade evidence and its
remaining blockers are recorded below. Full function-to-ID mapping, risk
findings and per-file coverage appear in `DATAMANAGEMENT_3A.md`.

## Phase 4G.1 project-IO facade characterization

`contract/projectio/ProjectIoFacadeContractTests` exercises the unchanged
public `UIDataManagementSetClass` facade offscreen, with temporary files,
directories and runtime-built plugin fixtures. `UIIO_001` establishes an
important observability limit: private `LoadPath`, `StdSavePath` and
`ChangeDetected` have neither a Qt property nor a public getter, so their
initial values and transitions are not publicly testable without changing the
API. `LoadForms()` is declared but has no repository definition and is a
separate implementation blocker.

`UIIO_002` records successful save/load and repeated form loading; the reader
loads forms through its direct MainWindow connection, not by emitting the
facade's public `LoadFormFromXML` signal (zero facade emissions). `UIIO_003`
records partial continuation after a missing form, ordered Messenger info then
error events, and the legacy process-CWD mutation to the experiment directory;
the test restores CWD and locale during cleanup. `UIIO_004` covers parameter
XML route/status conventions. `UIIO_005` covers public MAT/HDF5 return paths:
MAT reports an invalid target as `true`, while HDF5 emits `Error("Export
failed...")` but still returns `false`. `UIIO_006` maps valid plugin loading to
`false` and missing/incompatible fixtures to `true`, with exactly one Messenger
error per rejected fixture. All methods retain the established inverted
boolean convention where `false` normally represents success.

Unsafe null-MainWindow, null-device and unimplemented `LoadForms` paths are
excluded; detailed XML, export and plugin content contracts remain owned by
their existing `XML_*`, `PARAM_*`, `MAT_*`, `HDF5_*` and `PLUGIN_*` suites.

## DataManagement registry baseline 4B.1 characterization

`component/DataManagementCharacterizationTests` adds `DM_REG_001` through
`DM_REG_005` against the unchanged public `DataManagementClass` facade, as the
baseline for the planned private registry extraction. `DM_REG_001` maps
`AddFormFile`, `RemoveFormFile`, `GetFormFileCount` and `GetFormFileEntry`:
form entries retain insertion order, duplicate form names are retained, and a
remove call removes only the first matching entry. `DM_REG_002` maps
`AddSkipFormFile`/`GetSkipFormFile`: a later registration overwrites an earlier
flag and `CloseProjectLogic` clears the flags. `DM_REG_003` maps
`SetAlias`/`GetAlias`: an absent alias falls back to its ID, aliases for unknown
IDs are accepted, overwrites take effect, and empty and Unicode aliases are
preserved; no individual alias-removal API exists, while `CloseProjectLogic`
clears all aliases.

`DM_REG_004` maps safe plot/window registration, lookup, deletion, geometry and
numbering calls. Re-registering a name replaces its current QObject/geometry
lookup but leaves duplicate number history; one delete removes only one such
number. Consequently `GetUniquePlotNumber()` and `GetPlotWindowsIncrementer()`
can return `1` after the current named object/window has been removed. An
unknown `GetPlotWindowRowsCols` returns `(0, 0)`; source inspection shows that
this lookup uses insertion-capable map access, so extraction must preserve that
side effect unless separately approved. `DM_REG_005` demonstrates that these
registry values are instance-local and that a destroyed/recreated manager
starts with no prior form, alias or plot state. These unusual outcomes are
legacy contracts/defect candidates, not normalizations to make during 4B.1.

## LoadSave/XML 3B characterization

`contract/XmlExperimentContractTests` maps XML IDs `XML_001` through `XML_008`
to all public XML reader/writer entry points and their UI manager callers. The
test project uses the real application object hierarchy; it has no test-only
seams, so it cannot affect production builds. Detailed mappings, fixture rules,
defect candidates and exclusions are in `XML_CONTRACTS_3B.md`.

`XML_LEGACY_001..XML_LEGACY_005` extend that mapping with three externally
supplied, anonymized historic experiment structures. They verify the current
reader's missing-form/missing-plugin boundary, temporary read/write/read of
the smallest corpus member, and a temporary-only compatible-plugin
substitution. They do not claim compatibility of the historical plugin or its
custom data; absent historical UI files remain explicit external dependencies.
The focused XML suite passed 15 checks and the 12-project central runner passed
on 2026-08-10.

## Parameter XML 3C characterization

`contract/ParameterContractTests` maps `PARAM_001` through `PARAM_009` to the
public parameter loader/exporter surfaces and their UI callers. Private parser
and writer helpers are covered through their only safe public entry points.
Metadata persistence is intentionally unverified as a feature because source
inspection and characterization show it is absent. See
`PARAMETER_CONTRACTS_3C.md` for values, defects and fresh build/coverage
evidence. The former Qt code-generator failure was isolated to an ambient DLL
path conflict and does not block the documented runner.

## HDF5 export 3E characterization

`contract/Hdf5ExportContractTests` maps both public `Export2HDF5` members to
`HDF5_001` through `HDF5_006`.  Its explicitly test-only UI seam represents
the narrow `UIDataManagementSetClass::Export2Hdf5` catch-and-return convention
and is neither included nor linked by `LabAnalyser.pro`.  Public HighFive reads
validate output semantically.  Detailed file structure, coverage, unsafe-path
exclusions and defect candidates are in `HDF5_CONTRACTS_3E.md`.

## Plugin loading 3G characterization

`contract/PluginLoaderContractTests` maps `LoadPlugin::readDevice()` and its
narrow UI return convention to `PLUGIN_001` through `PLUGIN_007`. Runtime-built
compatible, wrong-IID and QObject-only plugins characterize load, rejection,
Messenger, registration, repeat and lifetime behavior. The only production
change is the approved null-cast guard: incompatible loadable plugins are
rejected without `GetInterface()` or registration. The public interface IID is
unchanged: `org.qt-project.Qt.Examples.EchoInterface`.

`DataManagementSetClassTestSeam` is compiled only into the plugin contract
target and does not prove complete GUI integration. Unverified limits include
parser/missing-attribute paths, null Messenger or `GetInterface()` results,
platform loader/permission faults and a third-party binary ABI matrix; see
`PLUGIN_CONTRACTS_3G.md`.

## DropWidget UI loader and tree drag source 3H.2 characterization

The extended `contract/dropwidgets/DropWidgetAdapterTests` maps
`DropWidgetsUiLoader::createWidget`, `TreeWidgetCustomDrop::mimeData` and the
safe `ConnectToID`/table-row boundaries to `DW_006` through `DW_010`.
`DW_006` verifies standard-widget replacement, names, parentage, properties
and accept-drops state. `DW_007` verifies LabAnalyser-specific adapter mapping
and nested UI hierarchy. `DW_008` characterizes missing, malformed and unknown
UI input. `DW_009` verifies source-tree ID serialization and a rejected
foreign drag event. `DW_010` verifies repeat `ConnectToID` update requests and
table XML row serialization/removal.

The `TreeWidgetCustomDrop` MIME contract is a defect candidate: it advertises
`text/uri-list` but its `mimeData` stores only text, so the advertised format
is absent. An unsupported widget class is warned about and omitted while the
containing UI still loads; this partial-success behavior is also a defect
candidate. Full external drop handling, PlotWidget, drag execution, context
menus requiring interactive dialogs and real MainWindow/UI-loader workflows
remain excluded. The test-only PlotWidget link
seam is never invoked and only permits compilation of the real loader's
otherwise out-of-scope PlotWidget branch.

## DropWidget DataManagement bindings 3H.3 characterization

The extended `contract/dropwidgets/DropWidgetAdapterTests` maps the safe
manager/widget boundary to `DW_011` through `DW_013`. `DW_011` uses the real
`DataManagementClass` and `MessengerClass` with a test-target-only narrow
`DataManagementSetClass` forwarding seam: a `set` message updates a linked
`QLineEditD`; real `ConnectToID` emits an ordered `get` request, then
`editingFinished` returns `MessageSender("set", ID, InterfaceData)`. `DW_012`
characterizes two direct connections of
the same widget signal: each user update produces two manager messages; after
the mapping is removed the remaining Qt connections emit no further manager
message because the sender no longer resolves to an ID. This duplicate-connect
behavior is a defect candidate.

`DW_013` characterizes `QTableWidgeD::LoadFromXML`/private delayed `CreateRow`
through two actual numeric containers. With an event-loop bounded `QTRY` it
creates two rows/two columns, binds the generated `QLineEditD` cell to
`DW13::0`, propagates a manager update, and clears the table through the real
`Clear Table` `QAction` without modal interaction. A container with only a
declared type but no typed `InterfaceData` value creates table geometry but no
cell editor; the test therefore initializes the real typed values before
characterizing the valid binding path. The 2-second internal `QTimer` delay is
production behavior; the test does not sleep.

The seam is compiled only into this DropWidget test target because the real
`DataManagementSetClass.cpp` pulls the excluded PlotWidget graph. It mirrors
only the exercised `SetData` and `SendNewValue` forwarding paths and is not
evidence for full UI integration. Directly constructed drag events cannot set
Qt's private drag source; valid source-dependent widget `dropEvent` paths,
missing-manager/null-ID crash paths, interactive context actions and complete
MainWindow workflows remain excluded. The established MIME-format mismatch is
unchanged and remains a defect candidate.

## DropWidget adapters 3H.1 characterization

`contract/dropwidgets/DropWidgetAdapterTests` maps the adapter constructors,
`SetVariantData`, `GetVariantData`, `LoadFromXML`, `SaveToXML`, bit accessors
and `CreateID`/`CreateIDs` to `DW_001` through `DW_005`. The data-driven suite
covers the named non-plot adapters: `QBLed`, `QCheckBoxD`, `QComboBoxD`,
`QDoubleSpinBoxD`, `QLCDNumberD`, `QLabelD`, `QLed`, `QLineEditD`,
`QListViewD`, `QProgressBarD`, `QPushButtonD`, `QSliderD`, `QSpinBoxD`,
`QTableWidgeD` and `QTSLed`. It verifies construction/parenting/defaults and
ID composition (`DW_001`), supported scalar conversion, range behavior and
signal blocking (`DW_002`), string/Unicode, selection and list transfer
(`DW_003`), bit/XML behavior (`DW_004`), and current XML-stub/label/table
special cases (`DW_005`).

Drag/drop event handlers, context-menu editing, `ConnectToID`, real
MainWindow/UI-loader workflows and table row creation are outside 3H.1. They
remain mapped but unverified because they require the intentionally excluded
full UI/drag-drop graph. The suite uses a documented test-only MainWindow host
seam solely for constructor-time manager connections; it is not linked by the
production target and does not validate that graph.

The suite is registered in `tests/run-tests-msys2.ps1` and uses offscreen Qt
only for the runner process. The 2026-08-04 split clean verification, follow-up
full runner, fresh Release/Debug builds and instrumented suite passed. Detailed
per-file gcov values, all fixtures and the test-only seam boundary are recorded
in `DROPWIDGET_CONTRACTS_3H.md`; no production DropWidget source was changed.

## DropWidget isolated closure 3H.5b

`DW_014` through `DW_017` passed offscreen and extend the existing suite to 19
checks. They map all currently safe standalone Indicator colors/state/render
and timer ownership paths, LED XML bits, list deletion, adapter signal
counts/payloads, clamping, repeat assignment, enabled/read-only and parenting.
The `QBLedIndicator` timer is observed active and repeating through the Qt
event loop and is safely destroyed with its owner; it has no public stop API.
`QTSLedIndicator::SetColor("Yellow")` preserves the preceding color, whereas
the separate `YellowColor()` slot selects yellow. This is characterized as
legacy behavior, not repaired. The remaining DropWidget gaps are 3I
manager/drag/MainWindow, 3J PlotWidget, visual-pixel, or unsafe null/index
paths; see `DROPWIDGET_CONTRACTS_3H.md` for fresh per-file gcov comparison.

## QSlider nonnumeric-update correctness fix (2026-08-10)

The GCC static-analysis baseline reported `DropWidgets/QSlider.cpp` may use
`value` uninitialized when an editable but nonnumeric `InterfaceData` reaches
`QSliderD::SetVariantData`. The unsafe calculation was not executed as a
baseline contract. The approved minimal guard now scales only after a supported
floating-point, signed or unsigned value was assigned. `DW_016` verifies the
safe rejection of editable `QString`, `QStringList` and `GuiSelection`: the
connected slider value is unchanged, no `valueChanged` occurs, and its normal
unblocked signal state remains usable. Existing numeric conversion behavior is
unchanged. The distinct `MinMax.second == MinMax.first` numeric division risk
remains an open defect candidate.

## MainWindow integration 3I.1 characterization

`integration/mainwindow/MainWindowIntegrationTests` exercises the real
`mainwindow.cpp`/`mainwindow.ui` target offscreen; it deliberately excludes
only production `main.cpp` so the test-owned `QApplication` can isolate
`QSettings` to a `QTemporaryDir`. `main.cpp` was inventoried: it constructs
the application, assigns the WindowsVista style and application metadata,
constructs `MainWindow`, then enters the event loop. The suite does not claim
to execute that production entry point directly.

`GUI_001` constructs and destroys the real `MainWindow` in a fresh temporary
INI user scope, verifies the parent-owned logic object and empty form/data
state, and leaves no top-level `MainWindow`. `GUI_002` maps the current title
and hierarchy (`LabAnalyser`, `centralWidget`, `menuBarI`, status bar and
toolbar), the File/Plot/Help menus, `ParameterDock`, `StateDock`, `DataDock`
and `OutputDock`, plus all declared actions: `actionBeenden`,
`actionLoad_Form`, `actionCreatePlot`, `actionCreate_Subplot`,
`actionLoadPlugin`, `actionSave_Experiment`, `actionLoadExperiment`,
`Close_Project`, `actionDaten_Exportieren_mat`, `actionSave`,
`actionMinimize_to_Tray`, `actionAbout_LabAnalyzer`,
`actionLoad_Parameter_File`, `actionSave_Parameter_Set`, `actionAbout`,
`actionExport_Data_h5`, `actionRemote_Connection_Port`,
`actionRemote_Connection_Port_2` and `actionFFT`. It records
the current default enabled, unchecked, non-checkable action state and the
right-dock movable/floatable/non-closable feature set. `GUI_003` verifies the
safe no-form `Close_Project` reset and the connected minimize-to-tray/
double-click restore action path. `GUI_004` characterizes repeated hide/show
and floating transitions of a real dock. `GUI_005` exercises the currently
direct `MainWindow::CreateSubPlotWindow` coupling: two 1x1 figure subwindows
receive sequential object/window names and close cleanly. It is lifecycle
coverage only, not PlotWidget semantics (3J).

The real target necessarily links the immediate application graph declared by
`LabAnalyser.pro`, including UIDataManagement, RemoteControl construction,
SubPlotMainWindow, PlotWidget and vendored qcustomplot. No MainWindow seam,
plugin, network action, user file, or real user settings is used. Modal file,
plugin, import/export, parameter, remote-port and About actions remain
inventoried only to prevent blocking interaction; dynamic user-form dock
closure, unsaved-form close prompts and full drag/drop remain subsequent 3I
work. No `QSettings`/`restoreState`/monitor-restoration implementation is
present in `mainwindow.cpp`; dangerous monitor or state-restore inputs were
therefore not manufactured in-process.

Every construction emits the existing Qt auto-connect warning that
`on_pushButton_clicked()` has no matching `pushButton` signal in the loaded
UI. This is a defect candidate because it indicates a stale slot/UI mismatch.
Offscreen additionally warns that the `QSystemTrayIcon` has no icon and that
the platform does not support selected window-management operations; these are
headless-platform observations, not evidence of a normal desktop regression.
The direct plot-window dependency and dynamic dock removal code remain risk
areas for later full GUI characterization.

## MainWindow dynamic forms and bindings 3I.2 characterization

`GUI_006` through `GUI_008` extend the same real offscreen MainWindow target
with the portable `tests/fixtures/gui/mainwindow-bindings.ui` fixture and the
existing standard, malformed and unsupported-widget UI fixtures. `GUI_006`
calls the public `LoadFormFromXML(path, formName, skip)` directly, avoiding the
modal file chooser. It characterizes repeated loading of one fixture under
distinct XML form names and loading a second fixture: each creates a left-area,
closable, delete-on-close dock named by `formName`, records the pair in
`GetFormFileEntry`, retains a `QScrollArea`/tab/form ownership chain, and
suffixes descendant DropWidget object names with `_<formName>`. Closing a dock
removes that form-file entry; an equivalent later load creates a new dock.

The same test proves that missing and malformed UI inputs do not create a dock
or form-file entry and each report the existing `Corrupt Form File` Messenger
error. An unknown widget class is instead reported by the Qt form builder and
the containing form still loads as a dock without that child widget. This
partial-load behavior remains a defect candidate. `LoadFormFromXML` also calls
`QDir::setCurrent` to the form's directory and does not restore the previous
process working directory; `GUI_006` records that global side effect as a
defect candidate.

`GUI_007` binds two actual `QLineEditD` instances from two loaded forms to the
same real `UIDataManagementSetClass` container (`GUI7::sharedText`). Two
ordered `get` `DataManagementClass::MessageSender` events arise during
`ConnectToID`; a Messenger `set` updates both widgets, and an invoked
`editingFinished` creates one ordered `set` event with the expected ID and
string payload. The direct non-modal `CloseProject` path, with
`ChangeForSaveDetected` set false, deletes both docks, clears form records and
containers, and restores the central widget. Thus its safe stored-form path
is mapped without entering the interactive unsaved-project question.

`GUI_008` sends deterministic drag-enter/drop events to a loaded DropWidget.
They have no Qt-private `QDrag` source and are safely rejected without creating
a mapping. A valid production drop requires `event->source()` to be a selected
`QTreeWidget`; Qt does not expose a setter for that source on a directly
constructed event, and a real `QDrag::exec()` is intentionally excluded from
this deterministic suite. Consequently, valid tree-source drop acceptance,
the source-empty `selectedItems()[0]` crash risk, and multi-selection drop
semantics remain unverified rather than being encoded as contracts.

No external plugin, network action, real user file or user setting is used.
Modal load/save/import/export/parameter/About and unsaved-form prompts remain
excluded. The existing stale auto-connect warning and headless tray/platform
warnings from 3I.1 remain observed but unchanged.

## MainWindow modal action cancellation 3I.3 characterization

`GUI_009` through `GUI_011` run the real QAction-to-slot connections with
`Qt::AA_DontUseNativeDialogs` set before the test `QApplication`. A zero-delay
event-loop `QTimer` observes `QApplication::activeModalWidget()` and rejects
or clicks the requested `QMessageBox` button; no native dialog, fixed sleep or
user interaction is used. The application working directory, settings and
dialog-output root are temporary for the suite. Per-test cleanup restores the
temporary working root and `main` restores the caller's original directory
after `qExec`.

`GUI_009` establishes one modal dialog per trigger for `actionLoad_Form`,
`actionLoadExperiment`, `actionSave_Experiment`, `actionSave`,
`actionLoadPlugin`, `actionLoad_Parameter_File`, MAT/HDF5 export and parameter
export (with one real selected test parameter), `actionAbout`, and
`actionRemote_Connection_Port_2`. All were deterministically aborted without
creating a file in the dedicated temporary output directory or changing the
process current directory. The same test confirms that `actionAbout` opens a
real dialog and that the remote-port action opens a one-button message box.
`actionAbout_LabAnalyzer` emits its QAction signal but opens no dialog because
its current slot is empty.

`GUI_010` records cancellation side effects rather than normalizing them:
cancelled Save-as and Save both emit `MainWindow::SaveExperiment` once with an
empty path, causing the observed downstream "No file name specified" warning;
Save-as also clears `ChangeForSaveDetected`. Cancelled Save-as and parameter
load replace an existing `StdSavePath` with an empty string. No output file or
working-directory mutation resulted in the test. These are defect candidates:
an abort does not preserve the selected path/dirty state and a save request is
still sent to the manager with an empty filename.

`GUI_011` covers the real unsaved `Close_Project` prompt with a loaded form.
Cancel keeps the dock and form-file record but still clears
`ChangeForSaveDetected`; a repeated prompt can be reached only after the test
marks it dirty again. Discard closes and deletes the dock, removes form records
and clears manager containers. The lost dirty state on Cancel is a defect
candidate. The About dialog is allocated without a parent and survives its
rejection until the test explicitly deletes it; this is a further ownership
defect candidate. `About.ui::setupUi` overwrites the preceding
`"About LabAnaylser"` title with its current `"Dialog"` title.

Successful XML/plugin/export/TCP paths are intentionally not duplicated here.
Uncovered modal work remains selecting a real file, accepting Save and
overwriting a target, interacting with custom subplot controls, native-dialog
platform behavior (explicitly disabled for determinism), and the unsafe
selection-dependent context-menu paths.

## MainWindow safe action, tree and recovery characterization 3I.4b

`GUI_012` through `GUI_015` use the same real, offscreen MainWindow graph and
temporary settings/working roots. The expanded suite passed all 17 Qt Test
entries (15 GUI tests plus initialization and cleanup) with exit code 0.
`GUI_012` maps `on_actionCreate_Subplot_triggered` to its real modal dialog:
Cancel creates no figure, while valid 2x3 and 1x2 selections create and then
cleanly close distinct `SubPlotMainWindow` instances across a repeat cycle.
This is window-lifecycle coverage only; PlotWidget data, rendering and FFT
semantics remain 3J work.

`GUI_013` covers repeated `on_actionMinimize_to_Tray_triggered` cycles, the
real Restore QAction and `TrayIconActivated(DoubleClick)`, together with
`OutputTextMenu`, its Clear Output action, `ErrorWriter` long-text trimming
and `InfoWriter`'s current 100-block document limit. The expected offscreen
warnings about raising windows and keyboard grabbing do not establish desktop
tray behavior.

`GUI_014` publishes actual nested Parameter, Data and State values through
the Messenger/manager and maps `AddElementToWidget` and
`HighLightConnection`: category/group/leaf structure, IDs, typed values,
manager-to-tree update, repeat-ID deduplication and valid-leaf selection are
covered. Empty or missing selections/IDs remain excluded because they reach
known unsafe index/null candidates.

`GUI_015` maps `LoadFormFromXML` form/dock recovery through the portable
`mainwindow-bindings.ui` fixture: load, record, close/removal and independent
recreation with a new temporary form name. No distinct compiled `LoadForms`
routine exists to test. Direct tree-source drag acceptance, context actions,
notification/scroll timer behavior, command-line paths and real system-tray
availability remain open or belong to later 3I/3J work; no production behavior
was changed.

## MainWindow context actions, plot routing and timers 3I.4c

`GUI_016` through `GUI_018` keep the real MainWindow graph under the same
offscreen and temporary-environment isolation. The rebuilt suite passed all 20
Qt Test entries (`GUI_001`–`GUI_018`, initialization and cleanup) with exit
code 0. `GUI_016` maps valid-leaf context behavior: Parameter exposes the
enabled/visible `Change Min/Max Values` action; Data exposes `Set Alias`, whose
controlled modal action commits `First Alias` to `Data::Device::First`, then
also exposes `Remove Alias`; a valid two-leaf Data selection still exposes Set
Alias. State exposes no leaf action because its production custom-context
connection is commented out and its direct slot handles only a top-level item.
Empty, invalid and top-level selections were not executed.

`GUI_017` maps `actionCreatePlot` and `actionFFT` to three repeatable 1x1
figure/plot registrations. `Figure#0`–`Figure#2` and `Plot#1`–`Plot#3` retain
the observed pairing and parent/target relation, then close cleanly. This does
not test PlotWidget samples, curves, FFT results or rendering, which remain
3J contracts.

`GUI_018` maps ordered notification signal payloads, current append-not-
replace OutputText behavior, the one-second status-message expiry, conditional
output autoscroll, and destruction while a status-message timer is pending.
No use-after-free or crash was observed. The current multiple-Data-leaf alias
branch concatenates IDs without a separator before routing; its actual commit
is therefore a defect candidate and is not treated as a safe contract.
Remaining risks include empty-selection/index paths, Parameter Min/Max commit,
Remove Device, alias removal, notification-dock raise/loading delays, real
system-tray availability and all PlotWidget semantics. No production behavior
was changed.

## MainWindow mutable context actions 3I.4d

`GUI_019` through `GUI_021` characterize only valid, temporary selections in
the real offscreen MainWindow graph. The suite passed all 23 Qt Test entries
(`GUI_001`–`GUI_021`, initialization and cleanup) with exit code 0.
`GUI_019` maps `ChangeMinMaxValue`: its first dialog displays a stored `1.25`
minimum as `1.3` at the current dialog precision, but cancelling both dialogs
preserves the stored `(1.25, 5.5)` pair. A valid `(-3, 9)` commit updates the
manager and emits one `MessageSender("get", ID, InterfaceData())`; `(10, -2)`
is also accepted and stored without range validation. The rounding and inverted
range acceptance are defect candidates.

`GUI_020` maps `SetAlias` and `RemoveAlias`: cancel preserves the ID, Unicode
`Mäßwert Ω` is accepted, an accepted empty alias is ignored, and removal maps
the alias back to the original ID without changing the displayed tree leaf.
For two valid Data leaves, the real dialog receives the delimiter-free
concatenation `GUI20Device::Data::FirstGUI20Device::Data::Second`; its alias is
stored only under that synthetic key, not either individual ID. This confirms
the multiple-selection alias defect candidate.

`GUI_021` uses a temporary, public-interface-only test device to map
`RemoveDevice`. Opening its valid root context menu has no effect; triggering
the single `Remove Device` action immediately destroys the device and removes
its roots/selections from all three trees. No confirmation or Cancel path is
implemented, and no `MessageSender` event occurs. Its data container remains
in the manager after removal, which is another defect candidate. Empty,
invalid and unknown-selection paths remain excluded; no production behavior
was changed.

## MainWindow phase 3I completion

All real MainWindow contracts `GUI_001` through `GUI_021` are now mapped in
`MAINWINDOW_CONTRACTS_3I.md` and run through the normal qmake runner. The
suite uses a real MainWindow without a production seam, a temporary settings
and working environment, and a portable fixture root. It sets offscreen only
for that suite. Current coverage and full function checklist are recorded in
the contract document. The known unsafe empty/invalid selection, null sender
and index paths remain deliberately excluded from in-process tests. Figure
lifecycle/action routing is covered, but PlotWidget data, FFT calculations,
curves, interaction and rendering are deferred to 3J. No production source was
changed in phase 3I.

## PlotWidget phase 3J.1 characterization

`contract/plotwidget/PlotWidgetContractTests` is an offscreen Qt Test target
which links the real qmake application graph, including the existing
DataManagement/Messenger route and the vendored qcustomplot runtime source.
It does not modify or unit-test qcustomplot. The smoke build and test run
completed with exit code 0; `PLOT_001` through `PLOT_006` cover construction,
data registration, graph state, the messenger `set` update route, axis/reset
state, XML persistence, navigation interaction state and cleanup.

| Production function | Test mapping / status |
| --- | --- |
| `PlotWidget::PlotWidget` | `PLOT_001` directly; `PLOT_002`–`PLOT_006` exercise real constructed instances. |
| `~PlotWidget` | `PLOT_006` directly after graph/connection cleanup. |
| `AddCustomGraph` | `PLOT_002`–`PLOT_004` directly, including multiple and repeated IDs. |
| `ClearAllGraphs` | Not invoked: its observed pointer-registration-only behavior is not a safe graph-removal contract. The real `removeAllGraphs` slot is covered by `PLOT_003`/`PLOT_006`. |
| `SetXDataName` / `XDataName` | Not yet directly exercised; XY mapping is deferred with plot interaction/data semantics to later 3J work. |
| `SetVariantData` / `GetVariantData` | `SetVariantData` is indirectly covered by the UpdateGraphs route; form-mapper serialization is deferred to the existing UI/XML contract boundary. |
| `LoadFromXML` / `SaveToXML` | `PLOT_005` directly for labels, ranges and state round-trip. |
| `ConnectToID` | `PLOT_006` directly against a real manager and messenger update. |
| `UpdateGraphs` | `PLOT_002`, `PLOT_004` and `PLOT_006` directly with forced deterministic updates. |
| `SetAsXAxis` | Not executed: XY mode has unchecked empty-vector/index paths and is deferred; no unsafe path is invoked in-process. |
| `keyPressEvent` / `keyReleaseEvent` | `PLOT_005` directly for control-navigation interaction removal/restoration. |
| `dragEnterEvent` / `dropEvent` | Deferred: source-dependent drag routing belongs to the real tree/form workflow; synthetic missing-source paths are unsafe/ambiguous. |
| protected `closeEvent` / `resizeEvent` | Not directly invoked; destruction is covered by `PLOT_006`, while visual geometry is intentionally excluded from the non-pixel contract. |

Observed defect candidates: each `AddCustomGraph` call appends a graph even for
an already-present data ID; callers must therefore avoid duplicate bindings.
`ClearAllGraphs` removes manager registrations but does not itself remove the
qcustomplot graphs, unlike the internal `removeAllGraphs` slot. The XY update
route dereferences first/last elements after pointer/length checks without an
empty-vector guard. These are documented observations, not repaired behavior.
FFT calculation, plot rendering/pixels, mouse gesture geometry, source-backed
drag/drop, selection/context menus, history limits and unsafe null/index paths
remain excluded from 3J.1.

## PlotWidget FFT phase 3J.2 characterization

`FFT_001` directly covers the minimal `FFTPlotWidget` adapter: it is parented
as supplied, sets `Qt::WA_AcceptTouchEvents`, contains no graphs initially and
is safely destructible. The FFT calculation is not implemented by that adapter
but by `PlotWidget::CalculateFFT`, reached only through the existing private
`ToggleTimeFreq` Qt slot via `QMetaObject::invokeMethod`; no production access
was widened.

`FFT_002` and `FFT_003` use exactly eight samples at 8 Hz (`t=n/8`,
`n=0..7`). The one-Hz sine has amplitude 2.0; the DC-plus-sine vector is
`1.5 + 2*sin(2*pi*n/8)`. The stored vendored-graph FFT vectors contain ten
entries (`2*(N/2+1)`), duplicate each frequency, report bin one at 1 Hz, DC
at 1.5 and the one-sided sine amplitude at 2.0. All analytic amplitude
assertions use an absolute tolerance of `1e-10`. The mode also changes labels
to `f [Hz]`/`Amplitude`, impulse style and cross scatter markers.

`FFT_004` verifies two independent graph vectors: a one-Hz amplitude-one
signal and a two-Hz amplitude-three signal retain their distinct expected
bins. `FFT_005` verifies the time-domain labels, original graph styles and a
saved `[-2, 4]` time range are restored after the second toggle. `FFT_006`
uses the real Messenger `set` route in FFT mode: amplitude one changes to two
without a new graph. `FFT_007` characterizes existing nonuniform sampling as
mean-delta-T behavior: `x={0,.1,.4,.6}`, mean delta `0.2`, `N=4`, therefore
the stored bin spacing is `1.25 Hz` (tolerance `1e-12`).

`FFT_008` covers only the existing safe early returns for empty and
length-mismatched vectors: no stored FFT vectors are produced. Single-sample
input (empty `dTs` division), FFTW allocation and plan creation failure, null
graph pointers, nonfinite numerical behavior, rendering/pixels, gesture
geometry and FFT quality/THD presentation remain deliberately unexecuted
danger paths. `CalculateFFT` has no checks for failed `fftw_malloc` or a null
FFTW plan before execution; these are defect candidates, not contracts.

## PlotWidget phase 3J completion

`PLOT_001` through `PLOT_006` and `FFT_001` through `FFT_008` are registered
as `contract/plotwidget/PlotWidgetContractTests` in the normal qmake runner.
The runner applies offscreen mode only while that executable runs and restores
the caller environment. Detailed contracts, numeric tolerances, per-file gcov
evidence, defect candidates and exclusions are in `PLOT_CONTRACTS_3J.md`.
PlotWidget remains unsuitable for broad refactoring: valid cursor,
context-menu, history-limit and quality-criteria paths still need direct
characterization; rendering, gesture and dangerous failure paths are excluded.

## Milestone 3 completion audit

The top-level table is the original milestone-1 source map. Its rows marked
`mapped, unverified` are superseded where the phase sections below assign test
IDs; they are retained as the historical inventory rather than rewritten to
hide still-open function-level detail. The concrete milestone-3 mappings are:

| Surface | Characterized evidence | Remaining function-level inventory gap or explicit exclusion |
| --- | --- | --- |
| `mainwindow.*` | `GUI_001..GUI_021` | `main.cpp` itself, CLI arguments, native dialogs, desktop tray and unsafe selection/index paths are not executed. These are explicit exclusions, not passing evidence. |
| DataManagement and UI manager | `DM_001..DM_010`, plus XML, parameter, export, plugin and GUI boundary IDs named above | `LoadForms` has no repository definition. Sender-less, absent-entry and raw-pointer paths remain unsafe exclusions; the remaining UI-manager surfaces are not yet a complete per-function map. |
| Plugin API and loader | `PLUGIN_001..PLUGIN_007`; `XML_002..XML_004` for the XML path boundary | Third-party ABI/configuration matrix, null Messenger/null plugin return and OS loader/permission faults remain explicit exclusions. |
| DropWidget adapters, indicators and loader | `DW_001..DW_017`; real-form boundaries `GUI_006..GUI_008` | Source-backed `QDrag::exec()` drops, complete interactive context paths, visual pixel appearance and dangerous null/index paths remain excluded. |
| Tree and subplot lifecycle | `DW_009`, `GUI_005`, `GUI_012`, `GUI_017` | Tree source-dependent drops, malformed selections and plot content semantics remain unexecuted. |
| Plot/FFT | `PLOT_001..PLOT_006`, `FFT_001..FFT_008`; separate PlotMeasurements unit suite | `ClearAllGraphs`, X-axis mapping, valid cursor/context/history/quality criteria, rendering/gesture paths and FFT resource-failure paths are either unexecuted or explicitly unsafe. |

Thus all inspected subsystem scopes have at least a test-ID mapping or an
explicit exclusion, but the inventory is not yet a complete one-row-per-public
or protected production-function register. That remains an open quality gate
before broad refactoring. Milestone 3 therefore records the characterization
of identified critical external contracts, not a claim that every production
function has been tested or fully inventoried.

## DataManagement registry extraction 4B.1

The approved first refactoring slice delegates precisely the existing
`DM_REG_001` through `DM_REG_005` contracts to the private RAII-owned
`DataRegistry`; the tests themselves were not changed for the extraction.
`DataManagementClass` remains the only public facade. The final focused suite
passed 17/17, and the central runner passed all 11 registered targets. Fresh
Release and Debug application builds passed. The private registry stores only
value state and non-owning plot observations; container, device, mapper,
widget-binding and Messenger state stays in the facade for later slices.

Focused gcov evidence is intentionally file-specific rather than project-wide:
`DataManagementClass.cpp` has 87.50% lines (161/184), 90.91% executed branches
(180/198), 57.58% branches taken at least once (114/198), and 78.33% calls
(141/180). The new `DataRegistry.cpp` has 91.58% lines (87/95), 97.06% executed
branches (66/68), 76.47% branches taken at least once (52/68), and 87.18% calls
(34/39). The historical pre-extraction focused facade figure was 88.89% lines
(200/225); the relocated two-file line total is likewise 88.89% (248/279).
Branch denominators changed with the extraction, so percentages are evidence,
not directly identical source metrics. Unreached registry paths are the safe
no-number plot/window overloads and the rename fallback without a number; they
retain their prior exclusions/coverage status.

## DataManagement container ownership baseline 4C.1

`DM_CONT_001..DM_CONT_005` characterize the unmodified public container facade
for the planned private owner extraction. The `GetContainerPointer()` address
is stable and maps known IDs to the same repeatedly returned `ToFormMapper*`.
Missing string lookup returns null without insertion, but unlinked QObject
lookup inserts the empty ID with a null mapper. Replacing an existing container
creates a new mapper pointer, retains its non-owning form-object bindings and
Min/Max values, and applies the new type/state metadata; the former mapper is
released and deliberately never dereferenced.

Container enumeration follows the current map-key order. `CloseProjectLogic`
empties the container map and releases all mappers, while a bound QObject owned
by another parent remains valid. Manager destruction likewise does not delete
such foreign QObjects, and manager instances do not share container state.
These released mapper pointers are explicitly invalid after replacement,
project cleanup or manager destruction; no desired raw-pointer lifetime beyond
those boundaries is asserted. `ElementsToContainerID` remains unmodified for
the later widget-binding slice.

## DataManagement container ownership extraction 4C.2

The unchanged facade contracts `DM_CONT_001..DM_CONT_005` now exercise the
private RAII-owned `ContainerStore`. `GetContainerPointer()` still returns the
actual stable address of the same raw `std::map<QString, ToFormMapper*>` for a
manager lifetime. Missing string IDs still do not insert; missing QObject
lookups still insert the empty key with a null mapper; lexical key traversal,
replacement metadata/form-binding preservation, mapper invalidation after
replacement/cleanup, foreign-QObject survival, and instance isolation are
unchanged.

The Store deletes only its currently mapped `ToFormMapper*` values once on
replacement, cleanup, and destruction. It never owns mapper-bound QObjects.
Because `GetContainerPointer()` exposes a mutable raw map, external mutation
can bypass Store ownership accounting; this is a retained public API/ownership
limit, not a new guarantee. `ElementsToContainerID` remains in the facade and
is explicitly deferred to the widget-binding slice.

Focused gcov is file-specific: `DataManagementClass.cpp` 91.53% lines
(162/177), 92.71% branches executed (178/192), 56.77% branches taken at least
once (109/192), 85.80% calls (151/176); `ContainerStore.cpp` 94.12% lines
(32/34), 100.00% branches executed (16/16), 93.75% branches taken (15/16),
100.00% calls (13/13). These are not project coverage and are not directly
denominator-comparable to the prior facade-only 87.50% line figure.

## DataManagement widget-binding characterization 4D.1

`DM_BIND_001..DM_BIND_005` characterize `ElementsToContainerID` and the public
facade paths without exposing its private map. `AddElementToContainerEntry`,
`GetContainerID(QString/QObject*)`, `GetContainer(QObject*)`,
`IsObjectLinked`, both `DeleteEntryOfObject` overloads and
`CloseProjectLogic` are covered directly; `DataManagementSetClass::SetData` and
`SendNewValue` are exercised through a real bound `VariantDropWidget` probe.

Bindings are keyed by `QObject::objectName()`, not by QObject pointer identity:
a different live QObject with the same name resolves to the original binding,
while the mapper retains the exact originally bound `QObject*`. An unknown
QObject lookup through `GetContainer(QObject*)` retains the existing paired
side effect: it inserts the empty string mapping and the empty/null container
entry, after which `IsObjectLinked` reports true. The direct QObject ID lookup
alone does not insert.

Repeated ordinary widget registration detaches then re-adds the binding, so the
mapper has one entry. Rebinding to a different ID removes the old mapper entry
and replaces the name-to-ID mapping. The ID-specific deletion removes only the
mapper entry; the QObject overload additionally removes the name mapping.
There is no public single-container removal operation to characterize safely;
`CloseProjectLogic` clears both containers and all name bindings.

No `destroyed(QObject*)` cleanup is connected: after a parented bound QObject
is destroyed, its name binding and stale mapper pointer remain until project
cleanup. Tests use `QPointer` and a fresh same-name QObject only; they never
dereference the stale pointer. This is a lifetime defect candidate. Bound form
QObjects remain foreign/non-owning: manager cleanup and manager destruction do
not delete them. Binding state is instance-local.

For an ordinary repeated bound widget, manager-to-widget propagation has one
mapper entry. A successful `SendNewValue` emits one `set` message and the
existing Messenger feedback updates that widget once. The special `ClassName ==
"PlotWidget"` registration path does not detach prior entries: repeated
registration stores two mapper entries and a manager update invokes
`SetVariantData` twice, while a single widget signal still produces one `set`
message. This signal/data-flow asymmetry is a second defect candidate. Senderless
`SendNewValue`, null objects, direct stale-pointer use and raw-map mutation are
dangerous exclusions, not contracts.

`DM_BIND_006` adds the name-identity boundary: two distinct live QObjects with
the same `objectName` share one binding key. Binding the second replaces the
name mapping and removes the first mapper entry by its stored name, so both
objects subsequently resolve to the second ID while the mapper stores only the
second pointer. Renaming that object does not migrate the old map key or its
stored `ObjectStruct`; its current name is unlinked until
`GetContainer(QObject*)` inserts an empty-ID/null-container entry. Deleting via
that renamed object removes only this newly inserted empty name mapping; after
renaming back, deletion removes the original binding. Empty object names are
ordinary keys and bind/lookup/remove normally. Sender routing also uses the
current name: a renamed bound widget emits no message; restoring its original
name restores the existing `set` route. These name-collision and rename states
remain instance-local. This strengthens the name-keyed/stale-binding defect
candidate; no pointer-keyed or automatic-cleanup behavior is implied.

## DataManagement widget-binding extraction 4D.2

The private `WidgetBindingRegistry` now implements only the prior
name-to-container-ID value map under the unchanged public facade. All
`DM_BIND_001..DM_BIND_006` contracts pass unchanged: names (including empty
names) remain the identity, collisions/rebinds and non-migrating renames retain
their behavior, unknown string lookups insert an empty ID, while direct QObject
lookups do not. `DataManagementClass` still changes mapper binding lists and
coordinates cleanup with `ContainerStore`; repeated `PlotWidget` registration
therefore still produces duplicate manager-to-widget propagation.

The Registry neither stores nor owns QObjects and intentionally makes no
`destroyed(QObject*)` connection. Existing stale mapper pointers after QObject
death, name-based lookup of another same-name QObject, foreign QObject
non-ownership and raw mapper-map exposure remain risks/exclusions, not safety
guarantees. Focused gcov is file-specific: `DataManagementClass.cpp` 91.38%
lines (159/174), 93.55% branches executed (174/186), 56.99% branches taken
(106/186), 86.59% calls (155/179); `WidgetBindingRegistry.cpp` 100.00% lines
(18/18), 100.00% branches executed (10/10), 70.00% branches taken (7/10), and
100.00% calls (5/5). These are not project coverage.

## DataManagement Messenger dispatch characterization 4E.1

`DM_MSG_001..DM_MSG_003` characterize the public `MessengerClass` boundary
before any dispatch-policy extraction. `DM_MSG_001` drives each known
`MessageReceiver` command twice through a live
`MessengerClass -> DataManagementSetClass -> LabAnalyser` QObject hierarchy:
`publish`, `set`, `get`, `error`, `info`, `notification`, `CloseProject`,
`publish_start`, and `publish_finished`. It records exact signal sequences,
counts, IDs and payloads. `publish` emits `AddContainerElement`, `SetData`,
`AddElementToWidget`, then the recursive `set` sequence (`SetData`,
`NewDataReceived`). `CloseProject` emits parent-parent ID `LabAnalyser` with
`Closing forced by: <ID>`, then `CloseProject`.

`DM_MSG_002` records that `MessageTransmitter` executes the identical receiver
path and emits exactly one `MessageSender` afterward, even for `get` and other
receiver-no-op paths; repeats preserve this sequence and `QString` Unicode
payload. `DM_MSG_003` covers safe empty/unknown commands and IDs, then a mixed
publish/set/get/CloseProject sequence. Empty and unknown receiver commands emit
nothing; an empty-ID publish creates an empty-ID container that later set
updates. These are observed legacy semantics, not policy rules.

Null parent, null sender and destroyed-QObject paths are deliberately excluded:
`CloseProject` dereferences `parent()->parent()`. TCP, plugin and XML suites
retain their transport, plugin and persistence contracts; this slice maps only
their shared Messenger command boundary.

## DataManagement MessageDispatchPolicy extraction 4E.2

`MessageDispatchPolicy::ReceiverIntents` is internally covered through the
unchanged `DM_MSG_001..DM_MSG_003` Messenger vectors. It maps all emitting
commands to typed ordered intents and returns none for empty, unknown and
legacy `remove` commands. `MessengerClass::MessageReceiver` remains the only
signal executor; `MessageTransmitter` remains a receiver invocation followed by
one `MessageSender`. No parent/null safety behavior was changed.

Focused gcov evidence is file-specific, not project coverage:
`DataMessengerClass.cpp` recorded 65/72 lines, 67/77 executed branches, 39/77
taken branches, 63/82 calls and 7/8 functions; the new policy recorded 19/20
lines, 18/18 executed branches, 17/18 taken branches, 9/10 calls and 1/1
function. Statusbar-only and registration/status-write paths remain outside the
safe messenger command vectors.

## DataManagement device ownership characterization 4F.1

`DM_DEV_001..DM_DEV_004` cover the public device-facade methods
`AddDevice`, `GetDevice`, `GetDevices`, `GetDevicePaths`, `CloseDevice`,
`RemoveDevices` and the device part of `CloseProjectLogic`, using instrumented
test-only `Platform_Interface` implementations. A first registration retains
the raw interface pointer and path; repeated lookup returns that same pointer.
Enumeration is lexical by device name. A duplicate name, whether supplied with
the same or a different interface pointer, retains the first pointer and first
path. The different rejected pointer is not destroyed by the manager, so its
creator must delete it.

Known devices are destroyed once by `CloseDevice`; unknown and repeated closes
have no visible effect. `RemoveDevices` destroys each owned interface in lexical
order but intentionally leaves `_Devicepaths` observable through
`GetDevices()`/`GetDevicePaths()`. `CloseProjectLogic` clears both maps and
destroys devices in lexical order. Re-registering after removal restores a live
device and replaces its surviving path. The manager does not own a QObject
returned by `Platform_Interface::GetObject()`: QPointer evidence shows that it
survives device/project cleanup until its external owner destroys it. Device
state and cleanup are instance-local.

Null-interface registration, released raw-pointer dereference and explicit
plugin unload remain unsafe exclusions. `PLUGIN_001..PLUGIN_007` remain the
authoritative contracts for plugin-loader rejection/recovery; this device slice
does not duplicate them. The stale-path result after `RemoveDevices`, exposed
raw device pointers and unclear plugin-loader lifetime remain refactoring risks
for a later, compatibility-preserving DeviceRegistry slice. The focused suite
passed 35 checks, the plugin suite passed 9, and the 11-target central runner
completed with exit code 0.

## DataManagement DeviceRegistry extraction 4F.2

`DeviceRegistry` is covered indirectly by the unchanged public-facade
`DM_DEV_001..DM_DEV_004` vectors. It retains first pointer/path wins, lexical
map order, the rejected-duplicate caller cleanup boundary, stable raw lookup,
known/unknown/repeated close, stale paths after `RemoveDevices`, re-registration
and lexical `CloseProjectLogic` deletion order. The registry owns no QObject or
QPluginLoader and does not extend a pointer returned by `GetDevice`.

Its registry-object RAII does not alter the old raw plugin-interface destruction
boundary: only the existing explicit cleanup operations delete an accepted
interface. Focused file metrics are `DataManagementClass.cpp` 94.44% lines,
96.47% executed branches, 55.29% taken branches and 86.56% calls; and
`DeviceRegistry.cpp` 93.55% lines, 83.33% executed branches, 75.00% taken
branches and 100.00% calls. They are not project coverage.
## Phase 4G.2a ProjectIoCoordinator extraction

`UIDataManagementSetClass::{ImportFromXml,Export2Xml,Export2Mat,Export2Hdf5}`
delegate only adapter construction/execution to the private non-QObject
`ProjectIoCoordinator`; their public method signatures, signal/slot surface,
Messenger emissions and return values remain in the facade. `UIIO_004` and
`PARAM_001..PARAM_009` map the parameter routes; `UIIO_005`, `MAT_001..008`
and `HDF5_001..006` map MAT/HDF5. The unchanged HDF5 error contract remains:
the facade emits `Error` and returns `false` after the coordinator's exporter
throws. No experiment XML, form, plugin, path, CWD, locale or dirty-state
operation delegated in this slice.

## Phase 4G.2b Experiment-read delegation

`UIDataManagementSetClass::LoadExperiment(QString)` remains the public
facade and is characterized by `UIIO_001..UIIO_006`, `XML_001..XML_008` and
`XML_LEGACY_001..XML_LEGACY_005`. It now calls the private
`ProjectIoCoordinator::ReadExperiment(QString)` solely to instantiate and run
the unchanged `XmlExperimentReader` with the historic facade/messenger/parent
arguments. The facade still owns `LoadPath`, its exact parse-error text and
the `CloseProject` message; the reader still owns all XML traversal and CWD
effects. Missing UI/plugin dependencies, relative `.LAdev` paths and legacy
state remain covered by the legacy vectors. Writer, plugins, form
implementation and private save/change state were not moved.

## Phase 4G.2c Experiment-write delegation

`UIDataManagementSetClass::SaveExperiment(QString)` remains a public Qt slot
covered by `XML_006..XML_008`, `XML_LEGACY_004` and `UIIO_001..UIIO_006`. It
now calls private `ProjectIoCoordinator::WriteExperiment(QString)` only to
instantiate/run the unchanged `xmlexperimentwriter` with the same UI facade,
Messenger reference and QObject parent. The facade retains its inverted return
value, exact status/error handling and post-write plugin `save` messages;
writer traversal preserves sections, attributes, UTF-8, paths and formatting.
Legacy files remain inputs only and are never overwritten by these vectors.

## ProjectIoCoordinator checkpoint

`ProjectIoCoordinator` now covers every implemented adapter operation exposed
by `UIDataManagementSetClass`: `ImportFromXml`, `Export2Xml`, `Export2Mat`,
`Export2Hdf5`, `LoadExperiment`, `SaveExperiment` and `LoadPlugin`. The public
facade remains the behavior boundary for all exposed bool conventions,
Messenger/status/error signal order, `LoadPath`/`StdSavePath`/`ChangeDetected`
state and MainWindow/UI routing. Contract mapping remains unchanged:
`UIIO_001..UIIO_006`, `XML_001..XML_008`, `XML_LEGACY_001..XML_LEGACY_005`,
`PARAM_001..PARAM_009`, `MAT_001..MAT_008`, `HDF5_001..HDF5_006` and
`PLUGIN_001..PLUGIN_007`.

`LoadForms()` has no implementation and is a documented blocker, not an
untested coordinator path. XML writer/reader, loader and plugin-interface
implementations remain their existing compatibility boundaries; legacy fixture
integrity and temporary-only round-trip targets remain required evidence.
