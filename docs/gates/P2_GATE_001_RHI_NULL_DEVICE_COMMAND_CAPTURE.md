# P2-GATE-001: Null RHI Device, Command, Present, And Capture

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文 if evidence boundary is questioned, 雾雨魔理沙 when implementation exists
Depends on: ADR-0011
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0009
Source baseline: Phase 2 through `eb79d22`

## Layer

L3 RHI boundary.

This gate proves a null backend with device creation, resource handle behavior,
bounded command recording, submit, present, and deterministic capture. It does
not introduce a real graphics backend, RenderCore, UI, resource loading, shader
loading, or game adapter behavior.

## Owns

This gate owns the first `YuRHI` implementation slice for:

- RHI backend kind vocabulary;
- null RHI device;
- device capabilities;
- RHI format and extent value types;
- exact `RGBA8_UNORM` byte-channel clear colors;
- generation-checked texture/color-target handles;
- bounded command list;
- clear-color command behavior;
- submit/present lifecycle;
- deterministic capture into caller-owned buffers;
- explicit RHI result/status values;
- deterministic counters for tests.

## Does Not Own

This gate does not own:

- D3D, Vulkan, OpenGL, Metal, SDL, GLFW, or OS window backend;
- real swapchain lifecycle;
- shader compiler, shader bytecode, pipeline state, or material semantics;
- mesh, render graph, RenderCore, render scene, UI renderer, or frame graph;
- Resource/File package reads, image decoding, asset import, or upload queues;
- async jobs, render thread, GPU fences, worker scheduling, or frame scheduler;
- report/capture/oracle/dashboard/editor output;
- TouhouNewWorld adapter behavior.

## UE/Unity Analogue

UE5 references:

- `Runtime\RHI`, `Runtime\RHICore`, `Runtime\NullDrv`, `Runtime\D3D12RHI`, and
  `Runtime\VulkanRHI` as responsibility references.

Unity references:

- Scriptable Render Pipeline concepts as a separation between render pipeline
  code and low-level graphics command execution.

YuEngine decision:

- Start with a null backend that exercises the same public boundary expected of
  future real backends.
- Keep RenderCore, scene traversal, materials, UI, and original-game renderer
  facts above or outside RHI.
- Count renderer progress only when submit, present, and capture are testable.

## Lifecycle

First-slice lifecycle:

1. Setup creates a null RHI device with fixed capacities.
2. Setup creates one or more synthetic RGBA8 color targets within bounds.
3. Frame setup resets a bounded command list without freeing storage.
4. Command recording writes `BeginFrame`, `ClearColor`, and `EndFrame` style
   commands within fixed capacity.
5. Submit validates lifecycle and executes commands on the null target model.
6. Present records the submitted target as the presented frame.
7. Capture copies the presented target into caller-owned RGBA8 storage.
8. Shutdown destroys targets and invalidates stale-generation handles.

Failure behavior:

- unsupported backend kind returns explicit unsupported status;
- unsupported format returns explicit format status and does not mutate device
  state;
- invalid or zero extent returns explicit invalid-descriptor status;
- target capacity overflow returns explicit capacity status and does not mutate
  target state;
- command-list capacity overflow returns explicit capacity status and does not
  mutate recorded commands;
- invalid or stale target handle returns explicit handle status;
- submit before `EndFrame` or without a valid target returns explicit lifecycle
  status;
- present without a successful submit returns explicit lifecycle status;
- capture before present returns explicit lifecycle status;
- capture with undersized destination returns explicit capacity status and does
  not write partial pixels, leaves destination bytes unchanged, and reports
  `capture_bytes_written == 0`;
- disabled diagnostics/logging does not change any RHI result.

## Inputs

- null backend device descriptor;
- fixed RGBA8 color target descriptors;
- fixed clear colors as exact `RGBA8_UNORM` byte channels;
- bounded command-list capacity;
- caller-owned capture buffers;
- optional memory tracker if P1-GATE-002 implementation is accepted.

## Outputs

- RHI device capabilities;
- texture/color-target handles;
- RHI result/status values;
- submit/present frame tokens;
- capture bytes in caller-owned storage;
- deterministic counters for created/destroyed targets, recorded commands,
  submitted frames, presented frames, captured frames, failed operations, and
  allocation/accounting status.

## Dependencies

Allowed dependencies:

- C++ standard library;
- CMake/CTest tooling;
- `YuMemory` for accounting vocabulary/signal tests when available and accepted;
- `YuDiagnostics` only for disabled-behavior observation when available.

Target dependency expectation:

```text
YuRHI
  -> optional YuMemory for accounting vocabulary/signal tests
  -> optional YuDiagnostics for disabled-behavior observation
```

`YuRHI` must not depend on `YuKernel`, `YuPlatform`, `YuThread`, `YuFile`,
`YuResource`, audio, input, script, scene/world, UI, tools, reports, or
TouhouNewWorld evidence in this first slice.

