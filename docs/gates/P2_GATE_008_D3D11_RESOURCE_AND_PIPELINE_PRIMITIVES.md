# P2-GATE-008: D3D11 Resource And Pipeline Primitives

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-004, P2-GATE-006, P2-GATE-007, ADR-0011, ENG-096
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0011
Source baseline: `fee76d6`
Approval evidence: ENG-106A boundary/reference PASS, ENG-106B implementability
PASS, and ENG-106C test-policy PASS.

## Layer

L3 RHI backend boundary, extending the landed D3D11 device/swapchain slice with
resource and pipeline primitive contracts.

This gate proposes the first D3D11 resource and pipeline primitive slice after
P2-GATE-007. The slice is limited to RHI-owned handles, descriptors, creation,
destruction, update/upload, and synchronization contracts for backend objects.

The gate does not approve visible geometry, draw-call proof, mesh asset
pipeline, shader authoring, shader compilation tools, RenderCore, material
system, Resource loading, Package/File streaming, UI, World, report, screenshot,
visual demo, or Game Adapter behavior.

```text
Caller-owned primitive data and precompiled shader bytecode
-> RHI buffer / texture / sampler / shader / input-layout / pipeline descriptors
-> RHI D3D11 resource and pipeline objects
-> optional upload/update/fence proof
-> later visible triangle fixture
```

## Current Reality

P2-GATE-007 landed the first real D3D11 backend path:

- D3D11 device/context/swapchain creation;
- swapchain backbuffer color-target query;
- clear/present/capture into caller-owned bytes;
- caller-owned factory storage and explicit device destruction;
- `HardwareSmoke` isolated from default `windows-fast-gate`.

Current `YuRHI` owns:

- `IRhiDevice`;
- `RhiDeviceFactory`;
- `RhiNativeSurfaceDesc`;
- `RhiDeviceDesc`;
- `RhiSwapchainDesc`;
- `RhiSwapchainSnapshot`;
- `RhiTextureHandle` for color targets;
- `RhiCommandList` with begin-frame, clear, and end-frame records;
- `NullRhiDevice`;
- private Windows-only `D3D11RhiDevice`.

Current RHI tests discover 41 tests under the `RHI` label. The current default
`windows-fast-gate` discovers 669 deterministic tests and excludes
`HardwareSmoke`, `D3D11`, and `Win32` tests. `windows-hardware-smoke` discovers
1 real D3D11 clear/present/capture test.

There are no public RHI buffer, shader module, input layout, pipeline state,
texture, sampler, upload/update, or fence contracts yet.

## Owns

This gate owns the proposal for:

- backend-neutral RHI handles and descriptors for vertex buffers, index buffers,
  constant buffers, textures, samplers, shader modules, input layouts, pipeline
  states, and lightweight fences;
- D3D11 private implementation for creating and destroying those primitive
  objects;
- caller-owned upload/update contracts for bounded buffer and texture data;
- backend-neutral status and snapshot fields for primitive object counts and
  failure reasons;
- deterministic Null RHI contract behavior for invalid descriptors, capacity,
  lifecycle, and dependency cleanliness;
- D3D11 hardware-smoke proof that primitive objects can be created, updated,
  and destroyed on a real backend without entering the default fast gate.

## Does Not Own

This gate does not own:

- visible triangle, draw command execution, render pass abstraction, frame graph,
  material system, mesh asset pipeline, static mesh fixture, scene traversal, UI
  renderer, World integration, or Game Adapter behavior;
- shader source language, shader compiler invocation, build-time shader tools,
  reflection, include processing, hot reload, or shader cache;
- Resource, Package, File streaming, image decoding, asset import, or upload
  scheduling policy;
- descriptor tables, bindless resources, root signatures, Vulkan/Metal/OpenGL
  abstractions, or cross-backend portability beyond backend-neutral value
  contracts;
- screenshots, reports, dashboard output, visual demos, or manual inspection as
  proof.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Platform owns OS windows, native handle identity, and raw platform events;
- RHI owns GPU resource handles, native D3D11 objects, pipeline-state objects,
  upload/update commands, fences, capabilities, and explicit lifecycle;
