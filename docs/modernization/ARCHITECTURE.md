# Architecture inventory (current baseline)

## Build and top-level flow

`main.cpp` constructs `MainWindow` from `mainwindow.ui`.  `MainWindow` owns or
coordinates the data manager, messenger, dynamic UI loader, project XML reader/
writer, exports, plots, and local remote-control server.  `LabAnalyser.pro` is a
single qmake application target; `tests/PlotMeasurementsTests.pro` is a separate
qmake console target.  There is no CMake build or CI workflow.

```text
Qt UI / .ui forms --> MainWindow --> DataManagementSetClass --> Messenger <-- plugins
                         |                  |                    |
                         |                  +--> widget/id mappings +--> Qt signals
                         +--> XML read/write, exports, plot windows, TCP server
```

## Subsystems and boundaries

| Area | Files/classes | Current responsibility and contracts |
| --- | --- | --- |
| Presentation | `MainWindow`, `SubPlotMainWindow`, `TreeWidgetCustomDrop`, `About.ui`, `mainwindow.ui` | Actions, dock/widget state, dynamic forms, drag/drop, object names, plotting windows. |
| Data/messaging | `DataManagementClass`, `DataManagementSetClass`, `MessengerClass`, `mapper.h`, `InterfaceData` | ID-to-data/widget mapping; aliases/min/max; plugin data dispatch; Qt signal/slot event flow. |
| Plugins | `platforminterface.h`, `loadplugin.*` | Qt `QPluginLoader` loads XML-described plugins. ABI includes `Platform_Fabric`, `Platform_Interface`, `InterfaceData`, and IID `org.qt-project.Qt.Examples.EchoInterface`. |
| Persistence/import | `xmlexperimentreader.*`, `xmlexperimentwriter.*`, `parameterloader.*`, `exportinputs2xml.*` | Experiment XML (`Experiment` with Tabs, Devices, Widgets, State, FigureWindows, Connections), parameter XML, absolute/relative paths, UI state. |
| Export | `Export2Mat.*`, `export2highfive.*` | MATLAB MAT via libmatio and HDF5 via HighFive/HDF5. Output schema has not yet been fixture-characterized. |
| Plot/numerics | `PlotWidget`, `FFTPlotWidget`, `PlotMeasurements`, vendored `qcustomplot` | Charts, FFT, interpolation, interval statistics, RMS and THD. `PlotMeasurements` is the only tested production module. |
| Custom/drop widgets | `DropWidgets/*`, `CustomWidgets/*` | Widget-specific binding, data updates, XML save/load, and visual indicators. |
| Network | `RemoteControlServer` | Loopback TCP server (initial port 4080, incremented until bound); binary `set`/`get` protocol. |

## External formats and dependencies

- Qt Designer `.ui` forms and `resources.qrc` are runtime/UI contracts.
- Experiment and parameter XML are parsed/written with Qt XML streams.
- Plugin descriptions are XML and plugins are runtime-loaded Qt binaries.
- MAT export uses libmatio 1.5.28 in the observed environment; HDF5 uses HDF5
  1.14.6 and HighFive 2.10.1.
- FFT behavior is built with FFTW 3.3.10 (`LABANALYSER_USE_FFTW`).

## Target direction (not implemented)

Incrementally separate GUI presentation from core numerical/data logic, IO
adapters, network transport/protocol, and plugin-loading adapter while retaining
the existing qmake build until CMake parity is demonstrated.  No such refactoring
is part of this audit.

## Phase 4A DataManagement refactoring direction (planned only)

`DataManagementClass` currently combines value registries, raw mapper/device
ownership, widget bindings and plot/form bookkeeping. `DataManagementSetClass`
adds QWidget/message dispatch, while `UIDataManagementSetClass` additionally
orchestrates MainWindow, XML, import/export and plugin loading. Phase 4A records
an incremental extraction plan in `DATAMANAGEMENT_REFACTOR_PLAN_4A.md`: first
extract private value-only registry helpers behind unchanged manager facades,
then separately treat container/binding ownership, messenger dispatch, devices
and GUI/IO orchestration. `InterfaceData`, plugin headers, Qt signals/slots and
the existing qmake target remain compatibility boundaries.

## Phase 4B.2 DataRegistry extraction

The first compatible DataManagement slice is implemented. `DataRegistry` is an
internal normal C++ type owned exclusively by `DataManagementClass` through a
private `std::unique_ptr` with a private deleter. It has no GUI, MainWindow,
Messenger, plugin, network or IO dependency; its `QObject*` plot entries are
strictly non-owning observations. `DataManagementClass` remains the complete
public QObject facade and delegates only form-file order/removal, skip-form
flags, aliases, plot pointers, plot-window geometry and number histories to the
registry. Container, device, widget-binding, mapper and ownership state remains
in `DataManagementClass` unchanged.

The facade preserves the characterized legacy contracts: form duplicates and
insertion order, first-match removal, alias fallback, duplicate plot/window
number history, and insertion-capable unknown window-geometry lookup. No
public signal, slot, method signature, plugin header or `InterfaceData` ABI was
changed. This private boundary can be rolled back by restoring the former
fields/delegation without a data-format migration.

## Phase 4C.2 ContainerStore extraction

`ContainerStore` is a second, private normal-C++ helper owned exclusively by
`DataManagementClass` through RAII. It owns exactly the existing
`std::map<QString, ToFormMapper*>` entries and destroys each mapper once on
replacement, project cleanup, or manager destruction. It never owns or deletes
the `QObject*` form bindings stored in a mapper's `Objects` list.

`DataManagementClass` remains the public facade and returns the address of the
Store's actual map through the unchanged `GetContainerPointer()` API; that
address is stable for the manager lifetime. The raw mutable map remains an
intentional legacy API/ownership boundary: callers can mutate mapper pointers
outside Store mediation, so the Store cannot make stronger ownership guarantees
than the former facade. No parallel `unique_ptr` map or compatibility copy was
introduced. `ElementsToContainerID` and all widget-ID mapping remain in
`DataManagementClass` for the later widget-binding slice.

## Phase 4D.2 WidgetBindingRegistry extraction

`WidgetBindingRegistry` is a third private normal-C++ helper, owned through
RAII by `DataManagementClass`. It holds only the legacy
`QObject::objectName()` to container-ID value map. It neither owns, observes,
nor destroys QObjects, and it deliberately has no `destroyed(QObject*)`
connection. Empty names are valid keys, while renaming a QObject does not move
the old key.

`DataManagementClass` remains responsible for coordinating the Registry with
`ContainerStore`: it removes/creates `ToFormMapper::Objects` entries and keeps
the existing `PlotWidget` duplicate-registration exception. The registry has no
Mapper, Messenger, device, GUI, plugin, network or IO dependency. Name-keyed
identity, stale mapper-pointer risk and foreign QObject ownership are unchanged
compatibility boundaries rather than strengthened ownership guarantees.
