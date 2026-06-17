# YuEngine Phase 2 Architecture Queue

Status: active architecture queue
Owner: 八云紫, 总架构师
Started: 2026-06-10
Applies after: Phase 1 architecture through `eb79d22`

## Purpose

Phase 2 prepares low-level runtime module boundaries after the Phase 1
host/kernel/service skeleton and adjacent primitives. This queue is architecture
only until each gate reaches an explicit approval state.

Phase 2 must not become demo-first renderer, audio-business, UI, gameplay, or
TouhouNewWorld adapter work. The goal is to define commercial-grade runtime
interfaces that can later support real backends and the validation workload.

## Governance

- 八云紫 owns ADR order, layer boundaries, and implementation sequencing.
- 红美铃 owns PM/final gate state before any implementation task is created.
- 八云蓝 reviews UE5/Unity responsibility comparisons for RHI, audio, package,
  and runtime boundary shape.
- 博丽灵梦 reviews frame/callback/hot-path cost, allocation, synchronization,
  and capture overhead.
- 大妖精 reviews whether proposed first slices are locally implementable and
  enforceable by tests.
- 射命丸文 reviews only evidence-boundary risks when original-game renderer,
  audio, package, or adapter facts might leak into lower-layer APIs.
- 雾雨魔理沙 starts code review after implementation exists.

Gate policy:

- `APPROVED_FOR_FIRST_SLICE` is required before code implementation.
- Approved Phase 2 implementation tasks should wait until current Phase 1
  implementation reviews are stable, or they must be assigned in a clean
  isolated worktree to avoid shared CMake/index churn.
- Original-game evidence remains validation input only. It must not define L3
  API shape.

## Baseline Inputs

| Input | Commit | File |
| --- | --- | --- |
| Architecture restart plan | `33324d0` | `docs/YUENGINE_ARCHITECTURE_RESTART_PLAN.md` |
| Phase 0 subsystem reference map | `7ceb272` | `docs/YUENGINE_PHASE0_SUBSYSTEM_REFERENCE_MAP.md` |
| Module entry gates | `d1f262b` | `docs/YUENGINE_MODULE_ENTRY_GATES.md` |
| Performance cost standards | `1fcf59b` | `docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md` |
| Phase 1 queue | latest | `docs/YUENGINE_PHASE1_ARCHITECTURE_QUEUE.md` |

## Phase 2 Objective

Define and then implement the first low-level runtime interfaces:

```text
Null RHI submit/present/capture
-> audio test backend and mixer sink
-> package/load boundary over File and Resource
-> async IO boundary after Thread/File/Resource stability
```

Phase 2 remains blocked from:

- renderer demo or blue-screen-only progress;
- real D3D/Vulkan backend before the null RHI contract is accepted;
- UI framework or title-screen business;
- BGM/SE business IDs;
- gameplay/world logic;
- original package parser as the first resource/file behavior;
- report/capture/oracle ownership inside runtime behavior.

## ENG-096 Hardware-First Resequence

ENG-096 reorders active YuEngine work back toward the hardware and lower-engine
layer before more World, Game Adapter, UI business, or scene policy expansion.
The previous upper-layer slices are useful contract baselines, but they must not
drive Platform, RHI, Audio, Input, Thread, File, Resource, Package, or diagnostics
runtime shape.

Current lower-engine reality:

- Platform has the P2-GATE-005 first-slice window, native surface, and event
  pump boundary; it still has no GPU adapter discovery, swapchain, or RHI
  ownership.
- RHI has D3D11 visible-triangle, indexed static-mesh, and texture-sampling
  capture-byte proof through P2-GATE-009, P2-GATE-013, and P2-GATE-014. The
  lower-engine path now has Resource upload queue, RenderCore fixture pass,
  material binding fixture, bounded submission batch fixture, and bounded frame
  packet fixture proof through P2-GATE-016, P2-GATE-017, P2-GATE-018,
  P2-GATE-019, and P2-GATE-020. The engine still has no render graph, material
  graph, scene traversal, report, visual proof, UI, World, or Game Adapter
  behavior.
- Audio has a private Windows XAudio2 callback first slice through
  P2-GATE-011, but no codec, streaming, Resource-backed audio asset pipeline,
  audio scene, BGM/SE service, UI, World, or Game Adapter behavior.
- Input has a private Windows platform input bridge through P2-GATE-012, but no
  UI navigation, text input, gameplay mapping, Script, World, or Game Adapter
  behavior.
- Thread/File/Resource/Package/Streaming have a worker lifecycle, async
  file-completion substrate, bounded package/resource staging bridge, Resource
  upload queue, Resource upload completion commit, Resource residency budget
  policy, and Resource cache payload ownership through P2-GATE-010,
  P2-GATE-015, P2-GATE-016, P2-GATE-021, P2-GATE-022, and P2-GATE-023, but no
  asset decode/import pipeline.

The immediate ordering is:

```text
test tier labels and optional hardware-smoke presets
-> Platform window/native surface/event pump
-> RHI backend-neutral device boundary
-> D3D11 device/context/swapchain clear/present/capture
-> D3D11 resource and pipeline primitives
-> visible triangle fixture
-> worker/job and async IO substrate
-> real audio backend callback
-> OS input bridge
-> static mesh fixture
-> texture sampling fixture
-> package/resource staging queue
-> resource upload queue
-> RenderCore fixture pass
-> material binding fixture
-> RenderCore submission batch fixture
-> RenderCore frame packet fixture
-> resource upload completion commit
-> Resource residency budget policy
-> Resource cache payload ownership
-> Resource asset decode/import boundary
-> RenderCore render graph skeleton
```

Hard sequencing constraints:

- Do not open additional World/Game Adapter gates until the hardware/lower-engine
  backlog above has a stable implementation path or an explicit architecture
  exception.
- Do not combine Platform, RHI, RenderCore, mesh, Resource, Package, or Game
  Adapter concerns in one gate.
- Do not use visual demo success, screenshots, log output, or report existence as
  proof for hardware backend progress. A promoted backend gate must prove public
  interface behavior, capture/present evidence, and bounded cost.
- UE and Unity remain responsibility references only. Do not copy proprietary
  source, API shape, names, or file layout into YuEngine.
- Test cost is reduced by tiering and labels, not by deleting deterministic
  boundary and performance evidence.

Required test-tier direction:

