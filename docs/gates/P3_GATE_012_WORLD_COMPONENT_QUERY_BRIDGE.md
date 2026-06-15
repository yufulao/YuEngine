# P3-GATE-012: World Component Query Bridge

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-004, P3-GATE-006, P3-GATE-011
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0014
Source baseline: `9bf52e8`

## Layer

L5 world runtime query fixture.

This gate proposes the first narrow YuWorld component query boundary. The slice
turns the fixed-capacity component attachment sidecar into deterministic
caller-owned query outputs. It is not an actor model, entity-component-system
runtime, component payload store, behavior lifecycle, script callback,
reflection system, scene graph, render scene, physics scene, editor query,
resource lookup, package loader, report adapter, or Game Adapter feature.

The first slice must keep `WorldInstance` ownership unchanged. It may read
validated attachment records through a const query/export path, but it must not
query or mutate `WorldInstance` membership and must not require `YuObject`,
`YuResource`, `YuScript`, `YuSerialize`, render, physics, audio, UI, tools, or
Game Adapter modules.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`

Responsibility conclusion:

- UE supports broad actor/component ownership, component iteration, ticking,
  replication, render and physics registration, editor visibility, and gameplay
  behavior. YuEngine must not copy that API shape or source code in this slice.
- The useful boundary lesson is that component discovery must be distinct from
  component behavior, scene graph ownership, render/physics state, script
  execution, resource loading, and editor tooling.
- YuEngine first slice should prove only deterministic, read-only filtering over
  the existing component attachment sidecar.

Unity responsibility reference:

- Unity separates scene object identity, Component attachment, query/use by
  systems, MonoBehaviour lifecycle, asset references, serialization, rendering,
  physics, and editor inspection.
- YuEngine must not copy Unity GameObject or Component API shape. This gate only
  introduces a bounded query contract over YuEngine attachment records.

## Owns

This gate owns a future first slice for:

- a `WorldComponentQueryBridge` fixture;
- explicit world component query statuses;
- POD query descriptors, query results, query records, and query snapshots;
- caller-owned output buffers for matching `WorldObjectId` values or full
  `WorldComponentAttachment` records;
- querying all attachments for one non-zero `WorldComponentTypeId`;
- querying all component attachments for one non-zero `WorldObjectId`;
- rejecting null source bridge pointers without mutation;
- rejecting null output buffers when output capacity is non-zero;
- rejecting invalid component type IDs and world object IDs without mutation;
- rejecting undersized caller-owned output buffers without overrun;
- returning deterministic partial counts only through explicit overflow status;
- exposing counters for query count, matched record count, overflow rejection
  count, failed operation count, and last status;
- tests proving read-only behavior, fixed capacity, no hidden allocation, no
  `WorldInstance` membership query, no actor/component behavior creep, and no
  forbidden module dependencies.

The implementation may add a minimal const export/query method to
`WorldComponentAttachmentBridge` if needed. That method must expose only POD
records through caller-owned buffers and must not expose mutable storage or add
new ownership semantics.

## Does Not Own

This gate does not own:

- UE-style `Actor`, `ActorComponent`, `SceneComponent`, component registration,
  component ticking, replication, construction scripts, editor selection,
  render state, physics state, or Blueprint-style behavior;
- Unity-style GameObject or Component API shape, MonoBehaviour lifecycle,
  `GetComponent`/`FindObjects` API shape, serialized scene object model, editor
  inspector, or asset database integration;
- component payload storage, component factories, generated component schemas,
  reflection, property bags, script object lifetime, behavior dispatch, event
  routing, gameplay services, animation, physics, render scene, audio scene,
  UI, save/load, scene streaming, tools, reports, profiling UI, or Game Adapter
  behavior;
- object construction or `YuObject` handle ownership;
- resource handles, package lookup, file paths, resource loading, decoding,
  upload, cache policy, hot reload, or direct renderer/physics submission;
- copied UE or Unity public API shape, source code, macro style, reflection
  vocabulary, lifecycle naming, or editor query vocabulary.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuWorld` public `WorldObjectId`, `WorldComponentAttachment`, status,
  snapshot, and value vocabulary.

Forbidden dependencies:

- `YuObject`, `YuResource`, `YuScript`, `YuSerialize`, `YuFile`, `YuPackage`,
  `YuThread`, `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`,
  UI, tools, reports, or game adapter modules;
- old runtime files, original-game component names, original-game actor names,
  original-game resource paths, or original-game data as API shape.

The first implementation must not add new inter-module target dependencies. It
may add only `WorldComponentQuery*` files under `YuWorld`, minimal const
attachment export updates under existing `WorldComponentAttachment*` files,
`Tests/World` coverage, and CMake/CTest registration for those files.

## Lifecycle

First-slice lifecycle:

1. Caller creates or uses a `WorldComponentQueryBridge` with no dynamic query
   storage.
2. Caller passes a const `WorldComponentAttachmentBridge` source.
3. Caller supplies a non-zero component type ID or non-zero world object ID.
4. Caller supplies caller-owned output records and output capacity.
5. The bridge validates source pointer, input IDs, output pointer, and output
   capacity.
