# L30 Backend Resource Creation Status

L30 consumes the L29 concrete HWND/D3D9 device creation path and keeps a runtime-owned backend
device service alive long enough to execute the ready resource creation queue. On the current
Windows test machine, YuEngine creates a hidden 1280x720 Win32 window, creates an `IDirect3D9`
device, and calls `IDirect3DDevice9::CreateTexture` / `CreateCubeTexture` for recovered resource
records.

This is still not a playable renderer. L30 proves persistent device ownership and real resource
handle creation. It does not upload DDS payloads, bind sampler/render state, draw, present,
capture, or compare an original-frame oracle.

## Contract

```text
project.json
-> L24 device resource creation records
-> L26 platform bridge resource queue
-> L29 concrete HWND / Direct3DCreate9 / IDirect3DDevice9 device path
-> persistent backend device service
-> real CreateTexture / CreateCubeTexture result records
-> upload/state/draw/present/capture queues preserved for L31+
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-resource-create samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_backend_resource_creation_contract --output-on-failure --parallel 8
tools\verify_runtime.ps1
```

Current metric:

```text
ok=true
device_creation_ok=true
device_execution_ok=true
resource_creation_runtime_ready=true
persistent_device_service_ready=true
ready_resource_creation_executed=true
tracked_open_resources_preserved=true
downstream_execution_deferred=true
backbuffer_extent_carried=true
source_resource_records=46
ready_source_resource_records=41
tracked_open_source_resource_records=5
result_records=46
real_resource_handles_created=41
failed_resource_handles=0
preserved_tracked_open_resources=5
create_texture_results=40
create_cube_texture_results=1
smaa_lookup_texture_results=2
stage_texture_results=39
deferred_render_target_candidates=3
deferred_depth_stencil_candidates=1
deferred_font_atlas_candidates=1
upload_subresource_records_deferred=458
state_binding_records_deferred=57
draw_present_capture_records_deferred=124
ready_payload_bytes_created=23949794
backbuffer_width=1280
backbuffer_height=720
resolved_resource_creation_contracts=5
tracked_resource_creation_obligations=5
open_resource_creation_obligations=5
```

## Contract State

- `persistent_device_service_handle`: `contract_ready`, the hidden HWND and D3D9 device stay owned
  by the L30 resource creation path until resource creation is complete.
- `ready_texture_resource_creation`: `contract_ready`, 40 ready 2D texture records call
  `IDirect3DDevice9::CreateTexture`.
- `ready_cube_resource_creation`: `contract_ready`, the recovered environment cube map calls
  `IDirect3DDevice9::CreateCubeTexture`.
- `smaa_lookup_resource_creation`: `contract_ready`, SMAA area/search lookup textures are created
  with `D3DFMT_A8L8` and `D3DFMT_L8`.
- `tracked_open_resource_preservation`: `contract_ready`, transient surfaces and the font atlas
  remain explicit tracked-open records instead of guessed resources.
- `texture_upload_after_resource_creation`: `tracked_open`, 458 upload subresource records remain
  deferred until lock/unlock upload execution exists.
- `state_binding_after_upload`: `tracked_open`, 57 state/texture binding records remain deferred
  until uploads produce usable resources.
- `draw_present_capture_after_resources`: `tracked_open`, draw, present, and capture remain
  deferred until resource handles and state are bound.
- `transient_surface_format_definition`: `tracked_open`, SMAA transient surface formats remain
  unknown and are not guessed.
- `font_atlas_resource_definition`: `tracked_open`, font atlas dimensions and glyph cache ownership
  remain unrecovered.

## Evidence Now Locked

- 46 source resource creation records are consumed from the recovered backend chain.
- 41 ready records create real D3D9 resource handles retained by the backend device service until
  the L30 execution layer exits, with 0 failed handle creations on this machine.
- Stage resources create 39 texture handles, including the recovered `cubeenvmap/doujou_1.dds`
  cube texture.
- SMAA lookup resources create `areaTex2D` and `searchTex2D`.
- 5 records stay tracked-open: `colorTex2D`, `depthTex2D`, `edgesTex2D`, `blendTex2D`, and
  `font_atlas_placeholder`.
- The inherited backbuffer extent remains 1280x720.

## Boundary

L30 does not:

- upload any DDS mip payload into the created resources;
- call `LockRect`, `UnlockRect`, `SetTexture`, `SetSamplerState`, or render-state APIs;
- compile or bind material shaders;
- create guessed transient surfaces or font atlas resources;
- issue draw calls;
- call `Present`;
- capture or compare frames.

## Next Edges

- L31: texture upload and backend state binding through the persistent device.
- L32: transient surface/font atlas resolution, draw queue execution, present, frame capture, and
  original-frame oracle parity.
