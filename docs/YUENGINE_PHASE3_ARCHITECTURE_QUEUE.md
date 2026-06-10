# YuEngine Phase 3 Architecture Queue

Status: active architecture queue
Owner: 八云紫, 总架构师
Started: 2026-06-10
Applies after: Phase 2 architecture through `5066376`

## Purpose

Phase 3 prepares core runtime boundaries above the low-level runtime modules.
This queue is architecture only until each gate reaches an explicit approval
state.

Phase 3 must not become scene/gameplay, UI business, original-game adapter, or
tool/report work. Its job is to define the core object, serialization, script,
and resource bridge contracts that later world systems can depend on without
polluting lower layers.

## Governance

- 八云紫 owns ADR order, layer boundaries, and implementation sequencing.
- 红美铃 owns PM/final gate state before any implementation task is created.
- 八云蓝 reviews mature-engine responsibility comparisons for object,
  serialization, script, and core runtime boundary shape.
- 博丽灵梦 reviews allocation, lifetime, dispatch, and hot-path cost.
- 大妖精 reviews whether proposed first slices are locally implementable and
  enforceable by tests.
- 射命丸文 reviews evidence-boundary risks only when original-game facts might
  leak into core runtime APIs.
- 雾雨魔理沙 starts code review after implementation exists.

Gate policy:

- `APPROVED_FOR_FIRST_SLICE` is required before code implementation.
- Approved Phase 3 implementation tasks should wait until active Phase 1 and
  Phase 2 implementation reviews are stable, or they must be assigned in a
  clean isolated worktree to avoid shared CMake/index churn.
- Original-game evidence remains validation input only. It must not define L2
  through L5 core runtime API shape.

## Baseline Inputs

| Input | Commit | File |
| --- | --- | --- |
| Architecture restart plan | `33324d0` | `docs/YUENGINE_ARCHITECTURE_RESTART_PLAN.md` |
| Module entry gates | `d1f262b` | `docs/YUENGINE_MODULE_ENTRY_GATES.md` |
| Performance cost standards | `1fcf59b` | `docs/YUENGINE_PERFORMANCE_COST_STANDARDS.md` |
| Phase 1 queue | latest | `docs/YUENGINE_PHASE1_ARCHITECTURE_QUEUE.md` |
| Phase 2 queue | latest | `docs/YUENGINE_PHASE2_ARCHITECTURE_QUEUE.md` |

## Phase 3 Objective

Define and then implement the first core runtime interfaces:

```text
object identity/lifetime registry
-> serialization/versioned value stream
-> script/native bridge boundary
-> world/scene fixture boundary after object and script stability
```

Phase 3 remains blocked from:

- scene/world gameplay behavior;
- UI screens, title flow, save/new-game, actor/camera/tutorial adapter logic;
- script service state copied from old runtime files;
- original-game resource or package facts as API shape;
- report/capture/oracle ownership inside runtime behavior;
- editor/tooling as runtime dependency.

## ADR Queue

| ID | Title | Owner | Reviewers | Status | Blocks |
| --- | --- | --- | --- | --- | --- |
| ADR-0014 | Object identity and lifetime registry boundary | 八云紫 | 红美铃, 八云蓝, 博丽灵梦, 大妖精 | Accepted | Object registry first slice |

## Module Gate Proposal Queue

| Gate | Module | Layer | Requested decision | Status | Notes |
| --- | --- | --- | --- | --- | --- |
| P3-GATE-001 | Object Identity And Lifetime Registry | L2-L4 | `APPROVED_FOR_FIRST_SLICE` | In review | Bounded synthetic object registry only; no component model, world/scene, script binding, Resource mutation, reflection, serialization payload, UI/gameplay, tools, reports, or Game Adapter |

## Current Active Gates

- P3-GATE-001 is in architecture review after `df8e326`. ADR-0014 is
  accepted and gate/test-coverage is clear, but the gate must not be approved
  for implementation until performance/cost and PM/final sequencing are clear
  and active implementation reviews are stable enough.
- No Phase 3 implementation task may be created until the owning gate is
  approved and PM confirms sequencing against active Phase 1 and Phase 2 review
  queues.

## Review Routing

1. Architecture posts ADR/gate proposal in #架构决策.
2. Engine-reference review checks mature-engine responsibility separation.
3. Performance review checks object lifetime cost, handle lookup cost, and
   allocation/accounting boundaries.
4. Implementability review checks public surface and required tests.
5. PM/gate review issues final gate state.
6. Code review starts only after implementation exists.

## Immediate Next Steps

1. Close P3-GATE-001 performance/cost and PM/final sequencing lanes.
2. Continue closing active Phase 1 and Phase 2 implementation reviews.
3. Do not create a `YuObject` implementation task until P3-GATE-001 receives
   explicit `APPROVED_FOR_FIRST_SLICE` and PM sequencing approval.
4. Prepare serialization boundary only after object identity review state is
   stable enough to avoid depending on unresolved lifecycle vocabulary.
