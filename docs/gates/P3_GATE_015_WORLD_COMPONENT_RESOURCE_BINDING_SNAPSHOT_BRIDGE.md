# P3-GATE-015: World Component Resource Binding Snapshot Bridge

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-002, P3-GATE-010, P3-GATE-011, P3-GATE-012, P3-GATE-014
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0009, ADR-0014, ADR-0015
Source baseline: `994e079`

## Layer

L5 world runtime snapshot adapter over component-resource binding records.

This gate proposes a narrow YuWorld snapshot boundary for deterministic
`WorldComponentResourceBinding` records. The first slice writes active
component-resource binding records to caller-owned YuSerialize buffers and reads
them back into caller-owned POD output records. It is not a resource restore
system, resource loader, package resolver, component payload store, component
behavior system, save-game policy, editor inspector, report adapter, or Game
Adapter feature.

The slice must keep `WorldInstance` ownership unchanged. It may read binding
records through a const export path on `WorldComponentResourceBindingBridge`.
It must not acquire or release resources while reading serialized data. Runtime
rebinding remains an explicit caller action through the already approved
`WorldComponentResourceBindingBridge::Bind` path, which owns acquire semantics.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\AssetManager.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Public\StreamableManager.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Private\World.cpp`

Responsibility conclusion:

- UE may serialize component asset references, object ownership, construction
  state, registration state, streaming handles, render state, physics state,
  replication, and editor metadata. YuEngine must not copy that public API shape
  or source code in this slice.
- The useful boundary lesson is that serialized component-resource reference
  records, runtime resource acquisition, loading policy, package lookup, upload
  policy, and component behavior lifetime must remain separate.
- YuEngine first slice should prove only deterministic serialization of the
  already bounded component-resource binding sidecar records.

Unity responsibility reference:

- Unity separates serialized component asset references, runtime asset loading,
  component lifecycle, scene object ownership, renderer/audio consumption, and
  editor inspection.
- YuEngine must not copy Unity GameObject, Component, serialized object,
  Addressables, asset database, or inspector API shape. This gate only introduces
  a bounded snapshot contract over YuEngine component-resource binding records.

## Owns

This gate owns a future first slice for:

- a `WorldComponentResourceBindingSnapshotBridge` adapter;
- explicit component-resource binding snapshot statuses;
- POD snapshot descriptors, records, results, and counters;
- writing active `WorldComponentResourceBinding` records from a const
  `WorldComponentResourceBindingBridge` source to caller-owned YuSerialize
  buffers;
- reading serialized binding records from caller-owned YuSerialize buffers into
  caller-owned output arrays;
- deterministic binding slot order preservation;
- validating source bridge pointers, reader pointers, writer pointers, output
  pointers, record counts, output capacities, world object IDs, component type
  IDs, component slot IDs, resource handles, resource type IDs, duplicate
  binding tuples, and serialized enum/status values before writing outputs;
- rejecting malformed or unsupported stream data without partial output
  mutation;
- explicit counters for write count, read count, written record count, read
  record count, rejected record count, failed operation count, and last status;
- allocation/accounting signal using accepted `YuMemory` vocabulary;
- tests proving deterministic round trip, no hidden allocation, no resource
  acquire/release on read, no component payload or lifecycle creep, no
  `WorldInstance` mutation, and no forbidden module dependencies.

The implementation may add a minimal const export helper to
`WorldComponentResourceBindingBridge` if needed. It must not expose mutable
binding storage, add new ownership semantics, or add validation APIs to
`YuResource` in this first slice.

## Does Not Own

This gate does not own:

- UE-style `Actor`, `ActorComponent`, `SceneComponent`, asset manager,
  streamable manager, component registration, ticking, replication, render
  state, physics state, editor metadata, or asset streaming APIs;
- Unity-style GameObject or Component API shape, MonoBehaviour lifecycle,
  prefab serialization, serialized object model, addressables, asset database,
  renderer components, audio components, or editor inspector behavior;
- automatic restore into an active component-resource binding bridge;
- resource acquire/release during snapshot read;
- resource loading, decoding, upload, package lookup, file paths, resource path
  strings, cache eviction, hot reload, or original-game resource lookup;
- component payload serialization, component factories, generated schemas,
  reflection, property bags, script object lifetime, behavior dispatch, event
  routing, gameplay services, animation, physics, render scene, audio scene, UI,
  save slots, scene streaming, tools, reports, profiling UI, or Game Adapter
  behavior;
- object construction or `YuObject` handle ownership;
- copied UE or Unity public API shape, source code, macro style, reflection
  vocabulary, lifecycle naming, asset path vocabulary, async-load vocabulary, or
  editor query vocabulary.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuSerialize` public value stream vocabulary;
