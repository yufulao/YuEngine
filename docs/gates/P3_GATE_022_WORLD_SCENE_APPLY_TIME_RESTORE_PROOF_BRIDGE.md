# P3-GATE-022: World Scene Apply-Time Restore Proof Bridge

Status: First-slice covered
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-017, P3-GATE-019, P3-GATE-021
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0014, ADR-0015
Source baseline: `d00baa6`
Approved proposal: `fb82695`
Candidate evidence: ENG-093A PASS with hard boundary/performance conditions,
ENG-093B PASS with hard scope conditions, and ENG-093C CONCERN recommending a
narrower apply-time proof before decoded active restore.
Approval evidence: ENG-094A2 boundary/performance PASS, ENG-094B2
implementability PASS, and ENG-094C3 test admission PASS on amended
`fb82695`.
Implementation evidence: ENG-095A PASS, ENG-095B PASS, ENG-095QA PASS,
and ENG-095QA2 PASS; first slice landed at `54b48a7` with fast gate
`646/646`.

## Layer

L5 no-mutation apply-time restore proof bridge over the landed decoded scene
restore plan and active restore record families.

This gate proposes a proof-only bridge for the apply-time scene restore path.
The first slice may validate the current caller-owned decoded records, current
destinations, current world membership, current object registry, and current
resource registry in one call. It may produce caller-owned POD proof values and
deterministic active-call slices that a later active coordinator gate can use as
its immediately preceding proof. It must not call active restore bridges, mutate
destinations, acquire or release resources, bind object handles, register
transforms, attach components, read streams, load files or packages, define
scene load/save policy, or become a Game Adapter path.

The useful first slice is an apply-time proof only:

```text
current caller-owned decoded records
+ current destination bridge snapshots and capacities
+ current world, object registry, and resource registry
-> same-call P3-GATE-021 plan/preflight
-> apply-time destination and plan consistency proof
-> deterministic caller-owned active-call slices
-> later explicit active coordinator gate
```

## Reference Boundary

ENG-093 showed that a decoded active scene restore coordinator is implementable
with current public APIs, but the direct active coordinator test surface is not
yet clean enough to prove cross-domain all-or-nothing behavior. YuEngine should
therefore prove the apply-time inputs and deterministic active-call slices before
opening active mutation.

The boundary lesson is:

- P3-GATE-021 proves decoded cross-family plan/preflight with no mutation.
- P3-GATE-017 and P3-GATE-019 prove local active restore after their own local
  preflight.
- P3-GATE-022 should prove that the current plan, current inputs, current
  destinations, and current registries are still aligned immediately before a
  later active coordinator calls active restore.

## Owns

This gate owns a future first slice for:

- a `WorldSceneApplyTimeRestoreProofBridge`,
  `WorldSceneRestoreApplyProofBridge`, or equivalent no-mutation proof bridge;
- explicit proof statuses, descriptors, result values, state values, snapshots,
  proof records, active-call slice records, and proof-local constants;
- running or validating the landed P3-GATE-021 plan/preflight inside the same
  proof call over current caller-owned decoded records;
- requiring a caller-owned P3-GATE-021 plan scratch buffer and capacity for the
  same-call plan/preflight path;
- requiring current identity, transform, component attachment, and
  component-resource binding destinations for public snapshot and
  empty-destination preflight;
- checking current const `WorldInstance` membership for decoded world object ids;
- checking current const `ObjectRegistry` projected acquire validation;
- checking current const `ResourceRegistry` projected acquire validation;
- checking proof output capacity before any proof output mutation;
- producing deterministic caller-owned POD active-call slices in this family
  order only: object identity, transform, component attachment,
  component-resource binding;
- reporting explicit proof failure statuses for stale inputs, non-empty
  destinations, plan failures, capacity failures, registry preflight failures,
  and forbidden mutation paths;
- deterministic counters for proof attempts, proven identity count, proven
  transform count, proven attachment count, proven binding count, emitted slice
  count, rejected operation count, failed operation count, last proof status,
  last plan status, and allocation/accounting status.

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
- cleanup, rollback, compensation, or partial-mutation recovery;
- accepting cached or old plans as apply authorization;
- stream reading, manifest parsing, File IO, Package IO, resource loading,
  scene loading, save policy, async IO, or streaming;
- object construction, object destruction, factories, prefabs, archetypes, or
  entity templates;
- transform hierarchy, scene graph behavior, component payload lifecycle,
  Script, reflection, gameplay, render, audio, physics, UI, tools, reports, or
  Game Adapter behavior.

## Dependencies

Allowed dependencies:

- P3-GATE-021 decoded restore plan bridge and POD plan record vocabulary;
- P3-GATE-017 component attachment and component-resource binding record
  vocabulary and public destination snapshots/validation helpers;
- P3-GATE-019 object identity and transform restore record vocabulary and
  public destination snapshots/validation helpers;
- const `WorldInstance` membership queries only;
- const `ObjectRegistry` validation helpers only;
- const `ResourceRegistry` validation helpers only;
- existing `YuMemory` allocation/accounting signal vocabulary for deterministic
  no-hidden-allocation tests.

Forbidden dependencies:

- File, Package, RHI, Audio, Input, UI, tools, reports, or Game Adapter modules;
- stream readers or writers as part of proof construction;
- mutable `ObjectRegistry` or `ResourceRegistry` operations;
- active sidecar mutation;
- dynamic containers used to hide staging or duplicate validation;
- runtime file paths, package names, asset names, string keys, or scene names.

## Lifecycle

The first slice lifecycle is:

1. Caller owns all decoded input record buffers.
2. Caller provides current active destination bridges for proof-only snapshots
   and empty-destination preflight.
