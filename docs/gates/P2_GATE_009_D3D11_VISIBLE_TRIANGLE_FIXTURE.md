# P2-GATE-009: D3D11 Visible Triangle Fixture

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `NOT_APPROVED`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P2-GATE-004, P2-GATE-005, P2-GATE-006, P2-GATE-007, P2-GATE-008,
ADR-0011, ENG-096, ENG-107
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0011
Source baseline: `2f7c8e1`

## Layer

L3 RHI backend boundary, with an isolated Windows-only hardware-smoke fixture.

This gate proposes the first visible geometry proof after the landed D3D11
device/swapchain and resource/pipeline primitive slices. The slice is limited to
RHI-owned draw command contracts that render one synthetic triangle through the
existing D3D11 backend and validate the result through capture bytes.

The gate does not approve RenderCore, render graph, frame pass scheduling,
material system, mesh asset pipeline, Resource loading, Package/File streaming,
shader authoring tools, shader compiler integration, UI, World, report,
screenshot, visual demo, or Game Adapter behavior.

```text
Synthetic vertex bytes and precompiled shader bytecode
-> RHI buffer / shader / input-layout / pipeline primitives
-> bounded RHI draw command records
-> D3D11 draw into the swapchain backbuffer
-> present
-> capture bytes with deterministic triangle coverage assertions
-> later static mesh fixture and RenderCore entry gates
```

## Current Reality

P2-GATE-007 landed the first real D3D11 device/context/swapchain path:

- device/context/swapchain creation;
- swapchain backbuffer color-target query;
- clear, present, and capture into caller-owned bytes;
- isolated `HardwareSmoke` proof outside the default `windows-fast-gate`.

P2-GATE-008 landed the first D3D11 primitive resource and pipeline slice:

- backend-neutral buffer, texture, sampler, shader module, input layout,
  pipeline, and lightweight fence contracts;
- caller-owned byte spans and precompiled shader bytecode input;
- private D3D11 resource and pipeline objects;
- deterministic Null RHI primitive lifecycle tests;
- isolated real D3D11 primitive resource/pipeline hardware-smoke proof.

Current `YuRHI` still has no public draw command, vertex-buffer binding command,
pipeline binding command, viewport/scissor policy, visible geometry proof,
RenderCore, mesh fixture, material system, Resource upload path, or shader
compiler integration.

## Owns

This gate owns the proposal for:

- minimal backend-neutral draw command records needed to bind a pipeline, bind a
  vertex buffer, and issue a bounded draw call;
- deterministic Null RHI validation for draw command ordering, invalid handles,
  capacity failure, and lifecycle misuse;
- private D3D11 implementation of the minimal draw path using already approved
  primitive resource and pipeline objects;
- one synthetic triangle hardware-smoke fixture using caller-owned vertex bytes
  and precompiled shader bytecode;
- capture-byte assertions that prove the draw path changed expected pixels
  without using screenshots, reports, dashboards, or manual visual inspection;
- CMake/CTest labels that keep real D3D11 work isolated under
  `HardwareSmoke`, `D3D11`, `RHI`, and `Win32`.

## Does Not Own

This gate does not own:

- RenderCore, render graph, render passes, render queues, frame scheduling, or
  scene rendering policy;
- static mesh asset pipeline, mesh file loading, material system, shader
  authoring language, shader compiler invocation, shader build tools,
  reflection, hot reload, or shader cache;
- Resource, Package, File streaming, image decoding, texture import, upload
  queues, dependency graphs, or asset lifetime policy;
- depth/stencil, blending models, descriptor tables, bindless resources,
  multiple render targets, instancing, indexed draws, indirect draws, compute,
  Vulkan/Metal/OpenGL portability, or cross-backend feature expansion;
- UI, World, gameplay, report, screenshot, visual demo, manual inspection, or
  Game Adapter behavior.

## Mature-Engine Boundary

UE and Unity are responsibility references only.

The intended responsibility split is:

- Platform owns OS windows, native handle identity, and raw platform events;
- RHI owns GPU resource handles, pipeline objects, draw command encoding,
  present, capture, and backend synchronization details;
- RenderCore owns frame passes, render queues, render graph decisions, and scene
  render submission above RHI;
- Resource owns asset identity, decoded data, dependency lifetime, and upload
  policy above File and Package boundaries.

YuEngine must not copy UE or Unity source, public API names, private layout, or
module naming. This gate uses those engines only to keep ownership boundaries
separate.

## Dependencies

Allowed dependencies:

- existing `YuRHI` public and private headers;
- existing P2-GATE-007 and P2-GATE-008 D3D11 backend implementation;
- Windows SDK, D3D11, DXGI, and COM usage in Windows-only RHI `.cpp` files or
  private internal headers;
