# ADR-0011: RHI Device, Command, Present, And Capture Boundary

Status: Accepted
Owner: 八云紫
Reviewers: 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文 if evidence boundary is questioned, 雾雨魔理沙 when implementation exists
Date: 2026-06-10
Depends on: ADR-0001, ADR-0002, ADR-0005, ADR-0006, ADR-0009, docs/YUENGINE_MODULE_ENTRY_GATES.md, docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md, docs/YUENGINE_PHASE0_SUBSYSTEM_REFERENCE_MAP.md

## Context

YuEngine needs a real rendering foundation before RenderCore, UI, world, frame
capture, oracle alignment, or TouhouNewWorld validation can make visual
progress. The restart plan states that renderer progress requires submit,
present, and capture capability. A blue screen, mesh viewer, or report-first
renderer is not enough.

The first RHI decision therefore defines a backend-agnostic boundary with a null
backend. It proves device creation, resource handles, command recording,
submission, present, and deterministic capture without opening D3D/Vulkan,
shader, material, scene traversal, package, UI, or Game Adapter scope.

## Decision

YuEngine introduces `YuRHI` as the owner of:

- RHI device identity and capabilities;
- backend selection vocabulary;
- generation-checked RHI handles;
- fixed-size color target descriptors;
- exact `RGBA8_UNORM` byte-channel clear colors;
- bounded command recording;
- frame submission;
- present counters;
- deterministic capture into caller-owned buffers;
- explicit result/status values for invalid handles, invalid descriptors, and
  capacity failures.

The first RHI slice is a null backend only. It must expose the same public
boundary shape a future real backend will implement, but it must not create a
window, swapchain, graphics API device, shader compiler, material system, render
scene, UI renderer, or original-game renderer adapter.

## RHI Model

Initial concepts:

```text
RhiBackendKind
RhiDeviceDesc
RhiCapabilities
RhiFormat
RhiExtent2D
RhiColor
RhiTextureHandle
RhiCommandList
RhiFrameToken
RhiCaptureBuffer
RhiStatus
```

Names may change during implementation review, but the responsibilities must
stay equivalent.

Rules:

- `Null` is the only approved backend in P2-GATE-001;
- device creation returns explicit capabilities;
- handles use slot/index plus generation, not raw pointers;
- stale-generation handles fail explicitly;
- color target creation validates format and extent bounds;
- clear colors are exact `RGBA8_UNORM` byte-channel values;
- command lists are fixed-capacity per setup/frame fixture;
- submit executes recorded commands deterministically on the null backend;
- present updates explicit counters and frame token state;
- capture writes deterministic bytes into caller-owned storage;
- disabled diagnostics/logging does not change any RHI result.

## Null Backend Semantics

The null backend is behavior-bearing test infrastructure, not a placeholder that
always succeeds.

P2-GATE-001 null backend may support only:

- fixed `RGBA8_UNORM` color targets;
- exact `RGBA8_UNORM` clear color input with one byte per channel;
- bounded extents for fast tests;
- `ClearColor` command;
- submit of a single command list per frame;
- present of the submitted target;
- capture of the presented target into a caller-owned RGBA8 buffer.

Rules:

- a clear command writes deterministic RGBA8 byte values to the target model;
- P2-GATE-001 does not accept float clear-color input or conversion, so
  rounding, clamping, NaN, Inf, and negative-zero float semantics are outside
  this slice;
- present without a submitted target returns explicit status;
- capture before present returns explicit status;
- capture with an undersized buffer returns explicit status, leaves destination
  bytes unchanged, and reports `capture_bytes_written == 0`;
- target destruction increments generation so stale handles fail;
- no real GPU, OS window, swapchain, shader, or driver object exists in this
  slice.

Future gates may add real D3D/Vulkan device creation and swapchain lifecycle only
after the null backend contract is accepted and tested.

## Command Boundary

RHI commands are lower-level graphics commands, not render scene instructions.

First-slice commands:

- `BeginFrame`;
- `ClearColor`;
- `EndFrame`;
- `Submit`;
- `Present`;
- `Capture`.

