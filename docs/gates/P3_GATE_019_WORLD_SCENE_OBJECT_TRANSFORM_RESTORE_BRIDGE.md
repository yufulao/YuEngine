# P3-GATE-019: World Scene Object Transform Restore Bridge

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-001, P3-GATE-004, P3-GATE-006, P3-GATE-007, P3-GATE-017, P3-GATE-018
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0014
Source baseline: `85fabf7`
Proposal commit: `5e69016`
Review fix commits: `39fa058`, `f5f6493`
Candidate evidence: P3-GATE-017 landed sidecar assembly baseline and P3-GATE-018 landed manifest stream baseline.
Review closure: ENG-081A PASS, ENG-081B PASS, and ENG-081C PASS after ENG-080A/B/C review findings and fixes.

## Layer

L5 world runtime active restore coordinator over caller-owned object identity
and transform records.

This gate proposes the first scene-assembly slice that may touch active object
identity and transform sidecars. It is still not a scene loader, object factory,
component factory, save-game system, package reader, resource loader, render
scene, audio scene, UI framework, tool adapter, report generator, or Game
Adapter feature.

The useful first slice is an explicit apply boundary:

```text
caller-owned world object identity restore records
+ caller-owned world transform restore records
+ caller-owned existing WorldInstance objects
+ caller-owned existing ObjectRegistry handles
-> full preflight without active mutation
-> bind existing object handles through WorldObjectIdentityBridge
+ register transform records through WorldTransformBridge
```

Object creation remains outside this gate. The caller must create or own all
`WorldInstance` objects and all `ObjectRegistry` handles before this bridge is
called.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Private\World.cpp`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\SceneComponent.h`

Responsibility conclusion:

- UE may combine actor construction, object identity, transform hierarchy,
  component registration, resource references, serialization, streaming, editor
  metadata, and gameplay lifecycle. YuEngine must not copy that public API shape
  or source code in this slice.
- The useful boundary lesson is that object identity binding, transform data
  restore, object construction, component payload lifetime, resource loading,
  and scene load policy must remain separate responsibilities.
- YuEngine first slice should prove only deterministic active restore for
  already existing world object ids, already existing object handles, and POD
  transform states.

Unity responsibility reference:

- Unity separates Scene files, GameObject identity, Transform data, Component
  behavior, asset references, runtime asset loading, renderer/audio
  consumption, and editor inspection.
- YuEngine must not copy Unity Scene, GameObject, Component, Transform,
  MonoBehaviour, prefab, Addressables, asset database, or inspector API shape.
  This gate only proposes a bounded active restore adapter over YuEngine world
  object identity and transform records.

## Owns

This gate owns a future first slice for:

- a `WorldSceneObjectTransformRestoreBridge` or equivalent coordinator;
- explicit object-transform restore statuses, descriptors, results, state
  values, snapshots, and caller-owned record types;
- caller-owned object identity restore records containing `WorldObjectId` and
  `yuengine::object::ObjectHandle` values;
- caller-owned transform restore records containing `WorldObjectId` and
  `WorldTransformState` values;
- validating destination `WorldObjectIdentityBridge` and `WorldTransformBridge`
  state before mutation;
- validating every referenced world object exists in the caller-owned
  `WorldInstance` before mutation;
- validating every object handle through a non-mutating object acquire
  preflight before mutation;
- validating every transform record references an identity record in the same
  input set before mutation;
- rejecting duplicate world object ids in identity records and duplicate world
  object ids in transform records before mutation;
- rejecting duplicate object handles in identity records before mutation;
- rejecting non-empty identity or transform destinations unless review approves
  rollback semantics;
- applying object identity bindings before transform registrations only after
  full preflight succeeds;
- preserving deterministic input order during active restore;
- counters for restore attempt count, restored identity count, restored
  transform count, rejected record count, rollback count if rollback is
  approved, failed operation count, last restore status, last identity status,
  last transform status, last object status, and allocation/accounting status;
- tests proving no partial active restore on validation failure, no hidden
  allocation, no object construction, no transform hierarchy, no component
  payload or lifecycle, no scene loading, and no forbidden module dependencies.

The current `ObjectRegistry::Validate` mutates registry accounting state. If
the first slice needs object acquire preflight, it may propose one minimal const
helper on `ObjectRegistry`, such as:

```text
ValidateAcquire(ObjectHandle handle, std::uint32_t projected_acquire_count) const
```

The helper checks handle validity and reference-count overflow without
mutating. Duplicate object handles in the same restore input set must be
rejected before this helper is used for active restore, so projected acquire
counts remain explicit and bounded. That helper must not include or depend on
`YuWorld`.

If review concludes that object acquire preflight cannot be proven without
mutating `ObjectRegistry`, the gate must remain `NEEDS_CHANGES` and no
implementation task may be created.

## Does Not Own

This gate does not own:

- object creation, object destruction, object type registration policy, object
  factory lookup, or ownership of `ObjectRegistry`;
