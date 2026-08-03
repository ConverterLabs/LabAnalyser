# Parameter import/export characterization (phase 3C)

## Scope

`ParameterContractTests` builds the unchanged qmake application graph without
`main.cpp`. It exercises `ParameterLoader`, `ExportInputs2Xml`, their
`DataManagementSetClass`/`MessengerClass` data flow, and the real
`UIDataManagementSetClass::{ImportFromXml,Export2Xml}` callers. No test seam
or production source modification is used.

Fixtures in `tests/fixtures/parameters/` are small UTF-8 XML files without
machine paths or personal data.

## Function mapping

| IDs | Public / observable production surface | Contract characterized |
| --- | --- | --- |
| PARAM_001–006 | `ParameterLoader::{ParameterLoader,Load}` | known values, types, signals, optional/unknown/duplicate content, invalid or absent files and repeated loads. Private `readParameterSets`/`readEntry` are reached only through `Load`. |
| PARAM_007–008 | `ExportInputs2Xml::{ExportInputs2Xml,Export2XML,WriteParameterSet,WriteEntry,WriteParameter,WriteValue}` | ordered, duplicated selection, escaping, path error and export/import round trip. Writer helper methods depend on an active writer device/counter and are covered through `Export2XML`, not called out of precondition. |
| PARAM_009 | `UIDataManagementSetClass::{ImportFromXml,Export2Xml}` | caller result convention and forwarding of direct import/export behavior. |

## Observed contracts and defect candidates

- Both loader and exporter use `false` for success and `true` for error.
- The format persists only parameter ID and text value. Alias, Min/Max, type,
  state dependency and all other metadata are neither exported nor restored.
- Values are converted using Qt string conversion without `ok` checks: invalid
  doubles become `0`, oversized `uint8_t` input is narrowed, and `2` becomes
  boolean `true`. Min/Max is not enforced during import.
- A second `Value` in one entry wins; duplicate entries are processed in file
  order. Unmentioned containers retain old state on repeated loads.
- Unknown entries emit an info message; unknown XML elements/attributes are
  skipped. Empty documents are reported as errors by `QXmlStreamReader`.
- `readEntry` assumes a previously initialized container value. A blank or
  unsupported `InterfaceData` variant can throw from `SetDataKeepType`; this
  dangerous path is documented, not asserted as contract.

## Fresh verification evidence

On 2026-08-03, with the MSYS2 MINGW64 directories prepended to `PATH`, the
following fresh checks passed. The preceding `uic.exe`/`rcc.exe` failure was a
process-environment DLL conflict (a MiKTeX Qt 6.5 `Qt6Core.dll` preceded the
MSYS2 Qt 6.9 DLL), not an application or test-source failure.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-tests-msys2.ps1 -Clean -Jobs 4
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\build-msys2.ps1 -Configuration release -BuildDir build\phase3c-release -Clean -Jobs 4
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\build-msys2.ps1 -Configuration debug -BuildDir build\phase3c-debug -Clean -Jobs 4
```

The clean runner built and executed all four registered projects: Plot
Measurements (7 passed), DataManagement characterization (12 passed), XML
contract (10 passed), and Parameter contract (11 passed); each reported zero
failures. Both fresh production builds produced `LabAnalyser.exe` successfully.
Existing compiler warnings were unchanged.

The Parameter contract was also rebuilt with GCC `--coverage` and executed
successfully (11 passed, 0 failed). The following are per-production-file gcov
figures, not project-wide coverage and not a gate:

| Production file | Lines | Branches executed | Branches taken at least once |
| --- | ---: | ---: | ---: |
| `Import/parameterloader.cpp` | 100.00% (43/43) | 100.00% (104/104) | 61.54% (64/104) |
| `Export/exportinputs2xml.cpp` | 100.00% (39/39) | 100.00% (38/38) | 57.89% (22/38) |
| `DataManagement/UIDataManagementSetClass.cpp` | 34.09% (30/88) | 26.88% (50/186) | 14.52% (27/186) |

The UI manager figure intentionally includes its unrelated plugin, HDF5, MAT,
and experiment operations; only its XML import/export forwarding is in scope.
No blocker remains for the fresh local builds or coverage collection.