- Keep `windows-fast-gate` deterministic and no-real-device by default.
- Add CTest labels before or with the first real hardware gate: `Fast`,
  `ModuleFixture`, `PerformanceSmoke`, `HardwareSmoke`, `EvidenceOracle`, plus
  module labels such as `Platform`, `RHI`, `D3D11`, `Audio`, and `AsyncIO`.
- Add optional local/CI presets for real devices, such as
  `windows-hardware-smoke`, while keeping unsupported real-device environments
  from blocking unrelated fast gates.
- Require full `windows-fast-gate` at final handoff, QA, dependency/CMake
  changes, and before landing. During implementation, targeted build/test is
  allowed for focus and speed.

## ADR Queue

| ID | Title | Owner | Reviewers | Status | Blocks |
| --- | --- | --- | --- | --- | --- |
| ADR-0011 | RHI device, command, present, and capture boundary | 八云紫 | 八云蓝, 博丽灵梦, 大妖精 | Accepted | Null RHI first slice |
| ADR-0012 | Audio test backend and mixer sink boundary | 八云紫 | 八云蓝, 博丽灵梦, 大妖精 | Accepted | Audio test backend/mixer first slice |
| ADR-0013 | Package manifest and load plan boundary | 八云紫 | 八云蓝, 博丽灵梦, 大妖精, 射命丸文 | Accepted | Package/load-plan first slice |

## Module Gate Proposal Queue

