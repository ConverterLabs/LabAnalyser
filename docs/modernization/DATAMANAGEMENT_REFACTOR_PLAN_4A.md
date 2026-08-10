# DataManagement refactoring plan — Phase 4A

## Scope, evidence and non-goals

This is a design and sequencing plan only. It is based on the current
implementations of `DataManagementClass`, `DataManagementSetClass`,
`UIDataManagementSetClass`, `MessengerClass`, `mapper.h` and
`InterfaceDataType`, their direct production callers, and the existing
characterization suites. No implementation, test, build, API or ABI change is
made in Phase 4A.

The plan preserves the public classes, Qt meta-object surface, existing
messages, plugin interfaces and result/error conventions. Relevant evidence is
`DM_001..DM_010`, `XML_001..XML_008`, `PARAM_001..PARAM_009`,
`MAT_001..MAT_008`, `HDF5_001..HDF5_006`, `TCP_001..TCP_007`,
`PLUGIN_001..PLUGIN_007`, `DW_001..DW_017`, `GUI_001..GUI_021` and
`PLOT_001..PLOT_006`/`FFT_001..FFT_008`.

Non-goals for every initial slice are a CMake migration, renaming public
classes or methods, changing `InterfaceData`/plugin ABI, changing the Qt
signal payloads or ordering, changing persistence formats, and a wholesale
replacement of raw pointers by smart pointers.

## Current responsibilities

| Class / file | Current responsibility | Refactoring assessment |
| --- | --- | --- |
| `DataManagementClass` | One QObject combines container data, aliases/min-max, widget/plot/form registries, plot-window numbering, plugin-device registration and project cleanup. It owns mapper and device pointers by explicit cleanup. | Primary compatibility facade. Its non-Qt registry state is the first extraction candidate. |
| `DataManagementSetClass` | Adds a QObject-owned `MessengerClass`; routes manager data to `VariantDropWidget`s; reads widget values through `sender()`; emits `get`/`set` messages. Constructor discovers/casts its parent as `MainWindow` to obtain the status bar and publish slots. | Keep as the Qt widget/messaging facade while replacing internal registry access incrementally. |
| `UIDataManagementSetClass` | Couples the manager to `MainWindow` signals/slots and orchestrates experiment XML, parameter XML, MAT/HDF5 export and plugin loading. It also stores load/save paths and forwards plugin lifecycle messages. | Long-term GUI/IO boundary only. Do not move business/registry logic into it; move orchestration dependencies out of its implementation behind injected adapters in later slices. |
| `MessengerClass` | Routes string commands (`publish`, `set`, `error`, `info`, notification, lifecycle) to manager signals, widgets, status bar and registered plugin QObjects. | Public message compatibility boundary. Preserve its QObject signals/slots and command order; isolate dispatch policy only after registry extraction is stable. |
| `ToFormMapper` in `mapper.h` | Extends `InterfaceData` with public mutable widget bindings and public Min/Max fields. | Compatibility data record. Keep its exact layout and public fields while new internal registry records use it by reference/pointer. |
| `InterfaceData` / `InterfaceDataType.*` | Plugin-visible tagged `boost::variant` data, string/type conversions, editability and metadata. | Plugin ABI/API boundary; do not move, rename or alter storage/semantics during Phase 4. |

## Dependency, coupling and ownership graph

```text
MainWindow (QObject owner)
  └─ UIDataManagementSetClass (QObject child; private destructor, owned by parent)
       └─ DataManagementSetClass
            └─ DataManagementClass
                 ├─ owns raw ToFormMapper* in Container; deletes in replacement/CloseProjectLogic
                 │    └─ non-owning ObjectStruct::FormP -> DropWidgets / PlotWidget QObjects
                 ├─ owns raw Platform_Interface* in _Devices; deletes in CloseDevice/RemoveDevices/CloseProjectLogic
                 │    └─ plugin object / QPluginLoader lifetime is external and coupled to loadplugin
                 ├─ non-owning QObject* PlotObjects -> PlotWidget instances
                 └─ value registries: forms, skip flags, aliases, plot/window numbering, widget-ID map
            └─ owns MessengerClass as QObject child
                 ├─ non-owning QStatusBar* from MainWindow
                 ├─ connects manager signals/slots and MainWindow slots
                 └─ connects plugin QObject signals/slots during registration

External adapters -> MessengerClass -> DataManagement slots / UI slots
  plugins, RemoteControlServer, XML reader/writer, parameter importer/exporter,
  MAT/HDF5 exporters, DropWidgets, PlotWidget, MainWindow actions
```

