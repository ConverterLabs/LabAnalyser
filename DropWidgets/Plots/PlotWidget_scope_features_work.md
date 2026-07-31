# PlotWidget Scope Features Work Definition

## Goal

Extend `DropWidgets/Plots/PlotWidget` with oscilloscope-like plot tools:

- A visible toolbox for selecting navigation, zoom, single cursor, and double cursor modes.
- Single-cursor readout for graph values at one x position.
- Double-cursor readout for two x positions, including dt, dy, slope, and related values.
- A statistics/readout box with values such as mean, RMS, min, max, and peak-to-peak.
- Integration with the existing `QCustomPlot`-based graph rendering and current XML save/load behavior where useful.

This file defines the requested work. It does not implement the feature yet.

## Current State

`PlotWidget` already provides:

- Drag and wheel zoom through `QCP::iRangeDrag` and `QCP::iRangeZoom`.
- Ctrl + left mouse rectangle zoom through the existing `QCPItemRect *rectZoom`.
- A context menu with reset zoom, PDF export, graph removal, marker toggle, FFT toggle, quality criteria, and update actions.
- Data updates from `MainWindow_p->GetLogic()` into each `QCPGraph`.
- Graph data access through `QCPGraph::GetXDataPointer()` and `QCPGraph::GetYDataPointer()`.
- FFT/quality criteria overlays through `QCPItemText`.

The local `qcustomplot.h` already contains item classes suitable for this work:

- `QCPItemStraightLine` or `QCPItemLine` for cursor lines.
- `QCPItemTracer` for graph snap markers.
- `QCPItemText` for plot-overlay readouts.
- `QCPItemRect` for rectangular zoom/selection overlays.

## User Experience

### Toolbox

Add a small toolbox inside or above the plot area with mutually exclusive tool modes:

- Navigate: current drag/wheel zoom behavior.
- Box zoom: click-drag rectangle zoom without needing the Ctrl key.
- Single cursor: click or drag one vertical cursor.
- Double cursor: click or drag cursor A and cursor B.
- Reset zoom.
- Toggle readout/statistics box.
- Optional: clear cursors.

Implementation preference:

- Because `PlotWidget` inherits from `QCustomPlot`, use child Qt widgets overlaid on the plot, or a lightweight helper widget owned by `PlotWidget`.
- Keep existing context-menu actions available.
- Do not make the toolbox depend on `MainWindow` layout changes unless necessary.

### Interaction Modes

Add an internal mode enum, for example:

```cpp
enum class PlotToolMode
{
    Navigate,
    BoxZoom,
    SingleCursor,
    DoubleCursor
};
```

Expected behavior:

- Navigate mode keeps existing panning, axis selection, and wheel zoom.
- Box zoom mode uses the existing `rectZoom` concept with normal left mouse drag.
- Single cursor mode creates or moves cursor A.
- Double cursor mode creates cursor A and cursor B, then moves the nearest cursor when dragging.
- Existing Ctrl-box-zoom behavior can remain as a compatibility shortcut, but the toolbox should be the primary control.

## Cursor Readout

### Single Cursor

For cursor A, show:

- Cursor x position.
- For each visible graph, interpolated y value at x.
- Optional graph name/color marker.
- If no graph has valid data at the x position, show an empty or "n/a" value instead of crashing.

### Double Cursor

For cursor A and cursor B, show:

- xA and xB.
- dt = xB - xA.
- For each visible graph: yA, yB, dy = yB - yA.
- slope = dy / dt when dt is non-zero.
- Optional frequency = 1 / abs(dt) for time-domain plots when dt is non-zero.

Cursor behavior should work for:

- Normal time-domain plots.
- XY plots where graph x data is not a uniform time vector.
- FFT plots, with x interpreted as frequency.

## Statistics Box

Add a readout box that can show statistics for selected or visible graph data.

Minimum statistics:

- count
- min
- max
- peak-to-peak
- mean
- RMS

Useful optional statistics:

- standard deviation
- median
- integral over x
- sample dt estimate for time-domain data

Statistics range policy:

- If double cursors are active, compute over the interval between cursor A and cursor B.
- Otherwise compute over the visible x-axis range.
- Optionally allow "full data" as a future range mode.

Graph policy:

- If one or more graphs are selected, calculate statistics for selected graphs.
- Otherwise calculate statistics for all visible graphs.
- Hidden graphs should not appear in the readout.

