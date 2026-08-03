# DataManagement characterization 3A

## Scope and call graph

The production scope is `DataManagementClass`, `DataManagementSetClass`,
`MessengerClass`, `UIDataManagementSetClass`, and `ToFormMapper` in
`DataManagement/`. `MainWindow` owns the UI manager; dynamic widgets call
`SendNewValue`/`UpdateRequest`; plugins and remote-facing code use the messenger
for `publish`, `set`, `get`, information and lifecycle messages. The manager
owns `ToFormMapper*` and `Platform_Interface*` only through its explicit
cleanup methods. Qt owns `MessengerClass` because it is constructed with the
set manager as QObject parent.

## Test IDs and covered surfaces

| IDs | Surface exercised |
| --- | --- |
| DM_001 | Empty manager lookups and repeated benign deletions. |
| DM_002 | Plot/window registries, numbering, rename/delete, forms and skip flags. |
| DM_003 | Containers, aliases, min/max, mapping, base `SetData`, interface extraction. |
| DM_004 | Repeated/reassigned mappings and lookup side effects. |
| DM_005 | Devices and `CloseProjectLogic` cleanup/ownership. |
| DM_006 | `publish` signal counts, order and values. |
| DM_007 | Messenger forwarding and special/unknown commands. |
| DM_008 | Plugin forwarding and Messenger QObject ownership. |
| DM_009 | Widget propagation and implicit/explicit update requests. |
| DM_010 | Set ownership, repeated parameter updates and read-only Data forwarding. |

All public/protected functions were inspected. The three compiled classes are
covered at their safe public/slot boundaries above; `GetMessenger` and
`GetMessengerRef` are covered by DM_010. `ToFormMapper` is characterized through
its inherited data state because it adds public fields rather than operations.

## Deferred or unsafe paths

`UIDataManagementSetClass::{SaveExperiment,LoadExperiment,Export2Xml,
Export2Mat,Export2Hdf5,LoadPlugin,ImportFromXml}` requires a real `MainWindow`
and XML, plugin, MAT and HDF5 boundaries. `LoadForms` is declared but has no
repository definition. These paths are deferred to contract/integration suites.

Invalid inputs to `GetFormFileEntry`, `GetObjectType`, `GetInterfaceData`,
`SetMinMaxValue`, `MinMaxValue`, and `AddElementToContainerEntry` can dereference
absent vector/map/raw-pointer entries; they are not executed as contracts.
Sender-less `SendNewValue`/`UpdateRequest` and `MessageReceiver("CloseProject")`
also dereference null sender/parent chains. These are risk findings, not desired
behavior.

## Observed legacy behavior

- A missing-object `GetContainer(QObject*)` lookup inserts an empty map entry;
  `IsObjectLinked` then returns true while `GetContainer` is null (DM_001/004).
- A declared mapper type is not returned by `GetDataType()` until data is set
  (DM_003).
- Reassigning a non-plot widget removes its previous mapping; repeated identical
  registration remains one mapping (DM_004).
- Raw mapper/plugin cleanup depends on explicit cleanup calls (DM_005).
- Generic test parents expose string-connect warnings; production parents must
  supply the expected Qt signals and slots (DM_006-008 setup).

## Per-file coverage evidence

GCC `--coverage` plus gcov was built and run separately. These are per-file
metrics, not project coverage.

| Production file | Lines | Branches executed | Calls executed |
| --- | ---: | ---: | ---: |
| `DataManagementClass.cpp` | 88.89% (200/225) | 93.33% (224/240) | 77.78% (133/171) |
| `DataManagementSetClass.cpp` | 91.18% (31/34) | 89.74% (70/78) | 69.09% (38/55) |
| `DataMessengerClass.cpp` | 80.36% (45/56) | 75.00% (66/88) | 73.39% (80/109) |
| `UIDataManagementSetClass.cpp` | not built in isolation | not measured | not measured |

No coverage threshold is enabled.
