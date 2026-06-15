# P3-GATE-013: World Component Attachment Snapshot Bridge

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-002, P3-GATE-009, P3-GATE-011, P3-GATE-012
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0015
Source baseline: `90ddb0b`

## Layer

L5 world runtime snapshot adapter over the component attachment sidecar.

This gate proposes a narrow YuWorld component attachment snapshot boundary. The
first slice serializes and restores deterministic `WorldComponentAttachment`
records and related counters through caller-owned YuSerialize buffers. It is
not a component payload store, actor/component model, behavior lifecycle,
reflection system, prefab system, scene graph, save-game policy, asset database,
resource loader, editor inspector, report adapter, or Game Adapter feature.

The slice must keep `WorldInstance` ownership unchanged. It may read validated
attachment records through the existing const export path and may write records
back through the public attachment bridge API. It must not query or mutate
`WorldInstance` membership and must not require `YuObject`, `YuResource`,
`YuScript`, render, physics, audio, UI, tools, reports, or Game Adapter modules.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Private\World.cpp`

Responsibility conclusion:

- UE persists and restores rich actor/component state, object ownership,
  construction flow, registration, replication, render and physics state, and
  editor metadata. YuEngine must not copy that API shape or source code in this
  slice.
- The useful boundary lesson is that attachment metadata persistence must stay
  separate from component payloads, behavior startup, scene graph ownership,
  render/physics registration, asset loading, and editor inspection.
- YuEngine first slice should prove only deterministic serialization of the
  already bounded component attachment sidecar.

Unity responsibility reference:

- Unity separates scene object identity, Component attachment, serialized scene
  data, MonoBehaviour lifecycle, asset references, rendering, physics, and
  editor inspection.
- YuEngine must not copy Unity GameObject or Component serialization API shape.
  This gate only introduces a bounded snapshot contract over YuEngine component
  attachment records.

## Owns

This gate owns a future first slice for:

- a `WorldComponentAttachmentSnapshotBridge` adapter fixture;
- explicit component attachment snapshot statuses;
- POD snapshot descriptors, results, records, and counters;
- writing active `WorldComponentAttachment` records from a const
  `WorldComponentAttachmentBridge` source to caller-owned YuSerialize buffers;
- reading serialized attachment records from caller-owned YuSerialize buffers
  into a caller-owned destination `WorldComponentAttachmentBridge`;
- deterministic attachment slot order preservation;
- validating source bridge pointers, destination bridge pointers, stream
  pointers, record counts, component type IDs, world object IDs, and component
  slot IDs before mutation;
- rejecting malformed or unsupported stream data without partial destination
  mutation;
- explicit counters for write count, read count, written record count, read
  record count, rejected record count, failed operation count, and last status;
- allocation/accounting signal using accepted `YuMemory` vocabulary;
- tests proving deterministic round trip, no hidden allocation, no component
  payload or lifecycle creep, no `WorldInstance` mutation, and no forbidden
  module dependencies.

The implementation may add a minimal clear-then-restore path through existing
`WorldComponentAttachmentBridge` public APIs. It must not expose mutable
attachment storage or add new ownership semantics.

## Does Not Own

This gate does not own:

- UE-style `Actor`, `ActorComponent`, `SceneComponent`, component registration,
  construction scripts, replication, ticking, editor selection, render state, or
  physics state;
- Unity-style GameObject or Component API shape, MonoBehaviour lifecycle,
  prefab serialization, serialized scene object model, editor inspector, or
  asset database integration;
- component payload serialization, component factories, generated component
  schemas, reflection, property bags, script object lifetime, behavior dispatch,
  event routing, gameplay services, animation, physics, render scene, audio
  scene, UI, save slots, scene streaming, tools, reports, profiling UI, or Game
  Adapter behavior;
- object construction or `YuObject` handle ownership;
- resource handles, package lookup, file paths, resource loading, decoding,
  upload, cache policy, hot reload, or direct renderer/physics submission;
- copied UE or Unity public API shape, source code, macro style, reflection
  vocabulary, lifecycle naming, or editor query vocabulary.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuSerialize` public value stream vocabulary;
- `YuWorld` public `WorldObjectId`, `WorldComponentAttachment`,
  `WorldComponentAttachmentBridge`, component status, snapshot, and value
  vocabulary.

Forbidden dependencies:

