# P2-GATE-007: D3D11 Device Swapchain Clear Present Capture

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-004, P2-GATE-005, P2-GATE-006, ADR-0011, ENG-096
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0011
Source baseline: `280d38c`
Proposal commit: `e231951`
Approval evidence: ENG-104A boundary/reference PASS, ENG-104B implementability
PASS, and ENG-104C test-policy PASS.

## Layer

L3 RHI backend boundary, with an isolated Windows-only hardware-smoke fixture.

This gate proposes the first real D3D11 backend slice after the backend-neutral
RHI device boundary. The slice is limited to device/context creation, swapchain
creation from an RHI-owned native surface descriptor, clear, present, and
capture proof.

The gate does not approve mesh rendering, shaders beyond what clear/copy needs,
buffers, textures outside the swapchain/backbuffer path, pipeline state, upload
queues, RenderCore, Resource, Package, screenshots, reports, or Game Adapter
behavior.

```text
Platform native surface
-> caller converts to RhiNativeSurfaceDesc
-> RHI D3D11 device/context/swapchain
-> clear swapchain backbuffer
-> present
-> capture presented backbuffer into caller-owned storage
-> later D3D11 resource and pipeline primitives
```

## Current Reality

Current `YuPlatform` owns a Windows window, bounded event pump, and opaque
`PlatformNativeSurface` value.

Current `YuRHI` owns:

- `IRhiDevice`;
- `RhiDeviceFactory`;
- `RhiNativeSurfaceDesc`;
- `RhiDeviceDesc`;
- `RhiCapabilities`;
- `NullRhiDevice`;
- deterministic submit, present, and capture behavior for the null backend.

Current RHI tests cover 36 tests under the `RHI` label. The current full
`windows-fast-gate` discovers 664 deterministic tests after P2-GATE-006 landed.
`HardwareSmoke` remains 0 and `windows-hardware-smoke` currently discovers 0
tests with zero-test discovery allowed.

`RhiBackendKind::D3D11` exists only as unsupported vocabulary. There is no real
D3D11 device, context, factory, adapter selection, swapchain, render target view,
staging capture path, or hardware smoke.

## Owns

This gate owns the proposal for:

- a Windows-only D3D11 RHI backend implementation behind the existing
  backend-neutral RHI factory;
- a small RHI-owned swapchain descriptor value if the current `RhiDeviceDesc`
  fields are not enough to initialize a D3D11 swapchain;
- a way to expose the swapchain backbuffer as an RHI color target handle through
  the backend-neutral interface;
- clear/present/capture behavior for the swapchain backbuffer;
- explicit failure behavior for invalid native surfaces, invalid swapchain
  descriptors, missing hardware, unsupported formats, and lifecycle misuse;
- CTest isolation that keeps real-device `HardwareSmoke` out of the default
  `windows-fast-gate`;
- deterministic tests for invalid descriptors, boundary contracts, and public
  header dependency cleanliness;
- isolated `HardwareSmoke` tests that prove a real D3D11 clear/present/capture
  path only when a suitable Windows graphics environment is available.

## Does Not Own

This gate does not own:

- shader modules, shader compilation, bytecode loading, input layouts, vertex
  buffers, index buffers, constant buffers, textures, samplers, pipeline state,
  descriptor tables, fences, upload queues, or frame graph behavior;
- mesh, material, scene traversal, UI renderer, visible triangle, static mesh
  fixture, World integration, or Game Adapter behavior;
- Resource, Package, File streaming, image decoding, asset import, or upload
  scheduling;
- GPU adapter selection policy beyond minimal default-adapter creation needed
  for the smoke path;
- screenshot, report, dashboard, visual demo, or manual inspection as proof;
- making hardware availability a requirement for the default fast gate.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Platform owns OS windows, native handle identity, and raw platform events;
- RHI owns D3D11 device/context/swapchain, backend capabilities, present, and
  capture contracts;
- RenderCore owns pass scheduling, render graph decisions, and frame composition
  above RHI;
- Resource owns asset identity, decoded data, and upload policy above File and
  Package boundaries.

YuEngine must not copy UE or Unity source, public API names, private layout, or
module naming. This gate uses those engines only to keep ownership boundaries
separate.

## Dependencies

Allowed dependencies:

- existing `YuRHI` public and private headers;
- `YuPlatform` only in hardware-smoke tests or caller-side conversion fixtures;
- Windows SDK, D3D11, DXGI, and COM usage in Windows-only RHI `.cpp` files or
  private internal headers;
- `d3d11` and `dxgi` private target linkage for `YuRHI` on Windows;
- CMake/CTest preset changes required to isolate `HardwareSmoke` from the
  default `windows-fast-gate`.

Forbidden dependencies:

- `YuPlatform` includes or target linkage from production `YuRHI`;
- Windows SDK, D3D11, DXGI, COM, WGL, SDL, GLFW, Vulkan, OpenGL, or Metal in
  public RHI headers;