Existing FFT quality criteria can remain separate initially. A later cleanup can merge quality criteria and statistics into one readout system.

## Data and Calculation Rules

Use existing `QCPGraph` data pointers:

- `GetXDataPointer()`
- `GetYDataPointer()`
- `GetXFFTPointer()`
- `GetYFFTPointer()`

Cursor interpolation:

- Use `std::lower_bound` on the x data.
- If the x position is between two points, linearly interpolate y.
- If x is outside the data range, report invalid/n/a.
- Guard all pointer, size, and range checks.

Statistics:

- Only include finite values.
- Only include samples whose x values are inside the active range.
- RMS is `sqrt(mean(y * y))`.
- Mean is arithmetic mean over included y samples.
- Min/max ignore invalid values.
- Avoid division by zero for slope, RMS, and frequency-derived values.

Performance:

- Do not recalculate full statistics on every paint.
- Recalculate after data updates, cursor movement, range changes, graph selection changes, and readout visibility changes.
- Keep calculations O(n) over the graphs/range being measured.

## Suggested Code Changes

### `PlotWidget.h`

Add:

- Tool mode enum.
- Cursor state structs for cursor A and cursor B.
- Readout/statistics data structs.
- Members for cursor line items, tracer items, readout text item, and toolbox widget/actions.
- Slots for tool-mode changes and cursor/readout updates.

Possible private helpers:

- `void setToolMode(PlotToolMode mode);`
- `void initializePlotTools();`
- `void updateCursorItems();`
- `void updateReadout();`
- `void updateStatistics();`
- `bool interpolateGraphValue(QCPGraph *graph, double x, double *y) const;`
- `QVector<QCPGraph*> graphsForReadout() const;`

### `PlotWidget.cpp`

Add:

- Toolbox creation in the constructor after basic plot setup.
- Mouse handling branches for the active tool mode.
- Cursor item creation, movement, visibility, and styling.
- Readout text formatting.
- Statistics calculation using graph data pointers.
- XML persistence for optional visible state, active mode, cursor positions, and readout visibility.

Reuse:

- Existing `rectZoom` for box zoom.
- Existing `QCPItemText` pattern from `CalculateQualityCriteria()`.
- Existing `UpdateGraphs()` as the point where data-refresh-triggered readout recalculation can happen.

## Open Decisions

- Whether cursor readout should follow the selected graph only or show all selected/visible graphs.
- Whether the toolbox should be always visible, collapsible, or available through context menu only.
- Whether cursor positions should be saved to XML.
- Whether the statistics box should be a `QCPItemText` overlay or a real Qt child widget.
- Whether RMS/mean/min/max should be calculated over raw time-domain data in FFT mode, FFT data in FFT mode, or both.

Recommended initial choices:

- Show selected graphs if any are selected, otherwise all visible graphs.
- Make the toolbox always visible but compact.
- Save readout visibility and cursor positions only if cursors are active.
- Use a Qt child widget for the toolbox and `QCPItemText` for the plot readout box.
- In FFT mode, calculate statistics over the data currently displayed by the graph.

## Acceptance Criteria

- User can switch between navigate, box zoom, single cursor, and double cursor modes without keyboard modifiers.
- Single cursor can be placed and dragged; readout shows x and y values for relevant graphs.
- Double cursor can be placed and dragged; readout shows xA, xB, dt, yA, yB, dy, and slope.
- Statistics box shows count, min, max, peak-to-peak, mean, and RMS for relevant graph data.
- Statistics update after data refresh, cursor move, zoom/range change, graph selection change, and graph removal.
- Existing context menu actions still work.
- Existing drag/drop graph adding still works.
- Existing Ctrl-box-zoom shortcut still works or is intentionally replaced by toolbox box zoom.
- Empty plots, hidden graphs, mismatched vector sizes, and null data pointers do not crash.
- Build succeeds with the current Qt/qmake project.

## Suggested Implementation Order

1. Add tool mode enum and readout/cursor state members.
2. Add toolbox UI and connect actions to mode changes.
3. Implement cursor graphics and mouse interaction.
4. Implement cursor interpolation and cursor readout.
5. Implement statistics calculation and readout range policy.
6. Hook readout refresh into `UpdateGraphs()`, range changes, graph selection, and graph removal.
7. Add XML persistence if the first implementation is stable.
8. Build and manually verify with empty plot, one graph, two graphs, XY plot, and FFT plot.

