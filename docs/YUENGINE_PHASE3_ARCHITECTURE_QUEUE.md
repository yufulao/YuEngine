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
| ADR-0015 | Serialization value stream boundary | 八云紫 | 红美铃, 八云蓝, 博丽灵梦, 大妖精, 射命丸文 if evidence boundary is questioned | Accepted | Serialization value stream first slice |

## Module Gate Proposal Queue

| Gate | Module | Layer | Current decision | Status | Notes |
| --- | --- | --- | --- | --- | --- |
| P3-GATE-001 | Object Identity And Lifetime Registry | L2-L4 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Bounded synthetic object registry only; no component model, world/scene, script binding, Resource mutation, reflection, serialization payload, UI/gameplay, tools, reports, or Game Adapter |
| P3-GATE-002 | Serialization Value Stream | L3-L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Bounded caller-provided-buffer value stream only; no File/package/Resource/object construction/reflection/script/scene/save/tool/report/Game Adapter scope |
| P3-GATE-003 | Script Native Bridge | L4 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Bounded native call registry only; no VM, bytecode, reflection, world/scene, Resource/Package/File/Object/Serialize dependency, UI/gameplay/tools/reports/Game Adapter |
| P3-GATE-004 | World Lifecycle Fixture | L5 | `NEEDS_SCRIPT_STABILITY` | Gate prepared, blocked by Script stability | Gate doc: `docs/gates/P3_GATE_004_WORLD_LIFECYCLE_FIXTURE.md`; must wait for Script first slice stability before implementation |

## Current Active Gates

- P3-GATE-001 is `APPROVED_FOR_FIRST_SLICE` after ADR-0014 acceptance,
  gate/test-coverage + implementability acceptance, performance acceptance, and
  PM/final sequencing closure in task #61. Resource/File/Package/Input closure
  is not a first-slice blocker because those modules remain forbidden
  dependencies. Implementation must stay within `YuObject` / `YuObjectTests`
  and use a clean worktree or clean sequencing for CMake target changes.
- P3-GATE-002 has positive technical review lanes at `10abe41`: engine-reference
  ADR context accepted, performance accepted, implementability/test coverage
  accepted, ADR-0015 accepted, and evidence review is not triggered while
  original-save/resource scope remains excluded. Task #69 closed PM/final
  sequencing and approved the first slice. Implementation must stay within
  `YuSerialize` / `YuSerializeTests`, caller-provided buffers, deterministic
  primitive record/field encoding, explicit statuses, no-mutation failures, and
  diagnostics-disabled equivalence. Diagnostics availability must remain outside
  the serialization write/read result vocabulary.
- P3-GATE-003 is approved from `32d4045` for a narrow `YuScript` native bridge
  first slice. It may add only `YuScript`, `YuScriptTests`, and CMake/CTest
  registration. It must use stable call IDs and caller-provided value slots, and
  it must not introduce a VM, bytecode, reflection, world/scene ownership,
  original-game services, reports, or dependencies on Resource/Package/File,
  Object, Serialize, Kernel, Platform, Diagnostics, RHI, Audio, Input, UI,
  Tools, or Game Adapter modules.
- P3-GATE-004 is prepared in
  `docs/gates/P3_GATE_004_WORLD_LIFECYCLE_FIXTURE.md`, but it is not approved
  for implementation. Its proposed first slice is bounded to world lifecycle,
  update phases, ownership, fixed capacities, and tests. Implementation must
  wait until the Script first slice is stable and this gate is explicitly updated
  to `APPROVED_FOR_FIRST_SLICE`.
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

1. Architect may create a scoped `YuObject` first-slice implementation handoff
   from P3-GATE-001, preserving the approved boundaries and clean-worktree
   sequencing guard.
2. Architect may create a scoped `YuSerialize` first-slice implementation
   handoff from P3-GATE-002, preserving the approved boundaries and
   clean-worktree sequencing guard.
3. Continue closing active Phase 1 and Phase 2 implementation reviews; current
   package review closure does not authorize package expansion or P3 dependency
   creep.
4. Architect may create a scoped `YuScript` native bridge implementation handoff
   from P3-GATE-003, preserving fixed-capacity storage, stable call IDs, and
   no-hot-path-allocation test requirements.
5. Keep `YuWorld` implementation blocked until Script first slice stability is
   confirmed and P3-GATE-004 is explicitly updated from
   `NEEDS_SCRIPT_STABILITY` to `APPROVED_FOR_FIRST_SLICE`.
6. Do not use P3-GATE-002 or P3-GATE-003 to introduce File/package/Resource/object
   construction/reflection/script/scene/save/tool/report/Game Adapter scope.