- `WorldInstance` object creation or removal;
- transform hierarchy, parent/child transforms, scene graph, spatial queries,
  physics state, animation state, render state, or audio state;
- component payload storage, component behavior lifecycle, generated schemas,
  reflection, property bags, script object lifetime, behavior dispatch, event
  routing, gameplay services, scene streaming, save-slot policy, UI, tools,
  reports, profiling UI, or Game Adapter behavior;
- reading manifest streams, parsing serialized streams, File IO, package
  lookup, resource path strings, resource loading, decoding, upload, cache
  eviction, hot reload, or original-game data as API shape;
- modifying `WorldInstance` core ownership to contain object identity restore,
  transform restore, scene assembly, serialization, resource, package, or Game
  Adapter storage;
- copied UE or Unity public API shape, source code, macro style, reflection
  vocabulary, lifecycle naming, asset path vocabulary, async-load vocabulary, or
  editor query vocabulary.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuObject` public `ObjectRegistry`, `ObjectHandle`, `ObjectStatus`, and
  minimal const acquire-preflight vocabulary if approved;
- `YuWorld` public `WorldInstance`, `WorldObjectId`,
  `WorldObjectIdentityBridge`, `WorldObjectIdentityStatus`,
  `WorldTransformBridge`, `WorldTransformState`, `WorldTransformStatus`, and
  value vocabulary.

Forbidden dependencies:

- `YuScript`, `YuSerialize`, `YuResource`, `YuFile`, `YuPackage`, `YuThread`,
  `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools,
  reports, or game adapter modules;
- File IO, package lookup, resource path strings, original-game resource names,
  original-game component names, original-game object names, or original-game
  data as API shape;
- object construction, component factory lookup, transform hierarchy, scene
  streaming, save policy, loading, decoding, uploading, cache eviction, or
  component behavior dispatch.

The first implementation may add only object-transform restore coordinator
files under `YuWorld`, minimal const object preflight support under `YuObject`
if review approves the need, `Tests/World` coverage, and CMake/CTest
registration. It must not modify `WorldInstance` core ownership, `YuScript`,
`YuSerialize`, `YuResource`, File, Package, RHI, Audio, Input, UI, tools,
reports, or Game Adapter behavior.

## Lifecycle

First-slice restore lifecycle:

1. Caller owns a `WorldInstance` containing already registered world object ids.
2. Caller owns an `ObjectRegistry` containing already created object handles.
3. Caller owns an empty destination `WorldObjectIdentityBridge`.
4. Caller owns an empty destination `WorldTransformBridge`.
5. Caller owns object identity restore records and transform restore records.
6. Caller calls the restore bridge with explicit pointers, counts, and
   descriptor values.
7. The bridge validates destinations, record counts, every world object id,
   every object handle, every transform state, duplicate identity world object
   ids, duplicate identity object handles, duplicate transform records, and
   transform-to-identity references before active mutation.
8. The bridge validates projected object handle acquire operations through a
   const preflight helper before active mutation.
9. The bridge binds object identities in deterministic input order.
10. The bridge registers transform records in deterministic input order.
11. The bridge returns explicit result and counters.

Failure behavior:

- null destination bridges, object registry, world instance, input records, or
  count pointers return explicit status and do not mutate;
- invalid counts, invalid world object ids, missing world objects, invalid
  object handles, stale object handles, object acquire overflow risk, duplicate
  identity world object ids, duplicate identity object handles, duplicate
  transforms, or transform records without matching identity records return
  explicit status and do not mutate;
- destination capacity overflow returns explicit status and does not mutate;
- unsupported non-empty destinations return explicit status and do not mutate;
- if a later review accepts rollback support, rollback failure must return a
  distinct explicit status and preserve evidence counters;
- failed operations do not mutate serialized buffers, component payload data,
  resource registries, package/file state, renderer/audio state, tools, reports,
  or Game Adapter state.

## Inputs

- caller-owned `WorldInstance`;
- caller-owned `ObjectRegistry`;
- destination `WorldObjectIdentityBridge`;
- destination `WorldTransformBridge`;
- caller-owned object identity restore records;
- caller-owned transform restore records;
- explicit identity and transform record counts;
- explicit object-transform restore descriptor.

## Outputs

- explicit object-transform restore statuses;
- POD object-transform restore result values;
- restored active world object identity bindings;
- restored active world transform records;
- bridge snapshot counters;
- last world object identity status;
- last world transform status;
- last object status;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No component class name, reflection name, file path, package key, resource path,
script call ID, render handle, physics handle, original-game name, report text,
log text, wall-clock time, pointer value, rendered frame data, or audio output
may define behavior.

## Performance Constraints

Budget classes:

- setup path for object-transform restore bridge construction;
- load/setup path for object-transform restore calls.

First-slice bounds:

- restore state is fixed size and does not grow;
- validation scans are bounded by configured identity and transform record
  counts;
