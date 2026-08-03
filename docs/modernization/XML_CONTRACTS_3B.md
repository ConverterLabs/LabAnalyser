# LoadSave XML characterization (phase 3B)

## Scope and test architecture

`contract/XmlExperimentContractTests` compiles the unchanged qmake application
graph, excluding only `main.cpp`, and creates the real `MainWindow` plus its
`UIDataManagementSetClass`. This is required because both XML classes traverse
their QObject parent chain and cast it to `MainWindow`; no test-only seam or
replacement production header is used.

Fixtures are intentionally tiny, UTF-8, deterministic and free of personal
data, absolute paths and binaries. There were no checked-in `.LAexp` or other
experiment XML examples to import or classify as non-sensitive. Relative form
paths are copied into a `QTemporaryDir` during the test so committed fixtures
never contain a machine path.

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

`CreateFigureWindows` and `CreateFigureWindow` are inspected but not executed
with arbitrary fixture geometry: the implementation indexes the discovered
plot-widget list without checking its length. A malformed or incompatible
`PlotWidgetName` count can therefore access outside the list; this is a defect
candidate, not a required behavior.

## Defect candidates / observed legacy behavior

- Reader and writer return `false` on success and `true` on failure.
- `XmlExperimentReader::errorString()` always returns an empty `QString`.
- Unknown elements and attributes are silently ignored.
- A missing form reports a messenger error but does not make `read` fail. A
  missing device path is silently ignored because its error block is commented
  out; the relative-device `FilenameT` also shadows the outer variable.
- Reader and writer require the exact `UIDataManagementSetClass →
  MainWindow(objectName=LabAnalyser)` parent hierarchy.
- `CreateFigureWindow` has no bounds check while consuming `PlotWidgetName`.

## Remaining untestable paths

- A successful plugin load requires a compatible binary plugin; none is
  committed or manufactured for characterization.
- Deterministic OS-level permission-denied tests are not portable on Windows.
  Missing-file and nonexistent-parent paths cover stable open failures.
- Loading actual historic user experiment files is unverified because no safe,
  checked-in example exists. `legacy-complete.xml` is an implementation-derived
  compatibility vector, not an external-file corpus.

## Executed evidence

All commands below were run on 2026-08-03 with MSYS2 MINGW64, GCC 15.2 and Qt
6.9.2.

| Scope | Command | Result |
| --- | --- | --- |
| Release application | `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\build-msys2.ps1 -Configuration release -BuildDir build\xml-3b-release -Jobs 4` | PASS |
| Debug application | `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\build-msys2.ps1 -Configuration debug -BuildDir build\xml-3b-debug -Jobs 4` | PASS |
| All local tests | `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-tests-msys2.ps1 -Clean -Jobs 4` | PASS; 3 projects. XML suite: 10 Qt Test checks, 0 failures. |
| XML coverage build/run | qmake `XmlExperimentContractTests.pro` with `QMAKE_CXXFLAGS+=--coverage QMAKE_LFLAGS+=--coverage`, then `mingw32-make -j1` and `XmlExperimentContractTests.exe -txt` | PASS; the first parallel build hit transient `uic.exe`/`rcc.exe` Windows error `-1073741511`, serial retries completed. |

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
