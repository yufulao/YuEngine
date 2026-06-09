# L36b Backend Material Program Binary Evidence Status

L36b consumes the L36 material-program checkpoint and adds read-only evidence from the original
`bin/game.exe`. It does not execute draw calls. Its purpose is to stop treating the stage material
program and sampleable-depth path as resource-only questions.

## Contract

```text
project.json
-> L36 material program runtime
-> original bin/game.exe ASCII string table
-> mgRenderMesh shader path table
-> deferred/deferredGrass/deferredMulti paths backed by the original binary
-> SMAA depthTex2D / DepthEdgeDetection binary symbols
-> mgRenderPostFilter INTZ/RAWZ/DF24/DF16 packed depth candidates
-> mgRender / mgShaderRSM depth format labels
-> selector CFG and depth runtime selection remain tracked-open
-> draw/present/capture/oracle remain deferred
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-material-program-binary samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_material_program_binary_contract
tools\verify_runtime.ps1 -Mode edge -Jobs 8
```

Current metric:

```text
ok=true
material_program_ok=true
original_binary_found=true
binary_shader_path_table_ready=true
binary_deferred_selector_table_ready=true
selected_programs_backed_by_binary_table=true
binary_smaa_depth_technique_evidence_ready=true
binary_depth_postfilter_evidence_ready=true
binary_depth_format_table_evidence_ready=true
sampleable_depth_binary_candidate_ready=true
selector_control_flow_still_open=true
sampleable_depth_selection_still_open=true
downstream_draw_present_deferred=true
binary_shader_path_tokens=28
binary_filter_path_tokens=10
binary_deferred_selector_tokens=3
selected_program_binary_path_hits=3
render_mesh_source_markers=1
smaa_binary_depth_tokens=2
depth_texture_name_tokens=2
postfilter_source_markers=1
postfilter_depth_packed_format_hits=1
render_source_markers=6
render_depth_format_tokens=6
rsm_depth_binary_tokens=2
preserved_depth_texture_bindings=1
preserved_material_program_bindings=38
draw_present_capture_records_deferred=124
resolved_binary_material_program_contracts=9
tracked_binary_material_program_obligations=4
open_binary_material_program_obligations=4
```

Measured on 2026-06-09:

- Direct `backend-material-program-binary`: about 66.3 seconds.
- `runtime-contract-suite --filter yuengine_backend_material_program_binary_contract`:
  elapsed_ms=64216.
- `tools\verify_runtime.ps1 -Mode edge -NoBuild -SkipPython -SkipDiffCheck`: L36b edge filter,
  elapsed_ms=65678.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: clean build, Python unittest, 43/43
  contracts, and `git diff --check` passed; runtime suite elapsed_ms=107854, wall time about 140
  seconds.

## Contract State

- `binary_shader_path_table`: `contract_ready`, `game.exe` exposes 28 `system/shader` paths, 10
  `system/filter` paths, and the `mgRenderMesh.cpp` source marker.
- `binary_deferred_selector_table`: `contract_ready`, original binary evidence includes
  `system/shader/deferred.bfx`, `system/shader/deferredGrass.bfx`, and
  `system/shader/deferredMulti.bfx`.
- `selected_programs_backed_by_binary_table`: `contract_ready`, the three unique L36 selected
  programs are present in the original binary shader table.
- `binary_smaa_depth_technique_names`: `contract_ready`, `game.exe` contains `depthTex2D` and
  `DepthEdgeDetection`.
- `binary_postfilter_depth_texture_formats`: `contract_ready`, `mgRenderPostFilter` evidence
  contains packed `INTZ/RAWZ/DF24/DF16` depth texture candidates.
- `binary_render_depth_format_table`: `contract_ready`, `mgRender`/`mgShaderRSM` evidence exposes
  `D24S8`, `R32F`, `RAWZ`, `DF16`, `DF24`, `INTZ`, `cascade intz`, and `RSM DS`.
- `sampleable_depth_binary_candidate`: `contract_ready`, binary evidence proves candidates exist
  and supersedes the L36 resource-only negative evidence.
- `exact_selector_control_flow`: `tracked_open`, the shader path table is binary-backed but the
  selector branch/control-flow is not reconstructed yet.
- `sampleable_depth_runtime_selection`: `tracked_open`, format candidates exist but runtime
  selection/copy behavior is not reconstructed yet.
- `draw_present_capture_after_binary_evidence`: `tracked_open`.

## Boundary

L36b does not:

- claim the exact material selector CFG is recovered;
- choose INTZ/RAWZ/DF24/DF16 for the running frame;
- create or bind a sampleable depth texture;
- issue draw/present/capture/oracle calls.

## Next Edge

L36c should reconstruct the selector control-flow and the sampleable-depth runtime selection/copy
path from binary-level evidence before draw execution can start.

L36c0 now consumes this checkpoint through `backend-material-program-binary-dispatch`: PE64 layout,
`.pdata` function ranges, adjacent depth/RSM function-table candidates, and D3D/D3DX import probes
are locked. It still leaves selector CFG and current-frame depth selection open.
