# DropWidget contracts â€” Phase 3H

## Scope and execution

Phase 3H characterizes the non-vendored DropWidget adapters, the UI loader,
tree drag source and the narrow DataManagement binding boundary. Production
sources and the vendored `DropWidgets/Plots/qcustomplot` code were not changed.
`DropWidgetAdapterTests` is registered in `tests/run-tests-msys2.ps1`; the
runner temporarily sets `QT_QPA_PLATFORM=offscreen` and restores the caller's
environment on exit. Fixtures use `QFINDTESTDATA` from the repository-relative
test source location and contain no machine paths.

## Internal adapter modernization checkpoint (2026-08-12)

The established `DW_001..DW_018` contracts now protect a structural migration
of the non-plot adapter implementation. The historical Designer-facing names
(`QSliderD`, `QLineEditD`, `QComboBoxD`, and peers) remain the compatibility
surface used by legacy `.ui` documents, but shared implementation is no longer
duplicated in each adapter:

- `DropWidgetUpdate` retains the historic programmatic-update signal-blocking
  convention;
- `DropWidgetBinding` centralizes the existing direct request/value signal
  connections without deduplication, name migration or lifetime changes;
- `DropWidgetConnectionMenu` centralizes the existing highlight/remove menu
  construction while preserving each adapter's separator, standard-line-edit
  menu, delete-on-close and dirty-state variants;
- `DropWidgetDataAccess` centralizes numeric admission and LED bit/XML value
  conversions while each adapter retains its own range, rounding and visual
  behavior;
- `DropWidgetsUiLoader` maps the same Designer class names through a
  data-driven factory and retains the base-loader fallback and PlotWidget
  special path;
- `DropWidgetTableCells` owns only historical table-cell construction,
  preserving the original object-name formula, type choice and manager binding.

No Qt public/metaobject surface, XML attribute, manager protocol, UI fixture or
legacy experiment fixture changed. The focused CMake checkpoint built the
application and ran `DropWidgetAdapterTests`, `MainWindowIntegrationTests` and
the complete `XmlExperimentContractTests` green (3/3); the reconfigured
existing qmake adapter target also rebuilt and passed offscreen. All five
fixture SHA-256 values matched `MANIFEST.md`, `git ls-files --eol` retained
`-text`/LF fixture bytes, and the configured sensitivity/artifact scan was
clean. The byte-protected files are never rewritten by this migration.

The 2026-08-04 instrumented suite passed with 15 Qt Test checks, zero failures
and exit code 0. The full runner passed after the accepted split verification:
the single `-Clean` invocation reached the hard 300-second execution limit only
after clearing and rebuilding the tree, with no observed compiler or test
failure; the immediately following run without `-Clean` completed that same
tree successfully (exit code 0). Fresh Release and Debug application builds
also passed (exit code 0).

## Fixtures and test-only seams

Fixtures under `tests/fixtures/gui/` are `standard-widgets.ui`,
`dropwidgets-nested.ui`, `unsupported-widget.ui` and `malformed.ui`.

`TestMainWindowSeam.cpp` is linked only into the DropWidget test target. It
provides the constructor-time `MainWindow`/manager host and mirrors only the
exercised `DataManagementSetClass::SetData` and `SendNewValue` forwarding
paths; it avoids linking the out-of-scope PlotWidget graph and is not evidence
of complete MainWindow integration. `PlotWidgetLinkSeam.cpp` supplies only the
otherwise uncalled loader constructor symbols; no PlotWidget instance is
created. Neither seam is included by `LabAnalyser.pro`.

## Characterized contracts

