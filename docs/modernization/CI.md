# Windows CI (Milestone 2C.1)

## Scope

`.github/workflows/windows-build-and-test.yml` is the first reproducible remote
Windows job. It runs on `windows-latest`, provisions MSYS2 `MINGW64`, and uses
the same PowerShell entry points as local development:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\build-msys2.ps1 -Configuration release -Clean -BuildDir build\ci-release -Jobs 2
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-tests-msys2.ps1 -Clean -BuildRoot build\ci-tests -Jobs 2
```

For each build and test process, the workflow puts `C:\msys64\mingw64\bin`
before `C:\msys64\usr\bin` and inherited PATH entries. GUI tests run with
`QT_QPA_PLATFORM=offscreen`, scoped to the CI test process.

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
