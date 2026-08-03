# Local test layout

Run all currently registered local tests from the repository root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-tests-msys2.ps1 -Clean
```

The script uses the existing MSYS2 MINGW64 qmake toolchain, builds each explicit
test project beneath the ignored `build\tests-msys2-mingw64` directory, runs its
test executable with Qt Test text output, and returns a non-zero exit code on
the first configuration, compilation, or test failure.  `-Clean` removes only
that build root after verifying it remains inside this repository.

Current suite placement:

| Directory | Purpose | Current contents |
| --- | --- | --- |
| `tests/unit/` | Isolated deterministic domain/numerical tests | `PlotMeasurementsTests` remains at its established root path and is registered as unit coverage. |
| `tests/component/` | Tests of a cohesive Qt/data component | Reserved. |
| `tests/integration/` | Cross-boundary workflows | Reserved. |
| `tests/contract/` | Compatibility/differential contract tests | Reserved. |
| `tests/fixtures/` | Small deterministic compatibility inputs/expected outputs | Directory structure only; no golden data yet. |

When adding a suite, place its source in the appropriate directory, add its
qmake test project to the explicit `$testProjects` manifest in
`run-tests-msys2.ps1`, and document its stable test IDs in the behavior
inventory.  Do not add `LabAnalyser.pro` to the test manifest.