| IDs | Contract |
| --- | --- |
| DW_001 | Construction, QObject parenting, defaults and `CreateID`/`CreateIDs` across all listed non-plot adapters. |
| DW_002 | Numeric transfer, range behavior and signal suppression for numeric adapters. |
| DW_003 | String/Unicode, selection and list adapter transfer. |
| DW_004 | Bit adapters and their XML attributes. |
| DW_005 | Current XML-stub returns plus label/table special cases. |
| DW_006 | UI-loader replacement of standard widgets; names, properties, parenting and accept-drops state. |
| DW_007 | LabAnalyser-specific widget mapping and nested UI hierarchy. |
| DW_008 | Missing, malformed and unsupported UI fixtures. |
| DW_009 | Tree ID text serialization and rejection of a foreign direct drag event. |
| DW_010 | Repeated `ConnectToID` update requests and table XML row save/remove behavior. |
| DW_011 | Real `DataManagementClass`/`MessengerClass` forwarding: manager `set` updates a bound line edit; `ConnectToID` emits ordered `get`, then user edit emits ordered `set` with ID/payload. |
| DW_012 | Two direct identical signal connections produce two manager messages; after mapping removal those connections cannot resolve an ID and produce no further manager message. |
| DW_013 | `QTableWidgeD::LoadFromXML` invokes delayed `CreateRow` for typed numeric containers, binds generated cells, forwards manager updates, and clears through the real `Clear Table` action. |

## File-specific gcov evidence

These are file-level values from the instrumented DropWidget suite, not
project-wide coverage and not a threshold. “Branches executed” means branch
sites reached; “taken” means branches taken at least once.

| Production file | Lines | Branches executed | Branches taken | Calls |
| --- | ---: | ---: | ---: | ---: |
| `DropWidgets/CreateID.cpp` | 97.56% | 91.18% | 57.35% | 75.93% |
| `DropWidgets/DropWidgetsUiLoader.cpp` | 64.91% | 46.48% | 28.87% | 37.27% |
| `TreeWidgetCustomDrop.cpp` | 100.00% | 94.44% | 58.33% | 75.76% |
| `DropWidgets/QBLed.cpp` | 11.58% | 10.81% | 5.86% | 10.00% |
| `DropWidgets/QCheckBox.cpp` | 21.31% | 13.14% | 7.30% | 14.07% |
| `DropWidgets/QComboBox.cpp` | 30.77% | 23.30% | 12.14% | 20.73% |
| `DropWidgets/QDoubleSpinBox.cpp` | 16.67% | 9.28% | 4.64% | 11.36% |
| `DropWidgets/QLCDNumber.cpp` | 14.67% | 10.11% | 5.06% | 11.54% |
| `DropWidgets/QLabel.cpp` | 33.78% | 21.43% | 10.71% | 24.76% |
| `DropWidgets/QLed.cpp` | 11.22% | 10.71% | 5.80% | 9.87% |
| `DropWidgets/QLineEdit.cpp` | 25.81% | 15.38% | 7.69% | 15.26% |
| `DropWidgets/QListView.cpp` | 19.19% | 15.97% | 7.98% | 12.80% |
| `DropWidgets/QProgressBar.cpp` | 17.57% | 10.47% | 5.23% | 12.24% |
| `DropWidgets/QPushButton.cpp` | 7.69% | 7.02% | 3.51% | 5.88% |
| `DropWidgets/QSlider.cpp` | 6.54% | 8.70% | 4.35% | 7.46% |
| `DropWidgets/QSpinBox.cpp` | 20.00% | 10.31% | 5.15% | 12.29% |
| `DropWidgets/QTableWidgeD.cpp` | 50.94% | 50.79% | 27.78% | 41.29% |
| `DropWidgets/QTSLed.cpp` | 11.46% | 10.71% | 5.80% | 9.95% |
| `CustomWidgets/QBLedIndicator.cpp` | 15.09% | 20.93% | 10.47% | 14.43% |
| `CustomWidgets/QLedIndicator.cpp` | 12.50% | 5.88% | 2.94% | 8.05% |
| `CustomWidgets/QTSLedIndicator.cpp` | 9.88% | 7.84% | 3.92% | 10.00% |
| `DataManagement/DataManagementClass.cpp` | 22.18% | 34.43% | 19.13% | 26.39% |
| `DataManagement/DataMessengerClass.cpp` | 45.61% | 53.01% | 27.11% | 34.95% |
| `plugins/InterfaceDataType.cpp` | 27.93% | 22.20% | 11.93% | 19.63% |

