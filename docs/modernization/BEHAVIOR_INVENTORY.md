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
| Parameter XML | `Import/parameterloader.*`, `Export/exportinputs2xml.*` | Parameter import/export structure, malformed-file handling and data updates. | — | mapped, unverified |
| MAT/HDF5 export | `Export/Export2Mat.*`, `export2highfive.*` | Names, types, dimensions, numerical data, empty/error behavior for libmatio and HDF5. | — | mapped, unverified |
| TCP remote control | `RemoteControl/RemoteControlServer.*` | Loopback port selection; binary length/header framing; `set`/`get` request and response bytes; disconnect/error behavior. | — | mapped, unverified |
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