| Gate | Module | Layer | Current decision | Status | Notes |
| --- | --- | --- | --- | --- | --- |
| P2-GATE-001 | Null RHI Device, Command, Present, And Capture | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Null backend only; current fast gate has `RHI_` coverage; no real backend, shader, material, RenderCore, resource loading, UI, or game adapter |
| P2-GATE-002 | Audio Test Backend And Mixer Sink | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Test backend only; current fast gate has `Audio_` coverage; no real device, callback thread, codec, streaming, resource, UI, script, gameplay, or game adapter |
| P2-GATE-003 | Package Manifest And Load Plan Boundary | L4-L5 | `FIRST_SLICE_CLOSED_QA_CLEARED` | First slice closed | `354f8e2` closed the approved `YuPackage` / `YuPackageTests` first-slice review baseline; no new package code/CMake/test expansion, File/VFS runtime reads, resource mutation, or P3 work |
| P2-GATE-004 | Test Tier Labels And Hardware Smoke Presets | L7 over L0-L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_004_TEST_TIER_LABELS_HARDWARE_SMOKE_PRESETS.md`; landed at `0c45d37`; CTest labels and optional hardware-smoke preset before real backend gates; no runtime behavior, no new backend, no test deletion |
| P2-GATE-005 | Platform Window Native Surface And Event Pump | L0-L1 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_005_PLATFORM_WINDOW_NATIVE_SURFACE_EVENT_PUMP.md`; landed at `e3e2ad7`; Platform window lifecycle, opaque native surface value, bounded event pump, resize/focus/close/raw platform events; fast gate is 656/656 PASS; `HardwareSmoke` remains 0; no RHI, UI, Resource, Game Adapter, or render policy |
| P2-GATE-006 | RHI Backend-Neutral Device Boundary | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_006_RHI_BACKEND_NEUTRAL_DEVICE_BOUNDARY.md`; landed at `fd3be16`; backend-neutral device/interface/factory/native-surface descriptor contracts only; fast gate is 664/664 PASS; `HardwareSmoke` remains 0; no real D3D11, DXGI, swapchain, RenderCore, Resource, report, visual proof, or Game Adapter |
| P2-GATE-007 | D3D11 Device Swapchain Clear Present Capture | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_007_D3D11_DEVICE_SWAPCHAIN_CLEAR_PRESENT_CAPTURE.md`; landed at `18d73a3`; first real D3D11 device/context/swapchain clear/present/capture through RHI boundary; default fast gate is 669/669 PASS; `HardwareSmoke` is isolated from default fast gate; `windows-hardware-smoke` discovers and runs 1 D3D11 capture-byte test; no mesh, shader pipeline expansion, RenderCore, Resource, report, visual proof, or Game Adapter |
| P2-GATE-008 | D3D11 Resource And Pipeline Primitives | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_008_D3D11_RESOURCE_AND_PIPELINE_PRIMITIVES.md`; landed at `2f7c8e1`; RHI-only buffer/texture/sampler/shader/input-layout/pipeline/fence primitive contracts after P2-GATE-007; default fast gate is `681/681` PASS; `windows-hardware-smoke` discovers and runs 2 D3D11 tests including primitive resource/pipeline snapshot; no visible triangle, draw proof, RenderCore, material system, Resource loading, mesh asset pipeline, scene traversal, report, visual proof, shader compiler, or Game Adapter |
| P2-GATE-009 | D3D11 Visible Triangle Fixture | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_009_D3D11_VISIBLE_TRIANGLE_FIXTURE.md`; landed at `f55f6dd`; first visible geometry proof through RHI capture bytes after P2-GATE-008; default fast gate is `685/685` PASS; `windows-hardware-smoke` discovers and runs 3 tests including `RHI_D3D11Hardware_VisibleTriangleCaptureBytes`; no static mesh asset pipeline, RenderCore, Resource upload, World, UI, report, screenshot, manual visual proof, or Game Adapter |
| P2-GATE-010 | Thread Worker And Async IO Substrate | L1-L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_010_THREAD_WORKER_AND_ASYNC_IO_SUBSTRATE.md`; landed at `5a26a53`; worker lifecycle and async file-completion substrate; default fast gate is `696/696` PASS; no Resource semantics, package streaming policy, upload queue, render submission, static mesh, RenderCore, real audio callback, OS input, or gameplay |
| P2-GATE-011 | Real Audio Backend Callback | L1-L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_011_REAL_AUDIO_BACKEND_CALLBACK.md`; landed at `1d7d2ca`; private Windows XAudio2 callback backend proof through existing mixer/test-sink contract; default fast gate remains deterministic; no codec, BGM/SE business IDs, Resource loading, UI, script, or gameplay |
| P2-GATE-012 | Platform Input Device Bridge | L1-L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_012_PLATFORM_INPUT_DEVICE_BRIDGE.md`; landed at `58088bd`; private Windows input bridge into existing Input value boundary; default fast gate is `713/713` PASS; no UI navigation, title menu behavior, script, scene, gameplay mapping, manual proof, or Game Adapter |
| P2-GATE-013 | Static Mesh Fixture | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_013_STATIC_MESH_FIXTURE.md`; landed at `1ee9fa4`; indexed static-geometry fixture through RHI/D3D11 value contracts; default fast gate is `718/718` PASS; `windows-hardware-smoke` discovers and runs indexed static mesh capture; no Resource loading, RenderCore, material system, scene traversal, reports, screenshots, manual visual proof, or Game Adapter |
| P2-GATE-014 | Texture Sampling Fixture | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_014_TEXTURE_SAMPLING_FIXTURE.md`; landed at `49a14ae`; texture/sampler binding and sampling proof through RHI/D3D11 value contracts; default fast gate is `726/726` PASS; `windows-hardware-smoke` discovers and runs texture-sampling capture; no Resource loading, image decode, RenderCore, material system, scene traversal, reports, screenshots, shader compiler, manual visual proof, or Game Adapter |
| P2-GATE-015 | Package Resource Staging Queue | L4-L5 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_015_PACKAGE_RESOURCE_STAGING_QUEUE.md`; landed at `6e29663`; bounded staging bridge over existing Package load-plan, Resource handle/type, and File async values; default fast gate is `736/736` PASS; no package parser, decode, Resource load completion, RHI upload, RenderCore, material, scene/UI/World/Script/Game Adapter, reports, screenshots, or manual proof |
| P2-GATE-016 | Resource Upload Queue | L4-L5 over L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_016_RESOURCE_UPLOAD_QUEUE.md`; landed at `55af599`; bounded upload bridge from package/resource staging completions and Resource validation into public `YuRHI` buffer/texture update value APIs; default fast gate is `753/753` PASS; no Resource load-state mutation, decode, RenderCore, material, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-017 | RenderCore Fixture Pass | L5 over L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_017_RENDERCORE_FIXTURE_PASS.md`; landed at `13ccdb3`; bounded RenderCore fixture pass over public `YuRHI` command and resource-handle values only; no Resource/Streaming ownership, material system, scene/UI/World/Script/Game Adapter, shader compiler/source tooling, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-018 | Material Binding Fixture | L5 over L3-L5 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_018_MATERIAL_BINDING_FIXTURE.md`; landed at `b5620a3`; bounded material binding fixture over public `YuRHI` and landed `YuRenderCore` fixture values only; no material graph, shader compiler/source tooling, Resource/Streaming ownership, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-019 | RenderCore Submission Batch Fixture | L5 over L3-L5 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_019_RENDERCORE_SUBMISSION_BATCH_FIXTURE.md`; landed at `f4c3f64`; bounded submission batch fixture over landed `YuRenderCore` fixture pass, material binding fixture, and public `YuRHI` values only; default fast gate is `784/784` PASS; no render graph, frame graph, pass sorting, command-list parallelism, Resource/Streaming ownership, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-020 | RenderCore Frame Packet Fixture | L5 over L3-L5 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_020_RENDERCORE_FRAME_PACKET_FIXTURE.md`; landed at `b275168`; bounded frame packet fixture over landed `YuRenderCore` submission batch fixture and public `YuRHI` values only; default fast gate is `793/793` PASS; no render graph, frame graph, renderer scheduling, pass sorting, command-list parallelism, Resource/Streaming ownership, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-021 | Resource Upload Completion Commit | L4-L5 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_021_RESOURCE_UPLOAD_COMPLETION_COMMIT.md`; landed at `475c371`; bounded Resource/Streaming upload completion commit bridge over landed `ResourceUploadCompletion` and ResourceRegistry values only; default fast gate is `809/809` PASS; no cache ownership, package parser, asset decode/import, render graph, frame graph, RenderCore scheduling, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-022 | Resource Residency Budget Policy | L4-L5 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_022_RESOURCE_RESIDENCY_BUDGET_POLICY.md`; landed at `d2f2059`; Resource-owned residency state, budget counters, pin/unpin, and eviction-candidate policy over landed upload commit state only; default fast gate is `820/820` PASS; no cache payload storage, package parser, asset decode/import, RHI resource destruction, render graph, frame graph, RenderCore scheduling, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-023 | Resource Cache Payload Ownership | L4-L5 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_023_RESOURCE_CACHE_PAYLOAD_OWNERSHIP.md`; landed at `aca6170`; Resource-owned opaque cache payload byte storage, cache-slot records, readback, release, and deterministic counters over landed load commit and residency state only; default fast gate is `832/832` PASS; no package parser, asset decode/import, RHI resource destruction, render graph, frame graph, RenderCore scheduling, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-024 | Resource Asset Decode Plan | L4-L5 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_024_RESOURCE_ASSET_DECODE_PLAN.md`; landed at `a6fbabf`; Resource-owned decode-plan records over landed cache payload bytes only; no File IO expansion, package parser, real image/audio/mesh decode, RHI upload, render graph, RenderCore scheduling, material graph, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-025 | RenderCore Render Graph Skeleton | L5 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_025_RENDERCORE_RENDER_GRAPH_SKELETON.md`; landed at `43dc361`; RenderCore-owned render graph declaration and dependency validation skeleton over landed fixture pass, material binding, submission batch, frame packet, and public RHI values only; default fast gate is `858/858` PASS; no render scheduler, frame graph execution, command-list parallelism, transient resource aliasing, Resource/Streaming/Package/File ownership, material graph, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-026 | Resource Decode Result Import-Ready Record | L4-L5 | `APPROVED_FOR_FIRST_SLICE` | Proposal approved | Gate doc: `docs/gates/P2_GATE_026_RESOURCE_DECODE_RESULT_IMPORT_READY_RECORD.md`; approved after ENG-143CR combined review PASS; Resource-owned import-ready decoded-result metadata over landed decode-plan records only; no real codec, decoded byte storage, RHI upload, RenderCore scheduling, material graph, scene/UI/World/Script/Game Adapter, native/backend leakage, reports, screenshots, logs, sleeps, or manual proof |
| P2-GATE-027 | RHI Primitive Retirement Ledger | L3 | `APPROVED_FOR_FIRST_SLICE` | Proposal approved | Gate doc: `docs/gates/P2_GATE_027_RHI_PRIMITIVE_RETIREMENT_LEDGER.md`; approved after ENG-144R combined review PASS; RHI-owned primitive retirement request, ledger, drain, and deterministic handle invalidation evidence over landed RHI primitive handles only; no Resource/Streaming/File/Package/RenderCore/material/scene/UI/World/Script/Game Adapter dependency, backend-native public leak, new renderer scheduling, reports, screenshots, logs, sleeps, or manual proof |

## Current Active Gates

