# P3-GATE-018: World Scene Assembly Manifest Stream Bridge

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-002, P3-GATE-013, P3-GATE-015, P3-GATE-017
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0015
Source baseline: `8df2fbb`
Proposal commit: `6dc4b85`
Review fix commit: `3c28ce4`
Candidate evidence: ENG-071A, ENG-071B, and ENG-071C PASS for a manifest/stream-only next gate.
Review closure: ENG-073A PASS, ENG-073B PASS, ENG-075 PASS after ENG-074 fix.

## Layer

L5 world runtime manifest stream adapter over caller-owned scene assembly
sidecar records.

This gate proposes a narrow YuWorld manifest/envelope stream boundary for the
records consumed by the landed P3-GATE-017 `WorldSceneAssemblyBridge`. The
first slice may write and read a deterministic manifest stream that groups
caller-owned component attachment snapshot records and caller-owned
component-resource binding snapshot records. It must not apply records to active
world sidecars, construct objects, restore transforms, load resources, read
files, resolve packages, define save policy, or become a scene loader.

The useful first slice is a transport boundary only:

```text
caller-owned attachment records
+ caller-owned component-resource binding records
-> deterministic YuSerialize manifest stream
-> caller-owned decoded attachment records
+ caller-owned decoded component-resource binding records
-> later explicit P3-GATE-017 assembly call
```

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Private\World.cpp`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`

Responsibility conclusion:

- UE may combine level packages, actor construction, component registration,
  resource references, serialization, streaming handles, and editor metadata.
  YuEngine must not copy that public API shape or source code in this slice.
- The useful boundary lesson is that decoded records, manifest streams, active
  sidecar restore, object ownership, transform ownership, resource loading, and
  scene load policy must remain separate responsibilities.
- YuEngine first slice should prove only a deterministic manifest stream around
  already bounded sidecar records.

Unity responsibility reference:

- Unity separates scene files, GameObject/component ownership, serialized scene
  data, asset references, runtime asset loading, renderer/audio consumption, and
  editor inspection.
- YuEngine must not copy Unity Scene, GameObject, Component, prefab,
  Addressables, asset database, or inspector API shape. This gate only proposes
  a bounded manifest stream over YuEngine world sidecar records.

## Owns

This gate owns a future first slice for:

- a `WorldSceneAssemblyManifestStreamBridge` or equivalent manifest adapter;
- explicit manifest stream statuses, descriptors, results, state values, and
  snapshots;
- writing caller-owned `WorldComponentAttachmentSnapshotRecord` buffers and
  caller-owned `WorldComponentResourceBindingSnapshotRecord` buffers into a
  caller-owned YuSerialize writer;
- reading manifest streams from a caller-owned YuSerialize reader into
  caller-owned attachment and component-resource binding output buffers;
- a deterministic manifest version, attachment record count, binding record
  count, and record ordering contract;
- validation of writer pointers, reader pointers, output pointers, output
  capacities, record counts, world object IDs, component type IDs, component
  slot IDs, resource handles, resource type IDs, duplicate attachment tuples,
  duplicate binding tuples, and manifest enum/status values;
- validating that every component-resource binding record has a matching
  component attachment tuple within the same decoded manifest output;
- preflight of write byte budget through the existing caller-owned
  `SerializeWriter` capacity helpers before any manifest write begins;
- rejecting malformed manifest streams without partial output mutation;
- counters for write count, read count, written attachment count, written
  binding count, read attachment count, read binding count, rejected record
  count, failed operation count, last status, last serialize status, and
  allocation/accounting status;
- tests proving deterministic round trip, no active world or sidecar mutation,
  no hidden allocation, no resource acquire/release, no scene loading, and no
  forbidden module dependencies.

The first slice may define manifest-specific record IDs and field IDs only for
this adapter. It must not redefine the lower-level YuSerialize value stream
format or change the P3-GATE-013/P3-GATE-015 snapshot record layouts.

The implementation must keep YuSerialize record and field limits explicit. It
may encode bounded attachment and binding record arrays as fixed-byte chunks
when needed, following the existing snapshot bridge pattern. It must not require
one YuSerialize record or one YuSerialize field per sidecar record if that would
exceed the current value stream caps.

## Does Not Own

This gate does not own:

- UE-style Actor, ActorComponent, SceneComponent, Level, World partition,
  streaming level, replication, ticking, render state, physics state, editor
  metadata, or asset streaming APIs;
- Unity-style Scene, GameObject, Component, MonoBehaviour, prefab serialization,
  serialized object model, Addressables, asset database, renderer components,
  audio components, or editor inspector behavior;
- active restore into `WorldComponentAttachmentBridge`,
  `WorldComponentResourceBindingBridge`, `WorldSceneAssemblyBridge`, or
  `WorldInstance`;
- object identity restore, object construction, object destruction, or
  `YuObject` handle ownership;