- restore calls do not allocate;
- restore calls do not create objects, load resources, read files, resolve
  packages, dispatch scripts, or dispatch component behavior;
- no dynamic maps, string keys, global mutable caches, reflection lookup,
  component factories, scene streaming registries, transform hierarchy storage,
  or hidden registries in measured paths;
- diagnostics or logs are not required for behavior.

Required deterministic signals:

- restore attempt count;
- restored identity count;
- restored transform count;
- rejected record count;
- rollback count if rollback is approved;
- failed operation count;
- last restore status;
- last world object identity status;
- last world transform status;
- last object status;
- allocation/accounting status.

Blocking conditions:

- requiring object construction or object destruction inside restore;
- requiring mutable object acquire as the only way to preflight object handles;
- requiring transform hierarchy, scene graph, component payload/lifecycle,
  resource loading, package lookup, file IO, decoder, uploader, renderer, audio,
  script, gameplay, save, editor, tools, reports, or original-game data for
  behavior;
- string lookup, dynamic maps, container growth, component factory lookup, scene
  streaming registries, or global mutable caches in measured paths;
- mutating identity bindings before complete restore input validation;
- mutating transform records before complete restore input validation;
- modifying `WorldInstance` to include object identity restore, transform
  restore, serialize, resource, package, or scene loading storage;
- modifying `YuObject` core to include or own `YuWorld`;
- copying UE or Unity public API shape or source code into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldSceneObjectTransformRestoreBridge_RestoresIdentityAndTransformRecordsInInputOrder`
- `WorldSceneObjectTransformRestoreBridge_RestoresIdentityOnlyRecords`
- `WorldSceneObjectTransformRestoreBridge_RestoresEmptyInputsWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsNullWorldWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsNullObjectRegistryWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsNullIdentityDestinationWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsNullTransformDestinationWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsNullIdentityInputWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsNullTransformInputWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsInvalidIdentityRecordWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsInvalidTransformRecordWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsMissingWorldObjectWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsMissingIdentityForTransformWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsDuplicateIdentityWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsDuplicateObjectHandleWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsDuplicateTransformWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsIdentityCapacityOverflowWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsTransformCapacityOverflowWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_RejectsNonEmptyDestinationsWithoutMutation`
- `WorldSceneObjectTransformRestoreBridge_ValidatesObjectHandlesBeforeMutation`
- `WorldSceneObjectTransformRestoreBridge_ObjectAcquirePreflightFailureDoesNotRestoreIdentitiesOrTransforms`
- `WorldSceneObjectTransformRestoreBridge_RestorePathDoesNotGrowStorage`
- `WorldSceneObjectTransformRestoreBridge_NoHiddenAllocation_UsesYuMemorySignal`
- `WorldSceneObjectTransformRestoreBridge_SnapshotReportsCountsAndLastStatus`
- `WorldSceneObjectTransformRestoreBridge_NoObjectConstructionOrDestruction`
- `WorldSceneObjectTransformRestoreBridge_NoTransformHierarchyOrSceneGraph`
- `WorldSceneObjectTransformRestoreBridge_NoComponentPayloadLifecycleOrScriptDependency`
- `WorldSceneObjectTransformRestoreBridge_NoSerializeFilePackageResourceLoadOrGameAdapterDependency`
- `WorldSceneObjectTransformRestoreBridge_WorldInstanceCoreRemainsObjectTransformRestoreFree`
- `WorldSceneObjectTransformRestoreBridge_ObjectCoreRemainsWorldFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-fast-gate -N -R "WorldSceneObjectTransformRestoreBridge"
```

## Allowed First Slice

If this gate is approved for first slice, the first implementation slice may
create or update only:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformRestore*.h
Src/YuEngine/World/Src/WorldSceneObjectTransformRestore*.cpp
Src/YuEngine/Object/Include/YuEngine/Object/ObjectRegistry.h
Src/YuEngine/Object/Src/ObjectRegistry.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

`ObjectRegistry` files are allowed only for a minimal const acquire-preflight
helper needed to prove validation-before-mutation. This approval would not
authorize object construction policy changes, object ownership changes,
`WorldInstance` core ownership changes, `YuSerialize`, `YuScript`, `YuResource`,
File, Package, RHI, Audio, Input, UI, tools, reports, or Game Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine responsibility separation, object identity
  versus object construction boundaries, transform data versus scene graph
  boundaries, dependency direction, and performance constraints before
  approval.
- 博丽灵梦 should review whether the existing public surfaces plus one minimal
  const `ObjectRegistry` helper can prove validation-before-mutation, whether
  non-empty destinations must remain rejected, and whether rollback can be
  avoided by full preflight.
- 雾雨魔理沙 should review test completeness, CMake/CTest registration
  boundaries, and whether object acquire preflight, no partial restore,
  no-hidden-allocation, no object construction, no transform hierarchy, and
  forbidden dependency tests are enforceable before any implementation task is
  created.