`QObject` parenting is reliable only for `UIDataManagementSetClass` under
`MainWindow` and `MessengerClass` under `DataManagementSetClass`. The registry
maps hold raw pointers with manually implemented ownership; widget and plot
pointers are observers, not owners. No slice may put a `QObject` simultaneously
under QObject-parent ownership and a deleting smart pointer. For mapper/device
storage, a future internal RAII container may own the pointees, but public raw
pointer access must remain a non-owning compatibility view until all callers
are migrated.

## Public and external contracts to preserve

| Surface | Contract that remains stable |
| --- | --- |
| `DataManagementClass` public methods | Plot/form/device/container lookup and registration, numbering, aliases, min/max, widget mapping, `GetContainerPointer`, `CloseProjectLogic`, and their documented legacy lookup side effects until an explicitly approved behavior change. |
| `DataManagementClass` signals/slots | `SetData(ID, InterfaceData)`, `AddContainerElement`, `CloseProject`, `MessageSender`, `Info`, `Error`, `PublishFinished`; payloads and observed order stay unchanged. |
| `DataManagementSetClass` | `GetMessenger`/`GetMessengerRef`; widget propagation `SetData(ID)`; sender-dependent `SendNewValue`; both `UpdateRequest` overloads. |
| `UIDataManagementSetClass` | Inverted legacy error booleans, `LoadFormFromXML` signal, save/load/export/import/plugin behavior, path state and MainWindow signal connections. |
| `MessengerClass` | Slots `MessageReceiver`, `MessageTransmitter`, status/info/error/device registration; all emitted signal names, payloads and command ordering. |
| Plugin and data ABI | `Platform_Fabric`, `Platform_Interface`, IID, `InterfaceData` variant alternatives, `ToFormMapper` public fields and existing plugin message connections. |
| Persistence/network/GUI | XML structure/path semantics, MAT/HDF5 content contracts, parameter representation, TCP command flow, object names, widget ID mapping and plot registrations. |

## Direct coupling and implicit state

* `DataManagementSetClass` includes widgets, `PlotWidget` and `mainwindow.h`,
  asserts a parent named `LabAnalyser`, downcasts that parent, and exposes the
  status bar to the messenger.
* `UIDataManagementSetClass` includes MainWindow/UI, widgets, subplots, XML,
  imports, exports and plugin loader. It stores `LoadPath`, `StdSavePath` and
  `ChangeDetected`; `LoadForms` is declared but has no repository definition.
* `MessengerClass` uses parent and parent-parent object names for emitted UI
  message IDs; it owns no status bar and keeps only a non-owning pointer.
* `DataManagementClass`'s mutable maps and vectors, current QObject sender,
  object names, global `GetMainWindow()` use in widgets, Qt connection state,
  process current directory during XML/UI loading and QLocale changes are
  implicit state relevant to callers.
* XML readers/writers use manager/messenger references or QObject-parent casts;
  exporters take manager pointers/references; plugin loader registers devices;
  remote control and plugins speak through messenger commands; widgets and
  plots retain manager-observed object pointers.

## Direct caller and test map