- world transform active restore, transform hierarchy, scene graph, component
  payload serialization, component factories, reflection, property bags, script
  object lifetime, behavior dispatch, gameplay services, animation, physics,
  render scene, audio scene, UI, tools, reports, profiling UI, or Game Adapter
  behavior;
- File IO, package lookup, package manifest parsing, resource path strings,
  resource loading, decoding, upload, cache eviction, hot reload, scene loading,
  save-slot policy, or original-game data as API shape;
- copied UE or Unity public API shape, source code, macro style, reflection
  vocabulary, lifecycle naming, asset path vocabulary, async-load vocabulary, or
  editor query vocabulary.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuSerialize` public value stream vocabulary;
- `YuWorld` public `WorldObjectId`, component attachment snapshot records,
  component-resource binding snapshot records, scene assembly status/result
  vocabulary if needed for manifest result mapping, and value vocabulary;
- `YuResource` public value types only: `ResourceHandle`, `ResourceTypeId`, and
  `ResourceStatus` if a manifest status needs to preserve a decoded resource
  validation signal.

Forbidden dependencies:

- `YuObject`, `YuScript`, `YuFile`, `YuPackage`, `YuThread`, `YuPlatform`,
  `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools, reports, or game
  adapter modules;
- `ResourceRegistry` mutation, resource acquire/release, package lookup, file
  paths, resource path strings, original-game resource names, original-game
  component names, or original-game data as API shape;
- object construction, transform active restore, scene streaming, save policy,
  loading, decoding, uploading, cache eviction, or component behavior dispatch.

The first implementation may add only manifest stream files under `YuWorld`,
`Tests/World` coverage, and CMake/CTest registration. It must not modify
`WorldInstance` core ownership, `YuObject`, `YuScript`, `YuResource` core
ownership, File, Package, RHI, Audio, Input, UI, tools, reports, or Game Adapter
behavior.

## Lifecycle

First-slice write lifecycle:

1. Caller owns attachment snapshot records and component-resource binding
   snapshot records.
2. Caller owns a bounded YuSerialize writer buffer.
3. Caller calls the manifest stream bridge with explicit record pointers,
   record counts, and descriptor values.
4. The bridge validates every input record before writing.
5. The bridge validates duplicate attachment tuples and duplicate binding
   tuples before writing.
6. The bridge validates that each binding tuple has a matching attachment tuple.
7. The bridge calculates the manifest write byte budget and verifies writer
   capacity before `BeginStream` or the first manifest record write. The budget
   must be derived from manifest-local constants or equivalent compile-time
   checks for byte count, record count, and field count.
8. The bridge writes a versioned manifest envelope, count records, attachment
   records, and binding records in deterministic input order.
9. The bridge returns explicit result and counters.

First-slice read lifecycle:

1. Caller owns a YuSerialize reader over a bounded buffer.
2. Caller owns attachment output and binding output arrays.
3. Caller passes output capacities and receives decoded record counts.
4. The bridge validates stream version, record counts, capacities, every record
   value, duplicate tuples, and binding-to-attachment references before writing
   output arrays.
5. The bridge writes caller-owned outputs only after validation succeeds.
6. Caller may later pass the decoded records to the P3-GATE-017
   `WorldSceneAssemblyBridge`.

Failure behavior:

- null writer, reader, input pointer, output pointer, or count pointer returns
  explicit status and does not mutate active world or sidecar state;
- invalid record counts, invalid output capacities, malformed stream data,
  unknown versions, invalid enum/status values, and duplicate records return
  explicit status and do not partially write decoded outputs;
- invalid world object IDs, component type IDs, component slot IDs, resource
  handles, or resource type IDs return explicit status and do not partially
  write decoded outputs;
- binding records missing matching attachment tuples return explicit status and
  do not partially write decoded outputs;
- writer capacity failure must be detected before any manifest stream mutation;
- read failures do not acquire or release resources;
- failed operations do not mutate `WorldInstance`, active component attachment
  bridges, active component-resource binding bridges, resource registries,
  package/file state, renderer/audio state, tools, reports, or Game Adapter
  state.

## Inputs

- caller-owned attachment snapshot record buffers;
- caller-owned component-resource binding snapshot record buffers;
- caller-owned YuSerialize writer buffer;
- caller-owned YuSerialize reader buffer;
- caller-owned attachment output record arrays;
- caller-owned component-resource binding output record arrays;
- explicit record counts and output capacities;
- explicit manifest descriptor.

## Outputs

- explicit world scene assembly manifest stream statuses;
- POD manifest stream result values;
- deterministic serialized manifest stream bytes;
- caller-owned decoded attachment records;
- caller-owned decoded component-resource binding records;
- manifest stream snapshot counters;
- last serialize status;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No component class name, reflection name, file path, package key, resource path,
script call ID, render handle, physics handle, original-game name, report text,
log text, wall-clock time, pointer value, rendered frame data, or audio output
may define behavior.