## Defect candidates and limits

- `TreeWidgetCustomDrop` advertises `text/uri-list`, but currently places only
  text in the MIME object.
- Repeated direct Qt signal connections duplicate manager messages.
- `QTableWidgeD::CreateRow` makes table geometry for a declared container type,
  but creates no editor until the container has a corresponding typed
  `InterfaceData` value.
- Unknown widget classes are warned about and omitted while the surrounding UI
  still loads.

Source-dependent real `dropEvent` paths cannot be reproduced by directly
constructed Qt events because Qt does not expose a way to set their drag
source. Missing-manager/null-ID paths may dereference null pointers and were
not run in-process. Interactive context actions, actual `QDrag::exec`, complete
MainWindow workflows and all PlotWidget/qcustomplot behavior remain excluded
for later phases. No candidate was repaired in this phase.

## Phase 3H.5a: coverage and contract-gap analysis

This is a source/inventory/gcov analysis only; no test, build or production
source was changed for it. “Direct” means a named DW test calls the method or
asserts its result. “Indirect” means it runs as construction, Qt dispatch or a
called implementation detail. “Not run” means no Phase-3H test reaches a
meaningful path. The risk column is the risk of refactoring without additional
characterization, not a claim that the current behavior is defective.

| Class / production functions (public or protected) | DW evidence / execution | Relevant unexecuted paths and classification | Refactoring risk |
| --- | --- | --- | --- |
| `VariantDropWidget`: ctor/dtor; pure `SetVariantData`, `GetVariantData`, `LoadFromXML`, `SaveToXML`, `ConnectToID` | ctor indirect DW_001; virtual surface exercised through concrete adapters DW_001–DW_013 | Base has no implementation. Concrete completeness is detailed below. | Low for the empty base; high if virtual/interface shape changes. |
| `CreateID`: `GetMainWindow`, `CreateID`, `CreateIDs` | `CreateID`/`CreateIDs` direct DW_001 and DW_009; `GetMainWindow` indirect throughout | Empty/missing top-level MainWindow and out-of-range top-level-widget scan are dangerous null/index paths; exclude from process tests. MainWindow dependency → 3I. | High: global top-level lookup and unchecked indexing. |
| `DropWidgetsUiLoader`: ctor, `createWidget` | ctor indirect; `createWidget` direct via DW_006–DW_008 | Every supported adapter mapping is not independently enumerated; PlotWidget branch requires 3J. Unknown-class partial-load behavior is covered. | Medium: mapping/object-name regressions; high for Plot branch. |
| `TreeWidgetCustomDrop`: ctor, protected `mimeTypes`, `mimeData`, `startDrag`; private `performDrag` | `mimeTypes`/`mimeData` direct DW_009; ctor indirect | `startDrag`/`performDrag` not run: real `QDrag::exec` is interactive. MIME/source mismatch covered as defect candidate. MainWindow not required, but real drag lifecycle remains 3I. | Medium: external MIME bytes and drag lifetime. |
| `QBLed`: ctor, drag enter/move/drop, bit accessors, five `VariantDropWidget` methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/bit/XML/value paths direct DW_001, DW_004; remaining value dispatch indirect | All drag paths, context menu and removal not run; drop dereferences tree/manager/ID. MainWindow dependency → 3I; visual indicator state → paint-only. | High for drop/remove; medium for isolated conversion. |
| `QCheckBoxD`: ctor, drag enter/move/drop, bit accessors, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/bit/value/XML direct DW_001, DW_002, DW_004; signal behavior direct DW_002 | Valid tree drop, context/removal and bit-level manager forwarding not run. Null tree selection/manager paths are dangerous; 3I. | High for event/manager behavior; medium for adapter conversion. |
| `QComboBoxD`: ctor, drag enter/move/drop, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/value transfer direct DW_001, DW_003; repeat `ConnectToID` direct DW_010 | Valid/invalid manager-backed drop, context/removal and selection user-to-manager payload are not run; 3I. | High for duplicate connections and selection forwarding. |
| `QDoubleSpinBoxD`: ctor, drag enter/move/drop, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/numeric conversion/range direct DW_001, DW_002 | Min/max populated by real dropped manager ID, drag, context/removal and manager signal ordering not run; 3I. | Medium isolated; high at manager boundary. |
| `QLCDNumberD`: ctor, drag enter/move/drop, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/value/XML stubs direct or indirect DW_001, DW_002, DW_005 | Drop, read-only display update by manager, context/removal and all error branches not run; 3I. | Medium; UI display semantics are externally visible. |
| `QLabelD`: ctor, drag enter/move/drop, five virtual methods, `setText`, `contextMenu`, `EditText`, `RemoveUserText` | ctor/value/override text and `RemoveUserText` direct DW_001, DW_003, DW_005 | Drag, menu and modal `EditText` excluded; modal interaction → 3I. | Medium; user-text preservation needs regression coverage. |
| `QLed`: ctor, drag enter/move/drop, bit accessors, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/bit/value/XML direct DW_001, DW_004 | Valid drops, context/removal and visual state rendering not run; unsafe tree/manager paths → 3I; paint → visual. | High for drops; low for bit accessor. |
| `QLineEditD`: ctor, drag enter/move/drop, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/string conversion direct DW_001, DW_003; foreign `dragEnterEvent` direct DW_009; real `ConnectToID`, ordered get/set and messenger forwarding direct DW_011 | Valid source-dependent drop, context/removal, absent ID/manager and conversion errors not run; 3I / dangerous null paths. | High: editing signal and mapping behavior are externally visible. |
| `QListViewD`: ctor, drag enter/move/drop, five virtual methods, `contextMenu`, `DeleteEntry`, `DeleteAllEntries`, `RequestUpdate`, `NewEntry` | ctor/list transfer direct DW_001, DW_003 | All DnD, menu mutation, entry deletion and user-to-manager forwarding not run; 3I. | High: model ownership and multi-selection behavior. |
| `QProgressBarD`: ctor, drag enter/move/drop, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/numeric value path direct DW_001, DW_002 | Real manager min/max and drop, context/removal and invalid IDs not run; 3I. | Medium. |
| `QPushButtonD`: ctor, drag enter/move/drop, five virtual methods, `StartTimeOut`, `TimeOut`, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/basic adapter path direct DW_001, DW_005 | Timer start/timeout, button activation, DnD, context/removal and manager effect not run; timer/event flow → 3I. | High: timeout has user-visible action semantics. |
| `QSliderD`: ctor, drag enter/move/drop, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/numeric adapter path direct DW_001, DW_002 and DW_016; DW_016 also covers connected editable `QString`, `QStringList` and `GuiSelection` rejection after the approved correctness fix. | Actual dropped min/max, slider user value signal, context/removal and errors not run; 3I. A numeric `MinMax.second == MinMax.first` scaling division remains a separate unsafe defect candidate. | High: rounding/range + user signal contract. |
| `QSpinBoxD`: ctor, drag enter/move/drop, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/numeric adapter path direct DW_001, DW_002 | Actual dropped min/max, user signal forwarding, context/removal and errors not run; 3I. | High: rounding/range + manager contract. |
| `QTableWidgeD`: ctor, drag enter/move/drop, five virtual methods, `contextMenu`, `RemoveConnection`, `RemoveSelectedRows`, `customHeaderMenuRequested`, private `CreateRow`, `RequestUpdate` | ctor/XML save/remove direct DW_005, DW_010; delayed private `CreateRow`, generated-cell `ConnectToID`, manager-to-cell update and `contextMenu` clear action direct/indirect DW_013 | `dropEvent`, drag acceptance, header menu, multi-row/mixed bool rows, duplicate IDs, repeated removal and missing manager/ID not run. `CreateRow` missing typed value is observed but only safe geometry path covered. 3I; null paths dangerous. | High: ownership, dynamically generated widgets and delayed timer behavior. |
| `QTSLed`: ctor, drag enter/move/drop, bit accessors, five virtual methods, `contextMenu`, `RemoveConnection`, `RequestUpdate` | ctor/bit/XML/value paths direct DW_001, DW_004 | Valid drops, context/removal, manager updates and painting not run; 3I / visual. | High for state/drop mapping. |
| `QBLedIndicator`: ctor, color setters/getters, `SetState`, protected `paintEvent`, `resizeEvent`, `TimeOut` | construction/state indirectly reached through `QBLed`; no direct indicator API assertion | Colors, timer blink transitions, paint and resize are untested. Purely visual paint is excluded; timer state can be isolated now. | Medium for timer/state; visual rendering needs a separate visual contract. |
| `QLedIndicator`: ctor, color setters/getters, `SetState`, protected `paintEvent`, `resizeEvent` | construction/state indirect through `QLed`; no direct indicator API assertion | Color/state getters, paint and resize are not run; paint-only. | Low for trivial setters, medium for rendering state. |
| `QTSLedIndicator`: ctor, color setters/getters, `SetState`, `GetColor`, `SetColor`, `GreenColor`, `RedColor`, `YellowColor`, protected `paintEvent`, `resizeEvent` | construction/state indirect through `QTSLed`; no direct color API assertion | All named color helpers and paint/resize are not run; helpers are isolatable now, paint visual. | Medium: color-name/state compatibility. |