- RenderCore owns pass scheduling, draw ordering, render graph decisions, and
  frame composition above RHI;
- Resource owns asset identity, decoded data, and upload policy above File and
  Package boundaries.

YuEngine must not copy UE or Unity source, public API names, private layout, or
module naming. This gate uses those engines only to keep ownership boundaries
separate.

## Dependencies

Allowed dependencies:

- existing `YuRHI` public and private headers;
- Windows SDK, D3D11, DXGI, and COM usage in Windows-only RHI `.cpp` files or
  private internal headers;
- private `d3d11` and `dxgi` target linkage already owned by `YuRHI`;
- `YuPlatform` only in hardware-smoke fixtures that need a real swapchain;
- CMake/CTest additions for deterministic RHI tests and isolated
  `HardwareSmoke` tests.

Forbidden dependencies:

- `YuPlatform` includes or target linkage from production `YuRHI`;
- Windows SDK, D3D11, DXGI, COM, WGL, SDL, GLFW, Vulkan, OpenGL, or Metal in
  public RHI headers;
- RenderCore, YuInput, World, UI, Resource, Package, File streaming behavior,
  report/evidence modules, tools, or Game Adapter;
- shader compiler libraries, shader build tools, or generated shader assets as
  required production dependencies;
- screenshots or reports as backend proof.

Rationale:

The first primitive slice should prove that RHI can own low-level D3D11 resource
and pipeline objects without deciding how a renderer, asset pipeline, material
system, or scene graph will use them.

## Public Surface Shape

The first slice should keep public API expansion value-based and small.
Suggested public additions:

- `RhiBufferDesc`, `RhiBufferHandle`, and `RhiBufferUsage`;
- `RhiTextureDesc` for non-swapchain texture creation;
- `RhiSamplerDesc` and `RhiSamplerHandle`;
- `RhiShaderModuleDesc`, `RhiShaderModuleHandle`, and `RhiShaderStage`;
- `RhiInputLayoutDesc`;
- `RhiPipelineDesc` and `RhiPipelineHandle`;
- `RhiFenceHandle` or a small fence status value if needed for upload/update
  proof;
- `RhiResourceSnapshot` or small resource sections in `RhiDeviceSnapshot`;
- narrow `IRhiDevice` methods for create, update, destroy, and query operations.

Public RHI headers must not include Windows, D3D, DXGI, or COM types. Shader
modules accept caller-provided bytecode as bytes; shader source and compilation
are not part of this gate.

The private D3D11 implementation may continue in:

```text
Src/YuEngine/Rhi/Src/D3D11RhiDeviceWindows.cpp
Src/YuEngine/Rhi/Src/D3D11RhiDeviceInternal.h
```

Additional private `.cpp` or internal header files are allowed only if they
remain under `Src/YuEngine/Rhi/Src/` and do not expose new public dependencies.

## Lifecycle

The intended first-slice lifecycle is:

1. Caller creates a D3D11 `IRhiDevice` through the landed caller-owned factory.
2. Caller provides value descriptors for buffers, textures, samplers, shader
   modules, input layouts, and pipeline states.
3. RHI validates descriptor sizes, formats, usage flags, shader stages, and
   object capacities before native creation.
4. RHI creates private D3D11 resources and returns backend-neutral handles.
5. Caller uploads or updates bounded caller-owned bytes through RHI methods or
   command records.
6. RHI optionally signals a lightweight fence or records upload completion
   status.
7. Caller destroys primitive handles explicitly.
8. Device destruction releases all remaining private native objects without
   hidden global state.

Failure behavior:

- invalid descriptors fail before native object creation;
- unsupported formats, usages, or shader stages return explicit status;
- capacity overflow returns `CapacityExceeded`;
- invalid handles and lifecycle misuse return explicit status;
- upload/update methods never read or write past caller-owned byte spans;
- factory or creation failure does not leave a live public handle;
- default fast gate never depends on a real graphics device.

## Inputs

- value descriptors for buffers, textures, samplers, shader modules, input
  layouts, pipeline states, and fences;
