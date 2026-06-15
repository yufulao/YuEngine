# P3-GATE-020: World Scene Object Transform Manifest Stream Bridge

Status: Proposed for review
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `PROPOSED_FOR_REVIEW`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-002, P3-GATE-019
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0014, ADR-0015
Source baseline: `baf964e`
Candidate evidence: ENG-084A, ENG-084B, ENG-084C, and ENG-084D next-gate audit.

## Layer

L5 world runtime manifest stream adapter over caller-owned object identity and
transform restore records.

This gate proposes the missing stream/envelope side for the landed
P3-GATE-019 `WorldSceneObjectTransformRestoreBridge`. The first slice may write
and read a deterministic YuSerialize manifest stream that groups caller-owned
object identity restore records and caller-owned transform restore records.
It must not apply records to active world sidecars, construct objects, restore
transforms, bind object handles, load resources, read files, resolve packages,
define save policy, or become a scene loader.

The useful first slice is transport only:

```text
caller-owned object identity restore records
+ caller-owned transform restore records
-> deterministic YuSerialize manifest stream
-> caller-owned decoded object identity restore records
+ caller-owned decoded transform restore records
-> later explicit P3-GATE-019 active restore call
```

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Private\World.cpp`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\SceneComponent.h`

Responsibility conclusion:

- UE may combine scene packages, actor construction, transform data, component
  registration, resource references, streaming handles, and editor metadata.
  YuEngine must not copy that public API shape or source code in this slice.
- The useful boundary lesson is that manifest stream transport, active object
  identity restore, transform restore, object ownership, component payload
  lifetime, resource loading, and scene load policy must remain separate
  responsibilities.
- YuEngine first slice should prove only deterministic stream transport around
  already bounded P3-GATE-019 caller-owned records.

Unity responsibility reference:

- Unity separates Scene files, GameObject identity, Transform data, Component
  behavior, asset references, runtime asset loading, renderer/audio
  consumption, and editor inspection.
- YuEngine must not copy Unity Scene, GameObject, Component, Transform,
  MonoBehaviour, prefab, Addressables, asset database, or inspector API shape.
  This gate only proposes a bounded manifest stream over YuEngine object
  identity and transform restore records.

## Owns

This gate owns a future first slice for:

- a `WorldSceneObjectTransformManifestStreamBridge`,
  `WorldSceneObjectTransformRestoreManifestStreamBridge`, or equivalent
  manifest adapter;
- explicit object-transform manifest stream statuses, descriptors, results,
  state values, snapshots, and manifest-local schema constants;
- writing caller-owned `WorldSceneObjectTransformRestoreIdentityRecord` buffers
  and caller-owned `WorldSceneObjectTransformRestoreTransformRecord` buffers
  into a caller-owned YuSerialize writer;
- reading manifest streams from a caller-owned YuSerialize reader into
  caller-owned identity and transform output buffers;
- deterministic manifest version, identity record count, transform record
  count, and record ordering contracts;
- validation of writer pointers, reader pointers, input pointers, output
  pointers, output capacities, record counts, world object ids, object handles,
  transform records, duplicate identity world object ids, duplicate object
  handles, duplicate transform world object ids, and transform-to-identity
  references inside the decoded manifest;
- preflight of write byte budget through existing caller-owned
  `SerializeWriter` capacity helpers before any manifest write begins;
- rejecting malformed manifest streams without partial output mutation;
- fixed-size chunking or caller-owned scratch/output spans when record counts
  would exceed value-stream record or field capacity;
- deterministic counters and last-status reporting for write attempts, read
  attempts, written identity count, written transform count, read identity
  count, read transform count, rejected record count, failed operation count,
  last stream status, last serialize status, and allocation/accounting status.

## Does Not Own

This gate does not own:

- active object identity restore, `WorldObjectIdentityBridge::Bind`, or
  `WorldSceneObjectTransformRestoreBridge::Restore`;
- active transform restore, `WorldTransformBridge::Register`, transform
  hierarchy, or scene graph behavior;
- object construction, object destruction, object ownership, object factories,
  archetypes, prefabs, or entity templates;
- `ObjectRegistry` mutation, object acquire/release, or object lifetime policy;
- component attachment restore, component-resource binding restore, component
  payloads, behavior lifecycle, or component factories;
- resource loading, resource decode/upload, package reads, file IO, async IO,
  streaming, save-game policy, or scene loading policy;
- `WorldInstance` ownership changes or hidden scene-level storage;
- Script, reflection, gameplay, render, audio, physics, UI, tools, reports, or
  Game Adapter behavior;
- a unified four-record-family scene manifest for object/transform plus
  component attachment/resource binding records;
- a full active scene restore coordinator.

## Dependencies

Allowed dependencies:

- P3-GATE-002 YuSerialize value stream vocabulary;
- P3-GATE-019 object identity and transform restore record vocabulary;
- existing `YuMemory` allocation/accounting signal vocabulary for deterministic
  no-hidden-allocation tests;
- standard fixed-size C++ utilities already used by YuWorld first slices.

Forbidden dependencies:

- File, Package, Resource loading, RHI, Audio, Input, UI, tools, reports, or
  Game Adapter modules;
- Script, reflection, gameplay, editor, or import pipeline code;
- dynamic containers used to hide stream decode staging;
- ownership changes in `WorldInstance`, `ObjectRegistry`, or active sidecar
  bridges;
