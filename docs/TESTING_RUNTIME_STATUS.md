# Testing Runtime Status

Full CTest used to be too slow because every contract test started a new CLI process and repeatedly
rebuilt the same runtime evidence chain. The default CTest registration now uses one single-process
aggregate runner, `yuengine_runtime_contract_suite`, and the older per-contract CTest cases are
opt-in through `YUENGINE_ENABLE_LEGACY_CTESTS=ON`.

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

Measured after adding `runtime-contract-suite`, default CTest suite registration, and public
scene-entry/scene-runtime report caches:

ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
1/1 yuengine_runtime_contract_suite passed, 50.12 seconds

build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root .
37/37 contracts passed, 50.41 seconds

tools\verify_runtime.ps1 -Mode full -SkipPython -SkipDiffCheck -NoBuild
1/1 yuengine_runtime_contract_suite passed, 50.55 seconds

tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild
fast L31 backend-upload-bind contract, 24.9 seconds
```

Default bare CTest is now acceptable for a checkpoint because it runs only the aggregate suite.
The fast script remains the edit-loop command.

## Standard Commands

Default fast verification while developing the current deepest contract:

```powershell
tools\verify_runtime.ps1
```

Filtered edge verification when a named contract must be exercised:

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

- Default bare `ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure` now runs only
  `yuengine_runtime_contract_suite`.
- The default loop command is `tools\verify_runtime.ps1`, which runs the current fast backend
  contract and `git diff --check`.
- Use `-Mode edge -Filter <test>` after narrow changes; this calls
  `yuengine_cli runtime-contract-suite --filter <test>` instead of starting legacy CTest cases.
- Use `-Mode full` before committing major checkpoints; this runs the aggregate CTest suite.
- Enable older per-contract CTest cases only when diagnosing one of those cases:
  `cmake -S . -B build\cmake-bt143 -DYUENGINE_ENABLE_LEGACY_CTESTS=ON`.
- Keep `git diff --check` in the standard verification path.
- Keep `--clean-first` only when C++ headers, ABI-like structs, or build system files changed.

## Remaining Performance Work

The aggregate runner removes the one-hour path. Remaining performance work is to share the boot,
resource, and title script-run reports inside the suite so full regression can approach the current
24-25 second deepest-contract cost instead of about 50 seconds.
