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
| TCP remote control | `RemoteControl/RemoteControlServer.*` | Loopback port fallback; native binary framing; `set`/`get` bytes, signals, fragmentation, disconnect and current-socket behavior. | `contract/RemoteControlContractTests::{TCP_001..TCP_007}` | baseline characterized; unsafe short frames and platform faults unverified |
| Plot widgets | `DropWidgets/Plots/PlotWidget.*`, `FFTPlotWidget.*` | Plot configuration, data mapping, legend/cursor/FFT presentation and event behavior. | — | mapped, unverified |
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
`UIDataManagementSetClass` remains unverified because its public operations
require the real MainWindow plus persistence/export/plugin boundaries. Full
function-to-ID mapping, risk findings and per-file coverage appear in
`DATAMANAGEMENT_3A.md`.

## LoadSave/XML 3B characterization

`contract/XmlExperimentContractTests` maps XML IDs `XML_001` through `XML_008`
to all public XML reader/writer entry points and their UI manager callers. The
test project uses the real application object hierarchy; it has no test-only
seams, so it cannot affect production builds. Detailed mappings, fixture rules,
defect candidates and exclusions are in `XML_CONTRACTS_3B.md`.

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
