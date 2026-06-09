# L27 Backend Executor Status

L27 consumes the L26 platform bridge call queue and turns it into backend executor result records.
This is the executor boundary between recovered runtime contracts and concrete platform execution.

This checkpoint still is not the finished engine and still is not a playable D3D backend. It adds
the executor interface, diagnostic D3D9 adapter accounting, and per-call result contracts so the
next layer can implement real HWND/Direct3D device creation without bypassing the recovered runtime
path.

## Contract

```text
project.json
-> L24 device execution records
-> L25 presentation/oracle records
-> L26 platform bridge call queue
-> backend executor results
-> diagnostic D3D9 adapter successes
-> real HWND/D3D9/draw/present/capture/oracle gates
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-executor samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_backend_executor_contract --output-on-failure
```

Current metric:

```text
ok=true
platform_bridge_ok=true
executor_runtime_ready=true
diagnostic_executor_ready=true
one_to_one_bridge_mapping_ready=true
ready_call_execution_results_ready=true
tracked_open_call_results_ready=true
concrete_d3d_execution_gate_tracked=true
hwnd_device_gate_tracked=true
draw_execution_gate_tracked=true
present_execution_gate_tracked=true
frame_capture_gate_tracked=true
original_oracle_gate_tracked=true
execution_result_records=10
diagnostic_success_records=4
tracked_open_execution_records=6
blocked_execution_records=0
consumed_bridge_call_records=10
ready_bridge_call_records=4
tracked_open_bridge_call_records=6
result_call_count_total=689
diagnostic_executed_calls=562
preserved_open_calls=127
submitted_diagnostic_batches=1
executed_resource_creation_calls=46
executed_upload_subresource_calls=458
executed_state_binding_calls=57
preserved_platform_surface_gates=1
preserved_device_creation_gates=2
preserved_draw_submission_gates=121
preserved_present_gates=1
preserved_capture_oracle_gates=2
linked_platform_input_records=110
ready_platform_input_records=57
tracked_open_platform_input_records=53
backbuffer_width=1280
backbuffer_height=720
resolved_executor_contracts=4
tracked_executor_obligations=6
open_executor_obligations=6
```

## Contract State

- `executor_consumes_platform_bridge`: `contract_ready`, 10 executor result records map one-to-one
  to 10 L26 bridge call records.
- `diagnostic_success_result_mapping`: `contract_ready`, the diagnostic adapter accepts 4 ready
  bridge records and 562 diagnostic calls.
- `ready_call_execution_results`: `contract_ready`, resource creation, upload, and state-binding
  queues produce diagnostic success.
- `tracked_open_result_preservation`: `contract_ready`, 6 tracked-open bridge records preserve
  127 concrete platform/oracle calls.
- `concrete_hwnd_and_d3d_device_execution`: `tracked_open`, HWND, `Direct3DCreate9`, and
  `CreateDevice` execution are still unimplemented.
- `draw_execution_backend`: `tracked_open`, 121 draw submissions remain queued until vertex/index
  buffers and draw calls exist.
- `present_execution`: `tracked_open`, `Present` remains queued until a concrete device and
  swapchain exist.
- `frame_capture_artifact`: `tracked_open`, frame capture remains queued until a YuEngine frame
  artifact exists.
- `original_frame_oracle_parity`: `tracked_open`, original screenshot/API trace parity is absent.
- `real_d3d9_adapter`: `tracked_open`, diagnostic execution exists but real D3D9 calls are not
  executed.

## Evidence Now Locked

- L27 consumes all 10 L26 bridge records and does not drop ready or tracked-open records.
- Diagnostic-success records account for 1 bridge batch, 46 resource-creation calls, 458 upload
  subresource calls, and 57 state-binding calls.
- Platform/device gates remain explicit: 1 window surface, 2 device-creation gates, 121 draw calls,
  1 present call, and 2 capture/oracle calls.
- The 1280x720 backbuffer extent and 110 platform input records carry through from L26.
- CTest now catches an executor that pretends tracked-open platform work succeeded.

## Boundary

L27 is not the concrete renderer backend. It does not:

- create an HWND;
- call `Direct3DCreate9` or `CreateDevice`;
- create real `IDirect3DTexture9` or render-target resources;
- upload bytes into GPU memory;
- bind vertex/index buffers or execute draw calls;
- call `Present`;
- capture or compare a frame against the original game.

Future work must implement real platform execution behind this executor boundary. It cannot replace
the boundary with a standalone launcher, clear screen, mesh viewer, hand-authored render path, or
manual game loop.

## Next Edges

- L28: real HWND and D3D9 device creation adapter records. It must consume L27 executor results and
  close the window/interface/device gates before resource calls can be real.
- L29+: real resource creation/upload/state execution, then draw execution, present, capture
  artifact, and original-frame oracle parity.