| Caller group | Direct dependency / use | Constraint on a slice |
| --- | --- | --- |
| `MainWindow`, `SubPlotMainWindow`, `mainwindow.ui` actions | Owns the UI manager; connects project, output, tree, dock, plot and tray workflows to manager/messenger. | Keep object names, parent ownership, slots and action-visible side effects. |
| `DropWidgets/*`, `PlotWidget` | Call `ConnectToID`, `SendNewValue`, `UpdateRequest`, container/min-max/alias and object registration APIs. | Preserve object-name mapping, duplicate connection behavior and signal ordering. |
| `xmlexperimentreader` / `xmlexperimentwriter` | Read/write forms, containers, mappings, devices, aliases, plot-window data and messenger commands. | Preserve XML order, return conventions and reader/writer QObject-parent assumptions. |
| `parameterloader`, `ExportInputs2Xml` | Add/set/read manager containers and their `InterfaceData` values. | Preserve duplicate, conversion and inverted bool behavior. |
| `MatExporter`, `Export2HDF5` | Read data containers through `DataManagementClass`. | Preserve raw-data types, aliases/names and exporter error behavior. |
| `LoadPlugin`, `platforminterface` implementations | Register/read devices and exchange `MessengerClass` messages. | Preserve `Platform_Interface` pointer identity, plugin IID/ABI and approved incompatible-plugin rejection. |
| `RemoteControlServer` | Uses messenger `get`/`set`/publish-style protocol flow and container data. | Preserve TCP-visible command bytes and message sequence. |
| Existing tests | Component `DM_001..DM_010`; XML/parameter/export/HDF5/MAT/plugin/TCP/DropWidget/MainWindow/plot contract suites. | Execute the mapped suites before and after each slice; add only direct helper tests for newly extracted pure logic. |

### API and Qt meta-object inventory

* `DataManagementClass` public API groups are plot/window registration and
  numbering; form/skip-form tracking; widget/container binding and lookup;
  device registration/cleanup; aliases and min/max; and project cleanup. Its
  public slots are `SetData(ID, InterfaceData)` and `AddContainerElement`; its
  signals are `CloseProject`, `MessageSender`, `Info`, `Error` and
  `PublishFinished`.
* `DataManagementSetClass` adds `GetMessenger`/`GetMessengerRef` and public
  slots `SetData(ID)`, `SendNewValue`, `UpdateRequest()` and
  `UpdateRequest(ID)`.
* `UIDataManagementSetClass` exposes save/load experiment, XML/MAT/HDF5 export,
  parameter import/export, plugin loading, `RegisterChange`, `LoadForms`, slot
  `SaveExperiment`, and signal `LoadFormFromXML`.
* `MessengerClass` exposes slots `MessageReceiver`, `MessageTransmitter`,
  `SendInfo`, `SendError`, `NewDeviceRegistration`, `WriteStatusMessage`; its
  signals are message forwarding, UI add/error/info/notification, data receive,
  container/data set, close-project and publish-start/finished events.
* `InterfaceData` is a public plugin data object with type/editability queries,
  typed getters/setters, raw variant access, conversion helpers and metadata;
  `ToFormMapper` additionally exposes public `Objects`, `MinValue` and
  `MaxValue`. Their headers are ABI-sensitive and are not refactoring targets.

## Testable core and proposed internal seams

The first extraction must be a plain C++ registry that depends only on QtCore
value types and `ToFormMapper`/`Platform_Interface` compatibility types. It
must not include MainWindow, widgets, XML, sockets, plugin loader or status-bar
types.

| Proposed internal file/class | Single responsibility | Compatibility boundary |
| --- | --- | --- |
| `DataManagement/DataRegistry.h/.cpp` | Alias, form/skip-form and plot/window registry operations; preserves lookup/order/number behavior. | Used privately by `DataManagementClass`; all existing `DataManagementClass` methods remain forwarding facade methods. |
| `DataManagement/ContainerStore.h/.cpp` | Own the existing raw mapper map, replacement, lookup, count and cleanup without owning mapper-bound QObjects. | Used privately by `DataManagementClass`; `GetContainerPointer()` continues to expose the actual stable map address. |
| `DataManagement/WidgetBindingRegistry.h/.cpp` | Map widget object names to IDs and maintain non-owning `ObjectStruct` binding lists. | Private helper only; `AddElementToContainerEntry`, lookup and deletion methods keep existing signatures and behavior. |
| `DataManagement/DeviceRegistry.h/.cpp` | Device/path registration and explicit device cleanup policy. | Deferred until plugin loader/object lifetime is re-characterized; facade keeps raw `Platform_Interface*` API. |
| `DataManagement/MessageDispatchPolicy.h/.cpp` | Pure classification of messenger command strings and ordered dispatch intents. | Deferred; `MessengerClass` remains the sole QObject signal emitter and status-bar adapter. |
| `DataManagement/ProjectIoCoordinator.h/.cpp` | Construct XML/import/export/plugin adapters from manager/messenger dependencies. | Deferred; `UIDataManagementSetClass` stays an unchanged GUI/IO facade and preserves bool conventions. |

