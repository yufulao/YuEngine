# L16 Renderer Backend Submission Status

L16 converts the joined gameplay-frame contract into a backend-facing renderer submission
contract. It does not open a window and does not claim original-frame visual parity. The purpose
is to ensure title UI and scene/world rendering are no longer separate diagnostics.

## Contract

The runtime command is:

```text
project.json
-> L15 gameplay-frame inputs
-> title 2D pass submissions
-> world 3D pass submissions
-> resource upload submissions
-> scene-state submissions
-> explicit backend obligations
```

The submission contract consumes:

- L13 title UI command payloads from original `renderProc`;
- L15 gameplay-frame readiness across title, scene, actor, camera, input, event, audio, and save;
- L9 scene-runtime mesh/material/texture handles;
- manifest renderer profile `d3d9_compatible`.

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe renderer-submit samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_renderer_backend_submission_contract --output-on-failure
```

Current metric:

```text
ok=true
scene_runtime_ok=true
title_ui_ok=true
gameplay_frame_ok=true
backend_frame_ready=true
title_pass_ready=true
world_pass_ready=true
resource_upload_ready=true
camera_submission_ready=true
actor_submission_ready=true
event_submission_ready=true
submission_passes=3
backend_command_count=181
draw_submissions=121
```

Payload counts:

```text
renderer_profile=d3d9_compatible
resource_upload_submissions=57
backend_obligations=6
title_2d_submissions=55
title_graph_submissions=3
title_text_submissions=11
scene_mesh_submissions=111
material_bindings=16
texture_bindings=39
collision_debug_submissions=1
camera_submissions=1
actor_submissions=1
event_marker_submissions=1
```

## Explicit Backend Obligations

These are tracked as obligations, not silently filled with fake behavior:

- `shader_effect_semantics`
- `blend_depth_state_semantics`
- `texture_upload_format_semantics`
- `font_atlas_and_glyph_metrics`
- `device_swapchain_presentation`
- `original_frame_oracle_parity`

## Boundary

L16 is not a playable renderer. It does not yet provide:

- D3D/OpenGL/Vulkan/DirectX device creation;
- swapchain presentation;
- real texture upload;
- shader/effect/material evaluation;
- font atlas generation;
- animation or skinning;
- original title/scene screenshot parity.

It does close the previous Project failure route where a title preview and a mesh preview could
exist separately while the engine had no unified renderer contract.

## Next Edges

- L17: create a service-owned frame scheduler/update graph that emits renderer submissions from
  a runtime frame, rather than assembling reports by CLI command.
- L18: start replacing backend obligations with concrete upload/shader/font/device contracts one
  subsystem at a time, each with regression gates.