### Prioritized additional-test plan

1. **3H.5b, isolated and safe:** direct tests for indicator color/state/timer
   helpers; adapter `ConnectToID`/range/signal paths for numeric, selection and
   list widgets using the existing narrow manager seam. This raises low-cost
   coverage without entering MainWindow.
2. **3I, MainWindow/drag integration:** a real or deliberately narrower
   MainWindow host for source-backed `QTreeWidget` drops, accepted/rejected
   types, repeated mappings, removal, context actions, list mutations,
   slider/spin/button user signals and multi-row table ownership.
3. **3J, plot boundary:** characterize only the loader's PlotWidget mapping and
   PlotWidget behavior with the real plot graph; do not expand this suite with
   qcustomplot code.
4. **Separate safety work, only with approval:** isolate missing top-level
   MainWindow, empty tree selection and missing manager/ID crash witnesses in
   child processes. Record them as defect evidence rather than contracts before
   considering a safety fix.

## Phase 3H.5b: isolated adapter and indicator closure

`DW_014` through `DW_017` passed in the offscreen suite on 2026-08-04; the
suite result is 19 passed, 0 failed (exit code 0). They close every
Phase-3H.5a item marked as presently isolated and non-visual, except where the
source exposes no safe standalone operation:

| IDs | Executed isolated contract | Remaining boundary / blocker |
| --- | --- | --- |
| DW_014 | `QLedIndicator`, `QBLedIndicator` and `QTSLedIndicator` color properties; `SetState`; named traffic colors; enabled state; resize plus offscreen `render()` through all LED states. | Pixel output is intentionally not asserted. `QTSLedIndicator::SetColor("Yellow")` leaves the preceding color unchanged; only `YellowColor()` selects yellow. This is characterized legacy behavior. |
| DW_015 | `QBLedIndicator` constructs an owned, active 500-ms `QTimer`; two timeout emissions are observed through the Qt event loop; repeated `SetState`/`TimeOut` calls and destruction while active are safe. | There is no public timer-stop operation; destruction is the only currently observable stop/ownership contract. |
| DW_016 | `QPushButtonD::GetVariantData`; slider/spin clamping, repeat-value signal suppression and scaled slider roundtrip; button, check, combo and line-edit signal count/order/payload. Since 2026-08-10 it also verifies that editable connected `QString`, `QStringList` and `GuiSelection` inputs leave a `QSliderD` unchanged, emit no `valueChanged`, and restore its normal usable unblocked state. | `QPushButtonD::StartTimeOut` requires a manager mapping when its delayed path queries a container; this is a 3I MainWindow boundary, not run standalone. Numeric slider scaling with equal Min/Max remains a separate defect candidate. |
| DW_017 | LED bit XML load/save, ignored XML attribute, `QListViewD::DeleteEntry`/`DeleteAllEntries` signal/data mutation, read-only/enabled/parent lifetime. | List DnD/context menu and manager forwarding remain 3I. |
| DW_018 | A product-like connected `QSliderD` with exact equal manager bounds preserves value 37 for floating, signed and unsigned input, emits neither `valueChanged` nor a manager message, and leaves signals usable for a subsequent direct value change. A 0..100 control vector retains existing rounding (`7.5 -> 8`). | The former equal-bound division/cast path was not executed on the unsafe baseline; source and static-analysis evidence establish the defect candidate. |

