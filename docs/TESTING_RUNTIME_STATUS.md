# Testing Runtime Status

Full CTest used to be too slow because every contract test started a new CLI process and repeatedly
rebuilt the same runtime evidence chain. The default CTest registration now uses `FAST` mode and
registers only the current deepest contract through the single-process `runtime-contract-suite`
filter. Full aggregate CTest and legacy per-contract CTest are explicit configuration modes.

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
fast L35 contract through runtime-contract-suite filter, elapsed_ms=43247

ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
1/1 yuengine_current_backend_contract passed with L35 filter, 43.94 seconds

tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
clean build, Python unittest, 41/41 contracts, and git diff --check passed; runtime suite
elapsed_ms=69923, wall time about 93.6 seconds
```

Default bare CTest is now acceptable for an edit-loop check because it runs only the current fast
contract. Full regression remains available through `tools\verify_runtime.ps1 -Mode full`.

## Standard Commands

Default fast verification while developing the current deepest contract:

```powershell
tools\verify_runtime.ps1
```

Filtered edge verification when a named contract must be exercised:

```powershell
tools\verify_runtime.ps1 -Mode edge -Filter yuengine_backend_upload_binding_contract -Jobs 8
```

Current deepest edge example:

```powershell
tools\verify_runtime.ps1 -Mode edge -Filter yuengine_backend_font_atlas_contract -Jobs 8
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
  `yuengine_current_backend_contract`, currently filtered to
  `yuengine_backend_font_atlas_contract`.
- The default loop command is `tools\verify_runtime.ps1`, which runs the current fast backend
  contract and `git diff --check`.
- Use `-Mode edge -Filter <test>` after narrow changes; this calls
  `yuengine_cli runtime-contract-suite --filter <test>` instead of starting legacy CTest cases.
- Use `-Mode full` before committing major checkpoints; this runs direct
  `yuengine_cli runtime-contract-suite` and avoids CTest process fan-out.
- Use full aggregate CTest only when CTest integration itself must be checked:
  `cmake -S . -B build\cmake-bt143 -DYUENGINE_CTEST_MODE=FULL`.
- Enable older per-contract CTest cases only when diagnosing one of those cases. It requires both
  switches so stale cache values cannot accidentally re-enable the one-hour path:
  `cmake -S . -B build\cmake-bt143 -DYUENGINE_CTEST_MODE=LEGACY -DYUENGINE_ENABLE_LEGACY_CTESTS=ON`.
- Keep `git diff --check` in the standard verification path.
- Keep `--clean-first` only when C++ headers, ABI-like structs, or build system files changed.

## Remaining Performance Work

The fast CTest mode removes the one-hour edit-loop path. Remaining performance work is to share the
boot, resource, and title script-run reports inside the suite so full regression can approach the
current 44 second deepest-contract cost instead of the full-suite cost.
