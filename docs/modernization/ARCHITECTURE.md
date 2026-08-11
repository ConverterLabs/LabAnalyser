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
| Network | `RemoteControlServer`, private `RemoteControlConnectionState`, `RemoteControlFrameSplitter`, `RemoteControlProtocol` | `RemoteControlServer` remains the Qt transport and dispatch facade for the loopback binary `set`/`get` protocol (initial port 4080, incremented until bound). `RemoteControlConnectionState` observes the current non-owning socket and its sole frame state; `RemoteControlFrameSplitter` bounds native frames to 16 bytes--1 MiB and separates complete frames from remainder bytes; `RemoteControlProtocol` validates/decodes complete frames before dispatch. Invalid prefixes abort the current connection; bounded structural-invalid frames are discarded. The implementation still has only the last-accepted-client contract, not independent multi-client sessions or new socket ownership. |

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

## Milestone 4 isolation closure

The identified internal boundaries for DataManagement, project I/O and remote
control are complete without replacing their public Qt facades or asserting a
full GUI-independent core. The scope, compatibility evidence and explicit
Milestone-5 handover are summarized in `MILESTONE_4_ISOLATION_REPORT.md`.

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

## Phase 4E.2 Message dispatch policy extraction

`DataManagement/MessageDispatchPolicy.h/.cpp` is a value-only internal helper.
It classifies the exact case-sensitive Messenger command strings into a small,
ordered `MessageDispatchIntent` list. It has no QObject, Manager, GUI, plugin,
network, status-bar or payload ownership. `MessengerClass` remains the only
QObject and retains all signal/slot, parent-chain, status-bar and InterfaceData
execution. The policy is deliberately not exposed through the Messenger public
header.

The `publish` list preserves `AddContainerElement`, `SetData`,
`AddElementToWidget`, `SetData`, `NewDataReceived`; `CloseProject` has a
distinct notification intent so its parent-parent-derived identifier remains a
Messenger concern. Unknown, empty and legacy no-op `remove` commands classify
to no intents. `MessageTransmitter` still invokes receiver processing before
exactly one `MessageSender` emission.

## Phase 4F.2 DeviceRegistry extraction

`DeviceRegistry` is a private normal-C++ helper owned as a registry object by
`DataManagementClass` through its existing private-deleter/`unique_ptr` pattern.
It holds private device records (raw `Platform_Interface*`, descriptor path and
an internal cleanup strategy) plus the retained legacy path map. The public
facade delegates device registration, lookup, listing and cleanup unchanged.

The helper deliberately preserves the old cleanup policy rather than
strengthening it: accepted interfaces are deleted on `CloseDevice`,
`RemoveDevices` or `CloseProjectLogic`; duplicate-name pointers are not
adopted; `RemoveDevices` leaves the path map intact; and implicit registry
destruction adds no new sweep of raw plugin interfaces. It owns neither a
QObject returned from `Platform_Interface::GetObject()` nor a `QPluginLoader`.
Plugin loading, Messenger, XML, GUI and interface/IID headers remain outside
this slice.

Phase 5E.3b prepares but does not activate `RetainLegacyPlugin` and
`PluginReleaseV2`: every reachable registration still uses `HostDelete`.
There is no loader lease, Messenger connection storage or logical legacy-plugin
removal in this structural slice.

## Phase 5E.3c1 successful plugin-loader lifetime

`LoadSave/PluginLeasePool` is an internal application-child owner for loaders
that reached successful existing device registration. It uses unique ownership
without assigning loaders a QObject parent and makes no explicit unload call.
It is deliberately separate from `DeviceRegistry`: all active devices still
use the legacy `HostDelete` strategy, and no Messenger connection or logical
removal policy changed.

## Phase 5E.3c2 Legacy plugin logical-removal boundary