## Performance Constraints

Required deterministic signals:

- device capacity;
- color-target capacity;
- command-list capacity;
- command count;
- submit count;
- present count;
- capture count;
- failed operation count;
- command storage capacity before/after frame fixture;
- capture bytes written;
- allocation/accounting status using `YuMemory` vocabulary or explicit
  deferral;
- disabled diagnostics/logging behavior equivalence.

First-slice bounds:

- backend kinds: `Null` only;
- color target capacity: 8 targets maximum;
- color target extent: maximum 16x16 pixels;
- capture fixture extent: maximum 4x4 pixels;
- command-list capacity: 32 commands maximum;
- supported format: `RGBA8_UNORM` only;
- command types: begin frame, clear color, end frame;
- clear color input: exact `uint8`-equivalent RGBA channels only;
- float clear-color input, conversion, rounding, clamping, NaN, Inf, and
  negative-zero semantics are outside this slice;
- frame submit/present/capture must not allocate or grow storage.

Pass/fail rule:

- exceeding target, extent, command, or capture bounds is an explicit failure;
- changing command storage capacity, allocating, or depending on diagnostics,
  reports, screenshots, or oracle output during the frame fixture is a gate
  failure unless this gate is amended.

Blocking conditions:

- real graphics API or OS window dependency;
- render scene, material, mesh, UI, or game adapter semantics;
- Resource/File reads or decoder/upload ownership;
- unbounded command list, target table, or capture buffer;
- hidden allocation in measured submit/present/capture paths;
- diagnostics/log/report output required for behavior;
- tests that validate behavior by parsing logs, JSON reports, screenshots, or
  oracle artifacts.

## Tests

Fast gate tests required before the slice can be considered complete:

- `RHI_CreateNullDevice_ReturnsCapabilities`
- `RHI_CreateDevice_RejectsUnsupportedBackend`
- `RHI_CreateColorTarget_ReturnsGenerationHandle`
- `RHI_CreateColorTarget_RejectsInvalidDescriptor`
- `RHI_TargetCapacityOverflow_DoesNotMutate`
- `RHI_DestroyTarget_InvalidatesStaleHandle`
- `RHI_CommandList_RecordsClearWithinCapacity`
- `RHI_CommandListCapacityOverflow_DoesNotMutate`
- `RHI_ClearColor_UsesExactRgba8ByteChannels`
- `RHI_SubmitRejectsIncompleteCommandListWithoutMutation`
- `RHI_SubmitExecutesClearIntoNullTarget`
- `RHI_PresentRequiresSuccessfulSubmit`
- `RHI_CaptureBeforePresent_ReturnsExplicitStatus`
- `RHI_CaptureReturnsDeterministicRgba8Bytes`
- `RHI_CaptureRejectsUndersizedBufferWithoutWritingBytes`
- `RHI_FrameSubmitPresentCapture_DoesNotGrowCommandStorage`
- `RHI_DisabledDiagnosticsDoesNotChangeResults`
- `RHI_NoPlatformRenderCoreResourceUiOrGameAdapterDependency`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate
```

The implementation handoff must record the exact commands used.

## Allowed First Slice

If approved, the first implementation slice may create:

```text
src/yuengine/rhi/include/yuengine/rhi/
src/yuengine/rhi/src/
tests/rhi/
```

It may update root `CMakeLists.txt` only to add `YuRHI` and `YuRHITests`.

It may not create placeholder directories or targets for real graphics backends,
RenderCore, renderer, UI, shader compiler, material system, resource upload,
tools, report, capture/oracle writer, or Game Adapter work.

## Non-Goals

- No real D3D/Vulkan/OpenGL/Metal backend.
- No OS window or real swapchain.
- No shader compiler or pipeline state.
- No mesh/material/render scene/render graph.
- No image decoder or resource upload queue.
- No screenshots or oracle files.
- No UI or gameplay rendering.
- No original-game renderer behavior.

## Evidence Inputs

No original-game evidence is required for the first slice.

TouhouNewWorld renderer/backend observations, DDS/material/font/depth evidence,
and old renderer reports remain future RHI/RenderCore validation evidence only.
They must not be read by P2-GATE-001 fast tests.

## Gate Decision Requested

Request `APPROVED_FOR_FIRST_SLICE` only after:

- ADR-0011 is accepted;
- 红美铃 confirms the proposal satisfies module-entry gate requirements and
  sequencing against active Phase 1 implementation review;
- 八云蓝 confirms the UE5/Unity boundary comparison is sound;
- 博丽灵梦 confirms submit/present/capture cost model and no-allocation frame
  path;
- 大妖精 confirms the public surface and tests are locally implementable;
- 射命丸文 confirms original renderer evidence is not being used as API shape if
  evidence boundary is questioned.

If those conditions are not met, return `NEEDS_ARCHITECTURE`,
`NEEDS_PERFORMANCE`, or `NEEDS_EVIDENCE` with exact missing fields.