No proposed internal type is a plugin interface or public header in its first
slice. `DataManagementClass`, `DataManagementSetClass`,
`UIDataManagementSetClass`, `MessengerClass`, `ToFormMapper` and
`InterfaceData` remain compatible facades/boundaries.

## Refactoring slices and rollback

| Slice | Change after baseline tests pass | Before/after evidence | Risks and rollback |
| --- | --- | --- | --- |
| 4A.1 — registry extraction | Introduce `DataRegistry` for form/skip-form, alias and plot/window value registries; forward existing `DataManagementClass` methods without changing signatures or pointer containers. | Existing `DM_001..DM_004`, plot/MainWindow tests and new direct registry equivalence tests for numbering, form order, alias fallback and cleanup. Run the same vectors before/after. | Map lookup insertion and numbering order are observable. Roll back by removing private delegation and retaining the original maps; no persisted format changes. |
| 4A.2 — container ownership boundary | Move only `Container` storage behind a private owner with raw non-owning facade access. Preserve replacement, pointer identity, public `GetContainerPointer` behavior and cleanup order. | DM container/mapping tests, XML/parameter/MAT/HDF5/DropWidget/plot contracts before and after. | Raw `ToFormMapper*` escape and widget observer lifetime. Roll back to map-owned raw pointers; do not use smart pointers across QObject/widget ownership. |
| 4A.3 — widget binding helper | Extract binding map/list mutation while leaving `DataManagementSetClass` slots and `VariantDropWidget` calls in place. | `DM_003/004/009/010`, DW and GUI binding tests; signal order unchanged. | Duplicate mapping and object-name collisions. Roll back helper delegation only. |
| 4A.4 — message dispatch seam | Extract pure command classification; retain `MessengerClass` routing and every Qt emission in the facade. | `DM_006..DM_008`, TCP/plugin/XML/UI message contracts and QSignalSpy order. | String command/error semantics and parent-chain IDs. Roll back policy call, not public signals. |
| 4A.5 — device ownership clarification | Introduce an internal explicit owner only after loader/plugin lifetime tests cover unload/cleanup interactions. | `DM_005`, `PLUGIN_001..007`, XML device and GUI remove-device tests. | Plugin loader may own/destroy objects independently; this slice requires a separate ownership decision. Roll back to current explicit delete policy. |
| 4A.6 — GUI/IO orchestration boundary | Move construction of XML/import/export/plugin adapters to `ProjectIoCoordinator`; retain `UIDataManagementSetClass` methods and bool/error conventions as forwarding facade. | XML, parameter, MAT, HDF5, plugin and MainWindow tests before/after. | Inverted bool conventions, working-directory/path state and MainWindow signals. Roll back coordinator delegation per operation. |

Every slice is one mergeable change: first add direct characterization tests for
the affected internal helper, then execute all mapped existing contracts on the
unchanged baseline, make the smallest delegation change, and rerun identical
vectors. A slice stops rather than expanding scope if it needs an API/ABI,
signal-order, persistence or ownership-policy change.

## Explicit risk register

* Public raw pointers escape through `GetContainer`, `GetContainerPointer`,
  `GetDevice` and widget/plot registries. Pointer identity and destruction
  timing are compatibility constraints, not implementation details.
* Current `std::map::operator[]` lookup insertion, unchecked indexes and
  sender/parent dereferences include unsafe paths. Do not normalize them as a
  side effect of extraction; isolate and approve any safety change separately.
* Plugin interfaces and `InterfaceData` cross a binary boundary. Moving their
  definitions, changing data layout, RTTI-visible names or Qt meta-types is
  outside Phase 4A.
* `UIDataManagementSetClass` has private destruction and MainWindow QObject
  ownership; no smart owner may be introduced for it.
* XML/UI loader current-directory changes, status-bar timeouts and QObject
  signal connection multiplicity are external side effects that must remain
  at the facade edges.

## Measurable completion criteria

