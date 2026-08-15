# LabAnalyser

LabAnalyser is a Qt 6 desktop application for acquiring, modifying,
visualising and exporting measurement data. It provides a plugin boundary for
devices, a data and parameter explorer, Designer-based form loading,
drag-and-drop bindings, plot/FFT tools and local/network data exchange.

## Features

- Plugin-based device integration through the stable `Platform_Fabric` and
  `Platform_Interface` boundary.
- Editable parameters and live data in the Parameter and Data Explorers.
- Dynamically loaded Qt Designer forms with drag-and-drop DropWidgets.
- Time-domain plots, FFT display and numerical measurements.
- Experiment persistence in `.LAexp` plus device sidecars (`.LAdev`).
- Parameter XML import/export and MAT v5 / HDF5 export.
- Import of LabAnalyser MAT exports and the native `.LAdat` archive format.
- Native `.LAdat` export/import for channels, scalars, booleans, text,
  selections and parameters; imported data remains usable without its source
  plugin.
- TCP remote control and the optional MATLAB connector package.

The application is GPL-3.0-or-later; see [LICENSE](LICENSE).

## Repository layout

Production sources are under `src/`; tests and fixtures remain under `tests/`.
The legacy qmake project and the additive CMake build describe the same
application graph.

```text
src/
  app/                  application entry point, MainWindow and forms
  DataManagement/       data containers, registry, messenger and project IO
  DropWidgets/          Designer widgets, bindings and plots
  Export/ Import/       MAT, HDF5, .LAdat and parameter IO
  LoadSave/             experiment XML and plugin loading
  plugins/              public plugin interface headers and data types
  RemoteControl/        TCP transport, framing and protocol
  UIFunctions/          MainWindow presentation and workflow helpers
resources/              Qt resources and icons
scripts/                MSYS2 build helper
tests/                  qmake suites, CTest targets and compatibility fixtures
docs/modernization/     architecture, contracts, plans and build notes
```

`qcustomplot` under `src/DropWidgets/Plots/` is vendored runtime code. It is
not a project refactoring target.

## Run a release build on Windows

The supported local toolchain is MSYS2 MINGW64 with Qt 6. Install:

```powershell
pacman -Syu
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-make `
  mingw-w64-x86_64-qt6-base mingw-w64-x86_64-qt6-tools `
  mingw-w64-x86_64-boost mingw-w64-x86_64-highfive `
  mingw-w64-x86_64-fftw mingw-w64-x86_64-hdf5 mingw-w64-x86_64-matio
```

From the repository root, create a deployable release directory:

```powershell
.\scripts\build-msys2.ps1 -Configuration release -Deploy
```

The executable and its runtime DLLs are written to
`dist\LabAnalyser-release\LabAnalyser.exe`. Use `-Configuration debug` for a
debug build. Add `-Clean` only when the toolchain, build configuration or build
directory is known to be stale.

The build depends on Qt 6, Boost, HighFive, HDF5, FFTW3 and libmatio. MAT I/O
uses [libmatio](https://github.com/tbeu/matio); `matOut` is not required.

## CMake build and CTest

CMake is supported in parallel with qmake on MSYS2 MINGW64:

```powershell
$env:Path = 'C:\msys64\mingw64\bin;C:\msys64\usr\bin;' + $env:Path
cmake -S . -B build\cmake-msys2 -G 'MinGW Makefiles' `
  -DCMAKE_BUILD_TYPE=Release -DLABANALYSER_BUILD_TESTS=ON
cmake --build build\cmake-msys2 --parallel 2
ctest --test-dir build\cmake-msys2 --output-on-failure --parallel 2
```

The target uses `AUTOMOC`, `AUTOUIC` and `AUTORCC`. Current CMake/qmake parity
and its limits are in [docs/modernization/CMAKE.md](docs/modernization/CMAKE.md).

## qmake build and tests

`LabAnalyser.pro` remains supported. Qt Creator can open it directly, or it can
be built through `scripts/build-msys2.ps1`.

Run the registered qmake test suites from the repository root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass `
  -File .\tests\run-tests-msys2.ps1 -Jobs 2
```

The runner builds plugin fixtures first and runs twelve deterministic Qt Test
targets: XML/legacy compatibility, data management, exports, remote control,
DropWidgets, MainWindow and plots. GUI targets use Qt offscreen only for their
test process. See [tests/README.md](tests/README.md) and
[docs/modernization/CI.md](docs/modernization/CI.md).

## Data formats

| Format | Direction | Notes |
| --- | --- | --- |
| `.LAexp` / `.LAdev` | Load and save | Experiment, form, device, figure, widget and state persistence. Existing legacy fixtures are read-compatible. |
| `.LAdat` | Export and import | Native archive for data and parameters, suitable for plotting and analysis without the originating plugin. |
| `.mat` | Export and import | MAT v5 through libmatio. Import reads the LabAnalyser `ExportedChannels` schema. |
| HDF5 | Export | HDF5/HighFive export. |
| parameter XML | Import and export | Parameter-oriented XML exchange. |

The historical fixtures under `tests/fixtures/xml/legacy/` are byte-protected
inputs. Do not regenerate, migrate in place or overwrite them. Old experiments
must retain their documented error and partial-state behaviour when historical
plugins or forms are absent.

## Plugin compatibility and shutdown

The existing plugin IID and the `Platform_Interface` / `Platform_Fabric` ABI
remain unchanged. Plugins must use the same architecture, MSYS2 MINGW64 compiler
family and compatible Qt 6 runtime as LabAnalyser. External plugin projects
should include public headers from `LabAnalyser/src/plugins/`.

When a project closes, LabAnalyser sends the established Messenger command
`CloseProject` to every loaded Legacy-V1 plugin before its Messenger connections
are removed. A plugin with a worker thread, TCP client, hardware link or other
active runtime resource must handle it in
`MessageReceiver(const QString&, const QString&, InterfaceData)` and start its
own non-blocking shutdown:

```cpp
void MyPlugin::MessageReceiver(const QString& command, const QString& id,
                               InterfaceData data)
{
    Q_UNUSED(data);
    if (command == QStringLiteral("CloseProject") && id == objectName()) {
        worker_.requestStop(); // The worker closes sockets/resources itself.
        return;
    }

    // Existing command handling.
}
```

The handler must be idempotent. LabAnalyser deliberately does not unload a
Legacy-V1 plugin or delete an interface with unproven ownership; plugin objects
can remain resident until process exit. Plugins remain responsible for stopping
their own threads, TCP connections and hardware activity on `CloseProject`.

## Development notes

- Build products belong in `build/` or `dist/` and must not be committed.
- Keep plugin IDs, public interfaces, `.LAexp` compatibility and remote-control
  bytes stable unless an explicit compatible migration is approved.
- `AGENTS.md` contains binding behavior-preservation and verification rules.
- Architecture, known risks and modernization work are maintained in
  `docs/modernization/`.

## Known limitations

- CMake/CTest has MSYS2 Windows evidence; Debug, Linux, install and packaging
  parity remain separate work.
- No broad coverage gate is currently enabled.
- `LoadForms()` is declared in legacy code but has no repository implementation.
- Missing historical plugins/forms are reported and leave the documented partial
  experiment state; they are not fabricated.
