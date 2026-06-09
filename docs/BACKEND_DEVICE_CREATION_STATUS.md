# L29 Backend Device Creation Status

L29 consumes the L28 real HWND/D3D9 precondition adapter records and executes the concrete platform
device creation path. On the current Windows test machine, YuEngine creates a hidden 1280x720 Win32
window, calls `Direct3DCreate9`, and successfully creates an `IDirect3DDevice9` windowed device.

This is still not a playable renderer. L29 proves concrete device creation and records the result
through the recovered backend contract chain. It does not keep a persistent device service handle,
create GPU resources, upload textures, set render state, draw, present, capture, or compare an
original-frame oracle.

## Contract

```text
project.json
-> L26 platform bridge call queue
-> L27 backend executor results
-> L28 platform/device precondition adapter records
-> hidden HWND / Direct3DCreate9 / IDirect3DDevice9::CreateDevice execution result
-> downstream resource/draw/present/capture queues preserved for L30+
```

## Verification

```powershell
build\cmake-bt143\yuengine_cli.exe backend-device-create samples\touhou_new_world\project.json --repo-root .
ctest --test-dir build\cmake-bt143 -C Debug -R yuengine_backend_device_creation_contract --output-on-failure --parallel 8
tools\verify_runtime.ps1
```

Current metric:

```text
ok=true
device_adapter_ok=true
device_creation_runtime_ready=true
adapter_preconditions_consumed_ready=true
platform_execution_attempted=true
window_surface_result_recorded=true
d3d_interface_result_recorded=true
d3d_device_result_recorded=true
downstream_queues_preserved=true
backbuffer_extent_carried=true
platform_supported=true
real_window_surface_ready=true
real_d3d_interface_ready=true
real_device_handle_ready=true
execution_result_records=3
adapter_record_count=10
adapter_precondition_records_consumed=3
source_downstream_blocked_records=6
downstream_real_calls_deferred=685
window_surface_attempts=1
d3d_interface_attempts=1
d3d_device_attempts=1
real_success_records=3
real_failed_records=0
blocked_by_dependency_records=0
linked_platform_input_records=110
ready_platform_input_records=57
tracked_open_platform_input_records=53
backbuffer_width=1280
backbuffer_height=720
resolved_device_creation_contracts=5
tracked_device_creation_obligations=5
open_device_creation_obligations=5
```

## Contract State

- `device_creation_consumes_adapter_preconditions`: `contract_ready`, L29 consumes the 3 L28
  platform/device precondition records.
- `window_surface_execution_result`: `contract_ready`, hidden Win32 HWND creation is recorded as
  a real execution result.
- `direct3d9_interface_execution_result`: `contract_ready`, `Direct3DCreate9` returns an
  `IDirect3D9` interface.
- `d3d9_device_creation_execution_result`: `contract_ready`, `IDirect3D9::CreateDevice` succeeds
  for a hidden 1280x720 windowed device on the current machine.
- `downstream_queues_preserved_after_device_attempt`: `contract_ready`, 685 downstream calls remain
  preserved for the next execution layer.
- `persistent_device_service_handle`: `tracked_open`, the successful D3D9 handle is not yet owned
  by a long-lived runtime service.
- `real_resource_execution_after_device`: `tracked_open`, texture/resource creation and upload
  remain deferred to L30.
- `real_draw_present_capture_after_device`: `tracked_open`, draw, present, and capture remain
  deferred.
- `frame_capture_oracle_after_present`: `tracked_open`, no YuEngine frame artifact exists.
- `original_frame_oracle_after_capture`: `tracked_open`, original-frame parity is still absent.

## Evidence Now Locked

- The platform path is not a diagnostic success: it uses Win32 window creation, dynamic
  `d3d9.dll` loading, `Direct3DCreate9`, and `IDirect3D9::CreateDevice`.
- The execution records preserve source links back to L28 adapter records, L27 executor results,
  and L26 bridge records.
- The original recovered backbuffer extent remains 1280x720.
- Resource/upload/state/draw/present/capture queues are not bypassed; they remain the explicit L30+
  consumer of the concrete device result.

## Boundary

L29 does not:

- introduce a standalone renderer or launcher;
- replace recovered backend records with a manual sample app;
- persist the D3D9 device as a runtime service object;
- call `CreateTexture`, `CreateCubeTexture`, `CreateRenderTarget`, or `CreateDepthStencilSurface`;
- upload DDS payloads;
- bind sampler/render state;
- issue draw calls;
- call `Present`;
- capture or compare frames.

## Next Edges

- L30: persistent backend device service handle and resource creation queue execution.
- L31: texture upload and backend state binding through the persistent device.
- L32: draw queue execution, present, frame capture, and original-frame oracle parity.
