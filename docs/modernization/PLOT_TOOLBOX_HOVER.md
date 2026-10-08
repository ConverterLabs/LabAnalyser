# Plot toolbox hover behavior

The user explicitly requested on 2026-10-08 that the top plot toolbox appear
only when the pointer enters its area. This authorizes the visibility and
viewport change below; persistence, graph data and tool actions are unchanged.

The baseline toolbox is a child QFrame named `PlotToolbox`, spanning the plot
width at y=0. `initializePlotTools` creates and wires its buttons and time-unit
combo. `updateToolboxGeometry` sizes it on construction and resize.
`updateMeasurementPanelGeometry` reserves its height above the plot viewport,
with an optional measurement panel below. `event` also handles existing touch
input; hover handling must preserve that forwarding.

The requested behavior hides the toolbox initially, reveals it in the top
strip (the toolbox's existing height), and hides it outside that strip or on
leaving the plot. It overlays the plot without changing viewport geometry.
Hover events propagate from toolbox children, so moving between buttons keeps
the toolbox open. A combo popup must remain usable when the pointer leaves
the plot to select an item. No animation, saved setting or file-format change
is introduced.

`PLOT_028` characterizes construction, toolbox bounds and the baseline reserved
viewport. Its approved assertions then cover the hidden default and full
viewport. `PLOT_029` covers hover entry, movement below the strip, leaving,
reentry, resize, button operation and the combo popup.

Verification on Windows/MSYS2 MINGW64, Qt 6.9.2, GCC 15.2:

- The unchanged production graph was built with the new baseline `PLOT_028`
  assertions before editing production code. Its invocation returned exit 0,
  but the redirected text log was empty, so no baseline check count is claimed.
- The candidate Release PlotWidget test graph built successfully. Explicit Qt
  Test file output reports 40 passed, 0 failed, including `PLOT_028..PLOT_029`.
  Two initial candidate assertions failed (pointer already in the strip and
  popup detection); the final run passed after the focused corrections.
- `PlotMeasurementsTests`: 8 passed, 0 failed.
- Scoped `git diff --check`: passed after normalizing edited file line endings.
- The subsequently requested CMake Release application target built successfully
  and was copied to the user's existing installation. Source and destination
  SHA-256 matched; the previous EXE is backed up in the ignored build directory.
- A full XML/MainWindow/DropWidget checkpoint, qmake parity and remote CI were
  not run within the five-minute local budget.
  No plot/figure factory, XML reader/writer, UI loader or registry was changed.
- Logs remain ignored under `build/plot-*-build.log` and
  `build/plot-hover-tests.txt`, `build/plot-measurements.txt`.

Deployment correction on 2026-10-08: after the user reported missing icons and
requested the established build, `scripts/build-msys2.ps1 -Configuration release
-BuildDir build/qmake-release -Jobs 6` completed successfully. Its two MATLAB
connector/package checks passed. The qmake application directly links
`qrc_resources.o` (55,550 bytes) and has Windows GUI subsystem 2. The EXE
(1,888,256 bytes) replaced the CMake deployment with matching source/destination
SHA-256. The hover source change is included; unchanged characterization suites
were not rerun. Interactive icon rendering after this replacement remains for
user confirmation. See `CMAKE.md` for the newly identified resource-parity gap.

## User-requested pin control

2026-10-08: a checkable pin button at the right of each toolbox keeps it visible
when the pointer leaves the strip or plot. Clicking again restores hover-only
visibility. The native Qt-drawn pin icon avoids another resource dependency;
the checked highlight and tooltip distinguish pinned/unpinned states. Pinning
is local to the current PlotWidget instance and adds no persisted XML/settings
field. Viewport geometry and existing tool modes remain unchanged.

Before production edits, the new `PLOT_030` hover/leave and independent-plot
vectors passed against the existing implementation (3 Qt Test checks including
setup/cleanup). The same vectors are retained and extended to cover the pin
icon, default state, toggling, leave/movement below the strip, resize, isolation
from another plot and return to hover-only behavior.

Candidate evidence: Release PlotWidget contract graph built and passed 41
checks with zero failures. The incremental established qmake/MSYS2 Release
script completed with both connector/package checks passing. The replacement
EXE was copied to the existing installation with matching SHA-256. Scoped
diff checks passed; unchanged numerical tests were not repeated. Logs remain
ignored under `build/pin-*`. Full remote/legacy checkpoint status is unchanged.

## Docked layout correction

The user then requested that the entire plot remain visible when pinned.
Pinned mode now reserves the toolbox height above the plot viewport; unpinned
hover mode retains its overlay. The toggle recalculates the viewport and queues
a replot, including when the bottom cursor measurement panel is visible.
Existing axis ranges and tool state are preserved. A window shorter than the
toolbar reserves at most the available height and has a zero-height viewport.

Before this correction, `PLOT_031` recorded the old overlay geometry across two
pin/unpin and cursor-panel cycles, with unchanged numerical axis ranges; all
3 Qt Test checks including setup/cleanup passed. The approved geometry assertions
now check separate toolbar, plot and measurement-panel space, the laid-out axis
rectangle, tiny-window bounds and full viewport restoration after unpinning.
`PLOT_030` retains its isolation and hover vectors with approved pinned geometry.

Candidate: 42 PlotWidget checks passed, 0 failed; both Release test graph and
the established incremental qmake Release build passed. The script's two
connector/package checks also passed. Scoped diff checks passed, and the
installed replacement EXE SHA-256 matches the qmake artifact. Logs are ignored
under `build/pin-layout-*`; unchanged numerical/remote suites were not repeated.

## Approved XML persistence

The user explicitly requested saving pinned/unpinned state on 2026-10-08.
The planned compatible extension is an optional `PlotToolboxPinned` attribute
on each existing plot `Widget` element. Writers emit `0` or `1`; readers use
the existing integer-to-bool convention and default to `false` when absent or
malformed. Loading an old file resets a previously pinned instance to hover
mode. The pin button's existing callback restores docked geometry, without
changing axis ranges, cursors, other tools, widget names or figure layout.
No fixture migration is needed; legacy fixture bytes remain untouched.

Compatibility scope: old readers already ignore unrecognized plot attributes,
and new readers retain all existing fields. No new section or schema version
is introduced. The risks are missed per-plot restoration, stale layout and
legacy-load regression. `PLOT_032` and the full-application `XML_009` record
the old loss of pin state before production changes and then verify the
explicitly approved state preservation and independent pinned/unpinned plots.

Baseline: both new tests passed against unchanged production (3 Qt Test checks
each, including setup/cleanup), recording that no attribute was saved and pin
state was lost. Candidate: PlotWidget 43/43, XML 24/24 including every
`XML_LEGACY_001..005`, MainWindow 34/34, DropWidget 39/39 passed. An initial
direct test expected the full toolbar height even for an unshown 30-pixel plot;
its assertion was corrected to use the already-characterized bounded viewport.
No production correction was needed for that test assumption.

The established incremental qmake Release build and its two connector/package
checks passed; installed EXE SHA-256 matched. All five legacy fixture hashes
matched the manifest before and after verification. Added-line secret-pattern,
tracked generated-artifact and scoped diff checks passed. Logs remain ignored
under `build/pin-xml-*`; full remote CI, coverage and unchanged numerical suites
were not rerun for this slice.
