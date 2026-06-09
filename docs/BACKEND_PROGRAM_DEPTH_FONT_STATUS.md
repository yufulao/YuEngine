# L34 Backend Program, Depth, And Font Status

L34 consumes the L33 shader sampler checkpoint and turns the next renderer blockers into explicit
runtime evidence. It does not choose a guessed mesh draw program and it does not execute draw. The
stage materials still do not carry an exact shader file, technique, or pass token, so program
selection remains blocked. The useful progress is that YuEngine now inventories the recovered
material shader candidate families, proves the light/depth sampler token evidence, and resolves the
font atlas resource ownership from real FMP/DDS files.

## Contract

```text
project.json
-> L33 shader sampler runtime
-> L22 backend state/font inputs
-> L19 material semantics
-> 12 mesh/grass/deferred shader program candidate files
-> 0 material program/pass tokens in parsed material records
-> 3 SamplerLight records and 33 depth sampler records
-> 4 FMP font maps referencing 7 atlas DDS files
-> 7 4096x4096 two-mip D3DFMT_A8 atlas DDS payloads
-> draw/present/capture/oracle remain deferred
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-program-depth-font samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_program_depth_font_contract
tools\verify_runtime.ps1
```

Current metric:

```text
ok=true
shader_sampler_ok=true
backend_state_ok=true
material_semantics_ok=true
shader_program_candidate_evidence_ready=true
material_program_selection_gap_tracked=true
lightmap_sampler_token_evidence_ready=true
lightmap_material_binding_gate_tracked=true
depth_sampler_token_evidence_ready=true
sampleable_depth_texture_gate_tracked=true
font_atlas_resource_evidence_ready=true
font_atlas_texture_implementation_gate_tracked=true
downstream_draw_present_deferred=true
backbuffer_extent_carried=true
shader_program_candidate_files=12
mesh_shader_candidate_files=4
grass_shader_candidate_files=5
deferred_shader_candidate_files=3
material_program_token_hits=0
materials=16
material_texture_slots=39
named_mesh_material_bindings=110
unresolved_mesh_material_bindings=1
lightmap_texture_slots=1
sampler_light_records=3
sampler_depth_records=33
sampler_depth_shader_files=17
font_map_files=4
font_atlas_links=7
font_atlas_dds_ready=7
font_atlas_4096=7
font_atlas_a8=7
font_atlas_mip2=7
font_atlas_payload_matches=7
font_query_records=6
text_draw_commands=6
string_size_queries=5
preserved_depth_texture_bindings=1
preserved_material_program_bindings=38
draw_present_capture_records_deferred=124
backbuffer_width=1280
backbuffer_height=720
resolved_program_depth_font_contracts=5
tracked_program_depth_font_obligations=6
open_program_depth_font_obligations=6
```

Measured on 2026-06-09:

- Direct `backend-program-depth-font`: about 34 seconds.
- `runtime-contract-suite --filter yuengine_backend_program_depth_font_contract`: 34.04 seconds.
- `tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild`: 33.73 seconds.
- `ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure`: 34.08-34.41 seconds.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: 40/40 contracts, runtime suite
  elapsed_ms=62881, wall time about 86.8 seconds.

## Contract State

- `shader_program_candidate_inventory`: `contract_ready`, 12 recovered shader candidate files expose
  material sampler records while parsed material blocks expose 0 program/pass tokens.
- `lightmap_sampler_token_evidence`: `contract_ready`, deferred shader candidates expose 3
  `SamplerLight` records for the lone lightmap material slot.
- `depth_sampler_token_evidence`: `contract_ready`, recovered shader/state evidence exposes 33
  depth sampler records across 17 shader files.
- `font_atlas_resource_evidence`: `contract_ready`, 4 FMP font maps reference 7 atlas DDS payloads;
  every atlas is 4096x4096, two mip levels, `D3DFMT_A8`, and payload size matches.
- `exact_material_program_selection_binding`: `tracked_open`, scene material blocks still do not
  choose an exact shader file, technique, or pass.
- `lightmap_material_program_binding`: `tracked_open`, `SamplerLight` exists but is not bound to
  the `ki:tree` lightmap slot through a proven program/pass.
- `sampleable_depth_texture_implementation`: `tracked_open`, `depthTex` is still a preserved D24S8
  depth/stencil surface, not a proven sampleable texture implementation.
- `font_atlas_texture_upload_and_glyph_layout`: `tracked_open`, atlas ownership is known, but the
  FMP glyph record layout and D3D texture upload path are not implemented yet.
- `draw_present_capture_after_program_depth_font`: `tracked_open`, draw/present/capture remain
  blocked until program, lightmap, depth, and font gates close.

## Boundary

L34 does not:

- choose `mesh30`, `deferred`, or any other shader program for scene mesh draws;
- bind the lightmap material slot to `SamplerLight`;
- sample from the D24S8 depth/stencil surface;
- create, upload, or bind font atlas textures;
- decode the FMP glyph record stride;
- issue draw calls, call `Present`, capture frames, or run oracle comparison.

## Next Edge

L35 should use the new FMP/DDS evidence to implement font atlas texture creation/upload and start
recovering the FMP glyph record layout. Material program selection and sampleable depth remain
separate blockers unless stronger original evidence is found.
