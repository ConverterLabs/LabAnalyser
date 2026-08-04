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
- XY update reads first/last samples without an empty-vector guard.
- FFT has no `fftw_malloc`/plan failure check; one sample reaches mean-delta-T
  division over an empty vector.

Cursor, context-menu, history-bound and quality-criteria paths are still
safely testable with valid graph/manager input but remain uncovered. Rendering,
pixel output, mouse/wheel/box-zoom gestures, source-dependent drag/drop, PDF,
null manager/graph paths, failed FFTW allocation/plan creation and unsafe index
paths are excluded. **PlotWidget is not cleared for broad refactoring** until
the safe cursor, context, history and quality-criteria contracts are covered.