3. Caller provides current const world, object registry, and resource registry.
4. Caller provides the required proof output buffer and capacity.
5. Caller provides the required P3-GATE-021 plan scratch buffer and capacity.
6. The bridge runs or validates P3-GATE-021 plan/preflight in the same call
   using only caller-owned plan scratch.
7. The bridge checks destination emptiness, destination capacity, registry
   projected acquire preflight, plan/input consistency, plan scratch capacity,
   and proof output capacity before any proof output mutation.
8. A successful operation writes only caller-owned POD proof records, active-call
   slices, and counters.
9. Active restore remains a later explicit gate.

Failures must leave caller-owned proof outputs unchanged unless the operation has
already returned success.

## Inputs

Expected first-slice inputs:

- caller-owned identity restore record buffer and count;
- caller-owned transform restore record buffer and count;
- caller-owned component attachment record buffer and count;
- caller-owned component-resource binding record buffer and count;
- current identity destination bridge;
- current transform destination bridge;
- current component attachment destination bridge;
- current component-resource binding destination bridge;
- const world instance for world object membership validation;
- required const object registry for projected object acquire validation;
- required const resource registry for projected resource acquire validation;
- caller-owned P3-GATE-021 plan scratch buffer and capacity;
- caller-owned output proof buffer and capacity;
- caller-owned output active-call slice buffer and capacity;
- descriptor-provided family capacities, proof capacity, and slice capacity.

## Outputs

Expected first-slice outputs:

- caller-owned POD apply-time proof values;
- caller-owned deterministic active-call slice values;
- explicit result values and status enums;
- snapshot counters and last-status values;
- no active destination mutation.

## Performance Constraints

- The bridge must not allocate during proof construction.
- It must not cache plans, proofs, slices, or decoded inputs in global or static
  storage.
- Duplicate, reference, and consistency validation must use bounded scans over
  explicit input counts or caller-owned fixed scratch.
- P3-GATE-021 plan scratch must be caller-owned and capacity-checked before
  proof or slice output mutation.
- Proof output capacity and slice output capacity must be validated before any
  output mutation.
- The same-call P3-GATE-021 plan/preflight cost is accepted; hidden staging or
  dynamic lookup growth is not.
- This gate must not introduce rollback paths. It proves no-mutation apply-time
  readiness only.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldSceneApplyTimeRestoreProofBridge_ProvesAllFamiliesInDeterministicApplyOrder`
- `WorldSceneApplyTimeRestoreProofBridge_ProvesEmptyDecodedScene`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullIdentityInputWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullTransformInputWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullAttachmentInputWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullBindingInputWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullIdentityDestinationWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullTransformDestinationWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullAttachmentDestinationWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullBindingDestinationWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullWorldWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullObjectRegistryWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullResourceRegistryWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullPlanScratchWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullProofOutputWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNullSliceOutputWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsPlanScratchCapacityTooSmallWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsProofOutputCapacityTooSmallWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsSliceOutputCapacityTooSmallWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsPlanFailureWithoutProofMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsMissingWorldObjectWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsObjectAcquirePreflightFailureWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsResourceAcquirePreflightFailureWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNonEmptyIdentityDestinationWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNonEmptyTransformDestinationWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNonEmptyAttachmentDestinationWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsNonEmptyBindingDestinationWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_RejectsDestinationCapacityOverflowWithoutMutation`
- `WorldSceneApplyTimeRestoreProofBridge_ReplansCurrentInputsWithoutAcceptingCachedPlan`
- `WorldSceneApplyTimeRestoreProofBridge_EmitsIdentityTransformAttachmentBindingSlicesOnly`
- `WorldSceneApplyTimeRestoreProofBridge_DoesNotCallActiveRestoreBridges`
- `WorldSceneApplyTimeRestoreProofBridge_DoesNotMutateDestinations`
- `WorldSceneApplyTimeRestoreProofBridge_DoesNotMutateObjectOrResourceRegistries`
- `WorldSceneApplyTimeRestoreProofBridge_ProofPathDoesNotGrowStorage`
- `WorldSceneApplyTimeRestoreProofBridge_NoHiddenAllocation_UsesYuMemorySignal`
- `WorldSceneApplyTimeRestoreProofBridge_SnapshotReportsCountsAndLastStatus`
- `WorldSceneApplyTimeRestoreProofBridge_NoStreamDecodeFilePackageResourceLoadOrGameAdapterDependency`
- `WorldSceneApplyTimeRestoreProofBridge_NoCleanupRollbackOrCompensationPath`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-fast-gate -N -R "WorldSceneApplyTimeRestoreProofBridge"
```

## Allowed First Slice

If this gate is approved for first slice, the first implementation slice may
create or update only:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldSceneApplyTimeRestoreProof*.h
Src/YuEngine/World/Src/WorldSceneApplyTimeRestoreProof*.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

The first slice may include landed P3-GATE-017, P3-GATE-019, and P3-GATE-021
record and public bridge headers. It may not modify
`WorldSceneAssemblyBridge`, `WorldSceneObjectTransformRestoreBridge`,
`WorldSceneDecodedRestorePlanBridge`, `WorldInstance`, `ObjectRegistry`,
`ResourceRegistry`, `YuSerialize` core stream format, `YuScript`, `YuResource`,
File, Package, RHI, Audio, Input, UI, tools, reports, or Game Adapter behavior.

## Review Notes

- 八云蓝 should review whether this proposal preserves the ENG-093A hard
  apply-time boundary without active mutation or hidden storage growth.
- 博丽灵梦 should review whether the proof API remains implementable without
  changing existing active restore bridge APIs.
- 雾雨魔理沙 should review whether the required tests close the ENG-093C proof
  quality concern while staying fast-gate friendly.