- P2-GATE-001 first-slice baseline is present in the current fast gate.
  `ctest --preset windows-fast-gate -N` at ENG-068A showed `RHI_` 28 registered
  tests. This does not authorize real graphics, RenderCore, Resource/File
  upload, reports, or Game Adapter scope.
- P2-GATE-002 first-slice baseline is present in the current fast gate.
  `ctest --preset windows-fast-gate -N` at ENG-068A showed `Audio_` 24
  registered tests. This does not authorize real audio devices, callback
  threads, codecs, streaming, resource coupling, UI/script/gameplay, reports,
  tools, or Game Adapter scope.
- P2-GATE-003 is `FIRST_SLICE_CLOSED_QA_CLEARED` after the 2026-06-11
  Architect decision and `354f8e2` first-slice fix. The closure is limited to
  the existing `YuPackage` implementation and `YuPackageTests` first-slice
  baseline. Architect and CodeReviewerQA both verified the `Package_` CTest
  filter at `23/23`. This does not authorize new package code, CMake targets,
  tests, scope expansion, File/VFS runtime reads, resource mutation, or P3 work.
- P2-GATE-004 is approved for first slice after ENG-097A boundary/reference
  PASS, ENG-097B CMake implementability PASS, and ENG-097C test admission PASS.
  The approved scope is CMake/CTest execution infrastructure only: tier/module/
  backend labels, preserved default `windows-fast-gate`, and optional
  hardware-smoke preset or documented preset slot. It does not authorize runtime
  source changes, real backend behavior, deleting deterministic tests, or
  treating unsupported/zero hardware smoke as backend proof.
- P2-GATE-004 first slice landed at `0c45d37` after ENG-098A implementation
  PASS, ENG-098B verification PASS, and ENG-098QA boundary/quality PASS. The
  default `windows-fast-gate` remains unfiltered at 646 deterministic tests, full
  gate passed at `646/646`, `Fast` and `ModuleFixture` labels each discover 646
  tests, `RHI` / `Audio` / `Platform` / `World` labels discover 28 / 24 / 3 /
  438 tests, and `windows-hardware-smoke` currently discovers 0 `HardwareSmoke`
  tests with zero-test discovery allowed. This is not backend proof.
- P2-GATE-005 first slice landed at `e3e2ad7` after ENG-101A implementation
  PASS, ENG-101B verification PASS, and ENG-101QA boundary/quality PASS. The
  Platform L0-L1 surface now has window descriptor/lifecycle, opaque native
  surface value, bounded event pump, resize/focus/close/raw platform events, and
  deterministic contract tests. The default fast gate is `656/656` PASS;
  `Fast` and `ModuleFixture` labels each discover 656 tests, `Platform`
  discovers 13, `PerformanceSmoke` discovers 39, `EvidenceOracle` discovers 80,
  and `HardwareSmoke` remains 0. `CMakePresets.json` was not changed. This does
  not authorize RHI, D3D11, DXGI, RenderCore, YuInput semantic mapping, UI,
  Resource, World, reports, visual proof, or Game Adapter behavior.
- P2-GATE-006 first slice landed at `fd3be16` after ENG-103A implementation
  PASS, ENG-103B verification PASS, and ENG-103QA boundary/quality PASS. The
  RHI L3 surface now has a backend-neutral `IRhiDevice`, RHI-owned native surface
  descriptor, factory/create result, null-backend interface implementation, and
  deterministic contract tests. The default fast gate is `664/664` PASS; `Fast`
  and `ModuleFixture` labels each discover 664 tests, `RHI` discovers 36,
  `PerformanceSmoke` discovers 40, `EvidenceOracle` discovers 87, and
  `HardwareSmoke` remains 0. `CMakePresets.json` was not changed. This does not
  authorize real D3D11, DXGI, swapchain, RenderCore, mesh, Resource, report,
  visual proof, or Game Adapter behavior.
- P2-GATE-007 first slice landed at `18d73a3` after ENG-105A implementation
  PASS, ENG-105B verification PASS, and ENG-105QA boundary/quality PASS. The
  RHI L3 surface now has the first real D3D11 device/context/swapchain
  clear/present/capture path, caller-owned factory storage, explicit device
  destruction, swapchain target query, and capture-byte hardware smoke proof.
  The default fast gate is `669/669` PASS; `Fast` discovers 669 tests, `RHI`
  discovers 41, `PerformanceSmoke` discovers 40, `EvidenceOracle` discovers 91,
  and default `HardwareSmoke`, `D3D11`, and `Win32` discovery remains 0.
  `windows-hardware-smoke` discovers 1 test and ENG-105B accepted a real D3D11
  capture-byte execution. Public RHI headers remain free of Windows SDK, D3D11,
  DXGI, COM, and Platform types; production `YuRHI` does not depend on
  `YuPlatform`. This does not authorize mesh, shader pipeline expansion,
  RenderCore, Resource, report, visual proof, UI, World, or Game Adapter
  behavior.
- P2-GATE-008 first slice landed at `2f7c8e1` after ENG-107A implementation
  PASS, ENG-107B verification PASS, ENG-107QA boundary/quality PASS, and
  ENG-108 commit/push closure. The RHI L3 surface now has backend-neutral
  buffer, texture, sampler, shader module, input layout, pipeline, resource
  snapshot, and lightweight fence contracts with private D3D11 native objects.
  The default fast gate is `681/681` PASS; default `HardwareSmoke`, `D3D11`,
  and `Win32` discovery remains 0; `windows-hardware-smoke` discovers and runs
  2 tests, including `RHI_D3D11Hardware_PrimitiveResourcePipelineSnapshot`.
  Public RHI headers remain free of Windows SDK, D3D11, DXGI, COM, Platform,
  RenderCore, Resource, Package, report, screenshot, visual proof, UI, World,
  and Game Adapter types. This does not authorize shader compiler, visible
  triangle, draw proof, RenderCore, material system, Resource loading, mesh
  asset pipeline, scene traversal, report, visual proof, UI, World, or Game
  Adapter behavior.
- P2-GATE-009 first slice landed at `f55f6dd` after ENG-110A implementation
  PASS, ENG-110B verification PASS, and ENG-110QA boundary/quality PASS. The
  RHI L3 surface now has backend-neutral draw, vertex buffer, input layout,
  topology, vertex-buffer view contracts, pipeline bind, vertex-buffer bind,
  draw records, Null validation/snapshot counters, private D3D11 draw
  execution, and capture-byte visible-triangle proof. The default fast gate is
  `685/685` PASS; default `HardwareSmoke`, `D3D11`, and `Win32` discovery
  remains 0; `windows-hardware-smoke` discovers and runs 3 tests including
  `RHI_D3D11Hardware_VisibleTriangleCaptureBytes`. This does not authorize
  static mesh, mesh asset pipeline, material system, RenderCore, Resource
  upload, Package/File streaming, shader compiler, report, screenshot, manual
  visual proof, UI, World, or Game Adapter behavior.
