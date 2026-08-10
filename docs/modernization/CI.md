# Windows CI (Milestone 2C.1)

## Scope

`.github/workflows/windows-build-and-test.yml` is the first reproducible remote
Windows job. It runs on `windows-latest`, provisions MSYS2 `MINGW64`, and uses
the same PowerShell entry points as local development:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\build-msys2.ps1 -Configuration release -Clean -Deploy -BuildDir build\ci-release -DeployDir dist\LabAnalyser-release -Jobs 2
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-tests-msys2.ps1 -Clean -BuildRoot build\ci-tests -Jobs 2
```

For each build and test process, the workflow puts `C:\msys64\mingw64\bin`
before `C:\msys64\usr\bin` and inherited PATH entries. GUI tests run with
`QT_QPA_PLATFORM=offscreen`, scoped to the CI test process.

## Combined coverage job

The separate `coverage` job starts from a fresh MSYS2 MINGW64 environment and
runs the unchanged local command:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-coverage-msys2.ps1 -Jobs 2
```

It requires both generated reports, adds the combined totals and the local
exclusion scope to the GitHub step summary, and uploads the Markdown, JSON and
coverage transcript as `LabAnalyser-coverage-${{ github.sha }}` for 14 days.
The report uses the same local exclusions: vendored qcustomplot, generated
Qt/build outputs and `tests/**`; uncompiled production sources remain listed
instead of being silently scored as zero. No 90/80 or other hard coverage gate
is active.

The documented local baseline is 41.32% lines (2419/5854), 36.44% executed
branches (3855/10579), 20.40% branches taken at least once (2158/10579), and
61.23% functions (338/552). It is coverage of instrumented compiled production
sources, not a repository-wide percentage.

The Release step uses the existing deploy mode with
`-DeployDir dist\LabAnalyser-release`. Before upload, CI requires
`LabAnalyser.exe`, Qt Core/Gui/Widgets DLLs, the MinGW compiler runtimes, and
HDF5, matio and FFTW runtime DLLs. The existing deployment script also follows
native dependencies recursively. The downloadable artifact is named
`LabAnalyser-windows-release`, contains only `dist/LabAnalyser-release/`, and
is retained for 14 days. This is a CI artifact only; it creates no GitHub
Release or published package.

## Provisioned packages

The official `msys2/setup-msys2@v2` action performs a full MSYS2 update, then
installs the documented MINGW64 packages:

- `mingw-w64-x86_64-gcc`, `mingw-w64-x86_64-make`
- `mingw-w64-x86_64-qt6-base`, `mingw-w64-x86_64-qt6-tools`
- `mingw-w64-x86_64-matio`, `mingw-w64-x86_64-hdf5`,
  `mingw-w64-x86_64-highfive`, `mingw-w64-x86_64-fftw`
- `mingw-w64-x86_64-boost`

These match the production qmake dependencies recorded in `BASELINE.md`.

## Action versions and validation

On 2026-08-10 the official MSYS2 CI guide documented
`msys2/setup-msys2@v2`; it is used as the supported major-action pin. The
official GitHub release pages identified `actions/checkout@v7.0.1` and
`actions/upload-artifact@v7.0.1`, which are pinned in the workflow. Sources:
[MSYS2 CI guide](https://www.msys2.org/docs/ci/),
[checkout release](https://github.com/actions/checkout/releases/tag/v7.0.1),
and [upload-artifact release](https://github.com/actions/upload-artifact/releases/tag/v7.0.1).

The workflow passed a local structural validation (required YAML entries and
tab-free indentation). `actionlint`, `yamllint`, and a YAML parser are not
installed in this workspace, so full GitHub Actions schema validation remains
remote-unconfirmed. No GitHub-hosted run is available from this workspace. The
workflow uploads only text build/test logs as an artifact; it does not publish
executables, packages, releases, secrets, or a coverage gate.