- caller-owned byte spans for buffer data, texture data, and precompiled shader
  bytecode;
- existing D3D11 device/swapchain setup from P2-GATE-007 for hardware-smoke
  fixtures only.

## Outputs

- explicit RHI status values;
- backend-neutral resource and pipeline handles;
- primitive object counts and capacity snapshots;
- upload/update byte counts or completion statuses;
- deterministic and hardware-smoke test evidence;
- no screenshot, report, visual demo, or manual inspection as proof.

## Test And Preset Strategy

Default `windows-fast-gate` must stay deterministic:

- deterministic contract tests are labeled `Fast`, `ModuleFixture`, and `RHI`;
- real D3D11 primitive tests are labeled `HardwareSmoke`, `D3D11`, `RHI`, and
  `Win32`;
- `HardwareSmoke` remains excluded from default `windows-fast-gate`;
- `windows-hardware-smoke` remains the explicit real-device lane;
- `PerformanceSmoke` is used only for bounded capacity/allocation checks;
- `EvidenceOracle` is used only for boundary/proof tests.

Hardware-smoke tests may be skipped only when the environment explicitly lacks
the required D3D11 capability. Skipped or zero hardware-smoke is not primitive
backend proof. Accepted proof requires at least one Windows graphics run where
the primitive smoke actually creates and updates D3D11 objects and validates the
expected status/snapshot evidence.

Expected deterministic first-slice growth is about 8 to 14 tests. Hardware smoke
growth is expected to be about 1 to 3 tests and must not enter the default fast
gate.

## Performance Constraints

Required signals:

- public RHI descriptors are small value contracts;
- shader bytecode and upload data are caller-owned spans at API boundaries;
- resource and pipeline object storage has explicit capacity and lifecycle;
- update/upload paths are bounded and report byte counts or explicit status;
- failure paths release partially created native objects;
- default fast gate cost remains close to the current 669 deterministic tests;
- real-device work is isolated under `HardwareSmoke`.

Blocking conditions:

- public RHI headers expose Windows SDK, D3D11, DXGI, COM, or Platform types;
- production `YuRHI` depends on `YuPlatform`;
- shader compilation, shader source tooling, mesh assets, RenderCore, Resource,
  Package/File streaming, UI, World, report, screenshot, visual demo, or Game
  Adapter scope is required;
- visible triangle or draw proof is required in this gate;
- screenshot/manual visual proof replaces object/status/update assertions;
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
- `ctest --preset windows-fast-gate -N -L D3D11`
- `ctest --preset windows-fast-gate -N -L Win32`
- `ctest --preset windows-hardware-smoke -N`
- one accepted `windows-hardware-smoke` execution on a suitable D3D11 machine
  where primitive creation/update tests actually run;
- `git diff --check`

The implementation handoff must record:

