# YuEngine Phase 1 Architecture Queue

Status: active architecture queue
Owner: 八云紫, 总架构师
Started: 2026-06-10
Applies after: Phase 0 restart inputs through `90c9a34`

## Purpose

Phase 0 established the restart baseline, reference map, evidence boundary, module-entry gates, and performance gates. Phase 1 now moves from baseline definition into reviewable architecture decisions and module gate proposals.

This queue does not authorize code by itself. It defines the ADRs and gate proposals that must be reviewed before implementation slices can be assigned.

## Governance

- 八云紫 owns final architecture direction, ADR order, module boundaries, and implementation sequence.
- 红美铃 acts as PM plus implementation gate reviewer. She tracks task flow, review rhythm, missing materials, and gate states under the architecture direction.
- 博丽灵梦 reviews performance gate content before lower-layer implementation is allowed.
- 八云蓝 provides deeper engine-reference evidence when a module ADR needs more UE5/Unity inspection.
- 射命丸文 maintains original-game evidence as future Game Adapter acceptance input, not engine design.
- 雾雨魔理沙 starts code review after an approved implementation slice exists.
- Implementation engineers wait for approved gate states before claiming code tasks.

Gate policy:

- `APPROVED_FOR_FIRST_SLICE` or `APPROVED_FOR_NEXT_SLICE` is required before any code implementation task is created.
- Strict gates are used to focus progress, not to freeze progress. If a proposal is blocked, the owner must identify the missing architecture, performance, evidence, or test material and route it back immediately.

## Baseline Inputs

| Input | Commit | File |
| --- | --- | --- |
| Architecture restart plan | `33324d0` | `docs/YUENGINE_ARCHITECTURE_RESTART_PLAN.md` |
| Performance gates | `abecd5e` | `docs/YUENGINE_PERFORMANCE_GATES.md` |
| Module entry gates | `d1f262b` | `docs/YUENGINE_MODULE_ENTRY_GATES.md` |
| UE5/Unity reference map | `7ceb272` | `docs/YUENGINE_PHASE0_SUBSYSTEM_REFERENCE_MAP.md` |
| Original evidence catalog | `90c9a34` | `docs/TOUHOU_NEWWORLD_EVIDENCE_CATALOG.md`, `docs/TOUHOU_NEWWORLD_EVIDENCE_INVENTORY.csv` |

## Phase 1 Objective

Prepare the first implementable lower-layer slice without writing gameplay, viewer/demo code, original-game adapter logic, or report-driven runtime.

The target first slice is:

```text
build/test skeleton
-> headless host
-> platform timer/log/error boundary
-> kernel module lifecycle
-> minimal service registry
-> deterministic tests and performance signals
```

The first slice explicitly excludes:

- RHI or visual rendering.
- Resource package parsing.
- Audio backend.
- Input action mapping beyond future interface planning.
- UI framework.
- Script VM.
- World/scene/gameplay.
- TouhouNewWorld title/save/new-game/scene/actor/camera/tutorial.
- Oracle/frame capture.

## ADR Queue

| ID | Title | Owner | Reviewers | Status | Blocks |
| --- | --- | --- | --- | --- | --- |
| ADR-0001 | Initial runtime, build, and test shape | 八云紫 | 红美铃, 博丽灵梦, 雾雨魔理沙 later | Accepted | Any code skeleton |
| ADR-0002 | Source tree and module boundary layout | 八云紫 | 红美铃, 雾雨魔理沙 later | Accepted | First implementation file layout |
| ADR-0003 | Module lifecycle and dependency declaration model | 八云紫 | 红美铃, 博丽灵梦 | Accepted | Engine Kernel first slice |
| ADR-0004 | Logging, diagnostics channel, and report boundary | 八云紫 | 红美铃, 博丽灵梦 | Accepted | Platform/Kernel diagnostics |
| ADR-0005 | Test taxonomy and fast gate command | 八云紫 | 红美铃, 雾雨魔理沙 later | Accepted | Implementation acceptance |
| ADR-0006 | Memory accounting and allocation policy skeleton | 博丽灵梦 with 八云紫 | 红美铃 | Accepted | Memory first slice |
| ADR-0007 | Thread/task model skeleton | 博丽灵梦 with 八云紫 | 雾雨魔理沙, 博丽灵梦 | Accepted | Thread/Task first slice |
| ADR-0008 | File/VFS first boundary | 八云紫 | 大妖精, 射命丸文, 博丽灵梦 | Accepted | Future File/VFS gate |
| ADR-0009 | Resource identity and lifetime boundary | 八云紫 | 红美铃, 大妖精, 博丽灵梦, 射命丸文 | Proposed | Resource handle/dependency first slice |

