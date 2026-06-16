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
- RHI is a null backend only: no backend-neutral device interface, no D3D/DXGI
  device, no swapchain, no GPU buffers, no shaders, no pipeline state, no depth,
  no upload/fence path, and no draw path.
- Audio is a test backend only: no WASAPI/XAudio2 device, callback thread,
  latency handoff, codec, streaming, or real sink.
- Input is replay/action-binding only: no OS device bridge or focus-aware event
  pump.
- Thread/File/Package/Resource are deterministic first slices: no worker pool,
  async IO, streaming, upload queue, cache ownership, or asset decode pipeline.

The immediate ordering is:

```text
test tier labels and optional hardware-smoke presets
-> Platform window/native surface/event pump
-> RHI backend-neutral device boundary
-> D3D11 device/context/swapchain clear/present/capture
-> D3D11 resource and pipeline primitives
-> visible triangle fixture
-> static mesh fixture
-> worker/job and async IO substrate
-> package/resource streaming and upload queue
-> real audio backend
-> OS input bridge
-> RenderCore fixture pass
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
| P2-GATE-007 | D3D11 Device Swapchain Clear Present Capture | L3 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P2_GATE_007_D3D11_DEVICE_SWAPCHAIN_CLEAR_PRESENT_CAPTURE.md`; approved after ENG-104A/B/C PASS; first real D3D11 device/context/swapchain clear/present/capture through RHI boundary; must isolate `HardwareSmoke` from default fast gate; no mesh, shader pipeline expansion, RenderCore, Resource, report, visual proof, or Game Adapter |
| P2-GATE-008 | D3D11 Resource And Pipeline Primitives | L3 | `NOT_APPROVED` | Proposed after P2-GATE-007 | Vertex/index/constant buffers, shader module boundary, input layout, pipeline state, texture/sampler, upload/update/fence; no RenderCore, material system, or scene traversal |
| P2-GATE-009 | D3D11 Visible Triangle Fixture | L3 | `NOT_APPROVED` | Proposed after P2-GATE-008 | First visible geometry proof through RHI capture; no static mesh asset pipeline, RenderCore, World, UI, or Game Adapter |
| P2-GATE-010 | Thread Worker And Async IO Substrate | L1-L3 | `NOT_APPROVED` | Parallel proposal candidate | Worker ownership, bounded queues, drain/cancel, async file completion; no Resource semantics, package streaming policy, upload queue, or gameplay |
| P2-GATE-011 | Real Audio Backend Callback | L1-L3 | `NOT_APPROVED` | Parallel proposal candidate | WASAPI/XAudio2 callback backend into existing mixer/test-sink contract; no codec, BGM/SE business IDs, Resource loading, UI, script, or gameplay |
| P2-GATE-012 | Platform Input Device Bridge | L1-L3 | `NOT_APPROVED` | Parallel proposal candidate | OS device events and focus-aware snapshots into Input boundary; no UI navigation, title menu behavior, script, scene, or Game Adapter |

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
- P2-GATE-007 is approved for first slice after ENG-104A boundary/reference
  PASS, ENG-104B implementability PASS, and ENG-104C test-policy PASS. The
  approved scope is the first real D3D11 device/context/swapchain
  clear/present/capture path through the landed RHI boundary. It must isolate
  `HardwareSmoke` from the default fast gate, keep public RHI headers free of
  Windows/D3D11/DXGI/COM/Platform types, keep production `YuRHI` independent
  from `YuPlatform`, and must not authorize mesh, shader pipeline expansion,
  RenderCore, Resource, report, visual proof, or Game Adapter behavior.
- P2-GATE-008 through P2-GATE-012 are not approved. They are the ENG-096
  hardware-first proposal queue and must go through normal gate review before
  implementation.
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

## Immediate Next Steps

1. Use the landed P2-GATE-004 labels and `windows-hardware-smoke` preset to keep
   future hardware work focused without weakening default deterministic evidence.
2. Use the landed P2-GATE-005 Platform window/native surface/event pump as the
   input boundary for the next RHI work, without treating it as RHI, DXGI,
   swapchain, present, capture, or render proof.
3. Use the landed P2-GATE-006 backend-neutral RHI device boundary before any
   D3D11 device or DXGI swapchain implementation task is created.
4. Implement P2-GATE-007 as the D3D11 device/context/swapchain
   clear/present/capture first slice before any P2-GATE-008 resource or pipeline
   primitive task is created. Do not create a D3D11 mesh or RenderCore task
   before D3D11 clear/present/capture is landed and reviewed.
5. Keep Phase 3/World expansion paused except for critical fixes. The landed
   P3 gates are contract baselines, not permission to keep moving upward while
   lower hardware remains null/test-only.
6. Prepare worker/job and async IO proposals in parallel with RHI planning, but
   keep Resource/Package streaming semantics out until the substrate is proven.
