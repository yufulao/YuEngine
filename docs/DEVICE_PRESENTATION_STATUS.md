# L20 Device Presentation Status

L20 defines the device/swapchain/render-state presentation contract. It does not create a fake
window or GPU device. It proves that device work now has to consume the same scheduler, renderer
submission, backend obligations, and material semantics spine as the rest of the runtime.

## Contract

```text
project.json
-> L17 frame scheduler
-> L16 renderer backend submission
-> L18 backend texture/material obligations
-> L19 material semantics
-> device profile contract
-> resource upload plan
-> draw queue contract
-> swapchain/render-state/present obligations
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe device-presentation samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_device_presentation_contract --output-on-failure
```

Current metric:

```text
ok=true
frame_scheduler_ok=true
renderer_submission_ok=true
backend_obligations_ok=true
material_semantics_ok=true
device_profile_ready=true
swapchain_contract_tracked=true
resource_upload_plan_ready=true
render_state_contract_tracked=true
draw_queue_contract_ready=true
present_contract_tracked=true
renderer_profile=d3d9_compatible
backbuffer_width_candidate=1280
backbuffer_height_candidate=720
renderer_backend_commands=181
resource_upload_submissions=57
draw_submissions=121
material_texture_slots=39
texture_bytes_found=39
post_effect_samplers=7
resolved_device_contracts=3
tracked_device_obligations=5
open_device_obligations=5
```

## Contract State

- `d3d9_device_profile`: `contract_ready`, renderer profile is `d3d9_compatible` and backend
  frame submission is ready.
- `resource_upload_plan`: `contract_ready`, 57 renderer upload submissions consume 39 texture
  payloads and 39 material texture slots.
- `draw_queue_submission`: `contract_ready`, 121 draw submissions include title 2D and
  111 world mesh submissions.
- `swapchain_backbuffer_candidate`: `tracked_open`, `SMAA_PIXEL_SIZE` exposes a 1280x720
  candidate but no OS surface is created.
- `sampler_state_semantics`: `tracked_open`, 7 SMAA samplers are counted while material sampler
  state values remain unbound.
- `blend_depth_state_semantics`: `tracked_open`, renderer submission carries the obligation but
  no device render state is set.
- `present_call_and_window_surface`: `tracked_open`, scheduler reaches renderer submission but no
  HWND/device `Present` call is executed.
- `original_frame_oracle_parity`: `tracked_open`, no original-frame screenshot or API trace parity
  exists yet.

## Boundary

L20 is not a renderer implementation. It does not:

- create an HWND;
- create a D3D9 device;
- allocate swapchain/backbuffer resources;
- upload DDS payloads to GPU memory;
- bind blend/depth/sampler state;
- call `Present`;
- compare output to original frames.

Do not replace these obligations with a blue screen, mesh viewer, or mock window. The next work is
to turn upload/state/font/oracle contracts into typed runtime records before any real device work.

## Next Edges

- L21: texture upload records, sampler/blend/depth split, font atlas/glyph metrics, and original
  frame oracle parity gates.
- Then: real device/swapchain creation only when upload/state/font/oracle gates have failure
  evidence.
