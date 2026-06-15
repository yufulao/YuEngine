# P3-GATE-011: World Component Attachment Bridge

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-004, P3-GATE-006, P3-GATE-007
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0014
Source baseline: `fa59550`

## Layer

L5 world runtime fixture.

This gate proposes the first narrow YuWorld component attachment boundary. The
slice introduces a bounded sidecar registry that attaches caller-supplied
component type IDs and component slot IDs to caller-supplied `WorldObjectId`
values. It is not an actor model, component behavior system, reflection system,
scene graph, transform hierarchy, render scene, physics scene, script binding,
resource lookup, package loader, editor feature, or Game Adapter feature.

The first slice must keep `WorldInstance` ownership unchanged. It may validate
the `WorldObjectId` value contract and store attachment records, but it must not
query or mutate `WorldInstance` membership and must not require `YuObject`,
`YuResource`, `YuScript`, or `YuSerialize`.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`

Responsibility conclusion:

- UE actors own component collections, component registration, ticking,
  replication, render/physics state hooks, construction, and editor/runtime
  lifecycle. YuEngine must not copy that API shape or source code in a first
  slice.
- The useful boundary lesson is that world object identity and component
  attachment must be distinct from render, physics, script, resource loading,
  networking, editor, and gameplay behavior.
- YuEngine first slice should prove only deterministic, fixed-capacity
  component attachment bookkeeping. Component behavior, registration callbacks,
  tick dependencies, scene components, and render/physics state remain future
  gates.

Unity responsibility reference:

- Unity separates Scene, GameObject identity, Component attachment, component
  behavior, asset references, script execution, rendering, physics, and editor
  serialization.
- YuEngine must not copy Unity GameObject or Component API shape. This gate only
  introduces a bounded attachment table over YuEngine `WorldObjectId` values.

## Owns

This gate owns a future first slice for:

- a `WorldComponentAttachmentBridge` fixture;
- explicit world component attachment statuses;
- POD `WorldComponentTypeId`, `WorldComponentSlotId`, and attachment records;
- fixed-capacity attachment storage;
- adding one component slot for a non-zero `WorldObjectId` and non-zero
  `WorldComponentTypeId`;
- rejecting duplicate `(WorldObjectId, WorldComponentTypeId)` attachments
  without mutation;
- rejecting invalid world IDs, invalid component type IDs, invalid component
  slot IDs, and capacity overflow without mutation;
- querying a single attachment by `(WorldObjectId, WorldComponentTypeId)`;
- removing one attachment without changing unrelated attachments;
- clearing all attachments in deterministic slot order;
- exposing a POD snapshot with capacity, active count, add, remove, clear,
  query, duplicate, and failure counters;
- tests proving fixed capacity, no hidden allocation, no `WorldInstance`
  membership query, no actor/component behavior creep, and no forbidden module
  dependencies.

## Does Not Own

This gate does not own:

- UE-style `Actor`, `ActorComponent`, `SceneComponent`, `PrimitiveComponent`,
  construction script, replication, networking, editor selection, registration
  callbacks, tick prerequisite graph, render state, physics state, or
  Blueprint-style behavior;
- Unity-style GameObject or Component API shape, MonoBehaviour lifecycle,
  serialized scene object model, editor component inspector, or asset database
  integration;
- component payload storage, reflection, properties, generated code, schema
  compilation, script object lifetime, behavior dispatch, event routing,
  gameplay services, animation, physics, render scene, audio scene, UI,
  save/load, scene streaming, tools, reports, profiling UI, or Game Adapter
  behavior;
- object construction or `YuObject` handle ownership;
- resource handles, package lookup, file paths, resource loading, decoding,
  upload, cache policy, or hot reload;
- copied UE or Unity public API shape, source code, macro style, reflection
  vocabulary, or lifecycle naming.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuWorld` public `WorldObjectId`, status, snapshot, and value vocabulary.

Forbidden dependencies:

- `YuObject`, `YuResource`, `YuScript`, `YuSerialize`, `YuFile`, `YuPackage`,
  `YuThread`, `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`,
  UI, tools, reports, or game adapter modules;
- old runtime files, original-game component names, original-game actor names,
  original-game resource paths, or original-game data as API shape.

The first implementation must not add new inter-module target dependencies. It
may add only `WorldComponentAttachment*` files under `YuWorld`, `Tests/World`
coverage, and CMake/CTest registration for those files.

## Lifecycle

First-slice lifecycle:

1. Caller creates a `WorldComponentAttachmentBridge` with fixed attachment
   capacity.
2. Caller supplies non-zero `WorldObjectId`, `WorldComponentTypeId`, and
   `WorldComponentSlotId` values.
3. The bridge validates only value contracts and local duplicate/capacity rules.
4. The bridge stores one POD attachment record on success.
5. Caller may query the attachment by world object ID and component type ID.
6. Caller may remove one attachment.
7. Caller may clear all attachments in deterministic slot order.
8. The bridge never constructs, starts, stops, updates, queries, mutates, or
   owns `WorldInstance`, `YuObject`, `YuResource`, `YuScript`, `YuSerialize`,
   render, physics, audio, UI, or Game Adapter state.