- `YuObject`, `YuResource`, `YuScript`, `YuFile`, `YuPackage`, `YuThread`,
  `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools,
  reports, or game adapter modules;
- old runtime files, original-game component names, original-game actor names,
  original-game resource paths, or original-game data as API shape.

The first implementation must not add new inter-module target dependencies
other than an existing YuWorld to YuSerialize relationship if it is already used
by World serialize adapters. It may add only `WorldComponentAttachmentSnapshot*`
files under `YuWorld`, minimal component attachment adapter updates if needed,
`Tests/World` coverage, and CMake/CTest registration for those files.

## Lifecycle

First-slice write lifecycle:

1. Caller owns a populated `WorldComponentAttachmentBridge`.
2. Caller owns a bounded YuSerialize writer buffer.
3. Caller calls the snapshot bridge with a const source bridge and writer.
4. The adapter validates the source and writer before writing records.
5. The adapter exports active attachment records in deterministic slot order.
6. The adapter writes a versioned record count and each POD attachment record.
7. The adapter writes an explicit result and updates snapshot counters.

First-slice read lifecycle:

1. Caller owns a YuSerialize reader over a bounded buffer.
2. Caller owns a destination `WorldComponentAttachmentBridge`.
3. The adapter validates reader, destination, record count, and every record.
4. The adapter rejects malformed data before mutating the destination bridge.
5. The adapter clears and restores the destination through public attachment
   bridge APIs only after validation succeeds.
6. The adapter returns explicit status and counters.

Failure behavior:

- null source bridge returns explicit status and does not write stream data;
- null destination bridge returns explicit status and does not mutate the
  destination;
- null stream pointer returns explicit status and does not mutate bridge state;
- invalid world object IDs, component type IDs, or component slot IDs return
  explicit status and do not mutate the destination;
- duplicate `(WorldObjectId, WorldComponentTypeId)` records return explicit
  status and do not partially restore the destination;
- stream truncation, unknown version, malformed record count, or writer overflow
  returns explicit status without overrun;
- failed operations do not mutate `WorldInstance`, other modules, type
  definitions, or component payload state.

## Inputs

- const component attachment bridge source;
- destination component attachment bridge;
- caller-owned YuSerialize writer buffer;
- caller-owned YuSerialize reader buffer;
- caller-owned snapshot descriptors.

## Outputs

- explicit world component attachment snapshot statuses;
- POD snapshot result values;
- deterministic serialized attachment records;
- restored attachment sidecar records through public attachment bridge APIs;
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
- write/read scans are bounded by configured attachment capacity and serialized
  record count caps;
- write/read calls do not allocate;
- no dynamic maps, string keys, global mutable caches, reflection lookup, or
  hidden component registries in measured paths;
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

- requiring component payload storage, script behavior, resource loading, object
  construction, render scene, physics scene, audio scene, UI, save slot policy,
  scene streaming, tools, reports, gameplay, or original-game data for behavior;
- string lookup, dynamic maps, container growth, or global mutable caches in
  measured paths;
- exposing mutable attachment storage from `WorldComponentAttachmentBridge`;
- modifying `WorldInstance` core to include snapshot, query, or component
  attachment storage;
- adding dependencies from `YuWorld` core files to `YuObject`, `YuResource`,
  `YuScript`, File, Package, render, physics, audio, UI, tools, or Game Adapter;
- copying UE or Unity public API shape or source code into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldComponentAttachmentSnapshotBridge_WriteReadRoundTripsAttachmentsInSlotOrder`
- `WorldComponentAttachmentSnapshotBridge_WriteEmptyBridgeProducesZeroRecords`
- `WorldComponentAttachmentSnapshotBridge_WriteRejectsNullSourceWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_WriteRejectsNullWriterWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_WriteRejectsWriterOverflowWithoutOverrun`
- `WorldComponentAttachmentSnapshotBridge_ReadRestoresAttachmentRecords`
- `WorldComponentAttachmentSnapshotBridge_ReadZeroRecordStreamClearsDestination`
- `WorldComponentAttachmentSnapshotBridge_ReadRejectsNullReaderWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_ReadRejectsNullDestinationWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_ReadRejectsUnknownVersionWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_ReadRejectsMalformedRecordCountWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_ReadRejectsInvalidWorldIdWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_ReadRejectsInvalidComponentTypeWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_ReadRejectsInvalidComponentSlotWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_ReadRejectsDuplicateAttachmentWithoutMutation`
- `WorldComponentAttachmentSnapshotBridge_WriteReadPathDoesNotGrowStorage`
- `WorldComponentAttachmentSnapshotBridge_SnapshotReportsCountsAndLastStatus`
- `WorldComponentAttachmentSnapshotBridge_NoActorComponentPayloadOrLifecycle`
- `WorldComponentAttachmentSnapshotBridge_NoObjectResourceScriptFilePackageOrGameAdapterDependency`
- `WorldComponentAttachmentSnapshotBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency`
- `WorldComponentAttachmentSnapshotBridge_WorldInstanceCoreRemainsSnapshotFree`

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
Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshot*.h
Src/YuEngine/World/Src/WorldComponentAttachmentSnapshot*.cpp
Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachment*.h
Src/YuEngine/World/Src/WorldComponentAttachment*.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

It may not modify `WorldInstance` core ownership, `YuObject`, `YuResource`,
`YuScript`, File, Package, RHI, Audio, Input, UI, tools, reports, or Game
Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine serialization responsibility separation,
  performance constraints, and AGENTS compliance before approval.
- 博丽灵梦 should review implementability of validation-before-mutation,
  public result vocabulary, and no-mutation failure requirements.
- 雾雨魔理沙 should review test completeness, CMake/CTest registration
  boundaries, and whether writer/reader overflow tests are enforceable before
  any implementation handoff.
