# MAT export characterization (phase 3D)

`MatExportContractTests` compiles the unchanged qmake application graph without
`main.cpp` and exercises `MatExporter::{MatExporter,Export2Mat}` plus the real
`UIDataManagementSetClass::Export2Mat` caller. Files are reopened using public
libmatio APIs (`Mat_Open`, `Mat_VarRead`, struct-field access), not the exporter.

MSYS2 provides libmatio 1.5.28-1. The exporter creates uncompressed MATLAB MAT
v5 (`MAT_FT_MAT5`) files with `Timestamp` (UTF-8 character row) and
`ExportedChannels` (rank-2 N x 1 struct) fields `ID`, `Time`, `Data`.

| IDs | Characterized contract |
| --- | --- |
| MAT_001 | Null manager returns `true`, no file. |
| MAT_002 | Empty selection succeeds; MAT v5 Timestamp and 0x1 struct. |
| MAT_003 | All scalar numeric variants become double columns; NaN/Inf survive. |
| MAT_004 | DataPair time/data vectors are independent double columns; unequal/empty accepted. |
| MAT_005 | QString, QStringList first element, GuiSelection text export as UTF-8 characters. |
| MAT_006 | Repeat export overwrites; requested ID order is preserved. |
| MAT_007 | Missing parent fails; unknown ID reserves a row of empty double fields. |
| MAT_008 | UI caller preserves false-success/true-error convention. |

Defect candidates: all numeric classes are coerced to double; no matrix/rank-3
source exists; mismatched DataPair lengths are accepted; unknown IDs reserve
empty rows. Timestamp is time-dependent, therefore no byte snapshot or MAT
golden binary is used. No production source or libmatio version was changed.

## Executed verification

On 2026-08-04, `tests/run-tests-msys2.ps1 -Clean -Jobs 4` passed all five
registered projects. `MatExportContractTests` reported 10 passed, 0 failed.
Fresh Release and Debug production builds both completed successfully (exit
code 0). A GCC `--coverage` rebuild of this test executable also passed (exit
code 0), with these per-file gcov results (not project-wide coverage):

| Production file | Lines | Branches executed | Branches taken at least once |
| --- | ---: | ---: | ---: |
| `Export/Export2Mat.cpp` | 98.44% (63/64) | 97.50% (78/80) | 65.00% (52/80) |

`MatExporter::{MatExporter,WriteTimeStamp,ExportChannels,Export2Mat}` all have
executed lines. The sole unreached source line and two non-executed branches
belong to the defensive `Mat_VarCreate`/struct-allocation null paths; deterministic
allocation failure is not available without a fault-injection seam. `QTemporaryDir`
owns every test MAT path; after test process exit no `.mat` file exists in the
repository, and no MAT, executable, object, build, or coverage artifact is
tracked. `git diff --check` passed and the production-source diff is empty.
