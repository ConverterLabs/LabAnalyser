# Plot and FFT contracts â€” Phase 3J

## Scope and test environment

`tests/contract/plotwidget/PlotWidgetContractTests` links the real qmake
application graph with the real DataManagement/Messenger route. It runs with
`QT_QPA_PLATFORM=offscreen`; the central runner sets and restores that variable
only for this suite. `qcustomplot` remains a vendored runtime dependency and
is neither modified nor tested as an independently owned component.

The PlotWidget/FFT suite passed with 16 Qt Test entries (including setup and
cleanup), zero failures and exit code 0. The central runner registration is
the reproducible local entry point.

## Characterized contracts

| IDs | Evidence |
| --- | --- |
| `PLOT_001` | Construction, supplied QWidget parent, empty graph model, default labels/ranges/interactions, owned tools and timer. |
| `PLOT_002` | Published vector creates a named graph; alias/legend identity and Messenger `set` reach graph data. |
| `PLOT_003` | Multiple/repeated IDs append graphs; visibility and selected/all graph removal alter the model. |
| `PLOT_004` | Empty, one-point, NaN, Inf and unequal source vectors remain stored in the time-domain route. |
| `PLOT_005` | Reset range, XML state round-trip and Control navigation state. |
| `PLOT_006` | `ConnectToID` uses real manager/Messenger data and a pre-cleaned graph model destroys safely. |
| `FFT_001` | `FFTPlotWidget` parenting, touch-event attribute, initial model and destruction. |
| `FFT_002` | N=8, fs=8 Hz, `2*sin(2*pi*n/8)`: 1-Hz bin amplitude 2.0, labels and FFT graph style. |
| `FFT_003` | N=8, fs=8 Hz, `1.5 + 2*sin(2*pi*n/8)`: DC 1.5 and bin 1 amplitude 2.0. |
| `FFT_004` | Separate vectors for 1-Hz/amplitude-1 and 2-Hz/amplitude-3 graphs. |
| `FFT_005` | Time labels, graph style and saved `[-2, 4]` X range return after toggling back. |
| `FFT_006` | Messenger update recalculates FFT without appending a graph. |
| `FFT_007` | Existing nonuniform mean-delta-T rule: `{0,.1,.4,.6}` produces a 1.25-Hz grid. |
| `FFT_008` | Empty and unequal vectors use the safe early return and leave FFT pointers unset. |

## Milestone 5 numerical/plot hardening

Work package B added narrow, behavior-preserving guards without touching
vendored `qcustomplot` or valid FFT results. `FFT_009` proves that zero- and
one-sample inputs retain their original time vectors and create no FFT vectors.
`FFT_010` uses test-target-only `LABANALYSER_PLOTWIDGET_TEST_SEAMS` fault
injection to fail each FFTW allocation and plan creation: no partial FFT
vectors are adopted and a following ordinary FFT remains usable. The hooks are
absent from normal application builds and add no public API.

`PLOT_007` drives the existing frequency dialog to zero and verifies bounded
quality-criteria indices. `PLOT_008` exercises cursor toggle/clear slots with
no graphs; they leave the empty plot model unchanged. Empty XY data now returns
before the existing first/last sample comparisons. Valid amplitudes, native
FFTW bins, mean-delta-T, styles and time/frequency toggling remain covered by
unchanged `FFT_002`--`FFT_007` vectors and tolerances.

Analytic amplitudes use absolute tolerance `1e-10`; nonuniform frequency-grid
checks use `1e-12`. These limits are specific to the small exact,
double-precision FFTW vectors.

## Per-file coverage evidence

Already-observed instrumented suite results, not project coverage or a gate:

| Production file | Lines | Branches executed | Branches taken at least once | Calls |
| --- | ---: | ---: | ---: | ---: |
| `DropWidgets/Plots/PlotWidget.cpp` | 42.89% | 37.09% | 20.65% | 39.24% |
| `DropWidgets/Plots/FFTPlotWidget.cpp` | 80.00% | 100.00% | 50.00% | 66.67% |
| `DropWidgets/Plots/PlotMeasurements.cpp` | Separate `PlotMeasurementsTests` unit suite is authoritative. |

## Defect candidates and remaining gaps

- Repeated `AddCustomGraph` calls for one ID append duplicate graphs.
- `ClearAllGraphs` removes manager registrations but not qcustomplot graphs;
  internal `removeAllGraphs` does both.
- Test seam failure handling is not an OS-memory-exhaustion or FFTW-internal
  failure proof.

Only safe empty-model cursor controls and bounded zero-frequency quality are
covered. Valid cursor readout, history retention and broad context-menu paths,
rendering/gesture behavior, non-finite FFT policy and large resource limits
remain open. **PlotWidget is not cleared for broad refactoring** until the
remaining safe paths are characterized.

## Plot drag/drop input hardening (2026-08-12)

`PLOT_009` maps direct and historical `Buffered` tree-ID resolution for leaf
`Data`/`vector<double>` containers, including null manager, branch and null
output rejection. `PLOT_010` covers a foreign drop event as a no-op on an
existing graph. `PlotWidgetDropBinding` centralizes only eligibility and ID
resolution; it does not alter qcustomplot, valid graph creation, Shift X-axis
behavior, data bytes or the historical buffered-ID spelling. Empty tree
selection, missing MainWindow/manager, an XY plot and unsupported containers
are rejected before index/dereference or graph update.

`PLOT_011` establishes the sender boundary for `selectionChanged()`: a direct,
senderless invocation is a no-op. The existing `selectionChangedByUser()`
connection remains the valid synchronization route.

`PLOT_012` records `graphClicked()` with a null plottable as a no-op that
preserves the statusbar text; a valid graph still reports its existing name.

`PLOT_013` records no-op behavior for `AddCustomGraph()` without a manager and
for `AddCustomXAxis()` without a graph or matching container. Valid manager
backed graph creation is unchanged.

`PLOT_014` invokes the existing legend-double-click slot through Qt's metaobject
with no legend item. It preserves the graph model and name; non-plottable or
null item inputs now return before the legacy dialog/alias route.
