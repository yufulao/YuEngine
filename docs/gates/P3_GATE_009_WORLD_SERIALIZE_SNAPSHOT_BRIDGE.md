# P3-GATE-009: World Serialize Snapshot Bridge

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `PENDING_REVIEW`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-002, P3-GATE-004, P3-GATE-007
Related decisions: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0015
Source baseline: `8cfa2dc`

## Layer

L5 world runtime adapter over the L3-L5 serialization value stream.

This gate proposes a narrow adapter that writes existing public world snapshot
data into a caller-provided `YuSerialize` value stream. It is not a save system,
file format, package format, resource format, object persistence system, or
gameplay checkpoint system.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\CoreUObject\Public\Serialization\ArchiveUObject.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\CoreUObject\Public\Serialization\ObjectAndNameAsStringProxyArchive.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\SaveGame.h`

Responsibility conclusion:

- UE separates archive/value serialization, world ownership, object lifetime,
  package loading, and save-game policy. YuEngine must keep those
  responsibilities separate and must not copy UE public API, archive operator
  shape, reflection serialization, object flags, package flow, or save-game
  APIs.
- YuEngine first slice should only prove deterministic world snapshot records on
  top of the existing memory-backed value stream.
- File paths, package names, resource identities, object reconstruction, and
  save slots remain future gates.

Unity responsibility reference:

- Unity separates Scene data, Component serialization, asset persistence, and
  runtime save policy.
- YuEngine must not copy Unity scene or serialization API shape. This gate only
  defines a fixed snapshot adapter over existing primitive record/field writes.

## Owns

This gate owns a future first slice for:

- a `WorldSerializeSnapshotBridge` adapter;
- explicit world-serialize statuses;
- fixed record IDs and field IDs for world snapshot records;
- writing `WorldSnapshot` values through `SerializeWriter`;
- writing existing `WorldPhaseTrace` records in deterministic order;
- optionally writing `WorldTransformSnapshot` counters, not transform state
  records;
- reading the same snapshot records through `SerializeReader` into caller-owned
  POD output structures;
- validating enum and status numeric ranges before assigning world or transform
  output values;
- caller-provided stream buffers and caller-provided phase trace output buffers;
- no-mutation behavior for invalid input, insufficient output trace capacity,
  and serialization failures;
- no hidden allocation in measured write/read paths;
- explicit tests proving `WorldInstance` core files remain Serialize-free and
  `YuSerialize` core files remain World-free.

## Does Not Own

This gate does not own:

- File IO, package parsing, resource loading, save slots, cloud saves, or
  original-game save compatibility;
- object construction, object ownership, object handle persistence, object
  registry mutation, reflection, properties, generated code, or schema compiler;
- transform record enumeration or full transform state persistence;
- script VM, script dispatch, actor/component/gameplay/UI callbacks, scene graph,
  prefab, level streaming, replication, networking, render, audio, physics,
  animation, input, tools, reports, profiling UI, or Game Adapter behavior;
- copied UE or Unity API shape, source code, reflection vocabulary, archive
  operator style, or save-game API naming.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuWorld` public snapshot, phase trace, lifecycle, update phase, and transform
  snapshot value types;
- `YuSerialize` public writer, reader, record ID, field ID, status, and snapshot
  value types.

Forbidden dependencies:

- `YuFile`, `YuPackage`, `YuResource`, `YuScript`, `YuThread`, `YuPlatform`,
  `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools, reports, or game
  adapter modules;
- `YuObject` construction, ownership, reference counting, handle lookup, or
  registry mutation;
- old runtime files, original-game saves, original-game resources, or
  original-game data as API shape.

The first implementation may add `YuSerialize` as a declared `YuWorld`
dependency only for `WorldSerializeSnapshotBridge` adapter files. Tests must
prove `WorldInstance.h` and `WorldInstance.cpp` do not include
`YuEngine/Serialize`, and `Src/YuEngine/Serialize/**` does not include
`YuEngine/World`.

## Lifecycle

First-slice lifecycle:

1. Caller creates a `SerializeWriter` over a caller-provided byte buffer.
2. Caller begins the stream.
3. Adapter writes one world snapshot record.
4. Adapter writes zero or more phase trace records in input order.
5. Adapter may write one transform snapshot counter record if a transform
   snapshot is provided.
6. Caller creates a `SerializeReader` over the committed bytes.
7. Adapter reads snapshot records into caller-owned POD output structures and a
   caller-provided phase trace output buffer.
8. Read path validates enum and status field ranges before assigning them to
   caller-owned outputs.
9. Read path does not construct, start, stop, update, or mutate a `WorldInstance`.
10. Snapshot exposes bridge write/read counters, skipped optional record count,
   last bridge status, last serialize status, and allocation/accounting status.

Failure behavior:

- null writer or reader pointers return explicit status and do not mutate bridge
  snapshot counters except failure counters;
- invalid world snapshot input returns explicit status and does not write a
  partial bridge record;
- null phase trace pointer with non-zero trace count returns explicit status and
  does not write trace records;
- phase trace count greater than caller-approved limit returns explicit status
  and does not write trace records;
- read output phase trace capacity that is too small returns explicit status and
  does not overrun caller output;
- malformed enum or status field values return explicit status and do not
  partially mutate caller output;
- serialization writer or reader failure maps to explicit bridge status and
  records the serialize status;
- failed bridge operations do not mutate `WorldInstance`, `WorldTransformBridge`,
  or `YuSerialize` type definitions.

## Inputs

- `SerializeWriter` over caller-provided bytes;
- `SerializeReader` over caller-provided committed bytes;
- `WorldSnapshot`;
- optional `WorldTransformSnapshot`;
- `WorldPhaseTrace` pointer and trace count;
- caller-provided output `WorldPhaseTrace` pointer and capacity.

## Outputs

- explicit world-serialize statuses;
- serialize status from the most recent writer or reader operation;
- POD output snapshot structures;
- output phase trace records;
- bridge snapshot;
- deterministic serialized bytes;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No file path, report text, log text, resource key, package key, original-game
data, wall-clock time, pointer value, or rendered frame data may define
behavior.

## Performance Constraints

Budget classes:

- setup path for bridge construction;
- write path for snapshot-to-stream;
- read path for stream-to-snapshot.

First-slice bounds:

- stream buffer remains bounded by `MAX_STREAM_BYTE_COUNT`;
- phase trace input/output count is bounded by `MAX_WORLD_PHASE_TRACE_COUNT`;
- record count remains bounded by `MAX_RECORDS_PER_STREAM`;
- field count remains bounded by `MAX_FIELDS_PER_RECORD`;
- serialized data is encoded through explicit primitive fields, not raw POD
  struct byte dumps;
- transform state floats are not serialized in this first slice because the
  current public transform bridge does not expose record enumeration and
  `YuSerialize` first slice does not own float primitives;
- storage must be fixed capacity and must not grow during measured write/read.

Required deterministic signals:

- written snapshot count;
- written trace count;
- read snapshot count;
- read trace count;
- failed operation count;
- skipped optional record count;
- last bridge status;
- last serialize status;
- allocation/accounting status.

Blocking conditions:

- File IO, package IO, resource lookup, save-slot naming, or original-game data
  required for behavior;
- object construction, object handle lookup, registry mutation, reflection, or
  generated serialization;
- string lookup, string field names, dynamic maps, or container growth in
  measured paths;
- raw struct byte dumps as the stable serialized representation;
- modifying `WorldInstance` to include or own `YuSerialize`;
- modifying `YuSerialize` core to include or own `YuWorld`;
- serializing actor/component/gameplay/scene graph/transform hierarchy state;
- copying UE or Unity public API shape into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldSerializeSnapshotBridge_WriteWorldSnapshot_RoundTripsDeterministically`
- `WorldSerializeSnapshotBridge_WritePhaseTraceRecordsInOrder`
- `WorldSerializeSnapshotBridge_WriteOptionalTransformSnapshotCounters`
- `WorldSerializeSnapshotBridge_ReadRejectsSmallTraceOutputWithoutOverrun`
- `WorldSerializeSnapshotBridge_WriteRejectsInvalidTraceBufferWithoutMutation`
- `WorldSerializeSnapshotBridge_WriteRejectsTraceOverflowWithoutMutation`
- `WorldSerializeSnapshotBridge_SerializeFailureMapsExplicitStatus`
- `WorldSerializeSnapshotBridge_ReadFailureMapsExplicitStatus`
- `WorldSerializeSnapshotBridge_ReadRejectsInvalidEnumValuesWithoutMutation`
- `WorldSerializeSnapshotBridge_NoWorldMutationDuringReadWrite`
- `WorldSerializeSnapshotBridge_ReadWritePathDoesNotGrowStorage`
- `WorldSerializeSnapshotBridge_SnapshotReportsCountsAndLastStatus`
- `WorldSerializeSnapshotBridge_NoFilePackageResourceSaveGameOrGameAdapterDependency`
- `WorldSerializeSnapshotBridge_NoActorComponentSceneGraphOrGameplayDependency`
- `WorldSerializeSnapshotBridge_WorldInstanceCoreRemainsSerializeFree`
- `WorldSerializeSnapshotBridge_SerializeCoreRemainsWorldFree`

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
Src/YuEngine/World/Include/YuEngine/World/
Src/YuEngine/World/Src/
Tests/World/
```

It may update root `CMakeLists.txt` only to add the
`WorldSerializeSnapshotBridge` source file, declare the `YuWorld` to
`YuSerialize` adapter dependency, and register the required CTest entries.

Suggested public files:

```text
WorldSerializeSnapshotBridge.h
WorldSerializeSnapshotBridgeDesc.h
WorldSerializeSnapshotConstants.h
WorldSerializeSnapshotResult.h
WorldSerializeSnapshotState.h
WorldSerializeSnapshotStatus.h
```

Suggested source files:

```text
WorldSerializeSnapshotBridge.cpp
```

Suggested test file:

```text
Tests/World/WorldTests.cpp
```

The implementation must not create a new module, file/save/resource/package
files, actor/component files, gameplay files, tools, reports, or adapter files
outside the allowed first-slice paths.

## Non-Goals

- No File IO or package IO.
- No save-game policy, save slots, cloud saves, or original-game save
  compatibility.
- No object construction, reflection, schema compiler, or generated code.
- No transform record enumeration or full transform state persistence.
- No Script, Actor, Component, GameObject, MonoBehaviour, Blueprint, UObject, or
  equivalent public API.
- No scene graph, prefab, resource loading, render, audio, input, physics,
  animation, UI, tools, reports, or Game Adapter dependency.
- No report/profiler/oracle/tool output.

## Evidence Inputs

No original-game evidence is required for the first slice.

UE5 and Unity are used only for responsibility separation. Their public API
shape, archive/save systems, reflection systems, generated code, and source
implementation must not be copied.

## Gate Decision Requested

`APPROVED_FOR_FIRST_SLICE`.

The first implementation task may begin only after reviewers confirm the
dependency boundary, file scope, test names, and verification commands from this
gate.
