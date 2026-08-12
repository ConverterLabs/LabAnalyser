# Work package B -- numerical and plot hardening

Focused hardening uses the real offscreen PlotWidget graph and the existing
pure PlotMeasurements suite. It does not modify `qcustomplot`, valid FFT
output, native FFTW layout, tolerance, styles, or interaction rendering.

- `9b01417` guards `CalculateFFT()` for fewer than two samples, null graph
  entries, and FFTW allocation/plan failure. The test-only build defines
  `LABANALYSER_PLOTWIDGET_TEST_SEAMS`; normal builds contain no fault control.
- `b0d3dbe` bounds quality-criteria FFT indices, prevents zero-frequency
  looping, and tolerates a not-yet-created text item.
- `ad3aaa8` characterizes pure PlotMeasurements normalization of mismatched,
  duplicate and non-finite input plus safe interpolation boundaries.
- `7a9431e` prevents XY first/last indexing when paired data is absent or empty.

Focused evidence: `PlotWidgetContractTests` passed 20 Qt Test entries and
`PlotMeasurementsTests` passed 8 entries, both exit code 0. The PlotWidget
target was rebuilt incrementally in the existing MSYS2/Qt tree; no clean or
broad local checkpoint was used. GitHub CI remains the package-wide remote
checkpoint.

Remaining risks: real allocator/FFTW failures outside the test seam, null
MainWindow ownership paths, valid cursor readout/history retention/context
workflows, rendering/gesture behavior, non-finite FFT numerical policy and
large input/resource limits.
