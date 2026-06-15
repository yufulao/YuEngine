# P3-GATE-010: World Resource Binding Bridge

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `NEEDS_ARCHITECTURE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P1-GATE-006, P3-GATE-004
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0009
Source baseline: `8b5dfdf`

## Layer

L5 world runtime adapter over the L4 Resource identity boundary.

This gate proposes a narrow adapter that binds existing `WorldObjectId` values
to existing `ResourceHandle` values through a caller-owned `ResourceRegistry`.
It is not a file lookup system, package resolver, asset loader, decoder, upload
scheduler, render scene, audio scene, gameplay component model, or Game Adapter
feature.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\AssetManager.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Public\StreamableManager.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`

Responsibility conclusion:

- UE separates world ownership, actor/component attachment, asset identity,
  streaming, loading, and render/audio upload. YuEngine must keep those
  responsibilities separate and must not copy UE public API shape, async load
  vocabulary, actor/component lifecycle, soft-object path behavior, or source
  code.
- YuEngine first slice should only prove deterministic binding between world
  object IDs and already-registered resource handles.
- Loading, decoding, dependency scheduling, streaming, package lookup, and upload
  remain future gates.

Unity responsibility reference:

- Unity separates Scene/GameObject ownership, asset references, asset database or
  addressable lookup, runtime loading, component behavior, and renderer/audio
  consumption.
- YuEngine must not copy Unity scene, component, addressable, or asset API shape.
  This gate only defines a fixed binding adapter over existing Resource handles.

## Owns

This gate owns a future first slice for:

- a `WorldResourceBindingBridge` adapter;
- explicit world-resource binding statuses;
- bounded binding slots keyed by `WorldObjectId`;
- each binding storing one `ResourceHandle` and expected `ResourceTypeId`;
- binding a world object ID to an already-registered resource handle;
- acquiring the resource handle during successful bind;
- releasing the resource handle during successful remove or clear;
- rejecting duplicate world object IDs without mutation;
- rejecting invalid handles, stale handles, wrong resource type, and capacity
  overflow without mutation;
- exposing a POD snapshot with binding, acquire, release, clear, and failure
  counters;
- no hidden allocation in measured bind/remove/clear/query paths;
- tests proving `WorldInstance` core files remain Resource-free and
  `YuResource` core files remain World-free.

## Does Not Own

This gate does not own:

- File IO, VFS, package manifests, package entry resolution, resource discovery,
  resource loading, decoding, async loading, streaming, upload, cache eviction,
  hot reload, or original-game resource lookup;
- object construction, object ownership, object handle persistence, reflection,
  properties, generated code, or schema compilation;
- actor/component/gameplay ownership, scene graph hierarchy, render scene,
  audio scene, physics, animation, UI, script VM, script service state,
  save/load policy, tools, reports, profiling UI, or Game Adapter behavior;
- copied UE or Unity API shape, source code, asset path vocabulary, async-load
  API shape, component lifecycle, or addressable API shape.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuWorld` public `WorldObjectId` and world status/value vocabulary;
- `YuResource` public `ResourceRegistry`, `ResourceHandle`,
  `ResourceTypeId`, `ResourceStatus`, and snapshot value types.

Forbidden dependencies:

- `YuFile`, `YuPackage`, `YuScript`, `YuSerialize`, `YuThread`,
  `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools,
  reports, or game adapter modules;
- `YuObject` construction, ownership, reference counting, handle lookup, or
  registry mutation;
- old runtime files, original-game resources, original-game package names, or
  original-game data as API shape.

The first implementation may add `YuResource` as a declared `YuWorld`
dependency only for `WorldResourceBindingBridge` adapter files. Tests must prove
`WorldInstance.h` and `WorldInstance.cpp` do not include `YuEngine/Resource`,
and `Src/YuEngine/Resource/**` does not include `YuEngine/World`.

## Lifecycle

First-slice lifecycle:

1. Caller owns and initializes a bounded `ResourceRegistry`.
2. Caller registers synthetic resource descriptors in that registry.
3. Caller creates a `WorldResourceBindingBridge` with fixed binding capacity.
4. Caller binds a non-zero `WorldObjectId` to a valid `ResourceHandle` with an
   expected `ResourceTypeId`.
5. The bridge acquires the resource handle from the caller-owned registry only
   after all local validation succeeds.
6. The bridge stores the binding only after the resource acquire succeeds.
7. Caller may query the binding snapshot by `WorldObjectId`.
8. Caller may remove one binding; the bridge releases the acquired handle and
   clears the binding only when release succeeds.
