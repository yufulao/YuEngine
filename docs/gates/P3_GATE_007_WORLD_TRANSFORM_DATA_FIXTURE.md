# P3-GATE-007: World Transform Data Fixture

Status: Approved
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙 when implementation exists
Depends on: P3-GATE-004, P3-GATE-005, P3-GATE-006
Related decisions: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0014
Source baseline: `a8ee4d0`

## Layer

L5 runtime world data fixture above the existing world lifecycle, Kernel module
adapter, and object identity bridge.

This gate approves the first data-only spatial state fixture for world objects.
`YuWorld` now has deterministic lifecycle/update behavior and a bounded
identity bridge from `WorldObjectId` to `YuObject` handles. The next integration
must provide transform data that later actor, component, script, render, and
physics systems can depend on without introducing those systems yet.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Core\Public\Math\Transform.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`

Responsibility conclusion:

- UE keeps transform math/data separate from high-level actor behavior, script
  callbacks, scene loading, and gameplay policy. YuEngine should keep the same
  responsibility order without copying UE public APIs, generated-code patterns,
  class hierarchy, or naming shape.
- This gate may store and query bounded transform records for existing
  `WorldObjectId` values. It must not create an Actor, Component, scene node,
  attachment hierarchy, spawn API, or gameplay update callback.

Unity responsibility reference:

- Unity exposes transform data through GameObject/Component concepts and
  MonoBehaviour callbacks, but those are higher-level responsibilities than this
  fixture.
- YuEngine may use that separation as a guardrail, but must not copy Unity
  Transform, GameObject, Component, or callback APIs.

## Owns

This gate owns the first implementation slice for:

- a small `WorldTransformBridge` data fixture around `WorldInstance`;
- fixed-capacity mapping from `WorldObjectId` to POD transform state;
- explicit transform status/result values;
- setup-path validation of world object IDs against `WorldInstance`;
- duplicate world ID rejection;
- set/update/query/remove/clear behavior for transform records;
- deterministic bridge snapshots and counters for tests;
- no-mutation failure behavior for invalid IDs, missing world objects,
  duplicates, missing records, and capacity overflow;
- dependency scans that keep transform data isolated from Script, Resource,
  Package, File, render, audio, physics, UI, tools, reports, and Game Adapter
  modules.

## Does Not Own

This gate does not own:

- script callback policy or `YuScript` integration;
- actor, component, scene graph, transform hierarchy, attachment, parenting,
  prefab, spawn, or gameplay object model;
- object construction, destruction, or identity ownership;
- matrix math libraries, quaternion policy, interpolation, animation, physics,
  render synchronization, culling, broadphase, or scene streaming;
- reflection, serialization payloads, resource handles, package loading, file
  IO, or save/load behavior;
- render, audio, physics, animation, input, UI, tools, reports, or Game Adapter
  behavior;
- original TouhouNewWorld runtime service names, scene facts, title flow, save
  logic, or backup data;
- copied UE or Unity public API shape.

## Dependencies

Allowed implementation dependencies for the first slice:

- C++ standard library;
- `YuWorld`;
- `YuMemory` only through existing world accounting vocabulary.

Forbidden dependencies:

- `YuScript`, `YuObject`, `YuSerialize`, `YuResource`, `YuPackage`, `YuFile`,
  `YuThread`, `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI,
  tools, reports, or game adapter modules in implementation files;
- `YuKernel` in new transform files unless a later gate explicitly needs module
  adapter integration;
- gameplay classes, old runtime files, original-game data, actor/component/
  transform hierarchy, attachment tree, scene graph, or copied UE/Unity API
  names.

The existing `WorldInstance` core files must remain free of transform bridge
storage. Only files named for the transform bridge may own transform fixture
state.

## Data Model

First-slice transform data is POD and explicitly bounded:

- `WorldTransformState` stores translation, rotation, and scale as numeric POD
  fields with deterministic defaults;
- no dynamic allocation is required for transform records;
- no owning strings, paths, resource handles, script handles, or object handles
  are stored in transform records;
- no parent ID, child list, attachment rule, scene node, component handle, or
  matrix cache is stored in the first slice.

## Lifecycle Mapping

First-slice adapter lifecycle:

1. Construction receives a `WorldInstance` reference and fixed bridge capacity.
2. `Register` validates a `WorldObjectId`, rejects duplicate IDs, rejects
   missing world objects, checks capacity, and stores a transform state.
3. `Set` updates an existing transform record without changing capacity.
4. `Query` returns an existing transform record without mutating state.
5. `Remove` clears one transform record.
6. `Clear` removes all active transform records.
7. `Snapshot` exposes bridge capacity, record count, updated record count,
   removed record count, failed operation count, and last explicit status.

