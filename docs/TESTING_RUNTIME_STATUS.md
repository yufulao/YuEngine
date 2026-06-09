# Testing Runtime Status

Full CTest was too slow because every contract test starts a new CLI process and repeatedly rebuilds
the same runtime evidence chain. Running it sequentially is no longer the default workflow, and the
default verification script no longer runs full CTest.

## Current Timing

Baseline measured before the cache/reuse fix:

```text
ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
34/34 passed, 1124.39 seconds

ctest --test-dir build\cmake-bt143 -C Debug --parallel 8 --output-on-failure
34/34 passed, 358.12 seconds

tools\verify_runtime.ps1 -Mode edge -Filter yuengine_backend_device_adapter_contract -Jobs 8 -NoBuild
1/1 CTest passed plus Python unittest and git diff --check, 113.16 seconds for the CTest part

Measured after adding in-process runtime report caches, scene-runtime reuse for mission reports,
and cached `.sqasm` module loading:

backend-device-adapter direct CLI
113.06 seconds -> 21.46 seconds

ctest --test-dir build\cmake-bt143 -C Debug --parallel 8 -R yuengine_backend_device_creation_contract --output-on-failure
1/1 passed, 21.88 seconds

backend-resource-create direct CLI
1/1 deepest fast contract, 22.8 seconds

backend-upload-bind direct CLI
1/1 deepest fast contract, 24.8 seconds

tools\verify_runtime.ps1 -SkipPython
fast contract plus git diff --check, about 25 seconds
```

Parallel full CTest is still the full regression command, but it is not the default edit-loop
command.

## Standard Commands

Default fast verification while developing the current deepest contract:

```powershell
tools\verify_runtime.ps1
```

Filtered CTest edge verification when a named test must be exercised:

```powershell
tools\verify_runtime.ps1 -Mode edge -Filter yuengine_backend_upload_binding_contract -Jobs 8
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
- The default loop command is `tools\verify_runtime.ps1`, which runs the current fast backend
  contract and `git diff --check`.
- Use `--parallel 8` for full CTest on this machine unless a test becomes race-prone.
- Use `-Mode edge -Filter <test>` after narrow changes, then run full parallel verification before
  committing.
- Keep `git diff --check` in the standard verification path.
- Keep `--clean-first` only when C++ headers, ABI-like structs, or build system files changed.

## Remaining Performance Work

The current cache removes the largest repeated in-process runtime construction cost. Remaining
performance work is to add a single-process aggregate CTest runner so full regression does not
start 35 separate CLI processes.
