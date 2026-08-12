# LoadSave XML characterization (phase 3B)

## Scope and test architecture

`contract/XmlExperimentContractTests` compiles the unchanged qmake application
graph, excluding only `main.cpp`, and creates the real `MainWindow` plus its
`UIDataManagementSetClass`. This is required because both XML classes traverse
their QObject parent chain and cast it to `MainWindow`; no test-only seam or
replacement production header is used.

Fixtures are intentionally tiny, UTF-8, deterministic and free of personal
data, absolute paths and binaries. Relative form paths are copied into a
`QTemporaryDir` during the test so committed fixtures never contain a machine
path. `fixtures/xml/legacy/` additionally contains three structurally faithful
anonymized copies of externally supplied historic experiments and their two
required device-descriptor sidecars. Their source hashes, fixture hashes,
provenance, unversioned-format status and anonymization rules are in
`tests/fixtures/xml/legacy/MANIFEST.md`.

## Function-to-test mapping

| ID | Production entry point / contract | Evidence |
| --- | --- | --- |
| XML_001 | `XmlExperimentReader::read`, `readExperiment` | Minimal `Experiment`; unknown root content and attributes are skipped without error. |
| XML_002 | `readTab`, `LoadForm`, `readDevices`, `readWidgets`, `CreateConnections`, `CreateConnection`, `ReadState` | Existing-format-shaped fixture loads a relative UTF-8 form name and creates a data reference with min/max/alias. |
| XML_003 | `XmlExperimentReader::read`, `errorString` | Wrong root, truncated XML and absent file return `true`; public `errorString()` remains empty. |
| XML_004 | `LoadForm`, `LoadDevice`, optional-element paths | Missing relative form reports one messenger error; missing device reference does not turn the XML parse into an error. |
| XML_005 | `xmlexperimentwriter::write`, `writeExperiment`, `writeTabs`, `writeDevices`, `writeWidgets`, `writeConnections`, `writeState` | Writer success is `false`; UTF-8 form metadata and canonical top-level element order are emitted. |
| XML_006 | `UIDataManagementSetClass::LoadExperiment`, `SaveExperiment` and reader/writer composition | Read → write preserves the semantic experiment root, form identity and data-reference ID. |
| XML_007 | `UIDataManagementSetClass::SaveExperiment`, `LoadExperiment` | Caller retains the same inverted boolean error convention for writable, valid and absent paths. |
| XML_008 | `xmlexperimentwriter::writeFigureWindows` | Real subplot window rows/columns and geometry are serialized. |
| XML_FIG_001 | `XmlExperimentReader::CreateFigureWindow` | A 1x2 `Window` with exactly two `PlotWidgetName` elements creates two plots and preserves their parent/registry mapping, geometry and requested names. |
| XML_FIG_002 | `CreateFigureWindow` | Fewer names than created plots are accepted; named plots are renamed and remaining plots retain their generated names. |
| XML_FIG_003 | `CreateFigureWindow` | No names are accepted; all generated plot names remain. |
| XML_FIG_004 | `CreateFigureWindow`, `read` | A 1x1 window with a second name is rejected safely: `read()` returns `true`, while the first rename and created figure remain. `errorString()` is historically empty. |
| XML_FIG_005 | `CreateFigureWindow`, `MainWindow::CreateSubPlotWindow` | Legacy-compatible 0x0 and 0xN dimensions create an empty registered figure without plot widgets. |
| XML_FIG_006 | `CreateFigureWindow` | Unknown children between valid names are skipped; the valid names retain their input order. |
| XML_FIG_007 | `readExperiment`, `CreateFigureWindows` | With two figures, an excess name in the second leaves the first complete and the second partially renamed, reports parser error and prevents later sections from being processed. |
| XML_FIG_008 | `XmlFigureDimensions::ParseAndValidate`, `CreateFigureWindow` | `0..32` dimensions whose product is at most 256 are accepted before GUI creation; negative, nonnumeric, greater-than-32 and product-over-256 inputs deterministically make `read()` return `true` without creating a figure. |
| XML_LEGACY_001 | `XmlExperimentReader::read`, `readTab`, `LoadForm`, `readDevices`, `LoadDevice` | Smallest external historic fixture: missing UI and deliberately missing historic plugin yield the observed messages and parser error result without a crash; committed XML/device hashes remain unchanged. |
| XML_LEGACY_002 | `XmlExperimentReader::LoadForm`, `LoadDevice` relative-path handling | External fixture with a two-parent relative path: the test recreates its relative hierarchy in `QTemporaryDir`, observes all missing-UI and missing-plugin messages, and records the error return without loading a proprietary dependency. |
| XML_LEGACY_003 | `readExperiment`, figure/window and connection traversal | Largest external fixture reaches the reader without a crash; its observed parser-error and empty facade form/device/container state are asserted. |
| XML_LEGACY_004 | reader/writer composition on external XML | The smallest fixture is read, written to a temporary file, and read again semantically. The first read reports the unresolved external dependency; the written partial state reads successfully with the same empty form/container state and canonical top-level section order. Byte equality is intentionally not asserted. |
| XML_LEGACY_005 | XML reader -> `.LAdev` -> `UIDataManagementSetClass::LoadPlugin` boundary | A temporary copy alone replaces `DevicePlugin` with the runtime-built compatible fixture. The XML and committed descriptor hashes remain unchanged; the device is registered under the anonymized name and missing-form messages remain the only errors. |

