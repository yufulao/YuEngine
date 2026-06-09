# Testing Runtime Status

Full CTest used to be too slow because every contract test started a new CLI process and repeatedly
rebuilt the same runtime evidence chain. The default CTest registration now uses `FAST` mode as a
smoke check and registers only `yuengine_smoke_validate_touhou`. Current deepest runtime evidence
contracts are explicit `EDGE` checks. Full aggregate CTest and legacy per-contract CTest are
explicit configuration modes.

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

backend-surface-material-font direct CLI
1/1 deepest fast contract, 24.1-25.3 seconds

tools\verify_runtime.ps1 -SkipPython
fast contract plus git diff --check, about 25 seconds

Measured after adding `runtime-contract-suite`, default CTest suite registration, and public
scene-entry/scene-runtime report caches:

ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
1/1 yuengine_runtime_contract_suite passed, 50.12 seconds

build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root .
37/37 contracts passed, 50.41 seconds before L32

tools\verify_runtime.ps1 -Mode full -SkipPython -SkipDiffCheck -NoBuild
1/1 yuengine_runtime_contract_suite passed, 50.55 seconds

tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild
fast L31 backend-upload-bind contract, 24.9 seconds before L32

build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_surface_material_font_contract
1/1 L32 contract passed, 23.35-24.8 seconds

tools\verify_runtime.ps1 -Mode full -SkipPython -SkipDiffCheck -NoBuild
1/1 yuengine_runtime_contract_suite passed with 38 contracts, 50.89 seconds

Measured after L33 and CTest mode split:

ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
1/1 yuengine_current_backend_contract passed, 23.97-24.63 seconds

tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild
fast L33 yuengine_backend_shader_sampler_contract through runtime-contract-suite filter, 23.88 seconds

tools\verify_runtime.ps1 -Mode full -SkipPython -SkipDiffCheck -NoBuild
39/39 contracts through direct runtime-contract-suite CLI, 51.78 seconds

tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
clean build, Python unittest, 39/39 contracts, and git diff --check, 75.02 seconds

Measured after L34:

build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_program_depth_font_contract
1/1 L34 contract passed, 34.04 seconds

tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild
fast L34 contract through runtime-contract-suite filter, 33.73 seconds

ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
1/1 yuengine_current_backend_contract passed with L34 filter, 34.41 seconds

ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
1/1 yuengine_current_backend_contract passed with L34 filter, 34.08-34.35 seconds

cmake -S . -B build\cmake-ctest-guard -DYUENGINE_CTEST_MODE=LEGACY
legacy cache guard forced FAST because YUENGINE_ENABLE_LEGACY_CTESTS was not ON

tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
clean build, Python unittest, 40/40 contracts, and git diff --check passed; runtime suite
elapsed_ms=62881, wall time about 86.8 seconds

Measured after L35:

build\cmake-bt143\yuengine_cli.exe backend-font-atlas samples\touhou_new_world\project.json --repo-root .
L35 direct command passed, about 43.9 seconds

build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_font_atlas_contract
1/1 L35 contract passed, elapsed_ms=43883

tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild
pre-smoke-split fast L35 contract through runtime-contract-suite filter, elapsed_ms=43247

ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
1/1 yuengine_current_backend_contract passed with L35 filter before the smoke split, 43.94 seconds

tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
clean build, Python unittest, 41/41 contracts, and git diff --check passed; runtime suite
elapsed_ms=69923, wall time about 93.6 seconds
```

Measured after L36:

```text
tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
clean build, Python unittest, 42/42 contracts, and git diff --check passed; runtime suite
elapsed_ms=114358, wall time about 138 seconds
```

Measured after splitting default smoke from the current deepest edge:

```text
build\cmake-bt143\yuengine_cli.exe validate samples\touhou_new_world\project.json
project manifest validate, about 0.04 seconds

ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
1/1 yuengine_smoke_validate_touhou passed; CTest total 0.01 seconds, outer measurement about 0.06
seconds

tools\verify_runtime.ps1 -NoBuild
smoke validate plus Python unittest and git diff --check, about 0.354 seconds

tools\verify_runtime.ps1 -Mode edge -Jobs 8
current deepest L36c0 edge; backend-material-program-binary-dispatch runtime-contract-suite filter

tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck
previous L36 edge, elapsed_ms=42281

build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_material_program_binary_contract
L36b edge, elapsed_ms=64216

tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck
L36b edge, elapsed_ms=65678

build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_material_program_binary_dispatch_contract
current L36c0 edge, elapsed_ms=53143

tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck
current L36c0 edge, elapsed_ms=48696

tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
current L36 clean full, 42/42 contracts, runtime suite elapsed_ms=114358, wall time about 138
seconds

tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
current L36b clean full, 43/43 contracts, runtime suite elapsed_ms=107854, wall time about 140
seconds

tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
current L36c0 clean full, 44/44 contracts, runtime suite elapsed_ms=84995, wall time about 112
seconds
```

Default bare CTest is now acceptable for an edit-loop check because it runs only the smoke validate.
Current-edge and full regressions remain available through explicit `tools\verify_runtime.ps1`
modes.

## Standard Commands

Default smoke verification while editing:

```powershell
tools\verify_runtime.ps1
```

Filtered edge verification when a named contract must be exercised:

```powershell
tools\verify_runtime.ps1 -Mode edge -Filter yuengine_backend_upload_binding_contract -Jobs 8
```

Current deepest edge example:

```powershell
tools\verify_runtime.ps1 -Mode edge -Jobs 8
tools\verify_runtime.ps1 -Mode edge -Filter yuengine_backend_material_program_binary_dispatch_contract -Jobs 8
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
  `yuengine_smoke_validate_touhou`.
- The default loop command is `tools\verify_runtime.ps1`, which runs smoke validate and
  `git diff --check`.
- Use `-Mode edge` for the current deepest contract and `-Mode edge -Filter <test>` after narrow
  changes; this calls
  `yuengine_cli runtime-contract-suite --filter <test>` instead of starting legacy CTest cases.
- Use `-Mode full` before committing major checkpoints; this runs direct
  `yuengine_cli runtime-contract-suite` and avoids CTest process fan-out.
- Use EDGE CTest only when CTest integration itself must be checked for the current deepest
  contract:
  `cmake -S . -B build\cmake-bt143 -DYUENGINE_ENABLE_SLOW_CTESTS=ON -DYUENGINE_CTEST_MODE=EDGE`.
- Use full aggregate CTest only when CTest integration itself must be checked:
  `cmake -S . -B build\cmake-bt143 -DYUENGINE_ENABLE_SLOW_CTESTS=ON -DYUENGINE_CTEST_MODE=FULL`.
- Enable older per-contract CTest cases only when diagnosing one of those cases. It requires both
  slow-test and legacy switches so stale cache values cannot accidentally re-enable the one-hour
  path:
  `cmake -S . -B build\cmake-bt143 -DYUENGINE_ENABLE_SLOW_CTESTS=ON -DYUENGINE_CTEST_MODE=LEGACY -DYUENGINE_ENABLE_LEGACY_CTESTS=ON`.
- Keep `git diff --check` in the standard verification path.
- Keep `--clean-first` only when C++ headers, ABI-like structs, or build system files changed.

## Remaining Performance Work

The smoke CTest mode removes the one-hour edit-loop path. Remaining performance work is to share the
boot, resource, and title script-run reports inside the suite so full regression can approach the
current deepest-contract cost instead of the full-suite cost.
