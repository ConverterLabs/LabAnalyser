# Milestone 3 characterization report

**Audit date:** 2026-08-04  
**Scope:** Characterization phases 3A through 3J. This report consolidates the
already recorded evidence; it does not add a build or test result.

## Result

**Charakterisierung der identifizierten kritischen externen Verträge
abgeschlossen; vollständiges Funktionsregister und Assurance-Gates offen.**

The planned phase scopes have reproducible characterization evidence in the
qmake/MSYS2 test runner. All reported successful Release, Debug, clean-tree
follow-up and instrumented runs are historical evidence from the respective
phase documents. Several combined `-Clean` commands reached the external
300-second execution limit only after cleaning/rebuilding; the documented
immediately following non-clean invocation completed the same tree. This is
split verification, not a claim that every single combined clean command
finished within that limit.

| Phase | Evidence and characterized contracts | Result and material limits |
| --- | --- | --- |
| 3A DataManagement | `DM_001..DM_010`; containers, aliases/min-max, mappings, ownership cleanup and Messenger order/payload. | Core manager boundary characterized. Unsafe absent-entry/raw-pointer paths and unimplemented `LoadForms` remain excluded. |
| 3B Experiment XML | `XML_001..XML_008`; read/write, UTF-8 fixtures, semantic round trip, UI forwarding and subplot serialization. | Inverted result convention, ignored unknown content and empty `errorString()` are recorded. No safe historic user-file corpus or deterministic permission failure. |
| 3C Parameter XML | `PARAM_001..PARAM_009`; values/types, malformed input, repeated load and export/import route. | Metadata persistence is absent; coercion/range behavior is characterized as legacy risk. |
| 3D MAT | `MAT_001..MAT_008`; MAT v5/libmatio public readback, scalars/vectors, UTF-8, errors and UI convention. | Allocation-failure and unavailable matrix/rank-3 source paths were not fault-injected. |
| 3E HDF5 | `HDF5_001..HDF5_006`; public HighFive readback, hierarchy, scalar/vector/string data, overwrite and error convention. | Null manager/vector, allocation/dataset faults and real UI construction remain excluded. |
| 3F TCP | `TCP_001..TCP_007`; loopback framing, commands, fragmentation, disconnect and repeat-client behavior. | Native framing is a contract; short/zero/oversized frames and OS network faults remain unsafe or unverified. |
| 3G Plugins | `PLUGIN_001..PLUGIN_007`; runtime-built compatible, wrong-IID and QObject-only plugins. | The only approved behavior change: incompatible loadable plugins are safely rejected before `GetInterface()`/registration. IID and public headers remained unchanged by the phase. |
| 3H DropWidgets | `DW_001..DW_017`; adapters, indicators, IDs, loader fixtures, safe bindings and table behavior offscreen. | Source-backed drag execution, visual pixels and dangerous null/index paths remain out of process scope. |
| 3I MainWindow | `GUI_001..GUI_021`; real offscreen window, forms/docks, action cancellation, safe trees, subplot/tray/output and mutable context actions. | CLI, native desktop/dialog/tray behavior, external files and unsafe selections remain excluded; plot semantics belong to 3J. |
| 3J Plot/FFT | `PLOT_001..PLOT_006`, `FFT_001..FFT_008`, plus the separate PlotMeasurements suite. | Data/FFT contracts are characterized. Cursor, context, history and quality criteria still need safe tests; rendering, gestures and dangerous FFT paths remain excluded. |

## Coverage evidence

Coverage was collected per production file with MSYS2 GCC/gcov. It is evidence
for executed source, not aggregate repository coverage and not a gate. The
stronger recorded examples are `Export2Mat.cpp` 98.44% lines,
`export2highfive.cpp` 100.00%, `loadplugin.cpp` 87.50%,
`RemoteControlServer.cpp` 87.41% and `FFTPlotWidget.cpp` 80.00%. Low or
partial evidence remains important: `PlotWidget.cpp` is 42.89% lines and the
broader GUI manager has only boundary-specific coverage. The detailed line,
branch-executed, branch-taken and call figures remain in the phase contract
documents rather than being conflated into a project total.

## Approved behavioral change

Phase 3G alone contains an approved security bugfix. The former deterministic
null dereference after loading a non-`Platform_Fabric` Qt plugin was proven in
an isolated process. `LoadPlugin::readDevice()` now rejects a null cast through
the existing Messenger and returns without calling `GetInterface()` or
registering a device. Compatible plugin behavior, the IID
`org.qt-project.Qt.Examples.EchoInterface`, and public interface headers were
verified unchanged at source level. This is not a claim of compatibility with
every independently built third-party binary.

## Behavior-inventory audit

The inventory contains phase-specific mappings for every reviewed subsystem.
It does **not** yet meet the stricter end-state requirement of a separate row
for every public/protected production function. The open inventory work is
explicitly recorded in its new “Milestone 3 completion audit” section:

- `main.cpp`, CLI, native-dialog and desktop-tray execution are excluded from
  the test-owned application process.
- `UIDataManagementSetClass` has only boundary mappings; `LoadForms` is
  declared but has no repository definition, and unsafe sender/entry paths are
  not contracts.
- Plugin loader/platform matrices, OS loader failures and externally retained
  plugin objects are outside the runtime-fixture scope.
- Source-backed tree drops, full interactive context paths, visual pixel
  rendering and dangerous null/index paths remain excluded.
- PlotWidget still lacks safe cursor, context-menu, history-limit and
  quality-criteria characterization. `ClearAllGraphs` and X-axis mapping also
  lack a safe direct contract.

These are gaps or exclusions, not successful evidence. The primary record is
`BEHAVIOR_INVENTORY.md`; individual phase documents retain function-level
details and test vectors.

## Open quality gates

1. No aggregate coverage report or enforceable coverage gate exists; the
   90% line / 80% branch target has not been demonstrated repository-wide.
2. CTest/CI, sanitizer, static-analysis, dependency scanning and secret
   scanning are not configured.
3. Only the documented MSYS2 MINGW64 environment is evidenced; Linux and
   broader deployment environments remain unverified.
4. Existing compiler warnings have not been made clean as a separate work
   item.
5. Several real-world compatibility inputs are unavailable or deliberately
   isolated: historic experiment corpus, third-party plugin binary matrix,
   OS permission faults and desktop integration.
6. The complete function-by-function inventory gate remains open, including
   the explicitly safe PlotWidget paths listed above.

## Refactoring disposition

The characterized contracts provide a substantial baseline for focused work,
but do not authorize broad GUI or PlotWidget refactoring. Any production change
must retain the mapped vectors and first close the relevant open function or
error path, unless an explicit behavior change is approved.