6. The bridge copies matching POD records in deterministic attachment slot
   order.
7. The bridge writes explicit result counts and status to caller-owned output.
8. The bridge never constructs, starts, stops, updates, queries, mutates, or
   owns `WorldInstance`, `YuObject`, `YuResource`, `YuScript`, `YuSerialize`,
   render, physics, audio, UI, or Game Adapter state.

Failure behavior:

- null source bridge returns explicit status and does not write output records;
- invalid world object IDs return explicit status and do not write output
  records;
- invalid component type IDs return explicit status and do not write output
  records;
- null output buffer with non-zero output capacity returns explicit status and
  does not write counts;
- output capacity overflow returns explicit status without writing past caller
  capacity;
- query of a missing component type or world object returns success with zero
  matches;
- failed operations do not mutate attachment storage, `WorldInstance`, other
  modules, or type definitions.

## Inputs

- const component attachment bridge source;
- `WorldObjectId`;
- `WorldComponentTypeId`;
- caller-owned output record pointer;
- output record capacity.

## Outputs

- explicit world component query statuses;
- POD query result values;
- copied `WorldComponentAttachment` records or `WorldObjectId` values;
- bridge snapshot counters;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No component class name, reflection name, file path, package key, resource path,
script call ID, render handle, physics handle, original-game name, report text,
log text, wall-clock time, pointer value, rendered frame data, or audio output
may define behavior.

## Performance Constraints

Budget classes:

- setup path for query bridge construction;
- frame path for read-only component queries.

First-slice bounds:

- query state is fixed size and does not grow;
- query scans are bounded by configured attachment capacity;
- query calls do not allocate;
- no dynamic maps, string keys, global mutable caches, reflection lookup, or
  hidden component registries in measured paths;
- diagnostics or logs are not required for behavior.

Required deterministic signals:

- query count;
- matched record count;
- overflow rejection count;
- failed operation count;
- last query status;
- allocation/accounting status.

Blocking conditions:

- requiring component payload storage, script behavior, resource loading,
  object construction, render scene, physics scene, audio scene, UI, save/load,
  scene streaming, tools, reports, gameplay, or original-game data for behavior;
- string lookup, dynamic maps, container growth, or global mutable caches in
  measured paths;
- exposing mutable attachment storage from `WorldComponentAttachmentBridge`;
- modifying `WorldInstance` core to include query or component attachment
  storage;
- adding dependencies from `YuWorld` core files to `YuObject`, `YuResource`,
  `YuScript`, `YuSerialize`, render, physics, audio, UI, tools, or Game Adapter;
- copying UE or Unity public API shape or source code into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldComponentQueryBridge_QueryTypeReturnsMatchingWorldObjectsInSlotOrder`
- `WorldComponentQueryBridge_QueryTypeReturnsZeroForMissingType`
- `WorldComponentQueryBridge_QueryObjectReturnsMatchingAttachmentsInSlotOrder`
- `WorldComponentQueryBridge_QueryObjectReturnsZeroForMissingObject`
- `WorldComponentQueryBridge_QueryRejectsNullSourceWithoutMutation`
- `WorldComponentQueryBridge_QueryRejectsInvalidComponentTypeWithoutMutation`
- `WorldComponentQueryBridge_QueryRejectsInvalidWorldIdWithoutMutation`
- `WorldComponentQueryBridge_QueryRejectsNullOutputWhenCapacityNonZero`
- `WorldComponentQueryBridge_QueryRejectsOutputOverflowWithoutOverrun`
- `WorldComponentQueryBridge_QueryIsReadOnlyForAttachmentStorage`
- `WorldComponentQueryBridge_QueryPathDoesNotGrowStorage`
- `WorldComponentQueryBridge_SnapshotReportsCountsAndLastStatus`
- `WorldComponentQueryBridge_NoActorComponentBehaviorOrLifecycle`
- `WorldComponentQueryBridge_NoObjectResourceScriptSerializeOrGameAdapterDependency`
- `WorldComponentQueryBridge_NoFilePackageThreadPlatformDiagnosticsDependency`
- `WorldComponentQueryBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency`
- `WorldComponentQueryBridge_WorldInstanceCoreRemainsQueryFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

## Allowed First Slice

If this gate is approved for first slice, the first implementation slice may
create or update only:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldComponentQuery*.h
Src/YuEngine/World/Src/WorldComponentQuery*.cpp
Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachment*.h
Src/YuEngine/World/Src/WorldComponentAttachment*.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

It may not modify `WorldInstance` core ownership, `YuObject`, `YuResource`,
`YuScript`, `YuSerialize`, File, Package, RHI, Audio, Input, UI, tools, reports,
or Game Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine query responsibility separation,
  performance constraints, and AGENTS compliance before approval.
- 博丽灵梦 should review implementability of the const attachment export,
  query result vocabulary, and no-mutation failure requirements.
- 雾雨魔理沙 should review test completeness and CMake/CTest registration
  boundaries before any implementation handoff.
