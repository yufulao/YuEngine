# L18 Backend Obligations Status

L18 splits the renderer backend obligations into concrete contracts. It resolves the first two
contracts, texture upload format and material binding, using real VFS payloads and scene-runtime
handles. Shader/effect, font, device/swapchain, and original-frame parity remain explicit open
obligations.

## Contract

The runtime command is:

```text
project.json
-> L17 frame scheduler
-> L16 renderer submission
-> VFS texture payload reads
-> material/mesh/texture binding checks
-> backend obligation accounting
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-obligations samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_backend_obligations_contract --output-on-failure
```

Current metric:

```text
ok=true
frame_scheduler_ok=true
renderer_submission_ok=true
texture_upload_contract_ready=true
material_binding_contract_ready=true
shader_effect_contract_tracked=true
font_contract_tracked=true
device_contract_tracked=true
oracle_parity_contract_tracked=true
texture_dependencies=39
texture_bytes_found=39
dds_textures=39
material_bindings=16
mesh_submissions=111
resolved_backend_contracts=2
tracked_backend_obligations=4
open_backend_obligations=4
```

Payload evidence:

```text
texture_byte_total=23773408
title_text_submissions=11
```

## Obligation State

- `texture_upload_format_semantics`: `contract_ready`, 39 DDS payloads resolved through VFS.
- `material_binding_semantics`: `contract_ready`, 16 material tags bound to 111 mesh submissions
  and 39 textures.
- `shader_effect_semantics`: `tracked_open`, material tags are counted but shader/effect bytecode
  semantics are not decoded.
- `font_atlas_and_glyph_metrics`: `tracked_open`, 11 title text submissions require font atlas
  and glyph metric contracts.
- `device_swapchain_presentation`: `tracked_open`, renderer profile is `d3d9_compatible` but no
  device backend is created.
- `original_frame_oracle_parity`: `tracked_open`, no original-frame screenshot or API trace parity
  exists yet.

## Boundary

L18 is not a renderer backend. It proves texture/material inputs are real and attached to the
scheduler and renderer submission contracts. It does not yet decode shader/effect semantics,
create a GPU device, upload textures, render fonts, or compare against original frames.

## Next Edges

- L19 has since decoded model material blocks into runtime material/texture-slot/mesh-binding
  semantics while keeping per-material shader program semantics open and tracked.
- L20 has since attached a device/swapchain/render-state presentation contract without creating a
  fake window or GPU device.
- L21: split texture upload, sampler/blend/depth, font, and oracle parity gates before any real
  device/swapchain implementation.
