# Testing Runtime Status

Full CTest was too slow because every contract test starts a new CLI process and repeatedly rebuilds
the same runtime evidence chain. Running it sequentially is no longer the default workflow.

## Current Timing

Measured on the current `build\cmake-bt143` tree:

```text
ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
34/34 passed, 1124.39 seconds

ctest --test-dir build\cmake-bt143 -C Debug --parallel 8 --output-on-failure
34/34 passed, 358.12 seconds

tools\verify_runtime.ps1 -Mode edge -Filter yuengine_backend_device_adapter_contract -Jobs 8 -NoBuild
1/1 CTest passed plus Python unittest and git diff --check, 113.16 seconds for the CTest part
```

Parallel full CTest is about 3.1x faster. It should be the default full regression command.

## Standard Commands

Fast edge verification while developing one contract:

```powershell
tools\verify_runtime.ps1 -Mode edge -Filter yuengine_backend_device_adapter_contract -Jobs 8
```

Full checkpoint verification before commit:

```powershell
tools\verify_runtime.ps1 -Mode full -Jobs 8
```

Clean full checkpoint verification after C++ header or ABI changes:

```powershell
tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
```

## Policy

- Do not run bare full `ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure` as the
  default loop command.
- Use `--parallel 8` for full CTest on this machine unless a test becomes race-prone.
- Use `-Mode edge -Filter <test>` after narrow changes, then run full parallel verification before
  committing.
- Keep `git diff --check` in the standard verification path.
- Keep `--clean-first` only when C++ headers, ABI-like structs, or build system files changed.

## Remaining Performance Work

Parallelism reduces wall time but does not remove the repeated evidence-chain recomputation. A
future optimization should add a runtime evidence cache or a single-process aggregate contract
runner so backend contract tests can share parsed VFS/script/resource state.