ADR-0001 is accepted as the initial runtime/build/test shape. ADR-0002 is accepted as the source tree and module boundary layout. ADR-0003 is accepted as the Kernel lifecycle and dependency model for the first slice.

## Module Gate Proposal Queue

| Gate | Module | Layer | Requested decision | Status | Notes |
| --- | --- | --- | --- | --- | --- |
| P1-GATE-001 | Platform Host + Engine Kernel Bootstrap | L0-L2 | `APPROVED_FOR_FIRST_SLICE` | Implemented | Headless host, timer, log sink, error boundary, module lifecycle, service registry, tests |
| P1-GATE-002 | Memory Accounting Skeleton | L1-L2 | `APPROVED_FOR_FIRST_SLICE` | Implementation in review | ADR-0006 accepted; accounting hooks and leak fixtures only, no full allocator |
| P1-GATE-003 | Thread/Task Primitive Skeleton | L1-L2 | `APPROVED_FOR_FIRST_SLICE` | Approved | ADR-0007 accepted; bounded queue and inline executor only, no worker pool |
| P1-GATE-004 | Diagnostics Channel Boundary | L2/L7 | `APPROVED_FOR_FIRST_SLICE` | Approved | ADR-0004 accepted; bounded synchronous observer only, no reports/profiler/async queue |
| P1-GATE-005 | File Primitive And Path Normalization | L1-L3 | `APPROVED_FOR_FIRST_SLICE` | Approved | ADR-0008 accepted; path normalization and loose fixture reads only, no package parser |
| P1-GATE-006 | Resource Identity And Lifetime Skeleton | L4 | `APPROVED_FOR_FIRST_SLICE` | Proposed | Needs ADR-0009; synthetic handles/dependencies only, no file/package/load/decode |

## Current Active Gates

- P1-GATE-002 implementation is in review as task #14.
- P1-GATE-003, P1-GATE-004, and P1-GATE-005 are approved for first implementation slices; task handoffs must preserve their dependency guards.
- ADR-0009 and P1-GATE-006 are Resource identity/lifetime proposals only; they do not authorize implementation before review.
- No implementation task may be created from a gate until that gate has `APPROVED_FOR_FIRST_SLICE`.

## Review Routing

1. Architecture posts ADR/gate proposal in #架构决策.
2. PM/gate review tracks status in #实现门禁.
3. Performance review checks deterministic signals and budgets where relevant.
4. Evidence review is requested only if original-game or old-backup facts affect validation.
5. After approval, implementation work is created as a new task with exact allowed scope.
6. Code review is assigned only after implementation exists.

## Stop Conditions

The next implementation slice must not be created if any of these remain true:

- ADR-0001 is not accepted.
- P1-GATE-001 is not approved by gate and performance review.
- Fast gate command is not named.
- First slice tests do not state public interfaces and deterministic signals.
- Runtime behavior depends on diagnostics/report output.
- Any TouhouNewWorld evidence is used to shape L0-L2 APIs.

## Immediate Next Steps

1. Close task #14 code/semantic review for the P1-GATE-002 memory implementation.
2. Create scoped implementation tasks for P1-GATE-003, P1-GATE-004, and P1-GATE-005.
3. Route ADR-0009 and P1-GATE-006 for Resource identity/lifetime review.
4. Preserve the Phase 1 exclusions: no renderer, resources package parser, async IO, gameplay/world, reports, capture/oracle, or original-game adapter behavior.
5. If a gate is blocked, amend the owning ADR/gate immediately instead of creating implementation work.
