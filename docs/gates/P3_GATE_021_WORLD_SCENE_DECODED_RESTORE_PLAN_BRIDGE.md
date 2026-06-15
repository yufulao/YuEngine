# P3-GATE-021: World Scene Decoded Restore Plan Bridge

Status: Proposed for review
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `PROPOSED_FOR_REVIEW`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-017, P3-GATE-019, P3-GATE-020
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0014, ADR-0015
Source baseline: `d80746d`
Candidate evidence: ENG-089A, ENG-089B, and ENG-089C next-gate audit.

## Layer

L5 decoded scene restore preflight and plan bridge over landed world sidecar
restore record families.

This gate proposes a no-mutation plan/preflight bridge for the decoded scene
restore path. The first slice may validate caller-owned decoded object identity,
transform, component attachment, and component-resource binding records as one
cross-family transaction candidate. It may output a POD plan, explicit status,
and deterministic counters. It must not call active restore bridges, mutate
destinations, bind object handles, register transforms, attach components,
acquire resources, read streams, load files or packages, define scene load/save
policy, or become a Game Adapter path.

The useful first slice is a transaction proof only:

```text
caller-owned decoded object identity records
+ caller-owned decoded transform records
+ caller-owned decoded component attachment records
+ caller-owned decoded component-resource binding records
-> cross-family validation and projected acquire/capacity checks
-> caller-owned POD scene restore plan
-> later explicit active coordinator gate
```

## Reference Boundary

Mature engines often combine decoded scene data, object references, component
records, resource references, active object construction, load policy, and
rollback. YuEngine must keep those responsibilities separate in this slice.

The boundary lesson from the landed P3 scene gates is:

- P3-GATE-017 proves active component attachment and component-resource binding
  restore after local preflight.
- P3-GATE-018 proves manifest-stream transport for decoded assembly records.
- P3-GATE-019 proves active object identity and transform restore after local
  preflight.
- P3-GATE-020 proves manifest-stream transport for decoded object and transform
  records.
- P3-GATE-021 should prove only cross-family decoded restore planning before any
  active mutation.

## Owns

This gate owns a future first slice for:

- a `WorldSceneDecodedRestorePlanBridge`, `WorldSceneRestorePlanBridge`, or
  equivalent no-mutation plan bridge;
- explicit plan statuses, descriptors, result values, state values, snapshots,
  plan records, and plan-local constants;
- validating caller-owned object identity restore records, transform restore
  records, component attachment records, and component-resource binding records
  as one decoded scene candidate;
- validating all input pointers, input counts, output plan pointers, output plan
  capacity, destination pointers when supplied, and descriptor capacities;
- checking a required const `WorldInstance` pointer and world-object membership
  for every decoded `WorldObjectId` through public membership queries before any
  plan output mutation;
- checking duplicate object identity world object ids, duplicate object handles,
  duplicate transform world object ids, duplicate attachment tuples, and
  duplicate component-resource binding tuples;
- checking missing transform identity references, missing attachment references
  for bindings, and attachment/binding world object ids that are not represented
  by the decoded identity set;
- checking object handle projected acquire through const `ObjectRegistry`
  validation only;
- checking resource projected acquire through const `ResourceRegistry`
  validation only;
- checking active destination capacity and empty-destination preconditions
  through existing public snapshots or restore-destination validation helpers
  without mutating those destinations;
- producing a deterministic caller-owned POD plan that records ordered counts,
  validated family ranges, projected object acquire counts, projected resource
  acquire counts, and plan status values;
- deterministic counters for plan attempts, planned identity count, planned
  transform count, planned attachment count, planned binding count, rejected
  record count, failed operation count, last plan status, last object status,
  last resource status, last attachment status, last binding status, and
  allocation/accounting status.

## Does Not Own

This gate does not own:

- active restore or active apply calls;
- `WorldSceneObjectTransformRestoreBridge::Restore`;
- `WorldSceneAssemblyBridge::Restore`;
- `WorldObjectIdentityBridge::Bind`;
- `WorldTransformBridge::Register`;
- component attachment mutation;
- component-resource binding mutation;
- `ObjectRegistry` acquire/release or mutation;
- `ResourceRegistry` acquire/release or mutation;
- rollback, compensation, or partial-mutation recovery;
- stream reading, manifest parsing, File IO, Package IO, resource loading,
  scene loading, save policy, async IO, or streaming;
- object construction, object destruction, factories, prefabs, archetypes, or
  entity templates;
- transform hierarchy, scene graph behavior, component payload lifecycle,
  Script, reflection, gameplay, render, audio, physics, UI, tools, reports, or
  Game Adapter behavior.

## Dependencies

Allowed dependencies:

- P3-GATE-017 component attachment and component-resource binding record
  vocabulary;
- P3-GATE-019 object identity and transform restore record vocabulary;
- P3-GATE-020 decoded object-transform manifest record vocabulary as a source
  of caller-owned records, without stream parsing in this gate;
- const `WorldInstance` membership queries only;
- const `ObjectRegistry` validation helpers only;
- const `ResourceRegistry` validation helpers only;
- existing world bridge public snapshots and restore-destination validation
  helpers only;
- existing `YuMemory` allocation/accounting signal vocabulary for deterministic
  no-hidden-allocation tests.

Forbidden dependencies:

- File, Package, RHI, Audio, Input, UI, tools, reports, or Game Adapter modules;
- stream readers or writers as part of plan construction;
- mutable `ObjectRegistry` or `ResourceRegistry` operations;
- active sidecar mutation;
- dynamic containers used to hide decoded staging or duplicate validation;
- runtime file paths, package names, asset names, string keys, or scene names.