Phase 4 implementation is complete only when, per accepted slice:

1. all existing mapped tests pass before and after the slice, with documented
   signal order, values and file/protocol equivalence where applicable;
2. fresh central runner plus Release and Debug builds pass;
3. changed production functions have direct/mapped test IDs and no file-level
   coverage regression against the existing DataManagement evidence;
4. no public header, Qt signal/slot signature, plugin IID or `InterfaceData`
   ABI changes unless a separately approved migration records them;
5. raw-pointer ownership is documented at every new private boundary, with no
   conflicting QObject/smart-pointer ownership; and
6. one slice can be reverted by removing private delegation without data-format
   migration or a consumer source change.

## Recommended first small slice

Start with **4A.1: extract only the value-only form/skip-form, alias and
plot/window numbering registries into private `DataRegistry`**. It avoids the
plugin, mapper, widget, MainWindow and QObject lifetime boundaries; has direct
coverage through `DM_002`/`DM_003` plus GUI/plot lifecycle contracts; and can
retain every existing `DataManagementClass` method as a simple forwarding
facade. Do not include container pointers or device ownership in this first
slice.

## Phase 4B.1 characterization evidence (unchanged facade)

The first-slice baseline is now explicit in the public-facade tests
`DM_REG_001..DM_REG_005`; no `DataRegistry` exists in this phase. The facade
currently treats form files as an ordered multiset and removes the first
matching form entry only. Skip-form flags are keyed values with last-write-wins
semantics. Alias fallback returns the requested ID, including after project
cleanup; unknown IDs, empty aliases and Unicode aliases are accepted. Plot and
figure/window name maps replace the current lookup on repeated registration,
while the associated number vectors retain duplicate history. One delete
therefore may leave the next-number query at `1` despite no lookup entry for
that name. Unknown window geometry reads return `(0, 0)` through the current
insertion-capable lookup. All of these are before/after equivalence vectors for
4A.1, not intended design improvements.

The tests also establish that separate `DataManagementClass` instances do not
share these value registries and that destruction followed by reconstruction
does not recover prior form, alias or plot state. They intentionally avoid
unchecked indexed form access and pointer/device/container ownership paths;
those remain later-slice risks, not safe registry characterization inputs.

## Phase 4B.2 implementation outcome

**Completed 2026-08-10.** `DataRegistry.h/.cpp` now holds only the initially
approved form/skip-form, alias and plot/window bookkeeping. `DataManagementClass`
owns it privately via RAII and forwards its unchanged public methods. The old
facade fields were removed only after every covered operation delegated to the
registry. Its non-owning plot `QObject*` entries preserve the former lifetime
policy; no container, device, widget, mapper or Messenger field moved.

The unchanged `DM_REG_001..DM_REG_005` suite passed (17/17 total), the central
runner passed all 11 registered targets, and fresh Release/Debug application
builds passed. A scoped GCC analysis build passed with the unchanged 162
filtered diagnostics; three pre-existing signed/unsigned loop diagnostics move
from `DataManagementClass.cpp` to `DataRegistry.cpp` with their code.
Focused coverage reports 87.50% lines for `DataManagementClass.cpp` and 91.58%
for `DataRegistry.cpp`; the pre/post denominator change is documented in the
inventory. The bundle retains 88.89% line coverage under the same focused
vector. Rollback remains local: remove the private delegation and restore the
former fields; no external API, signal/slot, ABI or persisted data migration is
involved.

The next slice remains 4A.2 container ownership. It must not absorb registry,
device or widget changes opportunistically.

## Phase 4C.1 container-ownership characterization

`DM_CONT_001..DM_CONT_005` establish the public-facade baseline before any
container owner exists. `GetContainerPointer()` repeatedly returns the same
map address; known string lookups return the map's exact mapper pointer, while
a missing string lookup returns null without insertion. In contrast, an
unlinked QObject lookup inserts the empty ID with a null mapper through the
existing `operator[]` path. This is an observable legacy side effect.