- runtime file paths, package names, asset names, string keys, or scene names.

## Lifecycle

The first slice lifecycle is:

1. Caller creates a `SerializeWriter` or `SerializeReader`.
2. Caller owns all input identity and transform record buffers.
3. Caller owns all output identity and transform record buffers.
4. Manifest writer validates all inputs, record counts, record invariants, and
   writer capacity before writing any manifest record.
5. Manifest reader validates stream schema, counts, output capacity, record
   invariants, duplicates, and transform-to-identity references before writing
   caller-owned outputs.
6. A successful read produces caller-owned decoded records only.
7. Active restore remains a separate later call to P3-GATE-019.

Failures must leave caller-owned output buffers unchanged unless the operation
has already returned success.

## Inputs

Expected first-slice inputs:

- caller-owned `SerializeWriter` or `SerializeReader`;
- caller-owned identity restore input buffer and count;
- caller-owned transform restore input buffer and count;
- caller-owned identity restore output buffer and capacity;
- caller-owned transform restore output buffer and capacity;
- descriptor-provided identity capacity, transform capacity, and stream budget
  limits.

## Outputs

Expected first-slice outputs:

- deterministic YuSerialize manifest records;
- caller-owned decoded identity restore records;
- caller-owned decoded transform restore records;
- explicit result values and status enums;
- snapshot counters and last-status values.

## Performance Constraints

- The bridge must not allocate during write or read.
- The bridge must not use hidden dynamic containers to validate duplicates or
  transform references.
- Input scans must be bounded by explicit identity and transform record counts.
- Writer byte budget must be computed and validated before the first write.
- Reader output capacity must be validated before any output mutation.
- If value-stream field or record caps would be exceeded, the first slice must
  use deterministic fixed-size chunks or require caller-owned scratch/output
  spans. It must not silently allocate heap storage.
- Object handles are treated as caller-owned synthetic first-slice data. This
  gate does not claim cross-session object persistence.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldSceneObjectTransformManifestStreamBridge_WriteReadRoundTripsObjectTransformRecordsInInputOrder`
- `WorldSceneObjectTransformManifestStreamBridge_WriteEmptyManifestProducesZeroRecords`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsNullWriterWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsNullIdentityInputWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsNullTransformInputWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsWriterCapacityBeforePartialWrite`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsInvalidIdentityRecordWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsInvalidTransformRecordWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsMissingIdentityForTransformWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsDuplicateIdentityWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsDuplicateObjectHandleWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_WriteRejectsDuplicateTransformWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadWritesCallerOwnedOutputs`
- `WorldSceneObjectTransformManifestStreamBridge_ReadRejectsNullReaderWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadRejectsNullIdentityOutputWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadRejectsNullTransformOutputWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadRejectsOutputCapacityTooSmallWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadRejectsUnknownVersionWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadRejectsMalformedRecordCountWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadRejectsInvalidRecordsWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadRejectsDuplicateRecordsWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadRejectsTransformWithoutIdentityWithoutMutation`
- `WorldSceneObjectTransformManifestStreamBridge_ReadDoesNotRestoreActiveIdentityOrTransformSidecars`
- `WorldSceneObjectTransformManifestStreamBridge_WriteReadPathDoesNotGrowStorage`
- `WorldSceneObjectTransformManifestStreamBridge_NoHiddenAllocation_UsesYuMemorySignal`
- `WorldSceneObjectTransformManifestStreamBridge_SnapshotReportsCountsAndLastStatus`
- `WorldSceneObjectTransformManifestStreamBridge_NoObjectConstructionOrDestruction`
- `WorldSceneObjectTransformManifestStreamBridge_NoTransformHierarchySceneLoadOrGameAdapterDependency`
- `WorldSceneObjectTransformManifestStreamBridge_NoFilePackageResourceLoadDecodeUploadDependency`
- `WorldSceneObjectTransformManifestStreamBridge_WorldObjectSerializeCoresRemainManifestFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-fast-gate -N -R "WorldSceneObjectTransformManifestStreamBridge"
```

## Allowed First Slice

If this gate is approved for first slice, the first implementation slice may
create or update only:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformManifest*.h
Src/YuEngine/World/Src/WorldSceneObjectTransformManifest*.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

The first slice may include P3-GATE-019 record headers. It may not modify
`WorldSceneObjectTransformRestoreBridge` active restore behavior,
`WorldSceneAssemblyBridge`, `WorldSceneAssemblyManifestStreamBridge`,
`WorldInstance`, `ObjectRegistry`, `ResourceRegistry`, `YuSerialize` core stream
format, `YuScript`, `YuResource`, File, Package, RHI, Audio, Input, UI, tools,
reports, or Game Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine responsibility separation, manifest stream
  versus active restore boundaries, object handle transport wording, fixed
  capacity/chunk constraints, and no-hidden-allocation requirements.
- 博丽灵梦 should review implementability of caller-owned input/output buffers,
  use of P3-GATE-019 record vocabulary, public result/snapshot shape, and
  whether separate snapshot record names are needed.
- 雾雨魔理沙 should review test completeness, CMake/CTest registration
  expectations, duplicate object-handle coverage, no active restore coverage,
  and whether the required test count stays fast-gate friendly.
