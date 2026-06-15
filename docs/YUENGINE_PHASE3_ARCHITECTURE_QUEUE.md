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
| P3-GATE-004 | World Lifecycle Fixture | L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_004_WORLD_LIFECYCLE_FIXTURE.md`; bounded world lifecycle/update fixture only, no Script callback, actor/component/transform, Resource/Package/File, render/audio/physics/UI/tools/reports/Game Adapter |
| P3-GATE-005 | World Kernel Module Bridge | L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_005_WORLD_KERNEL_MODULE_BRIDGE.md`; narrow World-to-Kernel module adapter only, no Script callback, actor/component/transform, Resource/Package/File, render/audio/physics/UI/tools/reports/Game Adapter |
| P3-GATE-006 | World Object Identity Bridge | L5 over L2-L4 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_006_WORLD_OBJECT_IDENTITY_BRIDGE.md`; narrow WorldObjectId-to-ObjectHandle adapter only, `WorldInstance` core remains Object-free, no Script callback, actor/component/transform, Resource/Package/File, render/audio/physics/UI/tools/reports/Game Adapter |
| P3-GATE-007 | World Transform Data Fixture | L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_007_WORLD_TRANSFORM_DATA_FIXTURE.md`; data-only WorldTransformBridge fixture, bounded POD transform records keyed by WorldObjectId, no Script callback, actor/component/scene graph/transform hierarchy, Object/Resource/Package/File, render/audio/physics/UI/tools/reports/Game Adapter |
| P3-GATE-008 | World Script Dispatch Bridge | L5 over L4 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_008_WORLD_SCRIPT_DISPATCH_BRIDGE.md`; narrow WorldPhaseTrace-to-ScriptCallId dispatch adapter only, `WorldInstance` core remains Script-free, no VM/bytecode/reflection, actor/component/gameplay, Resource/Package/File, Serialize payload, Object ownership, render/audio/physics/UI/tools/reports/Game Adapter |
| P3-GATE-009 | World Serialize Snapshot Bridge | L5 over L3-L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_009_WORLD_SERIALIZE_SNAPSHOT_BRIDGE.md`; narrow World snapshot/phase-trace to YuSerialize value-stream adapter only, `WorldInstance` core remains Serialize-free, no File/Package/Resource/save policy, Object construction, reflection, Script, actor/component/gameplay, render/audio/physics/UI/tools/reports/Game Adapter |
| P3-GATE-010 | World Resource Binding Bridge | L5 over L4 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_010_WORLD_RESOURCE_BINDING_BRIDGE.md`; narrow WorldObjectId-to-ResourceHandle binding adapter only, caller-supplied WorldObjectId value contract, fixed-capacity storage, explicit Resource acquire/release failure tests, `WorldInstance` core remains Resource-free, Resource core remains World-free, no File/Package/load/decode/upload/render/audio/script/actor/component/gameplay/tools/reports/Game Adapter |
| P3-GATE-011 | World Component Attachment Bridge | L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_011_WORLD_COMPONENT_ATTACHMENT_BRIDGE.md`; narrow WorldObjectId-to-component-type/slot attachment sidecar only, fixed-capacity storage, no `WorldInstance` membership query, no actor/component behavior lifecycle, no Object/Resource/Script/Serialize/render/physics/audio/UI/tools/reports/Game Adapter dependency |
| P3-GATE-012 | World Component Query Bridge | L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_012_WORLD_COMPONENT_QUERY_BRIDGE.md`; narrow read-only query bridge over component attachment records only, caller-owned outputs, fixed-capacity scans, no `WorldInstance` membership query, no actor/component behavior lifecycle, no Object/Resource/Script/Serialize/render/physics/audio/UI/tools/reports/Game Adapter dependency |
| P3-GATE-013 | World Component Attachment Snapshot Bridge | L5 over L3-L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_013_WORLD_COMPONENT_ATTACHMENT_SNAPSHOT_BRIDGE.md`; narrow deterministic YuSerialize snapshot adapter over component attachment records only, caller-owned streams, validation before restore mutation, no component payload/lifecycle, no `WorldInstance` mutation, no Object/Resource/Script/File/Package/render/physics/audio/UI/tools/reports/Game Adapter dependency |
| P3-GATE-014 | World Component Resource Binding Bridge | L5 over L4 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_014_WORLD_COMPONENT_RESOURCE_BINDING_BRIDGE.md`; narrow fixed-capacity binding from existing component attachment tuples to already-registered ResourceHandle values, caller-owned attachment/resource state, acquire/release atomicity, no resource loading, component payload/lifecycle, `WorldInstance` mutation, File/Package/render/audio/script/UI/tools/reports or Game Adapter dependency |
| P3-GATE-015 | World Component Resource Binding Snapshot Bridge | L5 over L3-L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_015_WORLD_COMPONENT_RESOURCE_BINDING_SNAPSHOT_BRIDGE.md`; deterministic YuSerialize snapshot adapter over component-resource binding records only, caller-owned streams and output records, no automatic active restore, no resource acquire/release on read, no component payload/lifecycle, no `WorldInstance` mutation, no Object/File/Package/render/physics/audio/script/UI/tools/reports or Game Adapter dependency |
| P3-GATE-016 | World Component Resource Binding Restore Bridge | L5 | `APPROVED_FOR_FIRST_SLICE` | Approved for first slice | Gate doc: `docs/gates/P3_GATE_016_WORLD_COMPONENT_RESOURCE_BINDING_RESTORE_BRIDGE.md`; explicit apply adapter from caller-owned component-resource binding snapshot records into active bindings, validation before mutation, resource acquire only through existing binding bridge, no stream parsing, no resource loading/package/file/render/audio/script/UI/tools/reports or Game Adapter dependency |
| P3-GATE-017 | World Scene Assembly Snapshot Restore Coordinator | L5 | `NEEDS_ARCHITECTURE` | Proposed for review amendment | Gate doc: `docs/gates/P3_GATE_017_WORLD_SCENE_ASSEMBLY_SNAPSHOT_RESTORE_COORDINATOR.md`; sidecar-only coordinator over caller-owned component attachment and component-resource binding records, full assembly preflight before attachment or binding mutation, no manifest stream, no object/transform active restore, no scene loading/save policy/object construction/resource loading/render/audio/UI/tools/reports or Game Adapter dependency |

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
- P3-GATE-004 is approved from `4d080f1` for a bounded `YuWorld` lifecycle
  fixture first slice. It may add only `YuWorld`, `YuWorldTests`, and CMake/CTest
  registration. It must prove deterministic lifecycle state transitions, fixed
  update phase order, bounded fixture object registration, no-mutation failures,
  and no hidden allocation in measured update paths. It must not introduce
  Script callbacks, actor/component/transform hierarchy, Resource/Package/File,
  render/audio/physics/UI/tools/reports, Game Adapter behavior, or copied UE or
  Unity API shape.
