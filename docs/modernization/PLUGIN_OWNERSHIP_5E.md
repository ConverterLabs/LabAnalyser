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
