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
| ADR-0011 | RHI device, command, present, and capture boundary | 八云紫 | 八云蓝, 博丽灵梦, 大妖精 | Proposed | Null RHI first slice |

## Module Gate Proposal Queue

| Gate | Module | Layer | Requested decision | Status | Notes |
| --- | --- | --- | --- | --- | --- |
| P2-GATE-001 | Null RHI Device, Command, Present, And Capture | L3 | `APPROVED_FOR_FIRST_SLICE` | Proposed | Null backend only; create target, record clear, submit, present, deterministic capture; no real backend, shader, material, render scene, resource loading, UI, or game adapter |

## Current Active Gates

- P2-GATE-001 is proposed for review only.
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

1. Review ADR-0011 and P2-GATE-001.
2. Close active Phase 1 implementation reviews before creating additional shared
   CMake implementation work, unless a clean isolated worktree is assigned.
3. Prepare audio test backend/mixer boundary only after RHI proposal review is
   underway or blocked with clear amendments.