`XML_FIG_001..XML_FIG_008` use temporary XML files and the real offscreen
MainWindow/subplot graph. They use per-window object/registry deltas rather
than global plot counters and deterministically close each created figure.
The pre-fix source indexed the discovered plot-widget list without checking
its length; this could access outside the list and the outer `catch (...)`
could not reliably protect against undefined behavior. The dangerous baseline
was not executed in-process.

## Defect candidates / observed legacy behavior

- Reader and writer return `false` on success and `true` on failure.
- `XmlExperimentReader::errorString()` always returns an empty `QString`.
- Unknown elements and attributes are silently ignored.
- A missing form reports a messenger error but does not make `read` fail. A
  missing device path is silently ignored because its error block is commented
  out; the relative-device `FilenameT` also shadows the outer variable.
- Reader and writer require the exact `UIDataManagementSetClass →
  MainWindow(objectName=LabAnalyser)` parent hierarchy.
- **Approved 5F.2 safety change:** before indexing a discovered plot, the
  reader rejects an extra `PlotWidgetName` with the deterministic parser text
  `Figure window contains more PlotWidgetName elements than created PlotWidgets.`
  and returns from the window. This preserves the historic `true == error`
  reader convention. Already-created figures, geometry, registrations and
  earlier renames remain (no rollback); fewer names remain valid and leave
  generated names in place.
- **Approved package-C safety change:** `XmlFigureDimensions` validates rows
  and columns before `CreateSubPlotWindow()`. Both values must be integral and
  in `0..32`; their product must not exceed 256. Rejection raises exactly
  `Figure window Rows and Cols must be integers between 0 and 32.` or
  `Figure window Rows x Cols must not exceed 256.`, retains the historical
  `true == error` reader convention, and creates no figure, plot or registry
  object. Zero-sized legacy grids remain accepted.

## Remaining untestable paths

- A successful plugin load requires a compatible binary plugin; none is
  committed or manufactured for characterization.
- Deterministic OS-level permission-denied tests are not portable on Windows.
  Missing-file and nonexistent-parent paths cover stable open failures.
- The external corpus establishes XML structure, failure handling and the
  replaceable XML-to-plugin boundary only. It does not establish compatibility
  of the historic proprietary plugin, its network transport, or `CustomData`.
- The fixture set intentionally contains no historical `.ui` files. Their
  absence is verified as an expected external dependency rather than hidden by
  synthetic substitutes. Full historical form/widget restoration therefore
  remains unverified.
- Geometry, position and allocation failures after an accepted grid, plus
  rollback of an already-created figure after a later malformed child, remain
  outside this focused safety fix.

## Executed evidence

All commands below were run on 2026-08-03 with MSYS2 MINGW64, GCC 15.2 and Qt
6.9.2.