The exact API shape may differ, but the lifecycle must remain explicit and
testable.

Rules:

- command recording has a fixed command capacity;
- command capacity overflow returns explicit status and does not mutate the
  command list;
- submit validates command-list lifecycle order;
- frame submit/present/capture counters are explicit signals;
- command execution does not allocate or grow storage in the measured frame
  fixture.

## Resource Boundary

RHI resources are backend resources, not asset resources.

P2-GATE-001 may create only synthetic color target handles. It must not use
`YuResource`, `YuFile`, package manifests, image decoders, shader assets, GPU
upload queues, or original resource names.

Future gates will connect Resource handles to GPU/audio upload scheduling only
after Resource load and backend upload policies are approved.

## Capture Boundary

Capture is an optional runtime capability used by tests and later verification.
It is not an oracle, report, or tool dashboard.

Rules:

- capture writes into caller-owned fixed storage;
- capture output is deterministic for the null backend;
- capture disabled or unused behavior must not change submit or present results;
- capture does not serialize JSON, markdown, screenshots, reports, or oracle
  records in P2-GATE-001.

## Diagnostics Boundary

Diagnostics may observe RHI counters later. Diagnostics must not own RHI
behavior.

Rules:

- RHI results are explicit status values.
- Tests observe results, counters, and capture bytes, not logs.
- Disabled diagnostics/logging does not change submit, present, or capture
  results.
- Report/capture/oracle output is not an RHI runtime API.

## Memory And Thread Boundary

Memory:

- device, target, command, and capture storage capacities are explicit;
- frame submit and capture must not allocate or grow storage;
- if `YuMemory` is available and accepted, RHI allocation/accounting signals use
  its vocabulary;
- if `YuMemory` is unavailable or under blocking rewrite, accounting is
  explicitly deferred and cannot be counted as zero CRT/STL/general heap cost.

Thread:

- P2-GATE-001 is single-threaded and caller-driven;
- no render thread, worker queue, GPU queue thread, fence wait thread, or async
  upload job is introduced;
- future real backend work must define synchronization, queue ownership, and
  shutdown separately.

## Evidence Boundary

TouhouNewWorld D3D9/backend observations, DDS/material/font/depth evidence, and
old renderer reports are future RHI/RenderCore validation inputs. They are not
inputs to P2-GATE-001 API shape.

P2-GATE-001 fast tests must use synthetic descriptors and null-backend fixture
colors only. They must not read:

- TouhouNewWorld `bin` or `resource` data;
- old backup runtime files;
- old renderer report/status files;
- screenshots or oracle artifacts as runtime behavior fixtures.

## P2-GATE-001 Compatibility

P2-GATE-001 may implement:

- `YuRHI` target;
- null backend device;
- RHI value/handle/status types;
- fixed color target descriptor and handle behavior;
- bounded command list;
- clear/submit/present/capture behavior;
- deterministic counters and snapshots for tests.

P2-GATE-001 must not implement:

- D3D, Vulkan, OpenGL, Metal, SDL, GLFW, or OS window backend;
- swapchain tied to a real window;
- shader compiler or shader bytecode loader;
- material, mesh, render scene, render graph, frame pass, or UI renderer;
- Resource/File package reads, image decoding, texture import, or upload queues;
- async jobs, render thread, GPU fence waiting, or worker scheduling;
- report, oracle, screenshot writer, dashboard, or editor tool output;
- TouhouNewWorld adapter behavior.

## Consequences

YuEngine gains a narrow RHI boundary that proves the minimum renderer contract:
create a target, record a command, submit, present, and capture deterministically.

The cost is that no real graphics backend or scene rendering exists yet. That is
intentional. A future real backend gate can focus on API-specific device and
swapchain lifecycle because the public RHI shape and null behavior will already
be testable.

## Requested Review

Reviewers should decide whether this ADR is accepted, rejected, or needs
amendment.

If accepted, ADR-0011 becomes the architecture input for P2-GATE-001 Null RHI
Device, Command, Present, And Capture.
