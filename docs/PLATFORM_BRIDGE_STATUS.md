# L26 Platform Bridge Status

L26 converts L24 device execution records and L25 presentation/oracle records into a backend
platform bridge submission queue. This is the first runtime-owned bridge between recovered
renderer/device contracts and concrete platform API work.

This checkpoint still does not execute Direct3D calls. It defines the bridge input, call batches,
ready/open accounting, and failure gates so the next layer can implement a concrete backend without
inventing a separate visual path.

## Contract

```text
project.json
-> L24 device execution records
-> L25 presentation/oracle records
-> BackendBridge::submitFrame diagnostic batch
-> D3D9-style resource/upload/state/present call batches
-> concrete platform executor / frame capture / oracle parity gates
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe platform-bridge samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_platform_bridge_contract --output-on-failure
```

Current metric:

```text
ok=true
device_execution_ok=true
presentation_oracle_ok=true
platform_bridge_runtime_ready=true
diagnostic_bridge_ready=true
creation_submission_queue_ready=true
upload_submission_queue_ready=true
state_submission_queue_ready=true
presentation_submission_queue_tracked=true
d3d_concrete_backend_gate_tracked=true
frame_capture_gate_tracked=true
original_oracle_gate_tracked=true
bridge_call_records=10
ready_bridge_call_records=4
tracked_open_bridge_call_records=6
diagnostic_call_batches=1
platform_surface_records=1
d3d_interface_records=1
create_device_records=1
resource_creation_call_records=46
texture_create_calls=40
cube_texture_create_calls=1
render_target_create_candidates=3
depth_stencil_create_candidates=1
font_atlas_create_candidates=1
upload_call_records=41
upload_subresource_calls=458
state_binding_call_records=57
ready_state_binding_calls=14
tracked_open_state_binding_calls=43
set_texture_calls=45
set_sampler_state_calls=7
set_render_state_bundle_calls=5
draw_submission_calls=121
present_call_records=1
capture_oracle_call_records=2
linked_device_execution_records=103
linked_presentation_records=7
ready_platform_input_records=57
tracked_open_platform_input_records=53
renderer_backend_commands=181
draw_submissions=121
backbuffer_width=1280
backbuffer_height=720
resolved_platform_bridge_contracts=4
tracked_platform_bridge_obligations=6
open_platform_bridge_obligations=6
```

## Contract State

- `diagnostic_backend_bridge`: `contract_ready`, bridge input links 103 L24 device records and
  7 L25 presentation/oracle records.
- `resource_creation_submission_queue`: `contract_ready`, 46 resource creation calls map to L24
  texture/cube/render/depth/font records.
- `texture_upload_submission_queue`: `contract_ready`, 41 ready textures expand to 458
  `LockRect`/`UnlockRect` style upload calls.
- `state_binding_submission_queue`: `contract_ready`, 57 binding calls map to `SetTexture`,
  `SetSamplerState`, and render-state bundles.
- `platform_window_surface`: `tracked_open`, window surface creation is queued but not executed.
- `d3d9_interface_and_device_creation`: `tracked_open`, `Direct3DCreate9` and `CreateDevice` are
  queued but no concrete device exists.
- `draw_submission_backend`: `tracked_open`, 121 draw submissions exist but vertex/index buffer
  binding and draw calls are not executed.
- `present_execution`: `tracked_open`, `Present` is queued but not called.
- `frame_capture_artifact`: `tracked_open`, frame capture is queued but no YuEngine frame artifact
  exists.
- `original_frame_oracle_parity`: `tracked_open`, original screenshot/API trace parity is still
  absent.

## Evidence Now Locked

- Bridge input is 110 records: 103 from L24 and 7 from L25.
- Ready platform inputs are 57; tracked-open platform inputs are 53.
- The 1280x720 backbuffer extent is carried into platform call records.
- D3D9-style queue coverage is explicit: resource creation, upload, state binding, draw,
  present, and capture/oracle.
- CTest locks the bridge against bypassing scheduler, renderer submission, device execution, and
  presentation/oracle evidence.

## Boundary

L26 is not a concrete D3D backend. It does not:

- create an HWND;
- call `Direct3DCreate9`, `CreateDevice`, `CreateTexture`, `SetTexture`, or `Present`;
- allocate vertex/index buffers;
- execute draw calls;
- capture a frame or compare oracle parity.

Future work must implement the concrete backend behind this bridge. It cannot replace the bridge
with a standalone launcher, clear screen, mesh preview, or hand-authored render path.

## Next Edges

- L27 has since created the backend executor interface and diagnostic D3D9 adapter result layer.
- L28: real HWND/D3D9 device creation adapter records. It must consume L27 executor results before
  any resource, draw, present, capture, or oracle work can execute for real.
- Later: real resource/upload/state execution, draw execution, frame capture artifact, then
  original-frame parity comparison.
