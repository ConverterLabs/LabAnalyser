# HDF5 contracts — phase 3E

Date: 2026-08-04.  This phase characterizes the unchanged
`Export/Export2HighFive.cpp` production implementation with HighFive 2.10.1
and HDF5 1.14.6-3 from MSYS2 MINGW64.  No HDF5 fixture or output file is kept
in the repository.

## Scope and mapping

| Production surface | Characterization IDs | Result |
| --- | --- | --- |
| `Export2HDF5::Export(QString, QStringList)` | `HDF5_001`–`HDF5_005` | Executed |
| `Export2HDF5::Export(QString, QStringList)` through the observed UI error-return convention | `HDF5_006` | Executed through a test-only seam |
| `Export2HDF5::Export2HDF5(DataManagementClass *)` | `HDF5_001`–`HDF5_006` | Executed as construction prerequisite |
| `UIDataManagementSetClass::Export2Hdf5` | `HDF5_006` | Its catch-and-return convention is mirrored; the real widget graph is not built in this focused target |

`tests/contract/hdf5/Hdf5ExportContractTests.pro` deliberately includes only
the exporter, `DataManagementClass`, and `InterfaceDataType`, never
`LabAnalyser.pro`.  The `uiExportSeam` is compiled only into this test
executable.  It represents the production UI method's narrow observable
boundary—construct exporter, return the exporter result, catch exceptions and
return `false`—without bringing MainWindow, generated UI headers and unrelated
GUI/plot code into the HDF5 contract target.  It cannot be included or linked
by the production build.

## Observed file contract

Each call opens the target with HighFive `ReadWrite | Create | Truncate`.
Consequently a repeated export replaces the previous file contents.  The root
dataset `Timestamp` is a string beginning `Measurement_` followed by the local
date/time.  It is intentionally checked by prefix, not byte snapshot.

An ID has every `::` replaced by `/`.  Numeric containers are dumped as scalar
double-readable datasets.  A pair of `vector<double>` becomes the two
one-dimensional datasets `<id>/Time` and `<id>/Data`; unequal vector lengths
are accepted.  Strings, string lists and GUI selections are dumped as strings.
The observed UTF-8 test vector is readable through the independent public
HighFive read API.  No export attributes, compression, chunking, or explicit
dataset-layout property is configured by the production source, so none is a
tested compatibility promise.

The function returns `false` after a successful export.  An unknown ID is
ignored.  A non-creatable direct path throws from HighFive; the UI convention
represented by `HDF5_006` catches this and also returns `false`, making success
and this error indistinguishable by return value alone.

## Executed tests

| ID | Contract exercised |
| --- | --- |
| `HDF5_001` | Minimal temporary file, root timestamp and successful (`false`) return. |
| `HDF5_002` | Negative scalar, `uint8_t`, NaN and negative infinity values read back through HighFive. |
| `HDF5_003` | `::` path conversion, `Time`/`Data` vector datasets and unequal lengths. |
| `HDF5_004` | String input, the current Unicode test vector and empty vector datasets. |
| `HDF5_005` | Truncate/overwrite, ignored unknown ID and direct invalid-parent-path exception. |
| `HDF5_006` | Test-only UI error-convention seam: success and caught invalid-path failure both return `false`. |

All files are created below a `QTemporaryDir`; the test leaves no `.h5` or
`.hdf5` file outside ignored build directories after completion.

## Evidence

The clean six-project local runner, a fresh Release application build, and a
fresh Debug application build all completed with exit code 0 on 2026-08-04.
The instrumented HDF5 contract executable also completed with exit code 0.
Expected invalid-parent-path cases print an HDF5 diagnostic stack on stderr;
the assertions catch the corresponding exception and the Qt Test process exits
successfully.

| Production file | Lines | Branches executed | Branches taken at least once | Calls executed |
| --- | ---: | ---: | ---: | ---: |
| `Export/export2highfive.cpp` | 100.00% (26/26) | 92.59% (100/108) | 50.93% (55/108) | 84.00% (63/75) |

This is source-level gcov evidence for the named production file only, not
project coverage and not a coverage gate.

## Defect candidates and untested paths

- `Manager` is dereferenced without a null check; a null-manager call is an
  unsafe path and is deliberately not made a contract.
- The vector branch checks only the `Time` pointer before dereferencing the
  `Data` pointer.  A null `Data` pointer is similarly unsafe and untested.
- The UI method's `false` return is identical for normal completion and a
  caught export exception.
- Permission-denied targets and failures during allocation/HDF5 dataset creation
  were not deterministically fault-injected on this Windows host.
- Actual UI construction and real widget-selected IDs are not exercised by this
  focused exporter target; `HDF5_006` only covers its narrow error convention.