- P3-GATE-005 is approved from `3b6ee1c` for a narrow World-to-Kernel module
  bridge. It may add only `WorldKernelModule` adapter files under `YuWorld`,
  `Tests/World` coverage, and CMake/CTest registration. `WorldInstance` core
  files must remain Kernel-free; only adapter files may include `YuEngine/Kernel`
  headers. It must not introduce Script callbacks, actor/component/transform
  hierarchy, Resource/Package/File, render/audio/physics/UI/tools/reports, Game
  Adapter behavior, or copied UE or Unity API shape.
- P3-GATE-006 is approved from `86517f8` for a narrow World object identity
  bridge. It may add only `WorldObjectIdentityBridge` adapter files under
  `YuWorld`, `Tests/World` coverage, and CMake/CTest registration. The first
  slice maps bounded `WorldObjectId` values to generation-checked `YuObject`
  `ObjectHandle` values through an adapter; `WorldInstance` core files must
  remain Object-free. It must not introduce Script callbacks,
  actor/component/transform hierarchy, Resource/Package/File, render/audio/
  physics/UI/tools/reports, Game Adapter behavior, or copied UE or Unity API
  shape.
- P3-GATE-007 is approved from `a8ee4d0` for a data-only World transform
  fixture. It may add only `WorldTransformBridge` fixture files under `YuWorld`,
  `Tests/World` coverage, and CMake/CTest registration. The first slice stores
  bounded POD transform records keyed by existing `WorldObjectId` values. It
  must not introduce Script callbacks, actor/component model, scene graph,
  transform hierarchy, Object ownership, Resource/Package/File, serialization
  payloads, render/audio/physics/UI/tools/reports, Game Adapter behavior, or
  copied UE or Unity API shape.