- P2-GATE-010 is approved for first slice after ENG-111A boundary/quality PASS,
  ENG-111B implementability PASS, and ENG-111C test-policy PASS. The approved
  scope is limited to worker lifecycle and async file-completion proof over the
  existing Thread and File modules. File may use Thread only through private
  implementation linkage for the async adapter; File public headers must remain
  value-based and OS-handle-free. The approved scope explicitly excludes
  Resource semantics, Package streaming, upload queues, render submission,
  static mesh, RenderCore, audio callback, OS input, UI, World, reports, and
  Game Adapter behavior.
- P2-GATE-010 first slice landed at `5a26a53` after ENG-112A implementation
  PASS, ENG-112B verification PASS, and ENG-112QA boundary/quality PASS. The
  Thread/File lower-engine surface now has explicit worker lifecycle,
  fixed-capacity work/completion queues, async file read completion over
  caller-owned request/result slots, and deterministic counters/status proof.
  The default fast gate is `696/696` PASS; `Thread` discovers 14 tests, `File`
  discovers 16, `AsyncIO` discovers 10, `PerformanceSmoke` discovers 47,
  `EvidenceOracle` discovers 112, and default `HardwareSmoke` remains 0.
  `windows-hardware-smoke` remains `3/3` PASS and unaffected. This does not
  authorize Resource semantics, Package streaming, RHI upload queues, static
  mesh, RenderCore, real audio callback, OS input bridge, UI, World, reports,
  or Game Adapter behavior.
- P2-GATE-011 first slice landed at `1d7d2ca` after ENG-114A implementation
  PASS, ENG-114B2 verification PASS, and ENG-114QA2 boundary/quality PASS. The
  Audio L1-L3 surface now has private Windows XAudio2 callback backend proof,
  value-based public callback contracts, fixed S16 stereo 48000 Hz buffer
  shape, preallocated slots, fixed completion storage, callback
  counters/statuses/snapshots, and isolated hardware-smoke unavailable-device
  evidence. The default fast gate is `704/704` PASS; default `HardwareSmoke`,
  `Win32`, and `XAudio2` discovery remains 0; `windows-hardware-smoke`
  discovers 4 tests including one `Audio` / `XAudio2` callback test. This does
  not authorize codec, streaming, Resource-backed audio, BGM/SE services, audio
  scene, UI, Script, World, reports, screenshots, manual listening proof,
  audible output proof, or Game Adapter behavior.
- P2-GATE-012 first slice landed at `58088bd` after ENG-116A implementation
  PASS, ENG-116B verification PASS, and ENG-116QA boundary/quality PASS. The
  Input lower-engine surface now has private Windows input bridge proof,
  value-based public bridge contracts, bounded event storage, focus-aware
  counters/statuses/snapshots, backend-neutral keyboard/mouse/button/wheel
  events, and isolated optional hardware-smoke labels. The default fast gate is
  `713/713` PASS; `Input` discovers 27 tests, `PerformanceSmoke` discovers 48,
  `EvidenceOracle` discovers 118, default `HardwareSmoke` remains 0, and
  `windows-hardware-smoke` discovers 5 tests. This does not authorize UI
  navigation, title/menu behavior, gameplay mapping, Script, World, reports,
  manual key/mouse proof, visual proof, platform handle leakage, or Game Adapter
  behavior.
- P2-GATE-013 first slice landed at `1ee9fa4`. It proves indexed static
  geometry through existing `YuRHI` and private D3D11 value contracts. It does
  not authorize Resource loading, Package streaming, RenderCore pass
  scheduling, material system, scene traversal, reports, screenshots, manual
  visual proof, UI, World, or Game Adapter behavior.
- P2-GATE-014 first slice landed at `49a14ae` after ENG-120A implementation
  PASS, ENG-120B verification PASS, and ENG-120QA boundary/quality PASS. It
  proves texture/sampler binding and sampling through existing `YuRHI` and
  private D3D11 value contracts. The default fast gate is `726/726` PASS and
  `windows-hardware-smoke` discovers and runs the texture-sampling capture
  proof. It does not authorize Resource loading, image decode, Package
  streaming, File IO, RenderCore pass scheduling, material system, scene
  traversal, reports, screenshots, shader compiler/source tooling, manual
  visual proof, UI, World, or Game Adapter behavior.
- P2-GATE-015 first slice landed at `6e29663` after ENG-122A implementation
  PASS, ENG-122B verification PASS, and ENG-122QA boundary/quality PASS. It is
  limited to a bounded package/resource staging queue over existing `YuPackage`
  load-plan values, `YuResource` handle/type validation values, and `YuFile`
  async request/completion values. The default fast gate is `736/736` PASS;
  `Resource` discovers 28 tests, `Package` discovers 34, `File` discovers 26,
  `AsyncIO` discovers 20, `Streaming` discovers 10, `Upload` discovers 10,
  `PerformanceSmoke` discovers 53, `EvidenceOracle` discovers 140, default
  `HardwareSmoke` remains 0, and `windows-hardware-smoke` remains 7 tests with
  no Streaming/Upload admission. It does not authorize package parsing,
  original-game package readers, image/audio/mesh/shader decode, Resource load
  completion mutation, RHI upload execution, RenderCore pass scheduling,
  material system, scene traversal, reports, screenshots, manual visual proof,
  UI, World, Script, or Game Adapter behavior.
- P2-GATE-016 first slice landed at `55af599` after ENG-124A implementation
  PASS, ENG-124B verification PASS, and ENG-124QA boundary/quality PASS. It is
  limited to a bounded bridge from P2-GATE-015 staging completion values and
  existing `YuResource` validation values into public `YuRHI` buffer/texture
  creation or update value APIs. The default fast gate is `753/753` PASS;
  `Resource` discovers 45 tests, `Streaming` discovers 27, `Upload` discovers
  27, `RHI` discovers 87, `PerformanceSmoke` discovers 56, `EvidenceOracle`
  discovers 157, default `HardwareSmoke` remains 0, and `windows-hardware-smoke`
  remains 7 tests with no RenderCore or Upload admission. It does not authorize
  Resource load completion state, package parsing, asset decode, native/backend
  types in public Package/Resource/Streaming contracts, RenderCore pass
  scheduling, material binding, scene traversal, reports, screenshots,
  logs/sleeps/manual proof, UI, World, Script, or Game Adapter behavior.
