# P2-GATE-006: RHI Backend-Neutral Device Boundary

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `REVIEW_REQUESTED`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-001, P2-GATE-004, P2-GATE-005, ADR-0011, ENG-096
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0011
Source baseline: `ac0ed13`

## Layer

L3 RHI boundary.

This gate takes the accepted Null RHI contract and extracts the next
backend-neutral device boundary needed before any D3D11 device or DXGI swapchain
gate can be implemented.

The gate does not approve a real backend. It does not create a D3D11 device,
DXGI factory, swapchain, backbuffer, shader, buffer, pipeline state, RenderCore
pass, mesh, material, Resource upload path, screenshot, report, or Game Adapter
behavior.

```text
Null RHI device, command, present, capture
-> backend-neutral device interface
-> RHI-owned native surface descriptor
-> backend factory vocabulary and unsupported-backend handling
-> deterministic contract tests
-> later D3D11 device/swapchain gate
```

## Current Reality

Current `YuRHI` owns:

- `NullRhiDevice`;
- `RhiBackendKind`;
- `RhiDeviceDesc`;
- `RhiCapabilities`;
- `RhiStatus`;
- `RhiCommandList`;
- `RhiTextureHandle`;
- deterministic submit, present, and capture behavior for the null backend.

Current RHI tests cover 28 deterministic tests under the `RHI` label. The
current full `windows-fast-gate` discovers 656 tests after P2-GATE-005 landed.

Current `YuPlatform` now owns an opaque `PlatformNativeSurface` value from
P2-GATE-005. That value is a Platform-owned input record for later binding, not
an RHI device, factory, swapchain, or present contract.

There is still no backend-neutral RHI device interface, no RHI-owned native
surface descriptor, no backend factory contract, no D3D11 backend, no DXGI
swapchain, no GPU adapter selection, and no real hardware smoke.

## Owns

This gate owns the proposal for:

- an RHI device interface contract that the existing null backend can satisfy;
- a factory boundary for selecting a backend kind and rejecting unsupported
  backends without mutating device state;
- an RHI-owned native surface descriptor that can be populated from Platform
  native surface values without including Platform headers in RHI;
- explicit capability fields for whether a backend supports native surfaces,
  swapchains, capture, and hardware devices;
- deterministic tests that exercise the device interface through the null
  backend;
- deterministic tests that prove D3D11/DXGI/surface-required creation remains
  unsupported until a later gate;
- dependency-boundary tests that keep RHI free of Platform, RenderCore, Resource,
  Package, Game Adapter, reports, screenshots, and evidence tooling.

## Does Not Own

This gate does not own:

- D3D11, DXGI, Vulkan, OpenGL, Metal, WGL, SDL, GLFW, or OS window backend
  implementation;
- real device/context creation;
- swapchain creation, resize, present to a real window, or capture from a real
  backbuffer;
- GPU adapter enumeration or selection policy;
- shader compiler, bytecode loading, input layout, buffers, textures, samplers,
  pipeline state, descriptor tables, fences, or upload queues;
- RenderCore passes, frame graph, material system, mesh, scene traversal, UI
  renderer, World integration, or Game Adapter behavior;
- Resource, Package, File streaming, image decoding, asset import, or upload
  scheduling;
- logs, reports, screenshots, visual demos, or oracle artifacts as proof.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Platform owns OS windows, native handle identity, and raw platform events;
- RHI owns backend selection, device capabilities, swapchain-facing descriptors,
  command submission, present, and capture contracts;
- RenderCore owns pass scheduling and frame composition above RHI;
- Resource owns asset identity and decoded data above File/Package boundaries.

YuEngine must not copy UE or Unity source, public API names, private layout, or
module naming. This gate uses those engines only to keep ownership boundaries
separate.

## Dependencies

Allowed dependencies:

- existing `YuRHI` public and private headers;
- C++ standard library types already accepted by RHI;
- CMake/CTest for deterministic RHI tests.

Potentially allowed only if review accepts an implementation need:

- `YuMemory` accounting vocabulary already used by RHI snapshots;
- `YuDiagnostics` disabled-behavior observation already accepted by RHI tests.

Forbidden dependencies:

- `YuPlatform` includes or target linkage from `YuRHI`;
- Windows SDK, D3D11, DXGI, COM, WGL, SDL, GLFW, Vulkan, OpenGL, or Metal;
- RenderCore, YuInput, World, UI, Resource, Package, File streaming behavior,
  report/evidence modules, tools, or Game Adapter.

Rationale:

RHI may define an RHI-owned native surface descriptor, but the conversion from
`PlatformNativeSurface` to that descriptor belongs to the future caller or
adapter layer. The first RHI boundary must not make `YuRHI` depend on
`YuPlatform`.

## Public Surface Shape

The first slice should add small RHI-owned contracts. Suggested names:

- `IRhiDevice`;
- `RhiNativeSurfaceDesc`;
- `RhiDeviceFactory`;
- `RhiDeviceCreateResult` if the factory needs a structured result;
- small capability fields in `RhiCapabilities` for native-surface and real
  hardware support.

`IRhiDevice` should be a pure interface with a virtual destructor and methods
matching the accepted RHI lifecycle:

- initialize from `RhiDeviceDesc`;
- create and destroy color targets;
- record clear commands;
- submit;
- present;
- capture into caller-owned storage;
- return capabilities and snapshot values.

The existing `NullRhiDevice` may implement `IRhiDevice` directly. The first
slice should not force heap allocation or ownership transfer for device creation.
Factory tests may use caller-owned device storage or simple explicit status
returns.

`RhiNativeSurfaceDesc` should be an RHI-owned POD-style value that carries opaque
surface identity without exposing platform or Windows SDK types. Suggested
fields are opaque integral values and a validity flag. It must not include
`Windows.h`, `PlatformNativeSurface.h`, or any D3D/DXGI type.

If `RhiBackendKind::D3D11` is added as vocabulary, it is only an unsupported
backend value in this gate. It must not imply a D3D11 device exists.

## Lifecycle

The intended first-slice lifecycle is:

1. Caller builds a `RhiDeviceDesc`.
2. Optional native surface data remains an RHI-owned descriptor value.
3. The factory or device initializer validates backend kind, surface usage,
   capacities, and null-backend constraints.
4. Null backend creation succeeds through the backend-neutral device interface.
5. D3D11, real hardware, or surface-required creation returns explicit
   unsupported status without mutating output state.
6. Existing command, submit, present, and capture behavior stays compatible
   through the interface.
7. Existing `NullRhiDevice` direct usage remains source-compatible unless review
   explicitly approves a narrow signature change.

Failure behavior:

- null or unsupported backend requests return explicit unsupported status;
- invalid native-surface descriptors fail before any device mutation;
- surface-required creation for the null backend returns explicit unsupported
  or invalid-descriptor status;
- factory failure does not allocate a partially initialized backend;
- existing null submit/present/capture failure semantics remain unchanged.

## Inputs

- existing `RhiDeviceDesc` values;
- backend kind vocabulary;
- optional RHI-owned native surface descriptor values;
- caller-owned output storage for device creation or result data;
- existing RHI command list, target, present, and capture inputs.

## Outputs

- explicit RHI status or create result;
- backend-neutral device interface behavior;
- backend capabilities including native-surface and hardware-support fields;
- deterministic snapshot values;
- unchanged null backend capture bytes and counters;
- no screenshot, log, report, visual demo, or hardware smoke as proof.

## Test And Preset Strategy

The first slice must stay deterministic and no-real-device by default:

- add RHI contract tests only;
- label new tests `Fast`, `ModuleFixture`, and `RHI`;
- use `PerformanceSmoke` only for no-growth or allocation/capacity tests;
- use `EvidenceOracle` only for boundary/proof tests;
- keep `HardwareSmoke` at 0;
- do not change `CMakePresets.json` unless review explicitly approves a separate
  isolation amendment.

Expected first-slice test growth is about 6 to 10 deterministic RHI tests. If no
other tests land in the same slice, `windows-fast-gate` discovery should move
from 656 to roughly 662-666.