| Scope | Command | Result |
| --- | --- | --- |
| Release application | `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-msys2.ps1 -Configuration release -BuildDir build\xml-3b-release -Jobs 4` | PASS |
| Debug application | `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\build-msys2.ps1 -Configuration debug -BuildDir build\xml-3b-debug -Jobs 4` | PASS |
| All local tests | `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-tests-msys2.ps1 -Clean -Jobs 4` | PASS; 3 projects. XML suite: 10 Qt Test checks, 0 failures. |
| XML coverage build/run | qmake `XmlExperimentContractTests.pro` with `QMAKE_CXXFLAGS+=--coverage QMAKE_LFLAGS+=--coverage`, then `mingw32-make -j1` and `XmlExperimentContractTests.exe -txt` | PASS. The earlier `uic.exe`/`rcc.exe` `-1073741511` event was later traced to a MiKTeX Qt DLL preceding MSYS2 on `PATH`, not a transient build failure. |
| Legacy XML smoke suite | qmake build in `build\xml-legacy-smoke`, then `XmlExperimentContractTests.exe -txt` with `QT_QPA_PLATFORM=offscreen` and runtime plugin root | PASS on 2026-08-10: 15 passed, 0 failed. |
| Central test runner | `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-tests-msys2.ps1 -Jobs 4` | PASS on 2026-08-10 (exit code 0): all 12 registered projects passed; the runner rebuilt the runtime plugin fixtures first. |

## Coverage by production file

GCC/gcov 15.2 was invoked with `gcov.exe -b -c -o release` for each listed
source after the coverage test execution. These are per-file results for this
XML test executable, **not** repository-wide coverage and not a quality gate.

| Production file | Lines | Branches executed | Branches taken at least once | Calls executed |
| --- | ---: | ---: | ---: | ---: |
| `LoadSave/xmlexperimentreader.cpp` | 71.90% (151/210) | 73.44% (376/512) | 42.38% (217/512) | 55.80% (207/371) |
| `LoadSave/xmlexperimentwriter.cpp` | 94.64% (106/112) | 90.18% (202/224) | 48.66% (109/224) | 76.00% (133/175) |
| `LoadSave/loadplugin.cpp` | 0.00% (0/30) | 0.00% (0/54) | 0.00% (0/54) | 0.00% (0/47) |
| `DataManagement/UIDataManagementSetClass.cpp` (XML caller) | 38.64% (34/88) | 34.41% (64/186) | 17.74% (33/186) | 32.21% (48/149) |

## Phase 4G.2b reader-construction delegation

On 2026-08-10 `UIDataManagementSetClass::LoadExperiment()` was reduced only
to delegation of reader construction/execution to the private
`ProjectIoCoordinator`. `XmlExperimentReader` is unchanged. The coordinator
constructs it with the same facade, messenger and QObject parent as the prior
inline call: `(&uiManager, uiManager.GetMessenger(), &uiManager)`.

`XML_001..XML_008` and `XML_LEGACY_001..XML_LEGACY_005` remained green through
the central instrumented 12-target run. This preserves the documented reader
order (forms, devices, figures, widgets, connections, then state), CWD change,
relative/absolute `.LAdev` resolution, return values and existing error
messages, including the expected missing UI/plugin integration boundaries.
The five committed legacy fixture SHA-256 values were checked before and after
the run and did not change; no XML fixture was migrated or normalized.

## Phase 4G.2c writer-construction delegation

On 2026-08-10 `UIDataManagementSetClass::SaveExperiment()` was reduced only
to delegation of writer construction/execution to the private
`ProjectIoCoordinator`. `xmlexperimentwriter` is unchanged. The coordinator
uses the identical historic arguments: `(&uiManager,
uiManager.GetMessengerRef(), uiManager)`.

The unchanged XML suite covers `XML_006` semantic read/write, `XML_007`
inverted save/load returns and `XML_008` figure persistence; `XML_LEGACY_004`
preserves the temporary-only legacy read/write/read contract. The full XML,
UIIO and MainWindow vectors passed in the central runner. Thus no byte or
fixture expectation was changed: section order, attributes, UTF-8, paths,
formatting, overwrite and invalid-path semantics remain those of the existing
writer. The five legacy fixture hashes match their manifest before and after
the run.