- P2-GATE-017 landed the RenderCore fixture pass first slice at `13ccdb3`. It
  proves a bounded RenderCore fixture pass over existing public `YuRHI`
  command, target, pipeline, buffer, texture, sampler, draw, submit, present,
  capture, and snapshot values. It does not own Resource/Streaming state,
  create or decode assets, compile shaders, expose native/backend types, define
  material or scene meaning, use reports/screenshots/logs/sleeps/manual proof,
  or involve UI, World, Script, or Game Adapter behavior.
- P2-GATE-018 landed the material binding fixture first slice at `b5620a3`.
  It proves bounded value grouping for public `YuRHI` pipeline, texture,
  sampler, fixed constant, material/pass ids, and landed `YuRenderCore` fixture
  request values. It does not add material graph behavior, shader compiler/
  source tooling, Resource/Streaming ownership, asset decode/import,
  scene/UI/World/Script/Game Adapter behavior, native/backend leakage, reports,
  screenshots, logs, sleeps, manual proof, hardware-only proof, or original-game
  evidence.
- P2-GATE-019 landed the RenderCore submission batch fixture first slice at
  `f4c3f64`. It proves bounded deterministic submission of caller-owned,
  prepared `RenderFixturePassRequest` values through the landed RenderCore
  fixture pass and material binding fixture values. The landed fast gate is
  `784/784` PASS with `RenderCore` at `31`, `Material` at `17`, `RHI` at `118`,
  `PerformanceSmoke` at `64`, `EvidenceOracle` at `188`, default
  `HardwareSmoke` at `0`, and `windows-hardware-smoke` at `7`. It does not
  authorize render graph, frame graph, pass sorting, command-list parallelism,
  Resource/Streaming ownership, material graph, shader compiler/source tooling,
  scene/UI/World/Script/Game Adapter behavior, native/backend leakage, reports,
  screenshots, logs, sleeps, manual proof, hardware-only proof, or
  original-game evidence.
- P2-GATE-020 landed the RenderCore frame packet fixture first slice at
  `b275168`. It proves one bounded frame packet envelope over a caller-owned,
  prepared submission batch fixture request and public `YuRHI` values. The
  landed fast gate is `793/793` PASS with `RenderCore` at `40`, `Material` at
  `26`, `RHI` at `127`, `PerformanceSmoke` at `67`, `EvidenceOracle` at `197`,
  default `HardwareSmoke` at `0`, and `windows-hardware-smoke` at `7`. It does
  not authorize render graph, frame graph, renderer scheduling, pass sorting,
  command-list parallelism, Resource/Streaming ownership, material graph, shader
  compiler/source tooling, scene/UI/World/Script/Game Adapter behavior,
  native/backend leakage, reports, screenshots, logs, sleeps, manual proof,
  hardware-only proof, or original-game evidence.
- P2-GATE-021 landed the Resource upload completion commit first slice at
  `475c371`. It consumes landed `ResourceUploadCompletion` values and commits
  only Resource-owned terminal upload state through ResourceRegistry value
  contracts. The landed fast gate is `809/809` PASS with `Resource` at `61`,
  `Streaming` at `36`, `Upload` at `43`, `RHI` at `127`, `RenderCore` at `40`,
  `Material` at `26`, `PerformanceSmoke` at `72`, `EvidenceOracle` at `213`,
  default `HardwareSmoke` at `0`, and `windows-hardware-smoke` at `7`. It does
  not authorize cache ownership, package parsing, asset decode/import, render
  graph, frame graph, RenderCore scheduling, scene/UI/World/Script/Game Adapter
  behavior, native/backend leakage, reports, screenshots, logs, sleeps, manual
  proof, hardware-only proof, or original-game evidence.
- P2-GATE-022 landed the Resource residency budget policy first slice at
  `d2f2059`. It classifies uploaded Resource slots as resident, pinned,
  evictable, or evicted and selects deterministic eviction candidates through
  ResourceRegistry value contracts only. The landed fast gate is `820/820` PASS
  with `Resource` at `72`, `Streaming` at `36`, `Upload` at `43`, `RHI` at
  `127`, `RenderCore` at `40`, `Material` at `26`, `PerformanceSmoke` at `78`,
  `EvidenceOracle` at `224`, default `HardwareSmoke` at `0`, and
  `windows-hardware-smoke` at `7`. It does not authorize cache payload storage,
  package parsing, asset decode/import, RHI resource destruction, render graph,
  frame graph, RenderCore scheduling, scene/UI/World/Script/Game Adapter
  behavior, native/backend leakage, reports, screenshots, logs, sleeps, manual
  proof, hardware-only proof, or original-game evidence.
- P2-GATE-023 landed the Resource cache payload ownership first slice at
  `aca6170`. It stores caller-provided opaque bytes in fixed-capacity
  Resource-owned cache payload slots and tracks deterministic cache records,
  counters, readback, and release behavior through ResourceRegistry value
  contracts only. The landed fast gate is `832/832` PASS with `Resource` at
  `84`, `Streaming` at `36`, `Upload` at `43`, `RHI` at `127`, `RenderCore` at
  `40`, `Material` at `26`, `PerformanceSmoke` at `83`, `EvidenceOracle` at
  `236`, default `HardwareSmoke` at `0`, and `windows-hardware-smoke` at `7`.
  It does not authorize package parsing, asset decode/import, File IO
  expansion, RHI resource destruction, render graph, frame graph, RenderCore
  scheduling, scene/UI/World/Script/Game Adapter behavior, native/backend
  leakage, reports, screenshots, logs, sleeps, manual proof, hardware-only
  proof, or original-game evidence.
- P2-GATE-024 landed the Resource asset decode-plan first slice at `a6fbabf`.
  It validates a fixed Resource-owned test header, records decode-plan metadata,
  and exposes deterministic counters through ResourceRegistry value contracts
  over landed cache payload bytes only. The landed fast gate is `844/844` PASS
  with `Resource` at `96`, `Streaming` at `36`, `Upload` at `43`, `RHI` at
  `127`, `RenderCore` at `40`, `Material` at `26`, `PerformanceSmoke` at `87`,
  `EvidenceOracle` at `248`, default `HardwareSmoke` at `0`, and
  `windows-hardware-smoke` at `7`. It does not authorize File IO expansion,
  package parsing, real image/audio/mesh decode, texture or audio import, RHI
  upload, render graph, RenderCore scheduling, material graph,
  scene/UI/World/Script/Game Adapter behavior, native/backend leakage, reports,
  screenshots, logs, sleeps, manual proof, hardware-only proof, or
  original-game evidence.