`DeviceRegistry` distinguishes internal registration provenance without
changing `DataManagementClass::AddDevice()`: public caller-created interfaces
remain `HostDelete`, while the private `LoadPlugin` success path records
Legacy-V1 interfaces as `RetainLegacyPlugin`. A retained record holds only
non-owning `QPointer`s to its Messenger and plugin device QObject. Logical
removal disconnects this endpoint pair and removes the active record; it does
not delete the interface or unload its application-lifetime loader. The
resident loader/interface boundary intentionally trades prompt plugin-resource
release for Legacy-V1 ownership safety.
## Phase 4G.2a Project I/O export coordination

`DataManagement/ProjectIoCoordinator` is a private, QObject-free operation
helper used only by `UIDataManagementSetClass` for parameter import, parameter
XML export, MAT export and HDF5 export. It retains a non-owning
`DataManagementSetClass&` and constructs `ParameterLoader`,
`ExportInputs2Xml`, `MatExporter` and `Export2HDF5` for each call. It owns no
adapter, QObject, plugin, messenger, UI state or persistent path state.

The public UI facade remains responsible for all status/error emission,
exception-to-return conversion and return conventions. In particular, it keeps
the HDF5 catch path that emits the existing error text and returns `false`.
Forms, plugin loading, MainWindow routing,
CWD/locale behavior and `LoadPath`, `StdSavePath` and `ChangeDetected` remain
in the facade and outside this slice.

## Phase 4G.2b Experiment-read coordination

`ProjectIoCoordinator::ReadExperiment()` now constructs only the unchanged
`XmlExperimentReader` with the exact pre-extraction arguments:
`(&uiManager, uiManager.GetMessenger(), &uiManager)`. The coordinator remains
non-QObject and non-owning; its additional `UIDataManagementSetClass&` is a
non-owning reference required solely to preserve the reader's existing
parent/manager hierarchy. The reader, writer, plugin loader, form
implementation and MainWindow routing were not changed.

`UIDataManagementSetClass::LoadExperiment()` remains the public facade. It
continues to assign `LoadPath`, retain the reader's boolean convention, emit
the unchanged parse-error text and send the same `CloseProject` message on
failure. Consequently the legacy CWD mutation, `.LAdev` resolution, relative
and absolute path handling, and form/device/figure/widget/connection/state
processing order remain owned by the unchanged reader path. The `XML_LEGACY`
fixtures exercise missing UI/plugin dependencies without any migration or
normalization.

## Phase 4G.2c Experiment-write coordination

`ProjectIoCoordinator::WriteExperiment()` now constructs only the unchanged
`xmlexperimentwriter` with the exact pre-extraction arguments:
`(&uiManager, uiManager.GetMessengerRef(), uiManager)`. As with reading, the
coordinator remains non-QObject and non-owning; the existing UI-facade
reference preserves the writer's QObject parent and manager hierarchy.

`UIDataManagementSetClass::SaveExperiment(QString)` remains the public Qt slot
and keeps all orchestration after the writer call: its inverted boolean/error
convention, existing error and success status text, plugin `save` messages,
and the private `LoadPath`, `StdSavePath`, `ChangeDetected` and MainWindow
state boundaries. The writer itself is unchanged, so its UTF-8 XML output,
section/attribute order, paths, formatting and overwrite/invalid-path behavior
remain governed by the existing XML contracts. Legacy fixtures are read only;
round trips write exclusively to temporary paths.

## ProjectIoCoordinator subsystem checkpoint

The ProjectIoCoordinator extraction is complete for every implemented adapter
operation of `UIDataManagementSetClass`: parameter import/export, MAT export,
HDF5 export, experiment read/write and plugin descriptor loading. The helper
remains a private, non-QObject, non-owning operation coordinator. It creates
the existing adapters with their historic manager, Messenger and QObject-parent
arguments; it does not absorb adapter implementation, plugin interfaces,
format policy or persistent UI state.

The public UI facade deliberately still owns `LoadPath`, `StdSavePath`,
`ChangeDetected`, all Qt signal/status/error routing, post-save plugin messages,
post-load/device routing and MainWindow-facing UI orchestration. `LoadForms()`
is declared but has no repository implementation, so it remains a blocked
boundary rather than a coordinator operation. XML legacy compatibility remains
mandatory: fixtures are read-only inputs and read/write/read vectors use only
temporary destinations.