Replacing an existing container deletes its old mapper, produces a different
mapper pointer, preserves its `Objects` list and Min/Max values, and applies
the new type/state metadata. The released pointer is never dereferenced by the
tests. Map traversal used by `GetContainerElementForms` is lexical-key order.
`CloseProjectLogic` clears containers and releases mappers while externally
parented bound QObjects survive. Destroying a manager likewise leaves such
foreign QObjects intact; separate manager instances keep independent container
state. These are the required before/after vectors for Phase 4C.2.

## Phase 4C.2 container-owner extraction

**Completed 2026-08-10.** `ContainerStore.h/.cpp` now owns only the existing
`std::map<QString, ToFormMapper*>` behind the unchanged `DataManagementClass`
facade. Its RAII cleanup deletes each currently owned mapper exactly once on
replacement, `CloseProjectLogic`, and destruction; mapper-bound `QObject*`
form objects remain non-owning and are never deleted by the Store.

The same `DM_CONT_001..DM_CONT_005` vectors passed unchanged (22/22 focused
DataManagement checks) before and after delegation: map address stability,
missing string lookup without insertion, empty-ID QObject lookup insertion,
lexical key order, replacement with Min/Max and form-binding preservation, and
foreign-QObject survival remain observable contracts. The Store retains the
actual exposed raw map rather than a parallel smart-pointer map or a copy. Its
public mutability therefore remains a documented ownership/API boundary: an
external caller can still mutate map entries outside Store mediation.

Fresh Release and Debug application builds, the 11-target central runner and
the scoped static-analysis build all passed. Focused gcov is file-specific:
`DataManagementClass.cpp` is 91.53% lines (162/177), 92.71% executed branches
(178/192), 56.77% branches taken (109/192), and 85.80% calls (151/176);
`ContainerStore.cpp` is 94.12% lines (32/34), 100.00% executed branches
(16/16), 93.75% branches taken (15/16), and 100.00% calls (13/13). The former
facade-only 87.50% line figure has a different denominator after delegation;
these figures are not project coverage. The scoped warning build retained 162
filtered existing diagnostics and produced none in `ContainerStore.cpp`.

No public API, Qt signal/slot, `InterfaceData` ABI or plugin IID changed.
`ElementsToContainerID`, device ownership, Messenger and GUI/IO remain out of
scope; the next slice must not absorb them opportunistically. Rollback is
local: restore the former private map/delegation without persistence or API
migration.

## Phase 4D.1 widget-binding characterization

**Completed 2026-08-10.** The unchanged public facade is now characterized by
`DM_BIND_001..DM_BIND_006` before any `WidgetBindingRegistry` exists. The
vectors cover valid and unknown lookups, ordinary repeated bind/rebind/remove,
the `PlotWidget` duplicate-entry exception, project cleanup, destroyed bound
QObjects, foreign QObject ownership, `SendNewValue` Messenger feedback, and
manager-instance isolation. `DM_BIND_006` additionally records duplicate live
object names, name changes, the empty-name key, return to the original name,
current-name sender routing and their per-manager isolation.

The binding map is name-keyed, so QObject pointer identity is preserved only in
the mapper `Objects` list, not as the lookup key. Two live QObjects with the
same name are therefore indistinguishable to lookup, and renaming never
migrates the old map key or mapper entry. Sender routing likewise follows only
the current name. A destroyed bound QObject is not removed automatically; no
stale pointer is dereferenced by the tests. `CloseProjectLogic` clears bindings
completely. The direct ID-specific removal does not erase the name map, whereas
the QObject overload does. There is no safe public single-container-removal
method; raw-map mutation through
`GetContainerPointer()` is deliberately excluded as the documented ownership
boundary.

The next implementation slice may introduce a private
`WidgetBindingRegistry` only for `ElementsToContainerID` and mapper binding-list
mutation. It must preserve name-keyed lookup, name collision/rename behavior,
empty-ID insertion behavior, ordinary detach/rebind semantics, PlotWidget
duplicate behavior, cleanup order, and non-ownership of QObjects under the
unchanged `DM_BIND_*` vectors. It must
not move ContainerStore, DataRegistry, device, Messenger, GUI or IO logic.

## Phase 4D.2 WidgetBindingRegistry extraction

