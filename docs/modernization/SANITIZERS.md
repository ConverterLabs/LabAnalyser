# Sanitizer pilot (MSYS2 MINGW64)

## Scope and command

The pilot is intentionally limited to the existing focused contract targets:
DataManagement, experiment XML, parameter import, MAT export, HDF5 export,
Remote Control and Plugin Loader. It does not build the full application, alter
production sources, or create a CI gate.

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tests\run-sanitizer-pilot-msys2.ps1 -Clean -Jobs 4
```

When the required runtime is present, the script configures each target with
`-fsanitize=address,undefined`, `-fno-omit-frame-pointer` and debug symbols;
it runs with `ASAN_OPTIONS=detect_leaks=1:halt_on_error=1:abort_on_error=1`
and `UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1`. A missing runtime is a
hard preflight failure, preventing an uninstrumented fallback.

## 2026-08-10 toolchain evidence

The installed MSYS2 MINGW64 `g++.exe` is GCC 15.2.0 (package
`mingw-w64-x86_64-gcc 15.2.0-8`). It accepts the compiler options, but separate
minimal ASan, UBSan and combined ASan+UBSan link probes each failed with exit
code 1:

```text
ld.exe: cannot find -lasan: No such file or directory
ld.exe: cannot find -lubsan: No such file or directory
```

ASan alone cannot resolve `-lasan`; UBSan alone cannot resolve `-lubsan`; the
combined form cannot resolve either. `g++ -print-file-name=libasan.dll` and
`libubsan.dll` returned unresolved names,
and neither runtime DLL exists under the installed MINGW64 tree. The local
package database shows `mingw-w64-x86_64-gcc` and `gcc-libs` installed, but no
installed sanitizer-runtime package. File-database lookups cannot identify a
provider because the local pacman file databases are absent; refreshing them or
installing packages was deliberately not performed during this pilot.

Therefore **no focused project test was compiled or run under ASan/UBSan**.
There are no project, third-party, leak, or undefined-behavior findings from
this pilot; the only demonstrated result is a sanitizer/toolchain limitation.
This must not be interpreted as a clean sanitizer result.

## Prioritized follow-up

1. Identify and install, with explicit approval, matching MSYS2 MINGW64 GCC
   ASan and UBSan runtimes (or select a documented compatible toolchain).
2. Re-run the unchanged focused script and classify each report as project,
   third-party, or Qt/toolchain behavior before any source fix.
3. Assess LeakSanitizer separately: Windows/MinGW support and Qt shutdown
   behavior must be demonstrated rather than assumed.
4. Only after reproducible clean pilot evidence, decide whether a non-blocking
   CI reporting job is useful; no mandatory sanitizer gate is enabled now.