- private `d3d11` and `dxgi` target linkage already owned by `YuRHI`;
- `YuPlatform` only in hardware-smoke fixtures that need a real window and
  native surface;
- fixed precompiled shader bytecode bytes embedded in the RHI hardware-smoke
  fixture or test helper as caller-owned input.

Forbidden dependencies:

- `YuPlatform` includes or target linkage from production `YuRHI`;
- Windows SDK, D3D11, DXGI, COM, WGL, SDL, GLFW, Vulkan, OpenGL, or Metal in
  public RHI headers;
- RenderCore, YuInput, World, UI, Resource, Package, File streaming behavior,
  report/evidence modules, tools, or Game Adapter;
- shader compiler libraries, shader build tools, shader source loading, or
  generated shader asset pipelines as required production dependencies;
- screenshots or reports as backend proof.

Rationale:

The first visible triangle should prove that RHI can execute a minimal draw
using its own resource and pipeline primitives. It must not decide how a renderer
will sort passes, how assets become GPU data, or how a game scene becomes draw
submissions.

## Public Surface Shape

The first slice should keep public API expansion small and value-based.
Suggested additions:

- `RhiPrimitiveTopology`, limited to triangle list if a topology value is
  required;
- a minimal extension to the existing `RhiInputLayoutDesc` contract so a caller
  can describe vertex input slots and attributes with backend-neutral values;
- `RhiVertexBufferView` or equivalent value contract with a buffer handle,
  offset, stride, and byte range;
- `RhiDrawDesc` or bounded draw fields containing vertex count and first vertex;
- `RhiCommandType` and `RhiCommandRecord` additions for binding a pipeline,
  binding a vertex buffer, and drawing;
- narrow `RhiCommandList` helpers that validate ordering and capacity without
  allocating during command recording;
- snapshot counters for submitted draw calls if needed for deterministic Null
  RHI verification.

Public RHI headers must not include Windows, D3D, DXGI, COM, Platform,
RenderCore, Resource, Package, UI, World, report, screenshot, visual proof, or
Game Adapter types.

Shader modules still accept caller-provided bytecode as bytes. Shader source
language and compilation are not part of this gate.

## Lifecycle

The intended first-slice lifecycle is:

1. Caller creates a D3D11 `IRhiDevice` through the landed caller-owned factory.
2. Caller creates a swapchain color target through the P2-GATE-007 path.
3. Caller creates a vertex buffer, vertex shader, pixel shader, input layout,
   and pipeline through the P2-GATE-008 primitive contracts.
4. Caller records a bounded command list that clears the target, binds the
   pipeline, binds the vertex buffer, draws one triangle, and ends the frame.
5. RHI validates command ordering, handle generations, buffer ranges, pipeline
   compatibility, and draw bounds before or during submit.
6. D3D11 executes the draw into the swapchain backbuffer.
7. Caller presents and captures the presented frame into caller-owned RGBA8
   storage.
8. The hardware-smoke fixture asserts deterministic byte evidence for triangle
   coverage and background color.
9. Caller destroys primitive handles explicitly, and device destruction releases
   any remaining private native objects.

Failure behavior:

- invalid handles fail explicitly and do not submit partial draw work;
- draw before binding a valid pipeline or vertex buffer returns explicit status;
- vertex buffer range overflow returns explicit status;
- command capacity overflow returns `CapacityExceeded` and does not mutate the
  command list;
- capture with undersized output writes no bytes past caller-owned storage;
- missing hardware or unsupported shader bytecode is reported as explicit
  status and is not accepted as backend proof;
- default fast gate never depends on a real graphics device.

## Inputs

- caller-owned vertex byte spans;
- caller-owned precompiled vertex and pixel shader bytecode spans;
- backend-neutral buffer, shader module, input layout, pipeline, and command
  descriptors;
- existing D3D11 device/swapchain setup from P2-GATE-007 for hardware-smoke
  fixtures only;
- caller-owned capture byte storage.

## Outputs

- explicit RHI status values;
- deterministic Null RHI command validation results and snapshots;
- one real D3D11 visible-triangle capture-byte proof;
- draw counters or snapshot fields if required for proof;
- no screenshot, report, dashboard, visual demo, or manual inspection as proof.

## Test And Preset Strategy

Default `windows-fast-gate` must stay deterministic:

- deterministic contract tests are labeled `Fast`, `ModuleFixture`, and `RHI`;
- real D3D11 visible-triangle tests are labeled `HardwareSmoke`, `D3D11`,
  `RHI`, and `Win32`;
- `HardwareSmoke` remains excluded from default `windows-fast-gate`;
- `windows-hardware-smoke` remains the explicit real-device lane;
- `PerformanceSmoke` is used only for bounded command capacity and allocation
  checks;