- P2-GATE-025 landed the RenderCore render graph skeleton first slice at
  `43dc361`. It records bounded RenderCore-owned graph declarations, dependency
  validation, deterministic rejection counters, and prepared submission batch
  snapshots over landed fixture pass, material binding, submission batch, frame
  packet, and public RHI value contracts only. The landed fast gate is
  `858/858` PASS with `RenderCore` at `54`, `Material` at `40`, `RHI` at
  `141`, `Resource` at `96`, `Streaming` at `36`, `Upload` at `43`,
  `PerformanceSmoke` at `91`, `EvidenceOracle` at `262`, default
  `HardwareSmoke` at `0`, and `windows-hardware-smoke` at `7`. It does not
  authorize render scheduling, frame graph execution, command-list parallelism,
  transient resource aliasing, Resource, Streaming, Package, File, material
  graph, scene, UI, World, Script, Game Adapter, native/backend leakage,
  reports, screenshots, logs, sleeps, manual proof, hardware-only proof, or
  original-game evidence.
- P2-GATE-026 is approved as a Resource decode result import-ready record first
  slice after ENG-143CR combined review PASS. It may validate
  caller-provided import-ready result metadata, record Resource-owned
  decode-result entries, and expose deterministic counters without storing
  decoded bytes. The proposal baseline is `7a620af`; discovery is
  `windows-fast-gate` `858/858`, `Resource` `96`, `Streaming` `36`, `Upload`
  `43`, `RHI` `141`, `RenderCore` `54`, `Material` `40`, `PerformanceSmoke`
  `91`, `EvidenceOracle` `262`, default `HardwareSmoke` `0`, and
  `windows-hardware-smoke` `7`. It does not authorize real image/audio/mesh
  decode, decoded byte storage, File IO expansion, package parsing, RHI upload,
  render graph execution, RenderCore scheduling, material graph,
  scene/UI/World/Script/Game Adapter behavior, native/backend leakage, reports,
  screenshots, logs, sleeps, manual proof, hardware-only proof, or
  original-game evidence.
- P2-GATE-027 is approved as an RHI primitive retirement ledger first slice
  over landed RHI buffer, texture, sampler, shader, pipeline, and fence value
  contracts. It may record bounded retirement requests, reject duplicate or
  invalid retirements, drain ready retirement entries, invalidate public handles
  deterministically, and expose counters after ENG-144R combined review PASS
  without introducing renderer
  scheduling or Resource ownership. The proposal baseline is `74189da`;
  discovery is `windows-fast-gate` `858/858`, `RHI` `141`, `Fast` `858`,
  `PerformanceSmoke` `91`, `EvidenceOracle` `262`, default `HardwareSmoke`
  `0`, and `windows-hardware-smoke` `7` with `RHI` `5`. It does not authorize
  Resource, Streaming, File, Package, RenderCore, material, scene, UI, World,
  Script, Game Adapter, backend-native public headers, new shader compilation,
  reports, screenshots, logs, sleeps, manual proof, hardware-only proof, or
  original-game evidence.
- No Phase 2 implementation task may be created until the owning gate is
  approved and sequencing confirms it will not pull in World/Game Adapter,
  RenderCore, scene policy, UI business, reports, or evidence tooling.

## Review Routing

1. Architecture posts ADR/gate proposal in #架构决策.
2. Engine-reference review checks mature-engine responsibility separation.
3. Performance review checks frame-path allocation, command/capture bounds, and
   disabled diagnostics behavior.
4. PM/gate review issues final gate state.
5. Code review starts only after implementation exists.

Acceleration note:

- Low-risk and medium-risk proposal reviews may use one combined lower-engine
  review task instead of default A/B/C lanes.
- Split review lanes are reserved for gates with high native/backend leakage
  risk, cross-module ownership drift, or test-policy uncertainty.
- Independent lower-engine proposal tracks may run in parallel when they do not
  authorize implementation before their own approval commits.

## Immediate Next Steps

1. Use the landed P2-GATE-004 labels and `windows-hardware-smoke` preset to keep
   future hardware work focused without weakening default deterministic evidence.
2. Use the landed P2-GATE-005 Platform window/native surface/event pump as the
   input boundary for the next RHI work, without treating it as RHI, DXGI,
   swapchain, present, capture, or render proof.
3. Use the landed P2-GATE-006 backend-neutral RHI device boundary before any
   D3D11 device or DXGI swapchain implementation task is created.
4. Use the landed P2-GATE-007 D3D11 device/context/swapchain
   clear/present/capture first slice before any P2-GATE-008 resource or pipeline
   primitive task is created. Do not create a D3D11 mesh or RenderCore task
   before a separate gate approves that scope.
5. Treat the landed P2-GATE-009 RHI-only visible-triangle first slice as the
   current graphics backend proof. It proves one synthetic visible triangle
   through capture-byte assertions, not screenshots, reports, or manual
   inspection. It still does not authorize static mesh, RenderCore, Resource
   upload, UI, World, or Game Adapter tasks.
6. Keep Phase 3/World expansion paused except for critical fixes. The landed
   P3 gates are contract baselines, not permission to keep moving upward while
   lower hardware remains null/test-only.
7. Treat the landed P2-GATE-010 worker lifecycle and async file-completion
   substrate as the lower-engine async baseline. It proves bounded worker and
   async completion behavior through counters/statuses, not sleeps, logs,
   reports, screenshots, manual inspection, or real hardware.
8. Treat the landed P2-GATE-011 real audio callback first slice as hardware
   backend proof only. It proves private XAudio2 callback ownership and
   unavailable-device evidence, not codec, streaming, audio scene, Resource,
   UI, World, or Game Adapter behavior.
9. Treat the landed P2-GATE-012 platform input bridge first slice as
   lower-engine input proof only. It proves private Windows input translation,
   bounded event counters, and value-only public contracts, not UI navigation,
   text input, gameplay mapping, World, or Game Adapter behavior.
