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
- RHI has a D3D11 visible-triangle capture-byte proof through P2-GATE-009, but
  still has no indexed static mesh fixture, Resource upload, RenderCore,
  material system, scene traversal, report, visual proof, UI, World, or Game
  Adapter behavior.
- Audio has a private Windows XAudio2 callback first slice through
  P2-GATE-011, but no codec, streaming, Resource-backed audio asset pipeline,
  audio scene, BGM/SE service, UI, World, or Game Adapter behavior.
- Input has a private Windows platform input bridge through P2-GATE-012, but no
  UI navigation, text input, gameplay mapping, Script, World, or Game Adapter
  behavior.
- Thread/File have a worker lifecycle and async file-completion substrate through
  P2-GATE-010, but no Resource/Package streaming, upload queue, cache ownership,
  or asset decode pipeline.

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
-> package/resource streaming and upload queue
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
| P2-GATE-007 | D3D11 Device Swapchain Clear Present Capture | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_007_D3D11_DEVICE_SWAPCHAIN_CLEAR_PRESENT_CAPTURE.md`; landed at `18d73a3`; first real D3D11 device/context/swapchain clear/present/capture through RHI boundary; default fast gate is 669/669 PASS; `HardwareSmoke` is isolated from default fast gate; `windows-hardware-smoke` discovers and runs 1 D3D11 capture-byte test; no mesh, shader pipeline expansion, RenderCore, Resource, report, visual proof, or Game Adapter |
| P2-GATE-008 | D3D11 Resource And Pipeline Primitives | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_008_D3D11_RESOURCE_AND_PIPELINE_PRIMITIVES.md`; landed at `2f7c8e1`; RHI-only buffer/texture/sampler/shader/input-layout/pipeline/fence primitive contracts after P2-GATE-007; default fast gate is `681/681` PASS; `windows-hardware-smoke` discovers and runs 2 D3D11 tests including primitive resource/pipeline snapshot; no visible triangle, draw proof, RenderCore, material system, Resource loading, mesh asset pipeline, scene traversal, report, visual proof, shader compiler, or Game Adapter |
| P2-GATE-009 | D3D11 Visible Triangle Fixture | L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_009_D3D11_VISIBLE_TRIANGLE_FIXTURE.md`; landed at `f55f6dd`; first visible geometry proof through RHI capture bytes after P2-GATE-008; default fast gate is `685/685` PASS; `windows-hardware-smoke` discovers and runs 3 tests including `RHI_D3D11Hardware_VisibleTriangleCaptureBytes`; no static mesh asset pipeline, RenderCore, Resource upload, World, UI, report, screenshot, manual visual proof, or Game Adapter |
| P2-GATE-010 | Thread Worker And Async IO Substrate | L1-L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_010_THREAD_WORKER_AND_ASYNC_IO_SUBSTRATE.md`; landed at `5a26a53`; worker lifecycle and async file-completion substrate; default fast gate is `696/696` PASS; no Resource semantics, package streaming policy, upload queue, render submission, static mesh, RenderCore, real audio callback, OS input, or gameplay |
| P2-GATE-011 | Real Audio Backend Callback | L1-L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_011_REAL_AUDIO_BACKEND_CALLBACK.md`; landed at `1d7d2ca`; private Windows XAudio2 callback backend proof through existing mixer/test-sink contract; default fast gate remains deterministic; no codec, BGM/SE business IDs, Resource loading, UI, script, or gameplay |
| P2-GATE-012 | Platform Input Device Bridge | L1-L3 | `APPROVED_FOR_FIRST_SLICE` | First-slice covered | Gate doc: `docs/gates/P2_GATE_012_PLATFORM_INPUT_DEVICE_BRIDGE.md`; landed at `58088bd`; private Windows input bridge into existing Input value boundary; default fast gate is `713/713` PASS; no UI navigation, title menu behavior, script, scene, gameplay mapping, manual proof, or Game Adapter |
| P2-GATE-013 | Static Mesh Fixture | L3 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P2_GATE_013_STATIC_MESH_FIXTURE.md`; approved after ENG-117A/B/C PASS; indexed static-geometry fixture through RHI/D3D11 value contracts; no Resource loading, RenderCore, material system, scene traversal, reports, screenshots, manual visual proof, or Game Adapter |

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
- P2-GATE-013 is approved for first slice after ENG-117A boundary/quality PASS,
  ENG-117B implementability PASS, and ENG-117C test-policy PASS. It is limited
  to the first indexed static-geometry fixture through existing `YuRHI` and
  private D3D11 value contracts. It does not authorize Resource loading,
  Package streaming, RenderCore pass scheduling, material system, scene
  traversal, reports, screenshots, manual visual proof, UI, World, or Game
  Adapter behavior.
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
10. Implement P2-GATE-013 only through the approved RHI/D3D11 static-mesh
    fixture first slice. The implementation must prove indexed static geometry
    through value contracts, private D3D11 backend state, capture bytes, and
    bounded counters/statuses; it must not add Resource, Package, RenderCore,
    material system, scene traversal, reports, screenshots, manual visual proof,
    or backend type leakage in public headers.
