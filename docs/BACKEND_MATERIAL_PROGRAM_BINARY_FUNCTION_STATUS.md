# Backend Material Program Binary Function Status

L36c1 consumes L36c0 and locks the exact function bodies behind the binary dispatch-table
candidates. It is still not a selector CFG or playable draw checkpoint.

## Commands

```powershell
build\cmake-bt143\yuengine_cli.exe backend-material-program-binary-function samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_material_program_binary_function_contract
tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck
tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild
```

## Locked Metrics

```text
ok=true
binary_dispatch_ok=true
original_binary_found=true
function_fingerprint_ready=true
depth_function_fingerprint_ready=true
rsm_depth_function_fingerprint_ready=true
function_body_format_immediate_probe_ready=true
direct_function_call_xref_probe_ready=true
selector_control_flow_still_open=true
sampleable_depth_runtime_selection_still_open=true
d3d_dispatch_wrapper_still_open=true
function_fingerprint_records=14
depth_function_fingerprint_records=9
rsm_depth_function_fingerprint_records=5
unique_function_fingerprints=14
function_bytes_total=3779
depth_function_bytes=2042
rsm_depth_function_bytes=1737
function_body_depth_format_immediate_hits=0
direct_function_call_rel32_xrefs=0
resolved_binary_function_contracts=7
tracked_binary_function_obligations=5
open_binary_function_obligations=5
```

## What This Proves

- The L36c0 adjacent dispatch-table candidates contain 14 exact `.pdata` function starts.
- Those 14 function bodies total 3779 bytes and have 14 unique FNV-1a 64-bit fingerprints.
- The packed `INTZ/RAWZ/DF24/DF16` side contributes 9 exact function-body fingerprints.
- The `cascade intz` side contributes 5 exact function-body fingerprints.
- Exact candidate function bodies contain no direct `INTZ`, `RAWZ`, `DF24`, `DF16`,
  `D3DFMT_D24S8`, or `D3DFMT_R32F` immediate constants.
- No direct `E8 rel32` call targets those exact function starts from executable sections.

## What It Does Not Prove

- It does not recover material selector CFG.
- It does not recover the selected frame depth format/copy/resolve path.
- It does not recover D3D/D3DX effect/texture wrapper dispatch.
- It does not unlock draw, present, capture, or original-frame oracle.

## Verification

- Direct `backend-material-program-binary-function`: passed.
- `runtime-contract-suite --filter yuengine_backend_material_program_binary_function_contract`:
  passed, elapsed_ms=58799.
- `tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck`: passed,
  elapsed_ms=73413.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: passed 45/45 contracts, runtime
  suite elapsed_ms=133847, wall time about 160 seconds.
- `tools\verify_runtime.ps1 -NoBuild`: passed smoke validate, Python unittest, and
  `git diff --check`.
- `ctest --test-dir build\cmake-bt143 -C Debug -N`: default FAST CTest registers only
  `yuengine_smoke_validate_touhou`.

## Next Edge

L36c remains active. The next layer should recover wrapper/control-flow around the locked function
bodies, likely by introducing a real instruction decoder or a dynamic D3D9/D3DX trace path rather
than treating byte fingerprints as source-level recovery.
