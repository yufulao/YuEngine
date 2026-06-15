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
- No Phase 2 implementation task may be created until the owning gate is
  approved and PM confirms sequencing against the active Phase 1 review queue.

## Review Routing

1. Architecture posts ADR/gate proposal in #架构决策.
2. Engine-reference review checks mature-engine responsibility separation.
3. Performance review checks frame-path allocation, command/capture bounds, and
   disabled diagnostics behavior.
4. PM/gate review issues final gate state.
5. Code review starts only after implementation exists.

## Immediate Next Steps

1. Use the current null RHI and audio test-backend fast-gate baselines as stable
   lower-layer vocabulary for approved upper gates, but do not expand either
   module without an explicit next-slice gate or amended owning gate.
2. Keep P2-GATE-003 package expansion held. The first slice is closed and QA
   cleared, but later package work still requires a new explicit Architect
   decision.
3. Prepare RenderCore, real graphics backend, real audio backend, and async IO
   only through new explicit gates. Do not infer those scopes from the first
   null/test backend slices.
4. Prepare async IO boundary only after Thread/File/Resource/Package review
   state is stable enough to avoid depending on unresolved vocabulary.