- `EvidenceOracle` is used only for boundary/proof tests.

Hardware-smoke tests may be skipped only when the environment explicitly lacks
the required D3D11 capability. Skipped or zero hardware-smoke is not visible
geometry proof. Accepted proof requires at least one Windows graphics run where
the triangle fixture actually executes and validates capture-byte evidence.

Expected deterministic first-slice growth is about 4 to 10 tests. Hardware
smoke growth is expected to be about 1 test and must not enter the default fast
gate.

## Performance Constraints

Required signals:

- public draw descriptors are small value contracts;
- vertex data and shader bytecode are caller-owned spans at API boundaries;
- command recording uses bounded storage and does not allocate or grow during
  the measured frame path;
- submit validates command state without Resource, RenderCore, or scene graph
  lookup;
- capture writes to caller-owned storage and reports exact byte counts;
- default fast gate cost remains close to the current deterministic test count;
- real-device work is isolated under `HardwareSmoke`.

Blocking conditions:

- public RHI headers expose Windows SDK, D3D11, DXGI, COM, or Platform types;
- production `YuRHI` depends on `YuPlatform`;
- shader compilation, shader source tooling, mesh assets, RenderCore, Resource,
  Package/File streaming, UI, World, report, screenshot, visual demo, or Game
  Adapter scope is required;
- screenshot/manual visual proof replaces capture-byte assertions;
- the first slice requires static mesh files, material semantics, asset
  pipeline, or scene traversal;
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
  where the visible-triangle smoke test actually runs;
- `git diff --check`

The implementation handoff must record:

- deterministic discovery before and after;
- `RHI`, `D3D11`, `Win32`, `Fast`, `PerformanceSmoke`, `EvidenceOracle`, and
  `HardwareSmoke` discovery counts;
- whether default `windows-fast-gate` excludes real hardware work;
- whether `windows-hardware-smoke` discovers the visible-triangle smoke test;
- whether at least one real D3D11 visible-triangle smoke run executed;
- capture-byte proof shape and sampled byte expectations;
- proof that RHI public headers do not include Platform, Windows SDK, D3D11, or
  DXGI headers;
- proof that existing Null RHI, D3D11 clear/present/capture, and primitive
  resource/pipeline tests still pass.

## Allowed First Slice

If approved, the first implementation slice may modify or create:

```text
Src/YuEngine/Rhi/Include/YuEngine/Rhi/IRhiDevice.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandList.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandListSnapshot.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandRecord.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandType.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDeviceSnapshot.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiDrawDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiInputLayoutDesc.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPrimitiveTopology.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiStatus.h
Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiVertexBufferView.h
Src/YuEngine/Rhi/Src/D3D11RhiDeviceWindows.cpp
Src/YuEngine/Rhi/Src/D3D11RhiDeviceInternal.h
Src/YuEngine/Rhi/Src/NullRhiDevice.cpp
Src/YuEngine/Rhi/Src/RhiCommandList.cpp
Tests/Rhi/RhiTests.cpp
Tests/Rhi/RhiD3D11HardwareSmokeTests.cpp
CMakeLists.txt
docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md
docs/gates/P2_GATE_009_D3D11_VISIBLE_TRIANGLE_FIXTURE.md
```

`CMakePresets.json` is not expected to change. Any amendment must be limited to
preserving `HardwareSmoke` isolation and must not weaken the default fast gate.

## Non-Goals

- No RenderCore.
- No render graph.
- No frame pass abstraction.
- No static mesh fixture.
- No mesh asset pipeline.
- No material system.
- No Resource, Package, File streaming, image decode, or upload queue.
- No shader source language, compiler invocation, shader build tool, reflection,
  hot reload, or shader cache.
- No texture sampling fixture beyond what the triangle byte proof requires.
- No depth/stencil, blending model, multi-target rendering, instancing, indexed
  draw, indirect draw, or compute.
- No UI, World, gameplay, report, screenshot, visual oracle, or Game Adapter.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- 八云蓝 confirms the mature-engine boundary keeps RHI visible-triangle proof
  separate from RenderCore, Resource, mesh, material, scene, and Game Adapter
  ownership;
- 博丽灵梦 confirms the proposed file layout and API additions are locally
  implementable without public Windows/DXGI pollution, shader compiler
  dependency, or heap-owned public factory output;
- 雾雨魔理沙 confirms the test/preset policy keeps default `windows-fast-gate`
  deterministic and treats visible-triangle proof as isolated hardware-smoke
  capture-byte evidence;
- 八云紫 confirms this gate remains after P2-GATE-008 and before static mesh,
  RenderCore, Resource upload, UI, World, or Game Adapter gates.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_IMPLEMENTABILITY`, or `NEEDS_TEST_POLICY` with exact missing fields.