- deterministic discovery before and after;
- `RHI`, `D3D11`, `Win32`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke` discovery counts;
- whether default `windows-fast-gate` excludes real hardware work;
- whether `windows-hardware-smoke` discovers the primitive smoke tests;
- whether at least one real D3D11 primitive smoke run executed;
- proof that RHI public headers do not include Platform, Windows SDK, D3D11, or
  DXGI headers;
- proof that existing Null RHI and D3D11 clear/present/capture tests still pass.

## Allowed First Slice

If approved, the first implementation slice may modify or create:

```text
Src/YuEngine/Rhi/Include/YuEngine/Rhi/IRhiDevice.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiBufferDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiBufferHandle.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiBufferUsage.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCapabilities.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandList.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandListSnapshot.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandRecord.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandType.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceSnapshot.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiFenceHandle.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiFormat.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiInputLayoutDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPipelineDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPipelineHandle.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiResourceSnapshot.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSamplerDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiSamplerHandle.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiShaderModuleDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiShaderModuleHandle.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiShaderStage.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiStatus.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiTextureDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiTextureHandle.h
Src/YuEngine/Rhi/Src/D3D11RhiDeviceWindows.cpp
Src/YuEngine/Rhi/Src/D3D11RhiDeviceInternal.h
Src/YuEngine/Rhi/Src/NullRhiDevice.cpp
Src/YuEngine/Rhi/Src/RhiCommandList.cpp
Tests/Rhi/RhiTests.cpp
Tests/Rhi/RhiD3D11HardwareSmokeTests.cpp
CMakeLists.txt
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/gates/P2_GATE_008_D3D11_RESOURCE_AND_PIPELINE_PRIMITIVES.md
```

`CMakePresets.json` is not expected to change. Any amendment to it must be
limited to preserving `HardwareSmoke` isolation and must not weaken the default
fast gate.

## Non-Goals

- No visible triangle.
- No draw call proof.
- No mesh or static mesh fixture.
- No material system.
- No RenderCore or render graph.
- No Resource, Package, File streaming, image decode, or asset upload policy.
- No shader source language, compiler invocation, shader build tool, or shader
  asset pipeline.
- No descriptor table abstraction beyond the minimal input layout and pipeline
  values needed to create D3D11 objects.
- No UI, World, gameplay, report, screenshot, visual oracle, or Game Adapter.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the mature-engine boundary keeps RHI resource/pipeline
  ownership separate from RenderCore, Resource, mesh, material, scene, and Game
  Adapter ownership;
- 博丽灵梦 confirms the proposed file layout and API additions are locally
  implementable without public Windows/DXGI pollution, shader compiler
  dependency, or heap-owned public factory output;
- 雾雨魔理沙 confirms the test/preset policy keeps default `windows-fast-gate`
  deterministic and treats real D3D11 primitive smoke as isolated hardware
  evidence;
- 八云紫 confirms this gate remains after P2-GATE-007 and before P2-GATE-009,
  and does not authorize visible geometry, RenderCore, Resource, or Game Adapter
  behavior.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_IMPLEMENTABILITY`, or `NEEDS_TEST_POLICY` with exact missing fields.

## Approval Decision

P2-GATE-008 is approved for first slice after ENG-106 review closure.

Hard implementation conditions:

- The first slice remains L3 RHI resource and pipeline primitive contracts only.
- Public RHI headers may expose backend-neutral handles, value descriptors,
  snapshots, status values, and caller-owned byte-span contracts only.
- Public RHI headers must not expose Windows SDK, D3D11, DXGI, COM, Platform,
  RenderCore, Resource, Package, File, UI, World, report, screenshot, visual
  proof, or Game Adapter types.
- D3D11, DXGI, COM, and Windows usage must stay in Windows-only private RHI
  implementation files or private internal headers.
- Production `YuRHI` must not include or link `YuPlatform`; `YuPlatform` may be
  used only by hardware-smoke fixtures.
- Shader modules accept caller-provided bytecode only. Shader source language,
  compiler invocation, shader build tools, generated shader assets, reflection,
  and shader caches are not approved.
- Visible triangle, draw proof, mesh, static mesh fixture, material system,
  scene traversal, RenderCore, Resource, Package/File streaming, report,
  screenshot, visual proof, UI, World, and Game Adapter behavior are not
  approved.
- P2-GATE-009 remains the separate visible-triangle gate and is still not
  approved.
- `CMakePresets.json` is not expected to change. Any amendment must be limited
  to preserving `HardwareSmoke` isolation and must not weaken the default fast
  gate.
- Default `windows-fast-gate` must remain deterministic and must not run real
  D3D11 hardware tests.
- `windows-hardware-smoke` must remain the explicit hardware lane. First-slice
  evidence must include at least one suitable Windows graphics run where the
  primitive smoke actually creates or updates D3D11 primitive objects and
  validates expected status or snapshot evidence.
- Skipped or zero hardware smoke is not primitive backend proof.
- Backend proof must be status/snapshot/update assertions, not screenshot,
  report, dashboard, visual demo, or manual inspection.
- Factory ownership must remain caller-owned or explicit-cleanup based. The
  public factory path must not return heap-owned polymorphic D3D11 device
  output.
- Allowed implementation paths must use exact repository casing:
  `Src/YuEngine/Rhi/...` and `Tests/Rhi/...`.