## Lifecycle

The first slice lifecycle is:

1. Caller owns all decoded input record buffers.
2. Caller provides a const world instance for membership checks.
3. Caller owns the required output plan buffer or output plan value.
4. Caller may provide active destination bridges only for const capacity and
   empty-destination precondition checks.
5. Caller may provide const registries only for projected acquire validation.
6. The bridge validates all records, references, duplicates, capacities,
   projected acquires, and destination preconditions.
7. A successful operation writes only the caller-owned POD plan and counters.
8. Active restore remains a later explicit gate.

Failures must leave caller-owned plan outputs unchanged unless the operation has
already returned success.

## Inputs

Expected first-slice inputs:

- caller-owned identity restore record buffer and count;
- caller-owned transform restore record buffer and count;
- caller-owned component attachment record buffer and count;
- caller-owned component-resource binding record buffer and count;
- caller-owned output plan buffer and capacity;
- const world instance for world object membership validation;
- descriptor-provided family capacities and plan capacity;
- const object registry for projected object acquire validation;
- const resource registry for projected resource acquire validation;
- optional active destination bridges for public capacity and empty-destination
  precondition checks.

## Outputs

Expected first-slice outputs:

- caller-owned POD decoded restore plan values;
- explicit result values and status enums;
- snapshot counters and last-status values;
- no active destination mutation.

## Performance Constraints

- The bridge must not allocate during plan construction.
- Duplicate and reference validation must use bounded scans over explicit input
  counts or caller-owned fixed scratch. It must not use hidden dynamic
  containers.
- Input family counts must be bounded by descriptor capacities and existing
  world record limits.
- Plan output capacity must be validated before any output mutation.
- Projected object and resource acquire checks must complete before any plan
  output mutation.
- This gate must not introduce rollback paths. It proves no-mutation preflight
  only.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldSceneDecodedRestorePlanBridge_PlansAllDecodedRecordFamiliesInDeterministicOrder`
- `WorldSceneDecodedRestorePlanBridge_PlansEmptyDecodedScene`
- `WorldSceneDecodedRestorePlanBridge_RejectsNullIdentityInputWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsNullTransformInputWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsNullAttachmentInputWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsNullBindingInputWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsNullPlanOutputWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsNullWorldWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsPlanOutputCapacityTooSmallWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsIdentityCountExceededWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsTransformCountExceededWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsAttachmentCountExceededWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsBindingCountExceededWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsInvalidIdentityRecordWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsMissingWorldObjectWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsInvalidTransformRecordWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsInvalidAttachmentRecordWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsInvalidBindingRecordWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsDuplicateIdentityWorldObjectIdWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsDuplicateObjectHandleWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsDuplicateTransformWorldObjectIdWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsDuplicateAttachmentTupleWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsDuplicateBindingTupleWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsTransformWithoutIdentityWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsAttachmentWithoutIdentityWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsBindingWithoutAttachmentWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsBindingWithoutIdentityWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsObjectAcquirePreflightFailureWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsResourceAcquirePreflightFailureWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsNonEmptyIdentityDestinationWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsNonEmptyTransformDestinationWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsNonEmptyAttachmentDestinationWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsNonEmptyBindingDestinationWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_RejectsDestinationCapacityOverflowWithoutMutation`
- `WorldSceneDecodedRestorePlanBridge_DoesNotCallActiveRestoreBridges`
- `WorldSceneDecodedRestorePlanBridge_DoesNotMutateObjectOrResourceRegistries`
- `WorldSceneDecodedRestorePlanBridge_PlanPathDoesNotGrowStorage`
- `WorldSceneDecodedRestorePlanBridge_NoHiddenAllocation_UsesYuMemorySignal`
- `WorldSceneDecodedRestorePlanBridge_SnapshotReportsCountsAndLastStatus`
- `WorldSceneDecodedRestorePlanBridge_NoStreamDecodeFilePackageResourceLoadOrGameAdapterDependency`
- `WorldSceneDecodedRestorePlanBridge_NoObjectConstructionTransformHierarchyOrComponentLifecycle`
- `WorldSceneDecodedRestorePlanBridge_WorldActiveRestoreBridgesRemainPlanFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-fast-gate -N -R "WorldSceneDecodedRestorePlanBridge"
```

## Allowed First Slice

If this gate is approved for first slice, the first implementation slice may
create or update only:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldSceneDecodedRestorePlan*.h
Src/YuEngine/World/Src/WorldSceneDecodedRestorePlan*.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

The first slice may include landed P3-GATE-017, P3-GATE-019, and P3-GATE-020
record headers. It may not modify `WorldSceneAssemblyBridge`,
`WorldSceneAssemblyManifestStreamBridge`,
`WorldSceneObjectTransformRestoreBridge`,
`WorldSceneObjectTransformManifestStreamBridge`, `WorldInstance`,
`ObjectRegistry`, `ResourceRegistry`, `YuSerialize` core stream format,
`YuScript`, `YuResource`, File, Package, RHI, Audio, Input, UI, tools, reports,
or Game Adapter behavior.

## Review Notes

- 八云蓝 should review whether the plan/preflight gate fully removes the
  transaction and rollback concern from ENG-089A.
- 博丽灵梦 should review whether coordinator-local validation remains
  implementable without copying private active-restore bridge internals too
  tightly.
- 雾雨魔理沙 should review whether the required tests are enough to prove
  no-mutation preflight and whether the count stays fast-gate friendly.
