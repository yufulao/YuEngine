# L25 Presentation Oracle Status

L25 consumes L20 device presentation records and L24 device execution records into
swapchain/present/original-frame oracle parity records. It is the gate between backend execution
contracts and real platform API submission.

This checkpoint still does not create an HWND, swapchain, D3D device, captured frame, or original
graphics trace. It records those missing pieces as independent obligations so L26 can implement
platform submission without turning into a blue-window or mesh-preview shortcut.

## Contract

```text
project.json
-> L20 device presentation profile
-> L24 device execution records
-> backbuffer extent record
-> device execution frame input record
-> window surface / swapchain / present candidates
-> frame capture / original-frame oracle trace candidates
-> L26 platform API submission gates
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe present-oracle samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_present_oracle_contract --output-on-failure
```

Current metric:

```text
ok=true
device_execution_ok=true
device_presentation_ok=true
presentation_oracle_runtime_ready=true
backbuffer_extent_records_ready=true
device_execution_input_records_ready=true
window_surface_gate_tracked=true
swapchain_creation_gate_tracked=true
present_call_gate_tracked=true
frame_capture_gate_tracked=true
original_frame_oracle_gate_tracked=true
presentation_records=7
ready_presentation_records=2
tracked_open_presentation_records=5
backbuffer_extent_records=1
backbuffer_width=1280
backbuffer_height=720
device_execution_frame_inputs=1
linked_device_execution_records=103
linked_device_ready_records=55
linked_device_open_records=48
window_surface_candidates=1
swapchain_creation_candidates=1
present_call_candidates=1
frame_capture_candidates=1
oracle_trace_candidates=1
renderer_backend_commands=181
draw_submissions=121
resolved_presentation_contracts=2
tracked_presentation_obligations=5
open_presentation_obligations=5
```

## Contract State

- `backbuffer_extent_record`: `contract_ready`, `SMAA_PIXEL_SIZE` and device presentation expose
  the 1280x720 backbuffer extent.
- `device_execution_frame_input`: `contract_ready`, L24 supplies 103 device execution records,
  including 55 ready and 48 tracked-open records.
- `window_surface_creation`: `tracked_open`, present requires an HWND/window surface, but no
  platform surface is created.
- `swapchain_creation`: `tracked_open`, backbuffer extent is known, but no swapchain object is
  created.
- `present_call_execution`: `tracked_open`, the renderer scheduler reaches the present gate, but
  no `Present` call is executed.
- `frame_capture_artifact`: `tracked_open`, draw submissions exist, but no captured frame artifact
  exists for parity.
- `original_frame_oracle_trace`: `tracked_open`, no original D3D/render trace or screenshot parity
  has been recorded.

## Evidence Now Locked

- L25 links back to 181 renderer backend commands and 121 draw submissions.
- L25 links back to 103 L24 device execution records: 46 resource creation records and 57 binding
  records.
- The 1280x720 backbuffer extent is carried forward as a contract-ready input, not inferred from a
  new window.
- Window surface creation, swapchain creation, present execution, frame capture, and original
  oracle trace are separately named gates. A future implementation can fail one gate without
  pretending the whole backend is ready.

## Boundary

L25 is not a platform backend. It does not:

- create an HWND or OS window surface;
- call `Direct3DCreate9`, `CreateDevice`, `CreateAdditionalSwapChain`, or `Present`;
- execute `CreateTexture`, `SetTexture`, `SetSamplerState`, or `SetRenderState`;
- capture a YuEngine frame;
- compare output against an original captured frame or graphics API trace.

Future backend work must consume these presentation/oracle records. It cannot satisfy the route
with a clear screen, mesh-only preview, or hand-authored render path that bypasses scheduler,
renderer submission, device presentation, resource allocation, and device execution evidence.

## Next Edges

- L26: real platform D3D API submission/backend bridge. It must consume L24 creation/binding
  records and L25 presentation/oracle gates before issuing real calls.
- Later: original-frame capture and parity comparison, after YuEngine can present a frame through
  the same scheduler/backend path.