- P3-GATE-008 is approved from `d49969e` for a narrow World-to-Script dispatch
  bridge. It may add only `WorldScriptDispatchBridge` adapter files under
  `YuWorld`, `Tests/World` coverage, and CMake/CTest registration. The first
  slice maps existing `WorldPhaseTrace` records to non-zero `ScriptCallId`
  values using caller-provided slots, with registry lookup failure reported
  during dispatch. `WorldInstance` core files must remain
  Script-free, and `ScriptNativeRegistry` core files must remain World-free. It
  must not introduce VM/bytecode/reflection, actor/component/gameplay,
  Resource/Package/File, Serialize payload, Object ownership, render/audio/
  physics/UI/tools/reports, Game Adapter behavior, or copied UE or Unity API
  shape.
- P3-GATE-009 is approved from `d08b239` for a narrow World-to-Serialize
  snapshot bridge. It may add only `WorldSerializeSnapshotBridge` adapter files
  under `YuWorld`, `Tests/World` coverage, and CMake/CTest registration. The
  first slice writes and reads `WorldSnapshot`, existing `WorldPhaseTrace`
  records, and optional `WorldTransformSnapshot` counters through caller-owned
  `YuSerialize` buffers. `WorldInstance` core files must remain Serialize-free, and
  `YuSerialize` core files must remain World-free. It must not introduce
  File/Package/Resource/save policy, Object construction, reflection, Script,
  actor/component/gameplay, render/audio/physics/UI/tools/reports, Game Adapter
  behavior, or copied UE or Unity API shape.
- P3-GATE-010 is approved from `98e2a12` for a narrow World-to-Resource binding
  bridge. It may add only `WorldResourceBindingBridge` adapter files under
  `YuWorld`, `Tests/World` coverage, and CMake/CTest registration. The first
  slice binds caller-supplied non-zero `WorldObjectId` values to
  already-registered `ResourceHandle` values through a caller-owned
  `ResourceRegistry`, uses fixed-capacity storage, validates stale handles and
  null registries without mutation, and preserves local bindings on release or
  clear failure. It must keep `WorldInstance` core Resource-free and
  `YuResource` core World-free, and it must not introduce File/Package/load/
  decode/upload/render/audio/script/actor/component/gameplay/tools/reports or
  Game Adapter behavior.
- P3-GATE-011 is approved from `92928d1` for a narrow World component
  attachment bridge. It may add only `WorldComponentAttachmentBridge` sidecar
  files under `YuWorld`, `Tests/World` coverage, and CMake/CTest registration.
  The first slice keeps fixed-capacity `(WorldObjectId, WorldComponentTypeId)
  -> WorldComponentSlotId` bookkeeping, with no `WorldInstance` membership
  query and no actor/component behavior lifecycle. It must not introduce
  Object/Resource/Script/Serialize/render/physics/audio/UI/tools/reports or
  Game Adapter dependency creep, copied UE or Unity API shape, reflection,
  scene graph, gameplay behavior, or component payload storage.
- P3-GATE-012 is approved from `9bf52e8` plus ENG-056A/B/C review closure for a
  narrow World component query bridge. It may add only read-only,
  caller-owned output queries over existing `WorldComponentAttachmentBridge`
  records, with no `WorldInstance` membership query, no mutable attachment
  exposure, no actor/component behavior lifecycle, and no
  Object/Resource/Script/Serialize/render/physics/audio/UI/tools/reports or Game
  Adapter dependency creep.
- P3-GATE-013 is approved from `90ddb0b` plus ENG-058A/B/C review closure for a
  deterministic `YuSerialize` snapshot adapter over component attachment
  records, with validation before destination restore mutation, caller-owned
  streams, no component payload/lifecycle, no `WorldInstance` mutation, and no
  Object/Resource/Script/File/Package/render/physics/audio/UI/tools/reports or
  Game Adapter dependency creep.
- P3-GATE-014 is approved from `7dc34d7` plus ENG-060A/B/C review closure for a
  narrow component-resource binding bridge. The first slice binds existing
  `(WorldObjectId, WorldComponentTypeId, WorldComponentSlotId)` attachment
  tuples to already registered `ResourceHandle` values through caller-owned
  component attachment/query data and caller-owned `ResourceRegistry` state. It
  must use const attachment validation, treat `ResourceRegistry::Acquire` as the
  final resource validation/acquire point, clear local POD bindings immediately
  after successful release, and keep resource loading, File/Package lookup,
  component payload/lifecycle, render/audio/physics/script/UI/tools/reports,
  Game Adapter behavior, and `WorldInstance` mutation out of scope.