- `YuWorld` public `WorldObjectId`, `WorldComponentAttachment`,
  `WorldComponentResourceBinding`, component-resource binding status, snapshot,
  and value vocabulary;
- `YuResource` public value types only: `ResourceHandle`, `ResourceTypeId`, and
  `ResourceStatus` if a result needs to carry prior resource status values.

Forbidden dependencies:

- `YuObject`, `YuScript`, `YuFile`, `YuPackage`, `YuThread`, `YuPlatform`,
  `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools, reports, or game
  adapter modules;
- `ResourceRegistry` mutation on snapshot read;
- resource path strings, package keys, file paths, original-game resource names,
  original-game component names, or original-game data as API shape.

The first implementation may add only `WorldComponentResourceBindingSnapshot*`
files under `YuWorld`, minimal const export support on
`WorldComponentResourceBindingBridge` if required, `Tests/World` coverage, and
CMake/CTest registration. It must not modify `WorldInstance` core ownership or
`YuResource` core ownership.

## Lifecycle

First-slice write lifecycle:

1. Caller owns a populated `WorldComponentResourceBindingBridge`.
2. Caller owns a bounded YuSerialize writer buffer.
3. Caller calls the snapshot bridge with a const source bridge and writer.
4. The adapter validates the source and writer before writing records.
5. The adapter exports active binding records in deterministic slot order.
6. The adapter writes a versioned record count and each POD binding record.
7. The adapter writes explicit result state and updates snapshot counters.

First-slice read lifecycle:

1. Caller owns a YuSerialize reader over a bounded buffer.
2. Caller owns an output array for `WorldComponentResourceBinding` records.
3. Caller passes output capacity and receives the read record count.
4. The adapter validates reader, output pointer, record count, output capacity,
   every record value, and duplicate binding tuples before writing outputs.
5. The adapter writes caller-owned output records only after validation succeeds.
6. The adapter returns explicit status and counters.
7. Caller may later rebind records explicitly through
   `WorldComponentResourceBindingBridge::Bind`.

Failure behavior:

- null source bridge returns explicit status and does not write stream data;
- null stream pointer returns explicit status and does not mutate bridge state;
- null output pointer returns explicit status and does not write outputs;
- output capacity too small returns explicit status and does not partially write
  outputs;
- invalid world object IDs, component type IDs, component slot IDs, resource
  handles, resource type IDs, or serialized enum/status values return explicit
  status and do not write outputs;
- duplicate `(WorldObjectId, WorldComponentTypeId, WorldComponentSlotId)` records
  return explicit status and do not partially write outputs;
- stream truncation, unknown version, malformed record count, or writer overflow
  returns explicit status without overrun;
- read failures do not acquire or release resources;
- failed operations do not mutate `WorldInstance`, `ResourceRegistry`,
  component attachment data, binding bridge state, type definitions, or component
  payload state.

## Inputs

- const component-resource binding bridge source;
- caller-owned YuSerialize writer buffer;
- caller-owned YuSerialize reader buffer;
- caller-owned output binding record array;
- caller-owned snapshot descriptors;
- explicit output capacity.

## Outputs

- explicit world component-resource binding snapshot statuses;
- POD snapshot result values;
- deterministic serialized binding records;
- caller-owned output binding records;
- bridge snapshot counters;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No component class name, reflection name, file path, package key, resource path,
script call ID, render handle, physics handle, original-game name, report text,
log text, wall-clock time, pointer value, rendered frame data, or audio output
may define behavior.

## Performance Constraints

Budget classes:

- setup path for snapshot bridge construction;
- frame path for write/read snapshot calls.

First-slice bounds:

- snapshot state is fixed size and does not grow;
- write/read scans are bounded by configured binding capacity and serialized
  record count caps;
- write/read calls do not allocate;
- read calls do not acquire, release, load, decode, upload, or resolve resource
  data;
- no dynamic maps, string keys, global mutable caches, reflection lookup,
  component factories, or hidden registries in measured paths;
- diagnostics or logs are not required for behavior.

Required deterministic signals:

- write count;
- read count;
- written record count;
- read record count;
- rejected record count;
- failed operation count;
- last status;
- allocation/accounting status.

Blocking conditions:

- requiring automatic restore into active bindings;
- requiring resource acquire/release, resource loading, package lookup, file IO,
  decoder, uploader, renderer, audio, script, actor/component behavior, gameplay,
  save, editor, tools, reports, or original-game data for behavior;
- string lookup, dynamic maps, container growth, component factory lookup, or
  global mutable caches in measured paths;
- exposing mutable binding storage from `WorldComponentResourceBindingBridge`;
- modifying `WorldInstance` core to include snapshot, component-resource binding,
  or `YuResource` storage;
- modifying `YuResource` core to include or own `YuWorld`;
- copying UE or Unity public API shape or source code into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldComponentResourceBindingSnapshotBridge_WriteReadRoundTripsBindingsInSlotOrder`
- `WorldComponentResourceBindingSnapshotBridge_WriteEmptyBridgeProducesZeroRecords`
- `WorldComponentResourceBindingSnapshotBridge_WriteRejectsNullSourceWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_WriteRejectsNullWriterWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_WriteRejectsWriterOverflowWithoutOverrun`
- `WorldComponentResourceBindingSnapshotBridge_ReadWritesCallerOwnedRecords`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsNullReaderWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsNullOutputWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsOutputCapacityTooSmallWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsUnknownVersionWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsMalformedRecordCountWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidWorldIdWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidComponentTypeWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidComponentSlotWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidResourceHandleWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsInvalidResourceTypeWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadRejectsDuplicateBindingWithoutMutation`
- `WorldComponentResourceBindingSnapshotBridge_ReadDoesNotAcquireOrReleaseResources`
- `WorldComponentResourceBindingSnapshotBridge_WriteReadPathDoesNotGrowStorage`
- `WorldComponentResourceBindingSnapshotBridge_SnapshotReportsCountsAndLastStatus`
- `WorldComponentResourceBindingSnapshotBridge_NoActorComponentPayloadOrLifecycle`
- `WorldComponentResourceBindingSnapshotBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency`
- `WorldComponentResourceBindingSnapshotBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency`
- `WorldComponentResourceBindingSnapshotBridge_WorldInstanceCoreRemainsSnapshotFree`
- `WorldComponentResourceBindingSnapshotBridge_ResourceCoreRemainsWorldFree`

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
Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingSnapshot*.h
Src/YuEngine/World/Src/WorldComponentResourceBindingSnapshot*.cpp
Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBinding*.h
Src/YuEngine/World/Src/WorldComponentResourceBinding*.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

It may not modify `WorldInstance` core ownership, `YuResource` core ownership,
`YuObject`, `YuScript`, File, Package, RHI, Audio, Input, UI, tools, reports, or
Game Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine serialization responsibility separation,
  Resource boundary usage, performance constraints, and AGENTS compliance before
  approval.
- 博丽灵梦 should review implementability of const binding export,
  validation-before-output-mutation, public result vocabulary, no-mutation
  failure requirements, and the decision to keep active restore outside this
  slice.
- 雾雨魔理沙 should review test completeness, CMake/CTest registration
  boundaries, and whether malformed stream, output capacity, duplicate record,
  and no resource acquire/release tests are enforceable before any
  implementation task is created.