10. Treat the landed P2-GATE-013 static mesh fixture as RHI-only geometry proof.
    It proves indexed static geometry through value contracts, private D3D11
    backend state, capture bytes, and bounded counters/statuses; it does not
    authorize Resource, Package, RenderCore, material system, scene traversal,
    reports, screenshots, manual visual proof, or backend type leakage in public
    headers.
11. Treat the landed P2-GATE-014 texture sampling fixture as RHI-only sampling
    proof. It proves texture/sampler binding and sampling through value
    contracts, private D3D11 backend state, capture bytes, and bounded
    counters/statuses; it does not authorize Resource loading, image decode,
    RenderCore, material system, scene traversal, shader compiler/source
    tooling, reports, screenshots, manual visual proof, or backend type leakage
    in public headers.
12. Treat the landed P2-GATE-015 package/resource staging queue as lower-engine
    streaming proof only. It proves bounded staging from Package load-plan,
    Resource handle/type, and File async values; it does not authorize package
    parsers, Resource load completion mutation, decode, RHI upload execution,
    RenderCore, material, scene/UI/World/Script/Game Adapter, reports,
    screenshots, logs, sleeps, manual proof, original-game package evidence, or
    public native/backend leakage.
13. Treat the landed P2-GATE-016 Resource upload queue as lower-engine upload
    proof only. It proves bounded staging-completion-to-RHI upload through
    public value contracts; it does not authorize decode, Resource load-state
    ownership, RenderCore, material, scene/UI/World/Script/Game Adapter,
    reports, screenshots, logs, sleeps, manual proof, hardware-only proof, or
    native/backend leakage.
14. Treat the landed P2-GATE-017 RenderCore fixture pass as lower-engine
    fixture proof only. It proves bounded pass scheduling over existing public
    `YuRHI` value contracts; it does not authorize Resource/Streaming ownership,
    asset decode/import, shader compiler/source tooling, material,
    scene/UI/World/Script/Game Adapter, reports, screenshots, logs, sleeps,
    manual proof, hardware-only proof, or native/backend leakage.
15. Treat the landed P2-GATE-018 material binding fixture as lower-engine
    material-value proof only. It groups caller-owned public `YuRHI` and landed
    `YuRenderCore` fixture values; it does not authorize material graph
    behavior, shader compiler/source tooling, Resource/Streaming ownership,
    scene/UI/World/Script/Game Adapter, reports, screenshots, logs, sleeps,
    manual proof, hardware-only proof, or native/backend leakage.
16. Treat the landed P2-GATE-019 submission batch fixture as lower-engine batch
    proof only. It proves deterministic sequential submission of caller-owned,
    prepared RenderCore pass requests and material binding values; it does not
    authorize render graph, scene traversal, Resource/Streaming ownership, UI,
    World, Script, Game Adapter, reports, screenshots, logs, sleeps, manual
    proof, hardware-only proof, or native/backend leakage.
17. Treat the landed P2-GATE-020 frame packet fixture as lower-engine frame
    envelope proof only. It proves one caller-owned prepared submission batch
    inside a bounded frame packet envelope; it does not authorize render graph,
    frame graph, renderer scheduling, scene traversal, Resource/Streaming
    ownership, UI, World, Script, Game Adapter, reports, screenshots, logs,
    sleeps, manual proof, hardware-only proof, or native/backend leakage.
18. Treat the landed P2-GATE-021 upload completion commit as a narrow
    Resource/Streaming terminal-state bridge only. It proves committed,
    rejected, duplicate, failed, and FIFO slot-reuse behavior through
    ResourceRegistry value contracts; it does not authorize cache ownership,
    package parsing, asset decode/import, render graph, frame graph, RenderCore
    scheduling, scene/UI/World/Script/Game Adapter behavior, native/backend
    leakage, reports, screenshots, logs, sleeps, manual proof, hardware-only
    proof, or original-game evidence.
19. Treat the landed P2-GATE-022 Resource residency first slice as Resource
    budget proof only. It proves Resource-owned residency state, budget
    counters, pin/unpin, and deterministic eviction-candidate policy; it does
    not authorize cache payload storage, package parsing, asset decode/import,
    RHI resource destruction, render graph, frame graph, RenderCore scheduling,
    scene/UI/World/Script/Game Adapter behavior, native/backend leakage,
    reports, screenshots, logs, sleeps, manual proof, hardware-only proof, or
    original-game evidence.
20. Treat the landed P2-GATE-023 Resource cache payload first slice as Resource
    cache ownership proof only. It proves Resource-owned opaque cache payload
    bytes, cache-slot records, deterministic counters, readback, and release
    behavior; it does not authorize package parsing, asset decode/import, File
    IO expansion, RHI resource destruction, render graph, frame graph,
    RenderCore scheduling, scene/UI/World/Script/Game Adapter behavior,
    native/backend leakage, reports, screenshots, logs, sleeps, manual proof,
    hardware-only proof, or original-game evidence.
21. Treat the landed P2-GATE-024 Resource asset decode-plan first slice as
    Resource-owned decode-plan value proof over already cached bytes. It does
    not authorize File IO expansion, package parsing, real codec/decode output,
    RHI upload, render graph, scene streaming, World, UI, Script, or Game
    Adapter behavior.
22. Treat the landed P2-GATE-025 RenderCore render graph skeleton first slice
    as RenderCore-owned graph declaration and dependency validation proof over
    landed fixture paths. It does not authorize render scheduling, frame graph
    execution, command-list parallelism, Resource/Streaming/Package/File
    ownership, material graph, scene/UI/World/Script/Game Adapter behavior,
    native/backend leakage, reports, screenshots, logs, sleeps, manual proof,
    hardware-only proof, or original-game evidence.
23. Treat approved P2-GATE-026 as a Resource decode-result import-ready record
    boundary. It must stay in Resource-owned metadata over landed decode-plan
    records and must not authorize real codec output, decoded byte storage, RHI
    upload, RenderCore scheduling, material graph, scene/UI/World/Script/Game
    Adapter behavior, native/backend leakage, reports, screenshots, logs,
    sleeps, manual proof, hardware-only proof, or original-game evidence.
24. Treat approved P2-GATE-027 as an RHI primitive retirement ledger boundary.
    It must stay in RHI-owned retirement request, ledger, drain, handle
    invalidation, and counter proof over landed primitive handles and must not
    authorize Resource/Streaming/File/Package ownership, RenderCore scheduling,
    material graph, scene/UI/World/Script/Game Adapter behavior, backend-native
    public leaks, new shader compiler/source tooling, reports, screenshots,
    logs, sleeps, manual proof, hardware-only proof, or original-game evidence.