The prior table's indicator rows and isolated adapter notes are superseded by
this executed mapping. All remaining unexecuted adapter functions are either
real source-backed drag/drop/manager paths for 3I, PlotWidget paths for 3J,
pixel-level visual assertions, or dangerous null/index paths that must first be
isolated in a child process.

### Fresh per-file gcov comparison for newly addressed files

Values are still per-file evidence, not project coverage. The `before` column
is the Phase-3H instrumented baseline; `after` is the fresh 3H.5b instrumented
run. Each cell is lines / branches executed / branches taken / calls.

| File | Before | After |
| --- | --- | --- |
| `DropWidgets/QBLed.cpp` | 11.58 / 10.81 / 5.86 / 10.00 % | 18.95 / 13.51 / 7.66 / 13.18 % |
| `DropWidgets/QLed.cpp` | 11.22 / 10.71 / 5.80 / 9.87 % | 18.37 / 13.39 / 7.59 / 13.00 % |
| `DropWidgets/QTSLed.cpp` | 11.46 / 10.71 / 5.80 / 9.95 % | 18.75 / 13.39 / 7.59 / 13.12 % |
| `CustomWidgets/QBLedIndicator.cpp` | 15.09 / 20.93 / 10.47 / 14.43 % | 100.00 / 97.67 / 51.16 / 89.69 % |
| `CustomWidgets/QLedIndicator.cpp` | 12.50 / 5.88 / 2.94 / 8.05 % | 100.00 / 100.00 / 51.47 / 90.80 % |
| `CustomWidgets/QTSLedIndicator.cpp` | 9.88 / 7.84 / 3.92 / 10.00 % | 100.00 / 100.00 / 54.90 / 92.31 % |
| `DropWidgets/QPushButton.cpp` | 7.69 / 7.02 / 3.51 / 5.88 % | 10.99 / 7.02 / 3.51 / 6.79 % |
| `DropWidgets/QSlider.cpp` | 6.54 / 8.70 / 4.35 / 7.46 % | 12.15 / 10.43 / 5.22 / 8.77 % |
| `DropWidgets/QSpinBox.cpp` | 20.00 / 10.31 / 5.15 / 12.29 % | 20.00 / 10.31 / 5.15 / 12.29 % |
| `DropWidgets/QCheckBox.cpp` | 21.31 / 13.14 / 7.30 / 14.07 % | 21.31 / 13.14 / 7.30 / 14.07 % |
| `DropWidgets/QComboBox.cpp` | 30.77 / 23.30 / 12.14 / 20.73 % | 30.77 / 23.30 / 12.14 / 20.73 % |
| `DropWidgets/QLineEdit.cpp` | 25.81 / 15.38 / 7.69 / 15.26 % | 25.81 / 15.38 / 7.69 / 15.26 % |
| `DropWidgets/QListView.cpp` | 19.19 / 15.97 / 7.98 / 12.80 % | 27.27 / 19.33 / 9.66 / 17.54 % |

Unchanged measurements are meaningful: their concrete adapter methods were
already directly exercised by DW_002/DW_003, while DW_016 observes the Qt
base-widget user-signal contract without entering the excluded manager paths.
