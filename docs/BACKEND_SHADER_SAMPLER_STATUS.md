# L33 Backend Shader Sampler Status

L33 consumes the L32 surface/material/font checkpoint and recovers the shader sampler evidence that
can be proven from shipped `.bfx` files. It does not guess draw programs. It reflects CTAB data from
the recovered shader binaries, maps non-lightmap material texture roles to concrete `mesh30.bfx`
pixel-shader sampler registers, and keeps the remaining draw blockers explicit.

This is still not draw/present. Material program/pass selection, the lone lightmap slot, DX9 depth
texture sampling, font atlas construction, draw, present, capture, and oracle parity remain
downstream gates.

## Contract

```text
project.json
-> L32 backend surface/material/font runtime
-> 39 shipped .bfx files scanned for CTAB shader constant tables
-> mesh30.bfx ps_3_0 material sampler registers recovered
-> 38 non-lightmap material texture slots mapped to sampler registers
-> 1 lightmap slot preserved as tracked-open
-> material program/depth/font/draw/present/capture/oracle gates preserved for L34+
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-shader-sampler samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_shader_sampler_contract
ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure
tools\verify_runtime.ps1
```

Current metric:

```text
ok=true
surface_material_font_ok=true
material_semantics_ok=true
shader_ctab_reflection_ready=true
material_sampler_role_map_ready=true
lightmap_sampler_gate_tracked=true
material_program_selection_gate_tracked=true
depth_texture_sampler_gate_tracked=true
font_atlas_gate_tracked=true
downstream_draw_present_deferred=true
backbuffer_extent_carried=true
shader_files_scanned=39
shader_files_with_ctab=39
ctab_chunks=1105
unique_sampler_records=337
material_compatible_sampler_records=107
material_shader_files_with_samplers=24
material_texture_slots=39
resolved_material_sampler_slots=38
unresolved_material_sampler_slots=1
lightmap_texture_slots=1
material_texture_binding_records=38
preserved_material_program_bindings=38
preserved_depth_texture_bindings=1
font_atlas_placeholders=1
draw_present_capture_records_deferred=124
backbuffer_width=1280
backbuffer_height=720
resolved_shader_sampler_contracts=4
tracked_shader_sampler_obligations=6
open_shader_sampler_obligations=6
```

Measured on 2026-06-09:

- `ctest --test-dir build\cmake-bt143 -C Debug --output-on-failure`: 1/1 fast CTest passed in
  23.97-24.63 seconds.
- `tools\verify_runtime.ps1 -SkipPython -SkipDiffCheck -NoBuild`: 23.88 seconds.
- `tools\verify_runtime.ps1 -Mode full -SkipPython -SkipDiffCheck -NoBuild`: 39/39 contracts in
  51.78 seconds.
- `tools\verify_runtime.ps1 -Mode full -Jobs 8 -CleanBuild`: clean build, Python unittest, 39/39
  contracts, and `git diff --check` in 75.02 seconds.

## Contract State

- `bfx_ctab_sampler_reflection`: `contract_ready`, all 39 `.bfx` files expose CTAB chunks and the
  recovered sampler set includes 337 unique sampler records.
- `material_texture_role_sampler_map`: `contract_ready`, 38 non-lightmap slots map to
  `SamplerDiffuse`, `SamplerNormal`, `SamplerSpecular`, or `SamplerEmissive` in
  `system/shader/mesh30.bfx` `ps_3_0`.
- `material_program_selection_preserved`: `contract_ready`, 38 material texture bindings remain
  preserved while exact draw program/pass ownership is still a separate gate.
- `material_program_selection_binding`: `tracked_open`, model material blocks still do not prove
  the exact shader program/pass per draw.
- `lightmap_sampler_semantics`: `tracked_open`, 1 lightmap slot is present but sampler ownership is
  not proven.
- `smaa_depth_texture_sampler_binding`: `tracked_open`, `depthTex` is still backed by a D24S8
  depth/stencil surface, not a proven sampleable depth texture.
- `font_atlas_texture_implementation`: `tracked_open`, font atlas dimensions and glyph cache
  ownership remain unrecovered.
- `draw_present_capture_after_shader_sampler`: `tracked_open`, draw/present/capture remain blocked
  until material program, lightmap, depth, and font gates close.

## Next Edge

L34 must recover or explicitly block material program/pass selection, the lightmap sampler, depth
texture sampling, and font atlas/cache ownership before any draw execution is allowed.
