# Plugin interface ownership — phase 5E

## Scope

Phase 5E.2a adds test-only ownership models and characterizes only safe
plugin/device lifetime boundaries. It does not change production ownership,
the plugin IID, the plugin ABI, `DeviceRegistry`, or loader behavior.

## Test fixtures

Both fixtures are built at test time with the host's MSYS2 MINGW64/Qt qmake
configuration and are never versioned as DLLs.

| Fixture | `GetInterface()` result | Ownership model | Safe coverage |
| --- | --- | --- | --- |
| `MemberOwnedInterfacePlugin` | Address of a C++ member of the plugin-root object | The plugin root owns the member; the host must not delete it. | Load, repeated pointer identity, QObject observation and local-loader lifetime only. |
| `HeapOwnedInterfacePlugin` | Separately `new`-allocated fixture object | The fixture deliberately leaves it to a controlled test cleanup after manager destruction. This is a test model, not a new third-party plugin rule. | Load, registration, no implicit manager cleanup and one fixture-only rest cleanup. |

Neither fixture claims how a third-party plugin allocates or releases its
interface. Both retain the established
`org.qt-project.Qt.Examples.EchoInterface` IID and use only existing public
plugin headers.

## Characterized safe paths

| ID | Contract |
| --- | --- |
| `PLUGIN_008` | The member model loads through `QPluginLoader`, casts to `Platform_Fabric`, returns one stable member pointer, and exposes a live `GetObject()` QObject without host deletion. |
| `PLUGIN_009` | The heap model loads, returns one stable heap pointer and exposes its QObject and instrumentation before any host cleanup. |
| `PLUGIN_010` | Destroying a manager/registry without explicit device cleanup does not itself delete either registered fixture interface. The heap model is then deleted once by controlled fixture-only cleanup; the member pointer is never deleted. |
| `PLUGIN_011` | After a local `QPluginLoader` helper ends, the observed root and member-device remain reachable in this installed Qt/MSYS2 configuration. This is observed runtime behavior, not a general binary-ABI guarantee. |

## Explicit exclusions for 5E.2b

- Calling `CloseDevice()` or `RemoveDevices()` for the member-owned model is
  unsafe and is not executed in the normal test process.
- Real plugin unload while a registered interface, device QObject, or Messenger
  connection remains alive is not tested.
- Null `GetInterface()`/`GetObject()`, cross-CRT/debug-release plugin matrices,
  arbitrary third-party allocation strategies and stale raw-pointer
  dereference remain excluded.
- No conclusion is drawn about the safety of host `delete` for real plugins.

## Current interpretation

`DeviceRegistry` performs explicit raw-pointer deletion, whereas destruction
of its enclosing manager alone has no device sweep. Consequently, the current
public plugin interface does not provide enough ownership information to make
host deletion universally safe. A later, separately approved 5E.2b decision
must distinguish a compatibility-preserving host-delete contract, a versioned
plugin release API, or another explicit ownership policy.

## Fast-verification evidence

On 2026-08-11 the portable plugin build completed with exit code 0 and
produced both new DLLs only below ignored `build/test-plugins/` directories.
`PluginLoaderContractTests` then completed with exit code 0: 13 Qt Test checks
passed (the eleven `PLUGIN_001..PLUGIN_011` cases plus init/cleanup). The
existing test-only `DataManagementSetClass` seam continues to issue the known
`QObject::CloseProject()` slot warning when its plain `QObject` parent is used;
it is not an ownership result and no warning was hidden.

The observed results are: both fixtures returned stable pointers; their
`GetObject()` QPointers stayed valid on the non-cleanup paths; manager
destruction did not delete either registered interface; controlled deletion of
the heap fixture then produced exactly one fixture destructor observation; and
the local `QPluginLoader` scope did not invalidate the member fixture root or
device in this Qt 6.9.2/MSYS2 run. No member-owned interface was passed to
`CloseDevice()` or `RemoveDevices()`.

## 5E.2b isolated cleanup diagnosis

The temporary process-isolated harness used the real `QPluginLoader` /
`Platform_Fabric::GetInterface()` boundary and the real `DataManagementClass`
and `DeviceRegistry`; it was removed after the diagnosis. Heap fixture paths
were repeatable and exited successfully: `CloseDevice`, `RemoveDevices` and
`CloseProjectLogic` each caused exactly one fixture destructor observation.
`RemoveDevices` retained one path, while `CloseProjectLogic` removed it.