**Completed 2026-08-10.** `WidgetBindingRegistry.h/.cpp` now contains only the
former `ElementsToContainerID` map and its exact value-map operations:
existing-name lookup, `operator[]` insertion, set, take-and-erase and clear.
`DataManagementClass` remains the unchanged public QObject facade and retains
all coordination with `ContainerStore`, including mapper `Objects` mutation and
the `PlotWidget` duplicate-registration exception. The old facade field was
removed only after all callers delegated.

The unchanged `DM_BIND_001..DM_BIND_006` suite passed (32/32 focused
DataManagement checks), as did all 11 central runner targets and fresh
Release/Debug application builds. The scoped static-analysis build passed with
the pre-existing 162 filtered diagnostics and none in
`WidgetBindingRegistry.cpp`. Focused gcov is file-specific:
`DataManagementClass.cpp` is 91.38% lines (159/174), 93.55% executed branches
(174/186), 56.99% branches taken (106/186), and 86.59% calls (155/179);
`WidgetBindingRegistry.cpp` is 100.00% lines (18/18), 100.00% executed
branches (10/10), 70.00% branches taken (7/10), and 100.00% calls (5/5).
The prior facade denominator was 177 lines; these figures are not project
coverage and are only a post-extraction comparison.

No public API, Qt signal/slot, plugin IID or `InterfaceData` ABI changed. The
Registry stores no QObject pointer, makes no destroyed connection and never
deletes a QObject. It intentionally preserves name collisions, empty-name keys,
non-migrating renames and stale mapper-pointer behavior. Rollback is local:
restore the private map/delegation without persistence or consumer migration.

## Phase 4E.1 Messenger dispatch characterization

**Completed 2026-08-10.** Before introducing any internal dispatch policy,
`DM_MSG_001..DM_MSG_003` establish the public
`MessengerClass::MessageReceiver`/`MessageTransmitter` command matrix: every
implemented command, exact synchronous signal order, IDs and `InterfaceData`
payloads, repeated calls, safe empty/unknown inputs, the
publish/container/update/widget/set chain, and CloseProject's valid
parent-parent identifier. `MessageTransmitter` dispatches first and emits one
`MessageSender` afterward.

Null-parent, null-sender and destroyed-QObject paths remain unsafe exclusions,
not contracts. A later 4E.2 may introduce a private, non-QObject
`MessageDispatchPolicy` only behind the unchanged Messenger facade and only if
all `DM_MSG_*` vectors remain unchanged; it must not absorb TCP, plugin, XML,
UI or device logic.

## Phase 4E.2 MessageDispatchPolicy extraction

**Completed 2026-08-10.** `MessageDispatchPolicy` now holds only the typed,
case-sensitive command-to-ordered-intent classification. It neither stores nor
interprets `InterfaceData`, derives parent IDs, emits signals, mutates manager
state, nor owns any QObject or infrastructure dependency. `MessengerClass`
iterates the returned intents and remains the sole executor. The unchanged
`DM_MSG_001..DM_MSG_003` vectors retain publish ordering, CloseProject's
parent-parent notification and transmitter-before-forward ordering.

Rollback is local: restore the former command branches in `MessengerClass` and
remove the private source from qmake targets. No public header, Qt signal/slot,
plugin IID or InterfaceData ABI change is involved.

## Phase 4F.1 device-ownership characterization

`DM_DEV_001..DM_DEV_004` now establish the public-facade baseline for the
still-unextracted device maps. `AddDevice` adopts the first non-null interface
for a name and its first path; repeated registration with the same pointer or a
different pointer preserves that entry and does not destroy the rejected
pointer. The caller of a rejected pointer therefore remains responsible for
its cleanup. Device and path enumeration follows lexical `std::map` key order,
and repeated `GetDevice` calls return the same raw pointer while it is owned.

`CloseDevice` deletes a known owned interface exactly once, removes its path,
and has no further visible effect for an unknown or already-closed name.
`RemoveDevices`, in contrast, deletes all currently owned interfaces but leaves
the legacy path map intact: `GetDevices()` and `GetDevicePaths()` can therefore
still report paths for unavailable devices until a later close/project cleanup
or replacement. Re-registration after removal replaces that surviving path.
`CloseProjectLogic` clears both maps and deletes devices in lexical map order.
It does not own the `QObject*` returned by `Platform_Interface::GetObject()`;
the QPointer vector records that this object remains alive until its external
owner deletes it. Separate managers isolate all device state and cleanup.