## Performance Constraints

Required deterministic signals:

- device creation through the backend-neutral interface does not allocate on
  frame submit/present/capture paths;
- factory failure does not leave partially initialized mutable state;
- interface dispatch does not change null backend result values or counters;
- native surface descriptors are small value types;
- existing command storage capacity stays stable across submit/present/capture;
- capability queries remain allocation-free.

Blocking conditions:

- heap-owned polymorphic device output required by the public factory path;
- `std::vector` or heap-owned output returned from creation, present, or capture
  paths;
- adding Windows SDK, D3D11, DXGI, COM, or Platform headers to RHI public
  headers;
- adding a real device, swapchain, GPU adapter query, shader, buffer, texture,
  pipeline, RenderCore, Resource, Package, report, screenshot, or Game Adapter
  dependency;
- registering real hardware smoke or making default `windows-fast-gate` depend
  on a real graphics device.

## Required Tests And Checks

Required non-interactive checks for the first slice:

- `cmake --preset windows-fast-gate`
- `cmake --build --preset windows-fast-gate`
- `ctest --preset windows-fast-gate -N`
- `ctest --preset windows-fast-gate --output-on-failure`
- `ctest --preset windows-fast-gate -N -L RHI`
- `ctest --preset windows-fast-gate -N -L Fast`
- `ctest --preset windows-fast-gate -N -L PerformanceSmoke`
- `ctest --preset windows-fast-gate -N -L EvidenceOracle`
- `ctest --preset windows-fast-gate -N -L HardwareSmoke`
- `ctest --preset windows-hardware-smoke -N`
- `git diff --check`

The implementation handoff must record:

- total deterministic discovery before and after;
- `RHI`, `Fast`, `PerformanceSmoke`, and `EvidenceOracle` discovery counts;
- whether `HardwareSmoke` remains 0;
- whether `windows-fast-gate` remains deterministic and no-real-device;
- proof that RHI public headers do not include Platform, Windows SDK, D3D11, or
  DXGI headers;
- proof that existing Null RHI tests still pass.

## Allowed First Slice

If approved, the first implementation slice may modify or create:

```text
Src/YuEngine/Rhi/Include/YuEngine/Rhi/IRhiDevice.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiNativeSurfaceDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceFactory.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceCreateResult.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiBackendKind.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCapabilities.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceSnapshot.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiStatus.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/NullRhiDevice.h
Src/YuEngine/Rhi/Src/NullRhiDevice.cpp
Tests/Rhi/RhiTests.cpp
CMakeLists.txt
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/gates/P2_GATE_006_RHI_BACKEND_NEUTRAL_DEVICE_BOUNDARY.md
```

`CMakeLists.txt` should change only if new RHI source files are required or new
tests are registered. `CMakePresets.json` should not change in the default first
slice.

## Non-Goals

- No D3D11 implementation.
- No DXGI swapchain.
- No real graphics device.
- No GPU adapter enumeration.
- No shader, texture, buffer, sampler, input layout, or pipeline state.
- No RenderCore.
- No mesh, material, scene traversal, or visible triangle.
- No Resource, Package, File streaming, image decode, or upload queue.
- No real input semantic mapping.
- No UI or gameplay focus policy.
- No report, screenshot, visual oracle, or Game Adapter.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the mature-engine boundary keeps Platform, RHI, RenderCore,
  Resource, and Game Adapter ownership separate;
- 博丽灵梦 confirms the proposed interface/factory layout is locally
  implementable without heap-owned public factory output or frame-path
  allocation growth;
- 雾雨魔理沙 confirms the deterministic test strategy protects the default
  `windows-fast-gate`, keeps `HardwareSmoke` at 0, and does not require real
  D3D11/DXGI hardware;
- 八云紫 confirms this gate remains before P2-GATE-007 and does not authorize
  D3D11 device or swapchain behavior.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_IMPLEMENTABILITY`, or `NEEDS_TEST_POLICY` with exact missing fields.
