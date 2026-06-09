# Backend Material Program Binary Dispatch Status

L36c0 adds a read-only PE dispatch evidence layer after L36b. It does not claim playable draw,
present, capture, or original-frame parity.

## Commands

```powershell
build\cmake-bt143\yuengine_cli.exe backend-material-program-binary-dispatch samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_material_program_binary_dispatch_contract
```

## Locked Metrics

`backend-material-program-binary-dispatch` currently reports:

```text
ok=true
binary_material_program_ok=true
original_binary_found=true
pe_image_ready=true
pe_section_table_ready=true
pe_pdata_ready=true
binary_string_direct_xref_probe_ready=true
selector_direct_xrefs_absent=true
selector_function_table_still_open=true
depth_adjacent_function_table_ready=true
rsm_depth_function_table_ready=true
sampleable_depth_dispatch_table_candidate_ready=true
d3d_import_table_ready=true
d3d_import_direct_calls_absent=true
d3d_dispatch_wrapper_still_open=true
exact_selector_control_flow_still_open=true
sampleable_depth_runtime_selection_still_open=true
pe_sections=8
pe_executable_sections=2
pdata_function_entries=20225
binary_probe_tokens=9
binary_probe_tokens_found=9
binary_string_direct_xref_hits=0
selector_direct_xref_hits=0
depth_packed_table_valid_pointers=24
depth_packed_table_text_pointers=20
depth_packed_table_pdata_resolved_functions=9
depth_packed_table_exact_function_starts=9
rsm_depth_table_valid_pointers=6
rsm_depth_table_text_pointers=5
rsm_depth_table_pdata_resolved_functions=5
rsm_depth_table_exact_function_starts=5
d3d_import_probe_records=5
d3d_imports_found=5
d3d_direct_iat_call_hits=0
resolved_binary_dispatch_contracts=10
tracked_binary_dispatch_obligations=5
open_binary_dispatch_obligations=5
```

## Verification

- Direct `backend-material-program-binary-dispatch`: passed, wall time about 54 seconds.
- `runtime-contract-suite --filter yuengine_backend_material_program_binary_dispatch_contract`:
  passed, elapsed_ms=53143.
- `tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck`: passed,
  elapsed_ms=48696.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: passed 44/44 contracts, runtime
  suite elapsed_ms=84995, wall time about 112 seconds.

## What This Proves

- `game.exe` is PE64 at image base `0x140000000`, with 8 sections and 2 executable sections.
- `.pdata` resolves 20225 x64 function ranges.
- The 9 shader/depth probe tokens have no direct VA/RVA/rel32 executable references.
- The deferred shader path selector strings also have 0 direct xrefs, so selector CFG is still not
  recovered.
- The packed `INTZRAWZDF24DF16` anchor is followed by 24 valid VA pointers; 20 point into `.text`,
  and 9 are exact `.pdata` function starts.
- The `cascade intz` anchor is followed by 6 valid VA pointers; 5 point into `.text`, and all 5 are
  exact `.pdata` function starts.
- `Direct3DCreate9Ex`, `D3DXCreateEffect`, `D3DXCreateEffectFromFileA`,
  `D3DXCreateTextureFromFileInMemory`, and `D3DXCreateTextureFromFileInMemoryEx` are present in the
  PE import table, but direct `FF 15`/`FF 25` IAT calls to those imports are absent.

## What It Does Not Prove

- It does not recover the material selector branch/control-flow.
- It does not name which `INTZ/RAWZ/DF24/DF16` path is selected for the current frame.
- It does not prove the D3D/D3DX wrapper/control-flow around effect or texture creation.
- It does not allow draw, present, capture, or original-frame oracle work.

## Next Edge

L36c remains active. The next work should recover actual control-flow around:

- the deferred/deferredGrass/deferredMulti shader path table;
- the adjacent depth function-table candidates;
- the D3D/D3DX effect/texture wrapper dispatch.

Only after selector CFG and sampleable-depth runtime selection are locked should L37 draw
submission/present/capture/oracle work start.

L36c1 now consumes this checkpoint through `backend-material-program-binary-function`: 14 exact
`.pdata` function bodies behind the depth/RSM dispatch-table candidates are fingerprinted. It still
leaves selector CFG, current-frame depth path, and D3D/D3DX wrapper dispatch open.
