# Plugin contracts — phase 3G

Date: 2026-08-04. Scope: `LoadSave/loadplugin.*`,
`plugins/platforminterface.h`, `plugins/InterfaceDataType.*`, and the narrow
`UIDataManagementSetClass::LoadPlugin` caller convention.

## Original defect and approved safety change

The original code deterministically dereferenced the null result of
`qobject_cast<Platform_Fabric *>` after a successfully loaded incompatible
plugin. The baseline crash was demonstrated only in an isolated process so it
could not damage the test runner; it is a defect proof, not a desired contract.

On 2026-08-04 the owner explicitly approved a security behavior change. The
only production change is a null check in `LoadPlugin::readDevice()`: on a null
cast it sends `the xml-Device: <path> is incompatible with Platform_Fabric.`
through the existing Messenger and returns. `GetInterface()` is not called,
no device is registered, and `GetNewDevice()` remains null. The outer
`UIDataManagementSetClass::LoadPlugin()` convention remains `true` for that
failure path. This is an approved safety bugfix, not behavior-neutral
refactoring.

The compatible-plugin flow is unchanged: it retains IID
`org.qt-project.Qt.Examples.EchoInterface`, calls `GetInterface()` with the
Messenger, registers the returned device under the XML device name, and makes
it available through `GetNewDevice()`.

## Runtime fixtures and regression tests

Three small plugins are built at test runtime with the same MSYS2/MINGW64/Qt
configuration as the host. Their DLLs live only below ignored `build/` paths:

- `CompatiblePlugin`: existing Platform_Fabric IID and interface.
- `WrongIidPlugin`: deliberately different IID and no Platform_Fabric cast.
- `QObjectOnlyPlugin`: loadable Qt QObject plugin without Platform_Fabric.

`PluginFixtureVerifier` independently loads each DLL through `QPluginLoader`.
It proved non-null instances, the advertised IIDs, expected cast results,
successful unloads and clean process exit. The runner builds these fixtures
first, sets `LABANALYSER_TEST_PLUGIN_ROOT` portably, then runs
`PluginLoaderContractTests`. Missing that variable is a clear test failure.

| ID | Verified contract |
| --- | --- |
| `PLUGIN_001` | Compatible plugin loads, receives the expected Messenger, calls `GetInterface()` once, registers the expected device and exposes it through `GetNewDevice()`. |
| `PLUGIN_002` | Wrong-IID plugin is safely rejected with exactly one relevant Messenger error; no `GetInterface()`, registration or device. |
| `PLUGIN_003` | QObject-only plugin has the same safe rejection result. |
| `PLUGIN_004` | Missing/non-loadable DLL follows the existing error and no-state convention. |
| `PLUGIN_005` | Repeated same-name loading uses state differences because plugin instances persist process-wide; the first load increments the fixture counter once and the observed duplicate path does not add a registration or call. |
| `PLUGIN_006` | The test-only UI caller seam verifies `LoadPlugin()` returns `false` for a valid plugin and `true` for both incompatible fixture types. |
| `PLUGIN_007` | Registered interface/device remain usable after the local `LoadPlugin` object is destroyed; the process terminates cleanly. |

## Verification, coverage and limits

`PLUGIN_001` through `PLUGIN_007`, the complete post-clean test runner, and
fresh Release and Debug production builds all completed successfully. The
initial combined `-Clean` runner was stopped solely by the hard 300-second
execution limit after it had cleaned the build tree; no compiler or test error
had occurred. The immediately following full run finished that same cleaned
tree successfully. It is not claimed that the single combined command finished
within 300 seconds.

Instrumented `LoadSave/loadplugin.cpp` evidence is file-specific, not project
coverage: 87.50% lines (28/32), 90.62% executed branches (58/64), 50.00%
branches taken at least once (32/64), and 76.79% calls (43/56). No coverage
threshold was enabled.

`DataManagementSetClassTestSeam.cpp` is compiled solely into the plugin
contract target, never `LabAnalyser.pro`; it supplies only the registry and
Messenger behavior needed because the real UI class would pull the unrelated
GUI/plot graph. Thus the UI-return assertion is a narrow caller-convention
seam, not proof of full GUI integration. The public plugin headers,
`EchoInterface_iid`, and ABI-relevant signatures were diffed unchanged. This
establishes source-level ABI/API preservation for those headers, not binary
compatibility with arbitrary third-party plugins.

Unreached or non-deterministic limits remain: missing XML attributes and parser
errors, null Messenger, null `GetInterface()` return, OS permission/loader
failures, mixed Debug/Release or architecture-incompatible third-party DLLs,
and unloading while externally retained plugin objects are alive. No generated
DLL, executable, object, build, gcov or coverage artifact is versioned.