Failure behavior:

- invalid world object IDs return explicit status and do not mutate storage;
- invalid component type IDs return explicit status and do not mutate storage;
- invalid component slot IDs return explicit status and do not mutate storage;
- duplicate `(WorldObjectId, WorldComponentTypeId)` returns explicit status and
  does not replace the existing attachment;
- capacity overflow returns explicit status and does not mutate existing
  attachments;
- query of a missing attachment returns explicit status and does not mutate
  attachment storage;
- remove of a missing attachment returns explicit status and does not mutate
  unrelated attachments;
- clear succeeds deterministically and leaves active count zero;
- failed operations do not mutate `WorldInstance`, other modules, or type
  definitions.

## Inputs

- explicit attachment capacity;
- `WorldObjectId`;
- `WorldComponentTypeId`;
- `WorldComponentSlotId`.

## Outputs

- explicit world component attachment statuses;
- POD attachment snapshot values;
- bridge snapshot counters;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No component class name, reflection name, file path, package key, resource path,
script call ID, render handle, physics handle, original-game name, report text,
log text, wall-clock time, pointer value, rendered frame data, or audio output
may define behavior.

## Performance Constraints

Budget classes:

- setup path for bridge construction;
- load/setup path for add/remove/clear;
- frame path only for read-only attachment queries.

First-slice bounds:

- attachment storage is fixed capacity and does not grow;
- lookup is bounded by the configured attachment capacity;
- add/remove/clear/query do not allocate;
- no dynamic maps, string keys, global mutable caches, or reflection lookup in
  measured paths;
- diagnostics or logs are not required for behavior.

Required deterministic signals:

- attachment capacity;
- active attachment count;
- added attachment count;
- removed attachment count;
- cleared attachment count;
- query count;
- duplicate rejection count;
- failed operation count;
- last attachment status;
- allocation/accounting status.

Blocking conditions:

- requiring component payload storage, script behavior, resource loading,
  object construction, render scene, physics scene, audio scene, UI, save/load,
  scene streaming, tools, reports, gameplay, or original-game data for behavior;
- string lookup, dynamic maps, container growth, or global mutable caches in
  measured paths;
- modifying `WorldInstance` core to include or own component attachment storage;
- adding dependencies from `YuWorld` core files to `YuObject`, `YuResource`,
  `YuScript`, `YuSerialize`, render, physics, audio, UI, tools, or Game Adapter;
- copying UE or Unity public API shape or source code into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldComponentAttachmentBridge_AddValidAttachment_StoresRecord`
- `WorldComponentAttachmentBridge_AddRejectsInvalidWorldIdWithoutMutation`
- `WorldComponentAttachmentBridge_AddRejectsInvalidComponentTypeWithoutMutation`
- `WorldComponentAttachmentBridge_AddRejectsInvalidComponentSlotWithoutMutation`
- `WorldComponentAttachmentBridge_AddRejectsDuplicateTypeForWorldObject`
- `WorldComponentAttachmentBridge_AddRejectsCapacityOverflowWithoutMutation`
- `WorldComponentAttachmentBridge_QueryReturnsStoredAttachment`
- `WorldComponentAttachmentBridge_QueryRejectsMissingAttachmentWithoutMutation`
- `WorldComponentAttachmentBridge_QueryIsReadOnlyAndBounded`
- `WorldComponentAttachmentBridge_RemoveClearsAttachment`
- `WorldComponentAttachmentBridge_RemoveRejectsMissingAttachmentWithoutMutation`
- `WorldComponentAttachmentBridge_ClearRemovesAllAttachmentsInSlotOrder`
- `WorldComponentAttachmentBridge_UpdatePathDoesNotGrowStorage`
- `WorldComponentAttachmentBridge_SnapshotReportsCountsAndLastStatus`
- `WorldComponentAttachmentBridge_DoesNotQueryOrMutateWorldInstance`
- `WorldComponentAttachmentBridge_NoActorComponentBehaviorOrLifecycle`
- `WorldComponentAttachmentBridge_NoObjectResourceScriptSerializeOrGameAdapterDependency`
- `WorldComponentAttachmentBridge_NoFilePackageThreadPlatformDiagnosticsDependency`
- `WorldComponentAttachmentBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency`
- `WorldComponentAttachmentBridge_WorldInstanceCoreRemainsAttachmentFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

## Allowed First Slice

With this gate approved for first slice, the first implementation slice may
create:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachment*.h
Src/YuEngine/World/Src/WorldComponentAttachment*.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

It may not modify `WorldInstance` core ownership, `YuObject`, `YuResource`,
`YuScript`, `YuSerialize`, File, Package, RHI, Audio, Input, UI, tools, reports,
or Game Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine component responsibility separation,
  performance constraints, and AGENTS compliance before approval.
- 博丽灵梦 should review implementability of the fixed-capacity storage, status
  vocabulary, and no-mutation failure requirements.
- 雾雨魔理沙 should review test completeness and CMake/CTest registration
  boundaries before any implementation handoff.
