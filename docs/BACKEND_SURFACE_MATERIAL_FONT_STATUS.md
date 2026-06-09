# L32 Backend Surface, Material, And Font Status

L32 consumes the L31 upload/binding checkpoint and closes the next backend edge that can be proven
without inventing renderer behavior. On the current Windows test machine, YuEngine creates the SMAA
transient render-target/depth resources against a real persistent D3D9 device and binds the
render-target textures that are legal DX9 sampler inputs.

This is still not draw/present. L32 deliberately keeps material shader slot ownership, the DX9 depth
texture sampler, font atlas construction, draw, present, capture, and original-frame oracle parity as
explicit downstream gates.

## Contract

```text
project.json
-> L31 upload/binding runtime
-> persistent hidden HWND / D3D9 device service
-> 3 SMAA D3DFMT_X8R8G8B8 render-target textures with retained surface levels
-> 1 SMAA D3DFMT_D24S8 depth/stencil surface
-> 4 transient SetTexture calls for colorTex/colorTexG/edgesTex/blendTex
-> material/font/depth/draw/present/capture/oracle gates preserved for L33+
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-surface-material-font samples\touhou_new_world\project.json --repo-root .
build\cmake-bt143\yuengine_cli.exe runtime-contract-suite samples\touhou_new_world\project.json --repo-root . --filter yuengine_backend_surface_material_font_contract
tools\verify_runtime.ps1
```

Current metric:

```text
ok=true
upload_binding_ok=true
resource_creation_ok=true
device_execution_ok=true
backend_state_ok=true
surface_material_font_runtime_ready=true
persistent_device_service_ready=true
base_resource_handles_ready=true
transient_surface_creation_executed=true
transient_surface_binding_executed=true
material_shader_evidence_tracked=true
material_shader_slot_binding_deferred=true
font_atlas_evidence_tracked=true
font_atlas_creation_deferred=true
downstream_draw_present_deferred=true
backbuffer_extent_carried=true
base_resource_handles_created=41
source_surface_records=4
real_transient_surfaces_created=4
render_target_surfaces_created=3
depth_stencil_surfaces_created=1
transient_texture_bindings=5
executed_transient_texture_bindings=4
preserved_depth_texture_bindings=1
material_texture_binding_records=38
preserved_material_texture_bindings=38
material_shader_evidence_files=10
material_shader_sampler_tokens=845
font_atlas_placeholders=1
font_query_records=6
text_draw_commands=6
string_size_queries=5
draw_present_capture_records_deferred=124
resolved_surface_material_font_contracts=6
tracked_surface_material_font_obligations=5
open_surface_material_font_obligations=5
```

## Contract State

- `persistent_device_service_for_surfaces`: `contract_ready`, L32 uses a persistent hidden
  HWND/D3D9 device service after L31 upload/binding readiness.
- `smaa_render_target_surface_creation`: `contract_ready`, `colorTex2D`, `edgesTex2D`, and
  `blendTex2D` create 1280x720 `D3DFMT_X8R8G8B8` render-target textures.
- `smaa_depth_stencil_surface_creation`: `contract_ready`, `depthTex2D` creates a 1280x720
  `D3DFMT_D24S8` depth/stencil surface.
- `smaa_transient_render_target_binding`: `contract_ready`, `colorTex`, `colorTexG`, `edgesTex`,
  and `blendTex` bind through `IDirect3DDevice9::SetTexture`; `depthTex` remains gated.
- `material_shader_binary_evidence`: `contract_ready`, recovered `.bfx` files expose material
  sampler tokens such as `SamplerDiffuse`, `SamplerNormal`, and `SamplerSpecular`.
- `title_font_metric_evidence`: `contract_ready`, title UI contributes recovered font queries,
  text draw commands, and string-size queries.
- `material_texture_shader_slot_binding`: `tracked_open`, 38 material texture bindings still need
  a recovered per-material shader pass and sampler slot ownership map.
- `smaa_depth_texture_sampler_binding`: `tracked_open`, the `D3DFMT_D24S8` depth surface exists,
  but no recovered DX9 depth texture sampler handle exists.
- `font_atlas_texture_implementation`: `tracked_open`, glyph atlas dimensions and cache ownership
  remain unrecovered.
- `draw_present_capture_after_surface_material_font`: `tracked_open`, draw, present, and capture
  remain deferred until material/font/depth gates are closed.
- `original_frame_oracle_after_capture`: `tracked_open`, original-frame oracle remains deferred
  until YuEngine can capture an executed frame.

## Boundary

L32 does not:

- bind stage material textures to guessed shader slots;
- treat `depthTex2D` as a sampleable depth texture without recovered evidence;
- create or populate a font atlas;
- issue draw calls;
- call `Present`;
- capture or compare frames.

## Next Edges

- L33: recover material shader slot/pass ownership or add a stronger blocker if `.bfx` reflection is
  insufficient.
- L34: recover depth texture sampler behavior and font atlas/cache dimensions.
- L35: execute draw queue only after material/font/depth gates are closed.