Failure behavior:

- invalid world ID returns explicit status and does not mutate state;
- missing world object returns explicit status and does not allocate or store a
  record;
- duplicate world ID returns explicit status and does not replace an existing
  record;
- missing transform record returns explicit status and does not mutate state;
- capacity overflow returns explicit status and does not mutate record count;
- clear is deterministic and touches only active records.

## Inputs

- `WorldInstance` reference;
- fixed transform bridge capacity;
- `WorldObjectId`;
- POD transform state value.

## Outputs

- explicit transform status/result values;
- transform snapshot values;
- copied POD transform state values;
- no log text, report text, filesystem data, original-game data, wall-clock
  time, rendered frame data, physics data, or script callback behavior.

## Performance Constraints

First-slice bounds:

- bridge capacity must be fixed at setup;
- register, set, query, remove, and clear may scan bounded arrays;
- `WorldInstance::Update` must not grow storage, format strings, perform file
  IO, call script, or update transform state implicitly;
- no `std::map`, `std::unordered_map`, or dynamically growing `std::vector` may
  be required by measured world update behavior;
- transform tests must prove world object capacity and phase trace capacity do
  not change across transform bridge operations.

Blocking conditions:

- storing transform bridge data directly in `WorldInstance` core files;
- creating actor/component/scene graph/attachment hierarchy;
- adding script callback dispatch or script-owned world state;
- adding Object, Resource, Package, File, Serialize, render, audio, physics, UI,
  tools, reports, or Game Adapter dependencies;
- copying UE or Unity public API shape;
- behavior defined by logs, reports, wall-clock time, original-game data, or
  filesystem state.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldTransformBridge_RegisterValidObject_StoresTransform`
- `WorldTransformBridge_RegisterRejectsInvalidWorldIdWithoutMutation`
- `WorldTransformBridge_RegisterRejectsMissingWorldObjectWithoutMutation`
- `WorldTransformBridge_RegisterRejectsDuplicateWorldObjectId`
- `WorldTransformBridge_RegisterRejectsCapacityOverflowWithoutMutation`
- `WorldTransformBridge_SetUpdatesExistingRecord`
- `WorldTransformBridge_SetRejectsMissingRecordWithoutMutation`
- `WorldTransformBridge_QueryReturnsStoredTransform`
- `WorldTransformBridge_RemoveClearsRecord`
- `WorldTransformBridge_ClearRemovesAllRecords`
- `WorldTransformBridge_UpdatePathDoesNotGrowWorldStorage`
- `WorldTransformBridge_NoScriptResourcePackageFileObjectOrGameAdapterDependency`
- `WorldTransformBridge_NoActorComponentSceneGraphOrHierarchy`
- `WorldTransformBridge_WorldInstanceCoreRemainsTransformStorageFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

## Allowed First Slice

The first implementation slice may update only:

```text
Src/YuEngine/World/Include/YuEngine/World/
Src/YuEngine/World/Src/
Tests/World/
CMakeLists.txt
```

Suggested public files:

```text
WorldTransformBridge.h
WorldTransformBridgeDesc.h
WorldTransformBinding.h
WorldTransformResult.h
WorldTransformSnapshot.h
WorldTransformState.h
WorldTransformStatus.h
```

Suggested source files:

```text
WorldTransformBridge.cpp
```

Suggested test files:

```text
WorldTransformBridgeTests.cpp
```

The implementation must not move transform state ownership into Kernel,
Platform, Script, Object, Resource, Package, File, Serialize, render, audio,
physics, UI, tools, reports, or Game Adapter modules.

## Approval Evidence

This gate is approved for implementation after:

1. ENG-043 completed and pushed the `WorldObjectIdentityBridge` first slice at
   `a8ee4d0`.
2. ENG-043-QA confirmed World/Object dependency boundaries, AGENTS compliance,
   and `242/242` fast-gate test pass.
3. ENG-044-PREFLIGHT completed a read-only architecture preflight recommending
   transform data before Script callbacks or Actor/Component expansion.
4. The worktree sequencing check starts from a clean `main...origin/main` state.

## Implementation Guard

The first implementation slice may now begin, but it must stay within the
allowed first-slice paths and tests listed above.

No Script callback policy, actor/component model, scene graph, transform
hierarchy, gameplay, resource/package/file loading, serialization payload
behavior, render/audio/physics integration, UI, tools, reports, Game Adapter
behavior, original-game runtime service names, or copied UE/Unity API shape may
be added in this slice.