- P3-GATE-015 is approved from `994e079` plus ENG-062A/B/C review closure for a
  deterministic component-resource binding snapshot adapter. The first slice writes
  active binding records to caller-owned YuSerialize buffers and reads them into
  caller-owned POD output records. It deliberately excludes automatic restore
  into active bindings and excludes resource acquire/release on read, keeping
  runtime rebinding as an explicit caller action through the already approved
  component-resource binding bridge.
- P3-GATE-016 is approved from `4994518` plus ENG-064A2/B/C review closure for
  an explicit component-resource binding restore adapter. The first slice keeps
  snapshot read separate from active restore: callers pass already decoded POD
  binding records, caller-owned attachment/resource state, and a destination
  binding bridge. The implementation must add the minimal const
  `ResourceRegistry` validation helper if the current registry surface cannot
  prove validation-before-mutation without acquire/release side effects. That
  helper must not add a `YuWorld` dependency to `YuResource`.
- P3-GATE-016 first slice landed at `5c31c7c` with QA PASS and fast gate
  `450/450`.
- P3-GATE-017 is proposed from `5c31c7c` for a narrow world scene-assembly
  snapshot/restore coordinator. ENG-067B/C/D review returned
  `NEEDS_ARCHITECTURE_AMENDMENT` / `NEEDS_TEST_AMENDMENT`, so the gate remains
  review-only. The amended scope is sidecar-only: caller-owned component
  attachment records plus caller-owned component-resource binding records, with
  full assembly preflight before any active mutation. It does not authorize
  manifest streams, object identity restore, transform active restore, scene
  loading, save policy, object construction, resource loading, component
  payload/lifecycle, render/audio/physics/UI/tools/reports, or Game Adapter
  behavior.
- No Phase 3 implementation task may be created until the owning gate is
  approved and PM confirms sequencing against active Phase 1 and Phase 2 review
  queues.

## Implementation Baseline

Current `HEAD` contains first-slice modules for `YuObject`, `YuSerialize`,
`YuScript`, and `YuWorld`.

Recently landed Phase 3 world/script slices:

- `4d080f1 [#ENG-039][Added]Add script bridge first slice`
- `3b6ee1c [#ENG-040][Added]Add world lifecycle first slice`
- `86517f8 [#ENG-041][Added]Add world kernel module bridge`
- `a8ee4d0 [#ENG-043][Added]Add world object identity bridge`
- `a2eb08b [#ENG-045][Added]Add world transform bridge`
- `9491abe [#ENG-048][Added]Add world script dispatch bridge`
- `8b5dfdf [#ENG-049][Added]Add world serialize snapshot bridge`
- `fa59550 [#ENG-052][Added]Add world resource binding bridge`
- `9bf52e8 [#ENG-055][Added]Add world component attachment bridge`
- `90ddb0b [#ENG-057][Added]Add world component query bridge`
- `7dc34d7 [#ENG-059][Added]Add world component attachment snapshot bridge`
- `994e079 [#ENG-061][Added]Add world component resource binding bridge`
- `4994518 [#ENG-063][Added]Add world component resource binding snapshot bridge`
- `5c31c7c [Added] Add world component resource binding restore bridge`

Future Phase 3 work must extend from this baseline instead of recreating the
same first slices.

## Review Routing

1. Architecture posts ADR/gate proposal in #架构决策.
2. Engine-reference review checks mature-engine responsibility separation.
3. Performance review checks object lifetime cost, handle lookup cost, and
   allocation/accounting boundaries.
4. Implementability review checks public surface and required tests.
5. PM/gate review issues final gate state.
6. Code review starts only after implementation exists.

## Immediate Next Steps

1. Re-review amended P3-GATE-017 before any implementation task is created. The
   required decision point is whether sidecar-only assembly can prove full
   preflight before component attachment or component-resource binding mutation.
2. Keep P3-GATE-017 limited to caller-owned component attachment records,
   caller-owned component-resource binding records, and existing bridge
   coordination. Do not use it to introduce manifest streams, object/transform
   active restore, scene loading, save policy, object construction, resource
   loading, component payload/lifecycle, render/audio/physics/UI/tools/reports,
   or Game Adapter behavior.
3. Continue closing active Phase 1 and Phase 2 implementation reviews; current
   package review closure does not authorize package expansion or P3 dependency
   creep.
4. Do not use P3-GATE-002 or P3-GATE-003 to introduce File/package/Resource/object
   construction/reflection/script/scene/save/tool/report/Game Adapter scope.
