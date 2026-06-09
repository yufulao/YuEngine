# L21 Texture Upload Status

L21 turns the L20 resource upload plan into typed texture upload records. It still does not create
a GPU resource or call a backend API. The point of this checkpoint is to prevent future renderer
work from accepting texture paths or byte counts without format, mip, payload, and consumer
evidence.

## Contract

```text
project.json
-> L17 frame scheduler
-> L16 renderer backend submission
-> L18 backend obligations
-> L19 material semantics
-> L20 device presentation contract
-> stage DDS dependencies
-> typed texture upload records
-> render-state/font/oracle parity gates
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe texture-upload samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_texture_upload_contract --output-on-failure
```

Current metric:

```text
ok=true
scene_runtime_ok=true
backend_obligations_ok=true
material_semantics_ok=true
device_presentation_ok=true
texture_upload_runtime_ready=true
dds_header_contract_ready=true
payload_layout_contract_ready=true
material_consumer_contract_ready=true
sampler_state_gate_tracked=true
blend_depth_state_gate_tracked=true
font_atlas_gate_tracked=true
oracle_parity_gate_tracked=true
stage_texture_dependencies=39
texture_upload_records=39
valid_dds_headers=39
dxt1_textures=31
dxt5_textures=8
cube_map_textures=1
cube_map_faces=6
material_slot_consumers=39
unique_material_texture_uploads=38
duplicate_material_consumers=1
stage_only_texture_uploads=1
compressed_payload_matches=39
payload_byte_total=23768416
expected_payload_byte_total=23768416
mip9_textures=2
mip10_textures=14
mip11_textures=23
texture_width_min=256
texture_width_max=1024
post_effect_samplers=7
title_text_submissions=11
resolved_upload_contracts=4
tracked_upload_obligations=4
open_upload_obligations=4
```

## Contract State

- `dds_header_decode`: `contract_ready`, all 39 stage texture dependencies decode as DDS DXT1 or
  DXT5 records.
- `dds_compressed_payload_layout`: `contract_ready`, computed mip-chain payload bytes match every
  DDS payload.
- `texture_material_consumer_map`: `contract_ready`, 39 material slots consume 38 unique material
  texture uploads, with one duplicate `kamidana_d.dds` consumer.
- `cube_environment_upload_record`: `contract_ready`, `cubeenvmap/doujou_1.dds` is a six-face DXT1
  cube upload record.
- `sampler_state_values`: `tracked_open`, sampler count is known, but filter/address state values
  are not bound.
- `blend_depth_state_values`: `tracked_open`, blend/depth obligations are named, but device render
  states are not bound.
- `font_atlas_glyph_metrics`: `tracked_open`, title text and string-size calls are counted, but
  atlas/glyph metrics are not implemented.
- `original_frame_oracle_trace`: `tracked_open`, no screenshot/API-trace parity exists for the
  upload output.

## Boundary

L21 is not a renderer backend. It does not:

- allocate `IDirect3DTexture9`;
- upload mip levels to GPU memory;
- compile or bind material shaders;
- set sampler, blend, or depth state;
- render text through a font atlas;
- compare a captured original frame.

Future device/backend work must consume these records. It is not allowed to rescan directories,
guess texture dimensions from filenames, or treat unreferenced DDS files as stage uploads.

## Next Edges

- L22 has since created backend-facing sampler, pass render-state, and font glyph metric records.
- L23 has since created D3D9-compatible resource allocation records that consume L21 texture
  uploads and L22 backend state records.
- L24 has since created device resource allocation/upload/state-binding execution records.
- L25 has since created swapchain/present/original-frame oracle parity records after state,
  allocation, and device execution gates became independently typed.
- L26 has since created the platform/backend bridge submission queue.
- L27 has since created backend executor results and diagnostic D3D9 adapter accounting.
- L28 has since created real HWND/D3D9 device adapter records.
- L29: concrete YuEngine-owned HWND and D3D9 device creation execution before upload records may
  become real GPU memory writes.
