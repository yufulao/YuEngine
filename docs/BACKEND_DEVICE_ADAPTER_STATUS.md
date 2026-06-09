# L28 Backend Device Adapter Status

L28 consumes the L27 backend executor results and turns them into real HWND/D3D9 device adapter
records. This checkpoint closes the precondition layer before real platform execution: no resource,
upload, state, draw, present, capture, or oracle call may execute until a concrete device handle
exists.

This checkpoint is still not a playable backend. It does not create an HWND or call D3D9. It makes
the next platform step narrow and enforceable: create a YuEngine-owned window surface, call
`Direct3DCreate9`, then call `IDirect3D9::CreateDevice` through the same recovered runtime path.

## Contract

```text
project.json
-> L26 platform bridge call queue
-> L27 backend executor results
-> L28 real HWND/D3D9 device adapter records
-> concrete HWND / Direct3DCreate9 / CreateDevice gates
-> downstream resource/draw/present/capture blocked until device handle
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-device-adapter samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_backend_device_adapter_contract --output-on-failure
```

Current metric:

```text
ok=true
backend_executor_ok=true
device_adapter_runtime_ready=true
executor_results_consumed_ready=true
platform_device_preconditions_tracked=true
downstream_execution_blocked_until_device=true
backbuffer_extent_carried=true
real_window_surface_gate_tracked=true
real_d3d_interface_gate_tracked=true
real_d3d_device_gate_tracked=true
real_device_handle_ready=false
resource_execution_requires_device=true
draw_present_capture_requires_device=true
adapter_record_count=10
consumed_executor_result_records=10
source_diagnostic_success_records=4
source_tracked_open_records=6
diagnostic_context_records=1
platform_device_adapter_records=3
window_surface_adapter_records=1
d3d_interface_adapter_records=1
create_device_adapter_records=1
downstream_blocked_records=6
resource_queue_blocked_records=3
render_queue_blocked_records=3
platform_device_precondition_calls=3
downstream_real_calls_blocked_until_device=685
blocked_real_calls_total=688
real_executed_calls=0
real_resource_creation_calls_blocked=46
real_upload_subresource_calls_blocked=458
real_state_binding_calls_blocked=57
real_draw_calls_blocked=121
real_present_calls_blocked=1
real_capture_oracle_calls_blocked=2
inherited_diagnostic_executed_calls=562
inherited_preserved_open_calls=127
linked_platform_input_records=110
ready_platform_input_records=57
tracked_open_platform_input_records=53
backbuffer_width=1280
backbuffer_height=720
resolved_device_adapter_contracts=4
tracked_device_adapter_obligations=6
open_device_adapter_obligations=6
```

## Contract State

- `device_adapter_consumes_executor_results`: `contract_ready`, 10 adapter records map one-to-one
  to 10 L27 executor results.
- `platform_device_preconditions_tracked`: `contract_ready`, window surface, Direct3D interface,
  and D3D9 device creation are distinct records.
- `downstream_execution_blocked_until_device`: `contract_ready`, 685 downstream calls cannot
  execute without a device handle.
- `backbuffer_extent_carried_to_adapter`: `contract_ready`, 1280x720 is inherited from the
  recovered backbuffer contract.
- `real_hwnd_surface_creation`: `tracked_open`, no YuEngine-owned HWND has been created.
- `real_direct3d9_interface_creation`: `tracked_open`, `Direct3DCreate9` has not been called.
- `real_d3d9_device_creation`: `tracked_open`, `IDirect3D9::CreateDevice` has not been called.
- `real_resource_execution_after_device`: `tracked_open`, resource creation, upload, and state
  binding remain blocked.
- `real_draw_present_capture_after_device`: `tracked_open`, draw, present, and capture remain
  blocked.
- `original_frame_oracle_after_capture`: `tracked_open`, parity remains blocked until a YuEngine
  frame capture exists.

## Evidence Now Locked

- L28 consumes all 10 L27 executor results and preserves their source bridge record names.
- The three platform/device preconditions are separate: 1 window surface, 1 Direct3D interface,
  and 1 D3D9 device creation record.
- All real D3D execution is currently zero: `real_executed_calls=0`.
- 688 real calls are blocked behind device creation: 3 platform/device calls plus 685 downstream
  resource, draw, present, capture, and oracle calls.
- The resource queue blocks 46 creation calls, 458 upload subresource calls, and 57 state-binding
  calls.
- The render queue blocks 121 draw calls, 1 present call, and 2 capture/oracle calls.
- Backbuffer dimensions remain 1280x720 and are not inferred from a new launcher.

## Boundary

L28 is not a real backend implementation. It does not:

- create an HWND;
- call `Direct3DCreate9`;
- call `IDirect3D9::CreateDevice`;
- create textures, buffers, render targets, or depth surfaces;
- upload GPU memory;
- set device state;
- draw, present, capture, or compare frames.

Future work must implement the real platform calls behind these records. It cannot replace the
path with a standalone window, clear screen, mesh preview, or hand-authored renderer.

## Next Edges

- L29: concrete YuEngine-owned HWND and D3D9 device creation execution. It must consume the L28
  three precondition records and produce a real or explicitly failed device-handle result.
- L30+: once a concrete device handle exists, execute resource creation/upload/state queues; then
  draw, present, capture, and original-frame oracle parity.
