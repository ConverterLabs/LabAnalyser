# Milestone 4 isolation report

## Goal and result

Milestone 4 completed the identified, small internal-isolation slices without a
big-bang rewrite. Existing public facades remain the observable boundaries;
their APIs, Qt signals and slots, plugin IID and `InterfaceData` ABI were not
changed by these extractions. This is evidence of compatibility on the recorded
contract vectors, not a claim of equivalence for every possible input or a
claim that the application is now independent of the GUI.

## DataManagement internal boundaries

| Internal boundary | Preserved facade evidence | Characterization and implementation commits |
| --- | --- | --- |
| `DataRegistry` | Form/skip-form, alias and plot/window numbering through `DataManagementClass`. | `DM_REG_001..DM_REG_005`; `a4a0d36`, `caf84e9` |
| `ContainerStore` | Existing mapper-map address, lookup, replacement and cleanup behavior. | `DM_CONT_001..DM_CONT_005`; `b59f8ba`, `e917540` |
| `WidgetBindingRegistry` | Object-name keyed bindings, duplicate/rebind and foreign-QObject non-ownership. | `DM_BIND_001..DM_BIND_006`; `3cb70a8`, `616f6e1`, `f8d4ed6` |
| `MessageDispatchPolicy` | Case-sensitive command classification and Messenger signal ordering. | `DM_MSG_001..DM_MSG_003`; `2a21b22`, `65c76fa` |
| `DeviceRegistry` | Ordered device/path maps and existing raw-interface cleanup policy. | `DM_DEV_001..DM_DEV_004`; `4a98a9a`, `9a9936d` |

These helpers are private implementation boundaries. Raw-pointer exposure,
QObject ownership and the public manager facades deliberately remain compatible
legacy boundaries rather than being silently hardened in this milestone.

## Project I/O boundary

`ProjectIoCoordinator` is a private, QObject-free and non-owning operation
coordinator for parameter import/export, MAT export, HDF5 export, experiment
XML read/write and plugin-descriptor loading. Its extraction is recorded by
`5440cea`, `813b6d1`, `d0f6b46`, `5800047`, with the documented subsystem
checkpoint in `842467f`.

`UIDataManagementSetClass` intentionally remains the QObject/MainWindow,
signal/status/error and mutable-state facade. It retains legacy boolean
conversion, path/change state, Messenger follow-ups and UI routing.
`UIIO_001..UIIO_006`, `PARAM_001..PARAM_009`, `MAT_001..MAT_008`,
`HDF5_001..HDF5_006`, `PLUGIN_001..PLUGIN_007`, `XML_001..XML_008` and
`XML_LEGACY_001..XML_LEGACY_005` are the recorded compatibility vectors.

Legacy XML coverage includes anonymized externally supplied real-world
fixtures. Their hashes are protected by the legacy contract suite; fixtures
remain read-only inputs and all round trips use temporary destinations.

## Remote-control boundary

| Internal boundary | Preserved scope | Evidence commit |
| --- | --- | --- |
| `RemoteControlFrameSplitter` | Complete frame versus remainder bytes only. | `20807a3` |
| TCP byte characterization | Safe native-frame lengths, NUL/padding, reply categories and coalesced replies. | `29edc00` |
| `RemoteControlProtocol` | Safe complete-frame decode/classification and existing `get` reply encoding. | `7e098d2` |
| Connection characterization | Last-client replacement, discarded remainder and controlled disconnect behavior. | `85e8aa7` |
| `RemoteControlConnectionState` | One non-owning current socket plus one splitter. | `56c22c9` |

`TCP_001..TCP_012` and the direct frame-splitter vector preserve the observed
single-current-connection contract: only the most recently accepted client is
processed; accepting a new client discards the prior client's buffered
remainder; earlier clients can remain connected but are not reactivated; and
replies target only the current socket. No independent multi-client session
architecture was introduced.

## Recorded verification evidence

This report references prior evidence only. Focused contract suites passed
before and after the individual extractions; the documented subsystem
checkpoints include central runners and fresh Release/Debug builds where
recorded. File-level coverage and scoped static-analysis results are retained
in the subsystem plans and inventory, not remeasured here. The Windows CI
workflow was locally validated, while a successful remote GitHub run remains
unconfirmed. No coverage gate is enabled.

The recorded API, Qt-metaobject and IID checks found no public API, signal or
slot change. XML, MAT, HDF5, plugin and TCP compatibility is therefore
supported for the documented test vectors only.

## Intentionally not implemented

* `LoadForms()` has no repository implementation and no invented coordinator
  contract.
* No speculative additional core/GUI abstraction was introduced without an
  observed dependency boundary and characterization evidence.
* Ownership, security and error corrections belong to Milestone 5.
* CMake migration, dependency modernization and toolchain work belong to
  Milestone 6.

## Handover to Milestone 5

The following items require explicit approval because they can change behavior
or ownership policy:

1. Qt-6 socket-error signal repair and accepted-socket lifetime policy.
2. Safe TCP frame validation for currently dangerous lengths and layouts.
3. TCP String/StringList `set` payload-byte loss.
4. Selection and container-mutation semantics.
5. DataManagement/plugin raw-pointer ownership and destruction boundaries.
6. Dangerous null, index and sender-dependent paths.
7. QSlider equal-Min/Max division risk.

These risks are not defects silently corrected by isolation. Their recorded
legacy behavior and exclusions remain in the relevant contract documents and
`BEHAVIOR_INVENTORY.md`.
