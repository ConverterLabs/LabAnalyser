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
| Experiment XML | `LoadSave/xmlexperimentreader.*`, `xmlexperimentwriter.*` | Read/write `Experiment` elements (Tabs, Devices, Widgets, State, FigureWindows, Connections), relative paths and Qt dock state. Reader/writer return conventions must be recorded before change. | — | mapped, unverified |
| Parameter XML | `Import/parameterloader.*`, `Export/exportinputs2xml.*` | Parameter import/export structure, malformed-file handling and data updates. | — | mapped, unverified |
| MAT/HDF5 export | `Export/Export2Mat.*`, `export2highfive.*` | Names, types, dimensions, numerical data, empty/error behavior for libmatio and HDF5. | — | mapped, unverified |
| TCP remote control | `RemoteControl/RemoteControlServer.*` | Loopback port selection; binary length/header framing; `set`/`get` request and response bytes; disconnect/error behavior. | — | mapped, unverified |
| Plot widgets | `DropWidgets/Plots/PlotWidget.*`, `FFTPlotWidget.*` | Plot configuration, data mapping, legend/cursor/FFT presentation and event behavior. | — | mapped, unverified |
| Plot measurements | `DropWidgets/Plots/PlotMeasurements.*` | Normalize sample order; interpolation; interval count/min/max/mean/RMS; THD and NaN invalid cases. | `PlotMeasurementsTests::{constantSignal,linearSignal,sineAndHarmonics,nonUniformSamples,invalidIntervals}` | baseline tested |
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
