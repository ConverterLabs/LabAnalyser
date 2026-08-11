# Static-analysis baseline (MSYS2 MINGW64)

## Reproducible pilot

Run the dedicated warning build from the repository root:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-static-analysis-msys2.ps1 -Clean -Jobs 4
```

It configures `StaticAnalysis.pro` in
`build/static-analysis-msys2-mingw64`. This test-only qmake wrapper includes
the unchanged `LabAnalyser.pro`, then removes its `-Werror=return-type` flag
for the isolated analysis target. The pilot deliberately adds no `-Werror`
flag:

```text
-Wall -Wextra -Wpedantic -Wformat=2 -Wshadow
```

`-Wformat=2` and `-Wshadow` are useful GCC checks for this Qt build and produced
actionable source locations without changing language or ABI settings.
`-Wconversion` is intentionally not enabled in this first baseline: Qt and
legacy numeric adapter boundaries would make it a much noisier follow-up than
the compiler-supported sign-comparison diagnostics already captured here.

The script records `tool-inventory.txt`, the complete compiler transcript, and
`own-production-warnings.txt` under its ignored build directory. The reviewed
filter resolves paths against the repository and excludes vendored
`DropWidgets/Plots/qcustomplot.*`, generated `moc_*`/`qrc_*`/`ui_*` sources,
`tests/**`, and all build-directory outputs. It does not rewrite or suppress
production diagnostics.

## Tool inventory on 2026-08-10

| Tool | Result |
| --- | --- |
| GCC diagnostics | Available: MSYS2 MINGW64 GCC 15.2.0; fresh Release warning build passed (exit 0). |
| clang++ | Available: 21.1.1; not used as a replacement compiler or analyzer because the qmake build has no reviewed `compile_commands.json`. |
| clang-tidy | Not installed. |
| cppcheck | Not installed. |

No package was installed or updated. Therefore no clang-tidy or cppcheck result
is claimed. The fresh GCC build is the complete available analysis result for
this pilot.

## Initial baseline findings

The filtered report contains 163 distinct diagnostic lines. This is a compiler
source-line baseline; repeated inclusion of `DropWidget.h` through different
translation-unit-relative spellings remains visible and should not be treated
as 163 independent defects.

| Classification | Count | Evidence and assessment |
| --- | ---: | --- |
| Potential correctness / memory | 1 | `DropWidgets/QSlider.cpp:155`: `value` may be used uninitialized. The code assigns it only for three recognized `InterfaceData` numeric categories before computing a scaled slider value. This is a high-priority defect candidate, not a confirmed memory error; unsupported data categories must be characterized before a fix. |
| Ownership / lifetime | 0 | No direct GCC warning. Compiler diagnostics cannot establish QObject ownership safety. |
| Null / index risk | 0 direct | No direct compiler diagnostic. `RemoteControlServer.cpp` has seven signed/unsigned size comparisons near framing/index logic; existing dangerous buffer-cast paths remain a separately documented protocol risk, not a new proven analyzer finding. |
| Implicit conversion / bounds | 23 | `-Wsign-compare`: PlotWidget 7, RemoteControlServer 7, DataManagementClass 5, QTableWidgeD 2, DataManagementSetClass 1, XML writer 1. These are review candidates for negative/large-value boundary behavior, not automatic defects. |
| Deprecated Qt API | 35 | PlotWidget 34 and QTableWidgeD 1. Qt 6 deprecation warnings are modernization candidates; current behavior remains unchanged. |
| Style / likely harmless | 104 | 77 unused parameters, 6 unused variables, 3 unused-but-set variables, 18 shadowing diagnostics. Most occur in widget adapter override surfaces; no blanket suppression or cleanup was made. |
| Possible false positive | 1 included above | The QSlider uninitialized report can be compiler-conservative, but code inspection shows a real unassigned path unless the caller contract forbids all other types. Treat it as a characterization/security candidate until proven. |

The highest-priority review order was: (1) QSlider's nonnumeric input path;
(2) RemoteControl framing comparisons together with existing unsafe cast/index
risks; (3) numeric adapter bounds; (4) Qt 6 API migration only after contracts
are protected. Ownership and lifetime need sanitizer/runtime support or focused
behavioral tests, not this warning baseline.

## QSlider correctness fix verification (2026-08-10)

The initial `-Wmaybe-uninitialized` diagnostic was safely characterized without
executing its unassigned-value path: an editable `QString`, `QStringList` or
`GuiSelection` reaches `QSliderD::SetVariantData()` without selecting any of
its three numeric assignments. `DW_016` now exercises those three real
`InterfaceData` variants against a connected slider with a valid 0..100
manager range. Each leaves its value at 37, emits no `valueChanged`, and leaves
the existing unblocked signal state usable for a subsequent normal set.

The approved minimal production change records whether a floating-point,
signed, or unsigned branch assigned `value` and performs the existing scaling
only in that case. It changes neither accepted numeric conversion nor the
existing signal-blocking convention. A fresh run of the same scoped GCC pilot
completed with exit code 0 and 162 filtered own-production diagnostic lines:
`-Wmaybe-uninitialized` is now 0. The other category counts are unchanged
(77 unused parameters, 35 deprecated APIs, 23 sign comparisons, 18 shadowing,
6 unused variables and 3 unused-but-set variables); no warning suppression or
filter change was used.

`(MinMax.second - MinMax.first)` remains deliberately unchanged. A zero range
can still make the numeric scaling division invalid; it is a separate
high-priority QSlider defect candidate and is not covered by this fix.

## QSlider equal-bound hardening (2026-08-11)

The explicitly approved follow-up adds an exact `MinMax.first !=
MinMax.second` guard immediately before the existing scaling division in
`QSliderD::SetVariantData()`. This preserves the current slider value and
suppresses programmatic value changes for equal bounds, while leaving every
non-equal range on the prior calculation path. `DW_018` covers floating,
signed and unsigned input with a product-like manager binding and reconfirms
the existing `7.5 -> 8` rounding vector in a `0..100` range. The unsafe
pre-fix division/cast was not executed as a test contract.

## Current 161-diagnostic count after MessageDispatchPolicy extraction

The scoped GCC report after Phase 4E.2 contains 161 filtered own-production
diagnostic lines, down from 162. This is not caused by a filter, compiler-flag
or compilation-coverage change: the extraction renamed the local
`InterfaceData Data` in `DataMessengerClass::MessageReceiver`'s `CloseProject`
branch to `closeData`, removing that one real `-Wshadow` diagnostic against the
method parameter. The branch remains compiled and behaviorally covered by
`DM_MSG_001..DM_MSG_003`; no warning was hidden by moving code out of the build.
All other recorded category counts are unchanged: 23 sign comparisons, 35 Qt
deprecations, 77 unused parameters, 6 unused variables, 3 unused-but-set
variables and 17 remaining shadowing diagnostics; `-Wmaybe-uninitialized`
remains zero after the separately approved QSlider fix.

## Gates and limits

This pilot adds no CI job and no mandatory warning, static-analysis, or quality
gate. It is a reproducible starting point only. Before any gate, obtain a
reviewed clang-tidy/cppcheck setup, decide the intended treatment of existing
warnings, and separate production correctness fixes from mechanical cleanup.
