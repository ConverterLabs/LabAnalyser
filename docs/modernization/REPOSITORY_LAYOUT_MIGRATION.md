# Repository-layout migration

## Scope and invariants

This is a structural migration only. It preserves public C++/Qt APIs, plugin
IID and ABI, Qt resource identifiers, legacy `.LAexp` bytes and semantics,
qmake support, CMake/CTest support, fixture locations, and all behavioral test
contracts. Moves use `git mv`; no production implementation is copied into
tests.

The initial support-file slice is already in the working tree:

| Previous path | Destination | Notes |
| --- | --- | --- |
| `resources.qrc`, `icons/`, root icon/RC files | `resources/` | QRC keeps `icons/` relative to itself, so `:/icons/...` identifiers stay unchanged. |
| `build-msys2.ps1` | `scripts/build-msys2.ps1` | Root-relative repository discovery in the script is retained. |
| `Doxygen/` | `docs/doxygen/` | Documentation only. |
| `readme_pictures/` | `docs/images/readme_pictures/` | README links are updated. |

## Production path map

The canonical target keeps module identities as directory names under `src/`
for the first move. This deliberately preserves existing `#include
"DataManagement/..."` and `#include "DropWidgets/..."` contracts through one
source-root include directory. A later lowercase-only rename, if wanted, is a
separate mechanical change rather than an include/API migration.

| Current path | Canonical path | Main affected boundaries |
| --- | --- | --- |
| root `main.cpp`, `mainwindow.*`, `About.ui` | `src/app/` | AUTOUIC, app target, MainWindow integration tests |
| `DataManagement/` | `src/DataManagement/` | component, UI/IO, plugin, remote and GUI tests |
| `Import/` | `src/Import/` | parameter and UI/IO contracts |
| `Export/` | `src/Export/` | MAT/HDF5/parameter/UI/IO contracts |
| `LoadSave/` | `src/LoadSave/` | XML, plugin and UI/IO contracts |
| `CustomWidgets/` | `src/CustomWidgets/` | DropWidget contracts |
| `DropWidgets/` and root `TreeWidgetCustomDrop.*` | `src/DropWidgets/` | DropWidget, plot, XML and MainWindow contracts |
| `UIFunctions/` | `src/UIFunctions/` | MainWindow and XML/UI contracts |
| `RemoteControl/` | `src/RemoteControl/` | Remote-control contracts |
| `MatlabRemoteConnector/` | `src/MatlabRemoteConnector/` | standalone CMake and connector contracts |
| `plugins/` | `src/plugins/` | plugin fixtures, ABI/IID and all manager users |

## Required migration order

1. Finish and validate the support-file slice.
2. Move `src/app/`, then update the two primary build definitions and direct
   MainWindow test references.
3. Move DataManagement and plugin infrastructure together: they share public
   include prefixes and are direct inputs to the greatest number of contract
   targets.
4. Move persistence, import and export together after their test-project
   paths are updated.
5. Move UI modules (`CustomWidgets`, `DropWidgets`, `UIFunctions`, and
   `TreeWidgetCustomDrop`) together, including AUTOUIC dependencies.
6. Move RemoteControl and MatlabRemoteConnector, preserving their standalone
   CMake boundaries.
7. Normalize remaining qmake, CMake/CTest, script, CI and documentation
   paths; then execute the global stale-path audit.

Each module step ends with: static include/path audit, affected qmake/CMake
configuration inspection, `git diff --check`, and a source/path audit. It does
not require a full build of every one of the 12 targets after each file move.
The complete qmake runner, fresh CMake build and CTest run occur once after
every production and test project points at canonical paths; focused builds are
reserved for a concrete configuration ambiguity or compiler diagnostic.

## Test-target checklist

| Target | Direct production-path families that must be updated |
| --- | --- |
| `PlotMeasurementsTests` | `DropWidgets/Plots` |
| `DataManagementCharacterizationTests` | `DataManagement`, `plugins`, test seams |
| `XmlExperimentContractTests` | app, `DataManagement`, `LoadSave`, `DropWidgets`, `UIFunctions`, `plugins` |
| `ParameterContractTests` | `DataManagement`, `Import`, `Export`, `plugins` |
| `MatExportContractTests` | `DataManagement`, `Export`, `plugins` |
| `Hdf5ExportContractTests` | `DataManagement`, `Export`, `plugins` |
| `RemoteControlContractTests` | `RemoteControl`, `DataManagement`, `plugins`, test seams |
| `DropWidgetAdapterTests` | `DropWidgets`, `CustomWidgets`, `DataManagement`, `plugins`, app seam |
| `PluginLoaderContractTests` | `LoadSave`, `DataManagement`, `plugins`, fixtures |
| `ProjectIoFacadeContractTests` | `DataManagement`, persistence/import/export/plugins |
| `PlotWidgetContractTests` | `DropWidgets/Plots`, `DataManagement`, `plugins` |
| `MainWindowIntegrationTests` | app, `UIFunctions`, `DropWidgets`, `DataManagement`, persistence/plugins |

## Completion audit

Before completion, search tracked files for every old production path and
classify each result as an intentional historical reference or a stale build,
test, script, CI or documentation path. Verify legacy fixture hashes and
resource aliases, then run both supported build systems and the registered
test suite without committing generated files.