For the member fixture, all five isolated runs of each explicit cleanup path
ended with `0xC0000374` immediately after the pre-cleanup marker. A diagnostic
gdb run stopped in `RtlFreeHeap`. This establishes that host deletion of a
member-owned legacy interface is unsafe; it is not a required behavior.

An explicit `unload()` after host cleanup of the heap fixture and after
manager-only destruction of the member fixture succeeded in the isolated
models. In the isolated unload-with-registry-alias case the plugin-root
`QPointer` became null while the observed heap `GetObject()` QPointer remained
non-null; the retained registry alias was intentionally never dereferenced.
This is evidence against unloading a plugin with registered legacy interfaces.

## 5E.3b structural preparation

`DeviceRegistry` now stores active interface records with a private
`CleanupStrategy`. The public `AddDevice()` path always creates
`HostDelete` records, and every current production registration still reaches
only that strategy. `RetainLegacyPlugin` and `PluginReleaseV2` are present only
as inactive internal enum values; no loader lease, persistent loader, QObject
observation, Messenger connection handle or logical legacy removal exists yet.
The separate path map remains deliberately in place so that the observed
post-`RemoveDevices` path semantics stay unchanged.

## 5E.3c1 successful-loader leases

`PluginLeasePool` is an internal QObject child of the current
`QCoreApplication`. It owns successful `QPluginLoader` instances exclusively
through `std::unique_ptr`; individual loaders have no QObject parent and the
pool never calls `unload()`. The pool is not a global mutable loader list and
has no public application API.

`LoadPlugin::readDevice()` now creates its loader with unique ownership and
transfers it only after instance creation, `Platform_Fabric` cast,
`GetInterface()` and the existing `AddDevice()`/lookup sequence all succeed.
Duplicate names bypass loader creation as before. Missing, wrong-IID and
QObject-only paths do not add a pool lease. The test-only lease-count probe is
compiled only in `PluginLoaderContractTests`.

This changes loader lifetime only. DeviceRegistry still registers every
reachable device as `HostDelete`; no Legacy retain strategy, loader release,
Messenger change or logical removal is active in 5E.3c1.

## 5E.3c2 approved Legacy-V1 logical removal

The isolated 5E.2b baseline established deterministic `0xC0000374` heap
corruption when the host deleted the member-owned fixture interface. The
approved compatibility fix therefore distinguishes registration provenance,
not pointer shape: only the private successful Legacy-V1 loader path records
`RetainLegacyPlugin`; the unchanged public `DataManagementClass::AddDevice()`
continues to record `HostDelete`.

`DeviceRegistry::DeviceRecord` now also retains non-owning `QPointer`s for
the plugin `GetObject()` QObject and its Messenger. On `CloseDevice`,
`RemoveDevices`, or `CloseProjectLogic`, a retained Legacy-V1 record removes
only the active lookup and applies the pre-existing path rule: close/project
remove its path, while remove-all retains it. It disconnects exactly the
Messenger/plugin QObject pair in both directions, performs neither `delete`
nor `unload`, and leaves the `PluginLeasePool` loader, root and interface
resident until application shutdown. `PluginReleaseV2` remains inactive.

`PLUGIN_014..PLUGIN_018` cover member/heap logical removal, path behavior,
pair-specific Messenger disconnection, and a fresh active registration after
logical removal. `PLUGIN_019` reconfirms one `HostDelete` destructor call for
a direct public `AddDevice()` registration. The member and heap fixtures are
test models only; this does not establish a third-party plugin matrix, a
hardware shutdown guarantee, or cleanup of plugin threads/resources before
process end.

Fast verification ran the rebuilt plugin fixtures, `PluginLoaderContractTests`
(21 passed), and `DataManagementCharacterizationTests` (35 passed), plus an
incremental Release compile of the changed loader/registry/facade units. The
direct `ProjectIoFacadeContractTests` rebuild was intentionally not completed:
its full application graph was still regenerating at the local 120-second
tool limit, so no UIIO result is claimed for this slice. The unmodified
ProjectIo adapter remains a required focused checkpoint before the next
Project-IO change; it was not substituted by a full runner.
