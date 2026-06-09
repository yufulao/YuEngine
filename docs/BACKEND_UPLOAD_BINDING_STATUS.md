# L31 Backend Upload And Binding Status

L31 consumes the L30 persistent D3D9 device/resource creation path and executes the next recovered
backend edge: DDS payload upload and ready backend state binding. On the current Windows test
machine, YuEngine uploads 458 DDS subresources through real `LockRect`/`UnlockRect` calls, binds the
two SMAA lookup textures with `SetTexture`, executes seven sampler-state bundles, and executes five
render-state bundles.

This is still not a playable renderer. L31 proves upload and ready state binding against real D3D9
handles. It does not resolve material shader slot ownership, create transient SMAA render targets,
create a font atlas, issue draw calls, present, capture, or compare an original-frame oracle.

## Contract

```text
project.json
-> L30 persistent HWND / D3D9 device / retained resource handles
-> 41 ready texture resources
-> 458 DDS subresource LockRect/UnlockRect uploads
-> 14 ready backend state binding records
-> material/transient/font/draw/present/capture/oracle gates preserved for L32+
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-upload-bind samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_backend_upload_binding_contract --output-on-failure --parallel 8
tools\verify_runtime.ps1
```

Current metric:

```text
ok=true
resource_creation_ok=true
device_execution_ok=true
backend_state_ok=true
upload_binding_runtime_ready=true
persistent_device_service_ready=true
resource_handles_ready=true
texture_upload_executed=true
lookup_texture_upload_executed=true
lookup_texture_binding_executed=true
sampler_state_binding_executed=true
render_state_binding_executed=true
tracked_open_bindings_preserved=true
downstream_draw_present_deferred=true
backbuffer_extent_carried=true
resource_handles_created=41
upload_texture_records=41
stage_texture_upload_records=39
lookup_texture_upload_records=2
cube_texture_upload_records=1
upload_subresource_records=458
uploaded_subresources=458
failed_upload_records=0
upload_payload_bytes=23949794
uploaded_payload_bytes=23949794
binding_records=57
ready_binding_records=14
tracked_open_binding_records=43
executed_binding_records=14
failed_binding_records=0
set_texture_calls=2
set_sampler_state_calls=7
set_render_state_bundles=5
set_render_state_calls=40
preserved_material_texture_bindings=38
preserved_transient_texture_bindings=5
draw_present_capture_records_deferred=124
backbuffer_width=1280
backbuffer_height=720
resolved_upload_binding_contracts=6
tracked_upload_binding_obligations=5
open_upload_binding_obligations=5
```

## Contract State

- `persistent_resource_handles_for_upload`: `contract_ready`, 41 L30 resource handles are retained
  long enough for upload and binding.
- `texture_subresource_upload_execution`: `contract_ready`, 458 DDS subresources upload through
  `LockRect`/`UnlockRect` with payload byte parity.
- `cube_and_lookup_upload_execution`: `contract_ready`, the cube environment texture and two SMAA
  lookup textures upload through real resource handles.
- `smaa_lookup_texture_binding_execution`: `contract_ready`, `areaTex` and `searchTex` bind through
  `IDirect3DDevice9::SetTexture`.
- `sampler_state_binding_execution`: `contract_ready`, seven recovered SMAA sampler state records
  execute as `SetSamplerState` bundles.
- `render_state_binding_execution`: `contract_ready`, five recovered SMAA pass records execute as
  `SetRenderState` bundles.
- `material_texture_shader_binding`: `tracked_open`, 38 material texture bindings still require
  recovered material shader slot ownership.
- `smaa_transient_surface_binding`: `tracked_open`, five transient SMAA texture bindings remain
  open until transient surfaces are created.
- `font_atlas_resource_binding`: `tracked_open`, font atlas dimensions and glyph cache ownership
  remain unrecovered.
- `draw_present_capture_after_upload_binding`: `tracked_open`, draw, present, and capture remain
  deferred until uploaded resources and state are consumed.
- `original_frame_oracle_after_present`: `tracked_open`, original-frame oracle remains deferred
  until frame capture exists.

## Boundary

L31 does not:

- bind stage material textures to recovered material shader slots;
- create SMAA transient render targets or depth surfaces;
- create a font atlas;
- issue any draw calls;
- call `Present`;
- capture or compare frames.

## Next Edges

- L32: transient surface/font atlas resolution and material shader binding.
- L33: draw queue execution, present, frame capture, and original-frame oracle parity.