- RenderCore, YuInput, World, UI, Resource, Package, File streaming behavior,
  report/evidence modules, tools, or Game Adapter;
- screenshots or reports as backend proof.

Rationale:

The hardware-smoke fixture may link both `YuPlatform` and `YuRHI` because it is
the caller that owns the conversion from `PlatformNativeSurface` to
`RhiNativeSurfaceDesc`. Production `YuRHI` must continue to consume only
RHI-owned descriptor values.

## Public Surface Shape

The first slice should keep public API expansion small. Suggested additions:

- `RhiSwapchainDesc`;
- `RhiSwapchainSnapshot` or a small swapchain section in `RhiDeviceSnapshot`;
- a backend-neutral way to query the swapchain color target handle, if needed;
- D3D11 capability bits through existing `RhiCapabilities` fields;
- additional `RhiStatus` values only if existing values cannot distinguish
  invalid surface, missing hardware, or device loss clearly enough.

Public RHI headers must not include Windows or D3D/DXGI types. Native handles
remain opaque integer values in `RhiNativeSurfaceDesc`.

The D3D11 backend implementation may be private, for example:

```text
Src/YuEngine/Rhi/Src/D3D11RhiDeviceWindows.cpp
Src/YuEngine/Rhi/Src/D3D11RhiDeviceInternal.h
```

The exact class name is not important. The boundary is important: public callers
use `IRhiDevice` and `RhiDeviceFactory`.

## Lifecycle

The intended first-slice lifecycle is:

1. Platform creates a Windows window and exposes `PlatformNativeSurface`.
2. Caller converts `PlatformNativeSurface` into `RhiNativeSurfaceDesc`.
3. Caller fills `RhiDeviceDesc` with `RhiBackendKind::D3D11`, native surface
   data, and swapchain requirements.
4. `RhiDeviceFactory` creates a D3D11-backed `IRhiDevice` through explicit
   caller-visible status or result data.
5. The D3D11 backend creates a device, immediate context, swapchain, backbuffer,
   and render target view.
6. Caller records a clear against the swapchain color target.
7. Submit clears the backbuffer.
8. Present presents the swapchain once.
9. Capture copies the presented backbuffer into caller-owned RGBA8 storage.
10. Cleanup is explicit and leaves no hidden global D3D11 state.

Failure behavior:

- invalid native surface descriptors fail before device creation;
- unsupported formats or extents fail without partial swapchain state;
- missing D3D11 device support returns explicit status and does not pass as
  backend proof;
- device creation failure does not mutate caller-owned output into a live
  interface;
- capture failure writes no bytes past caller-owned capacity;
- default fast gate never depends on a real graphics device.

## Inputs

- `RhiDeviceDesc` with backend kind, native surface, and swapchain requirements;
- optional RHI-owned swapchain descriptor values;
- caller-owned output storage or explicit result data for factory creation;
- caller-owned capture byte storage;
- Platform native surface values converted by the caller or test fixture.

## Outputs

- explicit RHI status or creation result;
- D3D11 backend capability values;
- swapchain/backbuffer target handle or equivalent backend-neutral target access;
- one clear/present/capture proof path;
- capture bytes in caller-owned storage;
- deterministic and hardware-smoke test evidence;
- no screenshot, report, visual demo, or manual inspection as proof.

## Test And Preset Strategy

This gate must first isolate real-device tests:

- update `windows-fast-gate` so `HardwareSmoke` tests do not run by default;
- keep `windows-hardware-smoke` as the only preset that includes
  `HardwareSmoke`;
- require proof that default `windows-fast-gate` still runs deterministic tests
  only;
- label real D3D11 tests `HardwareSmoke`, `D3D11`, `RHI`, and `Win32`;
- label deterministic contract tests `Fast`, `ModuleFixture`, and `RHI`;
- use `PerformanceSmoke` only for bounded allocation/capacity checks;
- use `EvidenceOracle` only for boundary/proof tests.

Hardware-smoke tests may be skipped only when the environment explicitly lacks
the required real D3D11 capability, and skipped/zero hardware-smoke must be
reported as not backend proof. Approval for the first slice requires at least one
accepted run on a Windows graphics environment where the D3D11 smoke actually
executes and captures expected bytes.

Expected deterministic first-slice test growth is about 4 to 8 tests. Hardware
smoke growth is expected to be small, about 1 to 3 tests, and must not enter the
default fast gate.

## Performance Constraints

Required signals:

- public RHI headers remain allocation-free value contracts;
- clear, present, and capture do not allocate unbounded containers on the frame
  path;
- capture writes to caller-owned storage and reports exact byte counts;
- swapchain/backbuffer state has explicit lifecycle and cleanup;
- factory failure does not leave partially initialized D3D11 state;
- default fast gate cost remains close to the current 664 deterministic tests;
- real-device work is isolated under `HardwareSmoke`.

Blocking conditions:

- public RHI headers expose Windows SDK, D3D11, DXGI, COM, or Platform types;
- production `YuRHI` depends on `YuPlatform`;
- default `windows-fast-gate` runs real D3D11 hardware tests;
- the first slice needs mesh, shader modules, buffers, textures beyond the
  swapchain/backbuffer path, pipeline state, RenderCore, Resource, Package,
  report, screenshot, visual demo, or Game Adapter;
- screenshot/manual visual proof replaces capture-byte assertions;
- hidden heap-owned public factory output is required.

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
- one accepted `windows-hardware-smoke` execution on a suitable D3D11 machine
  where the D3D11 smoke test actually runs;
- `git diff --check`

The implementation handoff must record:

- deterministic discovery before and after;
- `RHI`, `D3D11`, `Win32`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke` discovery counts;
- whether default `windows-fast-gate` excludes real hardware work;
- whether `windows-hardware-smoke` discovers the D3D11 smoke tests;
- whether at least one real D3D11 smoke run executed and captured expected
  bytes;
- proof that RHI public headers do not include Platform, Windows SDK, D3D11, or
  DXGI headers;
- proof that existing Null RHI tests still pass.

## Allowed First Slice

If approved, the first implementation slice may modify or create:

```text
Src/YuEngine/Rhi/Include/YuEngine/Rhi/IRhiDevice.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceFactory.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCapabilities.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceSnapshot.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiStatus.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSwapchainDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSwapchainSnapshot.h
Src/YuEngine/Rhi/Src/D3D11RhiDeviceWindows.cpp
Src/YuEngine/Rhi/Src/D3D11RhiDeviceInternal.h
Src/YuEngine/Rhi/Src/NullRhiDevice.cpp
Tests/Rhi/RhiTests.cpp
Tests/Rhi/RhiD3D11HardwareSmokeTests.cpp
CMakeLists.txt
CMakePresets.json
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/gates/P2_GATE_007_D3D11_DEVICE_SWAPCHAIN_CLEAR_PRESENT_CAPTURE.md
```

`CMakePresets.json` may change only to protect `windows-fast-gate` from
`HardwareSmoke` and keep `windows-hardware-smoke` as the explicit hardware lane.

## Non-Goals

- No visible triangle.
- No mesh.
- No shader module boundary beyond any D3D11 API calls strictly required for
  clear/copy.
- No vertex, index, constant, structured, or upload buffers.
- No texture creation outside swapchain/backbuffer and staging capture.
- No input layout, pipeline state, material, or render pass abstraction.
- No RenderCore.
- No Resource, Package, File streaming, image decode, or upload queue.
- No UI, World, gameplay, report, screenshot, visual oracle, or Game Adapter.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the mature-engine boundary keeps Platform, RHI, RenderCore,
  Resource, and Game Adapter ownership separate and accepts D3D11/DXGI as
  private RHI implementation details only;
- 博丽灵梦 confirms the proposed file layout and API additions are locally
  implementable without public Windows/DXGI pollution or heap-owned public
  factory output;
- 雾雨魔理沙 confirms the test/preset policy keeps default `windows-fast-gate`
  deterministic while giving D3D11 hardware smoke an isolated execution lane;
- 八云紫 confirms this gate remains before P2-GATE-008 and does not authorize
  resource/pipeline primitives, visible geometry, RenderCore, or Game Adapter
  behavior.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_IMPLEMENTABILITY`, or `NEEDS_TEST_POLICY` with exact missing fields.

## Approval Decision

P2-GATE-007 is approved for first slice after ENG-104 review closure.

Hard implementation conditions:

- The first slice remains L3 RHI D3D11 backend clear/present/capture only.
- D3D11, DXGI, and COM usage must stay in Windows-only private RHI source or
  private internal headers. Public RHI headers must not expose Windows SDK,
  D3D11, DXGI, COM, or Platform types.
- Production `YuRHI` must not include or link `YuPlatform`. Platform native
  surface conversion belongs to the caller, test fixture, or later adapter.
- `CMakePresets.json` may change only to isolate `HardwareSmoke` from the
  default `windows-fast-gate` and keep `windows-hardware-smoke` as the explicit
  hardware lane.
- Default `windows-fast-gate` must remain deterministic and must not run real
  D3D11 hardware tests.
- `windows-hardware-smoke` must discover the D3D11 smoke tests, and first-slice
  acceptance requires at least one suitable Windows graphics run where those
  tests actually execute and validate expected capture bytes.
- Zero or skipped hardware smoke is not backend proof.
- Backend proof must be capture-byte assertions into caller-owned storage, not
  screenshot, report, dashboard, visual demo, or manual inspection.
- Factory ownership must remain caller-owned or explicit-cleanup based. The
  public factory path must not return heap-owned polymorphic D3D11 device output.
- Swapchain backbuffer access may extend the backend-neutral RHI surface, but it
  must stay small and must not introduce shader, buffer, texture, pipeline,
  RenderCore, Resource, Package, report, screenshot, visual proof, UI, World, or
  Game Adapter scope.
- Allowed implementation paths must use exact repository casing:
  `Src/YuEngine/Rhi/...` and `Tests/Rhi/...`.