## Performance Constraints

Budget classes:

- setup path for manifest stream bridge construction;
- load/setup path for manifest write/read calls.

First-slice bounds:

- manifest stream state is fixed size and does not grow;
- write/read validation scans are bounded by configured attachment and binding
  record caps;
- write/read calls do not allocate;
- record, field, and byte budgets are manifest-local constants or equivalent
  compile-time checked values;
- write/read calls do not load, decode, upload, stream, resolve packages, query
  resource paths, construct objects, or dispatch component behavior;
- no dynamic maps, string keys, global mutable caches, reflection lookup,
  component factories, scene streaming registries, or hidden registries in
  measured paths;
- diagnostics or logs are not required for behavior.

Required deterministic signals:

- write count;
- read count;
- written attachment count;
- written binding count;
- read attachment count;
- read binding count;
- rejected record count;
- failed operation count;
- last status;
- last serialize status;
- allocation/accounting status.

Blocking conditions:

- requiring active restore as part of manifest read;
- requiring object construction, transform active restore, resource loading,
  package lookup, file IO, decoder, uploader, renderer, audio, script,
  actor/component behavior, gameplay, save, editor, tools, reports, or
  original-game data for behavior;
- string lookup, dynamic maps, container growth, component factory lookup, scene
  streaming registries, or global mutable caches in measured paths;
- mutating active attachments, active bindings, `WorldInstance`, or
  `ResourceRegistry` during read/write;
- modifying `YuSerialize` core stream format for manifest-only needs;
- copying UE or Unity public API shape or source code into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldSceneAssemblyManifestStreamBridge_WriteReadRoundTripsAssemblyRecordsInInputOrder`
- `WorldSceneAssemblyManifestStreamBridge_WriteEmptyManifestProducesZeroRecords`
- `WorldSceneAssemblyManifestStreamBridge_WriteRejectsNullWriterWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_WriteRejectsNullAttachmentInputWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_WriteRejectsNullBindingInputWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_WriteRejectsWriterCapacityBeforePartialWrite`
- `WorldSceneAssemblyManifestStreamBridge_WriteRejectsInvalidAttachmentRecordWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_WriteRejectsInvalidBindingRecordWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_WriteRejectsMissingAttachmentForBindingWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_WriteRejectsDuplicateAttachmentWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_WriteRejectsDuplicateBindingWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_ReadWritesCallerOwnedOutputs`
- `WorldSceneAssemblyManifestStreamBridge_ReadRejectsNullReaderWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_ReadRejectsNullAttachmentOutputWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_ReadRejectsNullBindingOutputWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_ReadRejectsOutputCapacityTooSmallWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_ReadRejectsUnknownVersionWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_ReadRejectsMalformedRecordCountWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_ReadRejectsInvalidRecordsWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_ReadRejectsDuplicateRecordsWithoutMutation`
- `WorldSceneAssemblyManifestStreamBridge_ReadDoesNotRestoreActiveSidecars`
- `WorldSceneAssemblyManifestStreamBridge_WriteReadPathDoesNotGrowStorage`
- `WorldSceneAssemblyManifestStreamBridge_NoHiddenAllocation_UsesYuMemorySignal`
- `WorldSceneAssemblyManifestStreamBridge_SnapshotReportsCountsAndLastStatus`
- `WorldSceneAssemblyManifestStreamBridge_NoObjectTransformSceneLoadOrGameAdapterDependency`
- `WorldSceneAssemblyManifestStreamBridge_NoFilePackageResourceLoadDecodeUploadDependency`
- `WorldSceneAssemblyManifestStreamBridge_WorldInstanceCoreRemainsManifestFree`
- `WorldSceneAssemblyManifestStreamBridge_SerializeCoreRemainsWorldFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-fast-gate -N -R "WorldSceneAssemblyManifestStreamBridge"
```

## Allowed First Slice

If this gate is approved for first slice, the first implementation slice may
create or update only:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyManifest*.h
Src/YuEngine/World/Src/WorldSceneAssemblyManifest*.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

It may not modify `WorldInstance` core ownership, `WorldSceneAssemblyBridge`
active restore behavior, `YuSerialize` core stream format, `YuObject`,
`YuScript`, `YuResource` core ownership, File, Package, RHI, Audio, Input, UI,
tools, reports, or Game Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine responsibility separation, manifest stream
  versus active restore boundaries, forbidden dependency exclusions, and
  whether the proposal should pause instead of expanding P3 if the stream-only
  boundary cannot hold.
- 博丽灵梦 should review implementability of caller-owned input/output buffers,
  write-size preflight before `SerializeWriter` mutation, public result
  vocabulary, and no-mutation failure requirements.
- 雾雨魔理沙 should review test completeness, CMake/CTest registration
  boundaries, and whether malformed stream, duplicate record, output capacity,
  writer-capacity, no-active-restore, and forbidden dependency tests are
  enforceable before any implementation task is created.