These tests deliberately do not register null interfaces, dereference released
raw pointers, or force plugin unload sequences. Those remain dangerous
ownership/plugin-boundary risks. The existing `PLUGIN_001..PLUGIN_007`
contracts cover failed plugin loading and safe incompatible-plugin rejection;
4F.1 relies on that evidence rather than duplicating plugin-loader tests.
The focused DataManagement suite passed 35 checks, the independent plugin
contract suite passed 9 checks, and the central runner completed all 11
registered projects with exit code 0. The runner now deliberately treats native
stderr as diagnostic output while retaining its explicit non-zero native
exit-code failure rule, because the HDF5 negative contract vectors emit
expected library diagnostics on stderr.
The next implementation slice may introduce only a private `DeviceRegistry`
behind the unchanged facade, preserving both the raw-pointer and stale-path
semantics above before any separately approved ownership hardening.

## Phase 4F.2 DeviceRegistry extraction

**Completed 2026-08-10.** `DataManagement/DeviceRegistry.h/.cpp` now holds
only the legacy ordered device-pointer and device-path maps. The private
`DataManagementClass` deleter owns the registry object, while the registry
retains the existing explicit raw-interface cleanup policy: it deletes accepted
interfaces in `Close`, `RemoveDevices` and `ClearProjectDevices`, but its own
implicit destruction does not introduce a new plugin-interface sweep.

The unchanged `DM_DEV_001..DM_DEV_004` vectors verify first pointer/path wins,
lexical lists, duplicate rejection without adoption, raw lookup identity,
unknown/repeated close, the intentionally stale path map after bulk removal,
re-registration, lexical project cleanup, non-owning `GetObject()` QObjects and
instance isolation. There is no QObject, QPluginLoader, Messenger, XML, GUI or
network dependency.

Focused gcov is file-specific, not project coverage: `DataManagementClass.cpp`
is 94.44% lines (153/162), 96.47% executed branches (164/170), 55.29% branches
taken (94/170), and 86.56% calls (161/186); `DeviceRegistry.cpp` is 93.55%
lines (29/31), 83.33% executed branches (20/24), 75.00% branches taken (18/24),
and 100.00% calls (10/10). The scoped GCC baseline remains 161 diagnostics
with none in `DeviceRegistry.cpp`.

Rollback remains local: restore the two private maps and their facade bodies,
then remove the helper from qmake targets. No persistence, public API, signal,
slot, plugin IID or `InterfaceData` ABI migration is involved. The raw-pointer
and manager-destruction lifetime boundary remains a later ownership-hardening
decision, not a side effect of this extraction.
## Phase 4G.1 Project-IO facade characterization

**Completed 2026-08-10.** `UIIO_001..UIIO_006` characterize the public
`UIDataManagementSetClass` boundary against the real offscreen `MainWindow`
graph without exposing private state or changing production code. The vectors
cover save/load, repeated form routing and partial XML loading, parameter
import/export, MAT/HDF5 return/error conventions, runtime-built valid and
rejected plugins, Messenger ordering, temporary-path handling, CWD/locale
effects and instance-local operation.

The characterization records three legacy limits that a future coordinator
must preserve or explicitly change: state fields `LoadPath`, `StdSavePath` and
`ChangeDetected` are not public observables; `LoadExperiment` routes form
events directly to MainWindow rather than the facade's declared signal and
leaves the process CWD at the experiment directory; and an HDF5 write failure
emits `Error` while returning the same `false` value as success. `LoadForms()`
has no repository definition, so it is a documented blocker rather than an
invented contract.

The next implementation slice may introduce only an internal
`ProjectIoCoordinator` behind this facade. It must retain all public method
signatures, inverted bool conventions, Messenger signal order, plugin/XML
routing and process-state vectors. It must neither absorb MainWindow UI policy
nor alter the unimplemented `LoadForms` boundary. Rollback is local: restore
the facade's adapter calls and remove the private coordinator sources; no file
format, plugin ABI or public API migration is allowed.