9. Caller may clear all bindings; the bridge releases each acquired handle in
   deterministic slot order.
10. The bridge never constructs, starts, stops, updates, mutates, or owns a
    `WorldInstance`.

Failure behavior:

- null resource registry pointers return explicit status and do not mutate bridge
  binding slots;
- invalid world object IDs return explicit status and do not acquire resources;
- invalid resource handles, stale handles, and wrong resource types return
  explicit status and do not mutate binding slots;
- duplicate world object IDs return explicit status and do not acquire a second
  resource reference;
- binding capacity overflow returns explicit status and does not acquire a
  resource reference;
- remove of a missing world object ID returns explicit status and does not call
  `ResourceRegistry::Release`;
- release failure returns explicit status and does not clear the local binding;
- clear failure records explicit status and stops before silently dropping
  unreleased bindings;
- failed bridge operations do not mutate `WorldInstance` or `YuResource` type
  definitions.

## Inputs

- caller-owned `ResourceRegistry`;
- `WorldObjectId`;
- `ResourceHandle`;
- expected `ResourceTypeId`;
- explicit bridge capacity.

## Outputs

- explicit world-resource binding statuses;
- `ResourceStatus` from the most recent registry operation;
- POD binding snapshot values;
- bridge snapshot counters;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No file path, package key, resource path string, original-game resource name,
report text, log text, wall-clock time, pointer value, rendered frame data, or
audio output may define behavior.

## Performance Constraints

Budget classes:

- setup path for bridge construction;
- load/setup path for bind/remove/clear;
- frame path only for read-only binding queries.

First-slice bounds:

- binding storage is fixed capacity and does not grow;
- world object ID lookup is bounded by the configured binding capacity;
- bind/remove/clear use explicit `ResourceRegistry` acquire/release calls and
  do not allocate;
- resource lookup remains handle/generation based, not string/path based;
- diagnostics or logs are not required for behavior.

Required deterministic signals:

- binding capacity;
- active binding count;
- acquired binding count;
- released binding count;
- cleared binding count;
- failed operation count;
- last bridge status;
- last resource status;
- allocation/accounting status.

Blocking conditions:

- File IO, package lookup, resource path lookup, decoder, uploader, renderer,
  audio, script, actor/component, gameplay, save, or original-game data required
  for behavior;
- string lookup, dynamic maps, container growth, or global mutable caches in
  measured paths;
- modifying `WorldInstance` to include or own `YuResource`;
- modifying `YuResource` core to include or own `YuWorld`;
- copying UE or Unity public API shape into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldResourceBindingBridge_BindValidResource_AcquiresHandle`
- `WorldResourceBindingBridge_BindRejectsInvalidWorldIdWithoutMutation`
- `WorldResourceBindingBridge_BindRejectsInvalidResourceHandleWithoutMutation`
- `WorldResourceBindingBridge_BindRejectsTypeMismatchWithoutMutation`
- `WorldResourceBindingBridge_BindRejectsDuplicateWorldObjectId`
- `WorldResourceBindingBridge_BindRejectsCapacityOverflowWithoutMutation`
- `WorldResourceBindingBridge_RemoveReleasesHandle`
- `WorldResourceBindingBridge_RemoveRejectsMissingWorldObjectWithoutMutation`
- `WorldResourceBindingBridge_ClearReleasesAllHandles`
- `WorldResourceBindingBridge_BoundResourceCannotRetireUntilReleased`
- `WorldResourceBindingBridge_QueryReturnsStoredBinding`
- `WorldResourceBindingBridge_UpdatePathDoesNotGrowWorldStorage`
- `WorldResourceBindingBridge_SnapshotReportsCountsAndLastStatus`
- `WorldResourceBindingBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency`
- `WorldResourceBindingBridge_WorldInstanceCoreRemainsResourceFree`
- `WorldResourceBindingBridge_ResourceCoreRemainsWorldFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

## Allowed First Slice

If this gate is updated to `APPROVED_FOR_FIRST_SLICE`, the first implementation
slice may create:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldResourceBinding*.h
Src/YuEngine/World/Src/WorldResourceBinding*.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

It may update `YuWorld` CMake linkage to reference `YuResource` only for the
adapter target boundary.

It may not modify `WorldInstance` core ownership, `YuResource` core ownership,
File, Package, Script, Serialize, Object, RHI, Audio, Input, UI, tools, reports,
or Game Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine responsibility separation and performance
  constraints before approval.
- 博丽灵梦 should review fixed-capacity storage, acquire/release atomicity, and
  no-mutation failure requirements.
- 雾雨魔理沙 should review implementability and required test names before any
  implementation task is created.
