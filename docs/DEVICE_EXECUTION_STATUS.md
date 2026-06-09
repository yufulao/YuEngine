# L24 Device Execution Status

L24 converts L23 resource allocation records and L22 backend state records into device-facing
resource creation, upload, and state-binding execution records. It still does not call
`IDirect3DDevice9`; the checkpoint exists to define the exact call graph and remaining execution
gates before HWND/swapchain/present work.

## Contract

```text
project.json
-> L23 resource allocation records
-> L22 sampler/pass/font records
-> L20 device presentation profile
-> device resource creation records
-> texture upload subresource records
-> texture/state binding records
-> D3D API submission / present / oracle obligations
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe device-execution samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_device_execution_contract --output-on-failure
```

Current metric:

```text
ok=true
resource_allocation_ok=true
backend_state_ok=true
device_presentation_ok=true
device_execution_runtime_ready=true
resource_creation_records_ready=true
upload_execution_records_ready=true
state_binding_records_ready=true
lookup_texture_binding_records_ready=true
material_texture_binding_gate_tracked=true
transient_surface_binding_gate_tracked=true
font_atlas_execution_gate_tracked=true
d3d_api_call_submission_gate_tracked=true
present_oracle_gate_tracked=true
resource_creation_records=46
ready_resource_creation_records=41
tracked_open_resource_creation_records=5
create_texture_records=40
create_cube_texture_records=1
render_target_creation_candidates=3
depth_stencil_creation_candidates=1
font_atlas_creation_placeholders=1
texture_upload_execution_records=41
upload_subresource_records=458
ready_upload_payload_bytes=23949794
binding_records=57
ready_binding_records=14
tracked_open_binding_records=43
material_texture_binding_records=38
material_texture_binding_slots=39
sampler_texture_binding_records=7
lookup_texture_binding_records=2
transient_sampler_binding_candidates=5
sampler_state_binding_records=7
render_state_binding_records=5
resolved_device_execution_contracts=4
tracked_device_execution_obligations=5
open_device_execution_obligations=5
```

## Contract State

- `device_resource_creation_records`: `contract_ready`, all 46 L23 allocation records map to
  D3D9-style creation operations.
- `texture_upload_subresource_records`: `contract_ready`, 41 ready textures expand to 458
  upload subresources with payload byte parity.
- `smaa_lookup_texture_bindings`: `contract_ready`, `areaTex` and `searchTex` bind ready lookup
  textures.
- `sampler_and_render_state_value_records`: `contract_ready`, 7 sampler state records and 5 pass
  render-state records are ready as device-binding values.
- `material_texture_shader_binding`: `tracked_open`, 39 material texture slots have texture
  records but material shader program ownership is not recovered.
- `smaa_transient_surface_creation_and_binding`: `tracked_open`, color/depth/edges/blend
  surfaces have candidate operation records but exact formats and ownership are unknown.
- `font_atlas_device_resource`: `tracked_open`, font metric inputs exist but atlas dimensions,
  glyph cache, and texture upload are not implemented.
- `d3d_api_call_submission`: `tracked_open`, YuEngine has execution records but no
  `IDirect3DDevice9` calls.
- `present_and_original_frame_oracle`: `tracked_open`, no swapchain present or original-frame
  parity exists yet.

## Evidence Now Locked

- `cubeenvmap/doujou_1.dds` maps to `CreateCubeTexture`, `D3DFMT_DXT1`, 6 faces, 54 upload
  subresources.
- Stage and lookup 2D textures map to 40 `CreateTexture` records.
- `colorTex2D`, `edgesTex2D`, and `blendTex2D` map to 3 render-target candidates.
- `depthTex2D` maps to 1 depth/stencil candidate.
- `font_atlas_placeholder` maps to 1 font texture candidate.
- 38 material texture records cover 39 material texture binding slots.
- SMAA sampler texture bindings split into 2 ready lookup texture bindings and 5 transient
  texture binding candidates.
- Backend state contributes 7 `SetSamplerState` records and 5 `SetRenderStateBundle` records.

## Boundary

L24 is not a real device backend. It does not:

- create an HWND, swapchain, or `IDirect3DDevice9`;
- call `CreateTexture`, `CreateCubeTexture`, `SetTexture`, `SetSamplerState`, or `SetRenderState`;
- upload bytes into GPU memory;
- compile material shaders;
- create a font atlas texture;
- call `Present` or compare captured frames.

Future backend work must consume these execution records. It cannot skip from allocation records to
a visual window without proving device API submission, swapchain/present, and oracle parity gates.

## Next Edges

- L25 has since created swapchain/present/original-frame oracle parity records from L20 and L24.
- L26 has since created the platform/backend bridge submission queue from L24 and L25.
- L27 has since created backend executor results and diagnostic D3D9 adapter accounting.
- L28 has since created real HWND/D3D9 device adapter records.
- L29: concrete YuEngine-owned HWND and D3D9 device creation execution.
