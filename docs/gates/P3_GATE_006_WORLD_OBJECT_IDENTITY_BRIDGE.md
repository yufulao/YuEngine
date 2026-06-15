# P3-GATE-006: World Object Identity Bridge

Status: Approved
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙 when implementation exists
Depends on: P3-GATE-001, P3-GATE-004, P3-GATE-005
Related decisions: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0014
Source baseline: `86517f8`

## Layer

L5 runtime world systems over the L2-L4 object identity boundary.

This gate approves the first bridge between bounded `YuWorld` fixture object
IDs and generation-checked `YuObject` handles. `YuWorld` now has lifecycle,
deterministic update phases, and a Kernel module adapter, but its fixture object
IDs are still local POD IDs. The next integration must make world object
identity able to reference an `ObjectRegistry` handle without introducing actor,
component, transform, script callback, scene graph, resource, or game adapter
behavior.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\CoreUObject\Public\UObject\Object.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`

Responsibility conclusion:

- UE keeps object identity and object lifetime below world ownership, actor
  lifetime, component composition, and gameplay callbacks. YuEngine should keep
  the same responsibility order without copying UE public APIs, reflection
  macros, generated-code patterns, or object class hierarchy.
- This gate may bridge `WorldObjectId` to `ObjectHandle`. It must not create an
  Actor, Component, UObject, reflection, or spawn API.

Unity responsibility reference:

- Unity separates scene ownership, GameObject identity, Component data, and
  MonoBehaviour callback semantics.
- YuEngine may use that responsibility split as a guardrail, but must not copy
  Unity scene APIs, GameObject APIs, callback names, or component model.

## Owns

This gate owns the first implementation slice for:

- a small `WorldObjectIdentityBridge` adapter around `WorldInstance` and
  `object::ObjectRegistry`;
- fixed-capacity mapping from `WorldObjectId` to `object::ObjectHandle`;
- explicit bridge status and result values;
- setup-path validation of world IDs and object handles;
- duplicate world ID and duplicate object handle rejection;
- acquire-on-bind and release-on-remove behavior through `ObjectRegistry`;
- clear-all release behavior for world stop or bridge shutdown fixtures;
- deterministic bridge snapshots and counters for tests;
- stale generation rejection through `ObjectRegistry::Validate`;
- dependency scans that keep `WorldInstance` core independent from `YuObject`
  unless a later gate explicitly changes that boundary.

## Does Not Own

This gate does not own:

- script callback policy or `YuScript` integration;
- actor, component, transform hierarchy, prefab, scene graph, or gameplay object
  model;
- object construction from world spawn requests;
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
- `YuObject` only for `ObjectRegistry`, `ObjectHandle`, `ObjectTypeId`,
  `ObjectStatus`, and object snapshot data needed by tests;
- `YuMemory` only through existing `YuWorld` or `YuObject` accounting
  vocabulary.

Forbidden dependencies:

- `YuScript`, `YuSerialize`, `YuResource`, `YuPackage`, `YuFile`, `YuThread`,
  `YuKernel`, `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI,
  tools, reports, or game adapter modules in implementation files;
- gameplay classes, old runtime files, original-game data, actor/component/
  transform hierarchy, or copied UE/Unity API names.

The existing `WorldInstance` core files must remain `YuObject`-free in the first
slice. Only files named for the identity bridge may include `YuEngine/Object/*`.

## Lifecycle Mapping

First-slice adapter lifecycle:

1. Construction receives a `WorldInstance` reference, an `ObjectRegistry`
   reference, and fixed bridge capacity.
2. `Bind` validates a `WorldObjectId`, validates an `ObjectHandle`, rejects
   duplicate world IDs, rejects duplicate object handles, and acquires the
   object handle.
3. `Remove` releases the acquired object handle and clears the binding.
4. `Clear` releases all acquired handles and resets bridge counters that are
   scoped to active bindings.
5. `Validate` checks a binding through `ObjectRegistry::Validate` without
   mutating the world object slot.
6. `Snapshot` exposes bridge capacity, binding count, acquired handle count,
   released handle count, failed operation count, and last explicit status.

Failure behavior:

- invalid world ID returns explicit bridge status and does not mutate state;
- missing world object returns explicit bridge status and does not acquire an
  object handle;
- invalid or stale object handle returns explicit bridge status and does not
  mutate bridge state;
- duplicate world ID returns explicit bridge status and does not replace an
  existing binding;
- duplicate object handle returns explicit bridge status and does not create a
  second binding;
- capacity overflow returns explicit bridge status and does not mutate binding
  count;
- release failure from `ObjectRegistry` returns explicit bridge status and keeps
  the bridge snapshot truthful;
- clear is deterministic and attempts release only for active bindings.

## Inputs

- `WorldInstance` reference;
- `object::ObjectRegistry` reference;
- fixed bridge capacity;
- `WorldObjectId`;
- `object::ObjectHandle`;
- optional expected `object::ObjectTypeId` when the implementation needs type
  validation.

## Outputs

- explicit bridge status/result values;
- bridge snapshot values;
- object reference count changes through `ObjectRegistry`;
- no log text, report text, filesystem data, original-game data, wall-clock
  time, rendered frame data, or script callback behavior.

## Performance Constraints

First-slice bounds:

- bridge capacity must be fixed at setup;
- bind, remove, clear, and validate may scan bounded arrays;
- `WorldInstance::Update` must not grow storage, perform string lookup, or call
  into `ObjectRegistry` for per-frame validation;
- acquire/release must happen on setup/removal/clear paths, not on every frame;
- no `std::map`, `std::unordered_map`, or dynamically growing `std::vector` may
  be required by measured world update behavior;
- bridge tests must prove world object capacity and phase trace capacity do not
  change across bridge operations.

Blocking conditions:

- adding `YuObject` includes to `WorldInstance` core files in this first slice;
- creating actor/component/transform hierarchy;
- adding script callback dispatch or script-owned world state;
- adding Resource, Package, File, Serialize, render, audio, physics, UI, tools,
  reports, or Game Adapter dependencies;
- copying UE or Unity public API shape;
- behavior defined by logs, reports, wall-clock time, original-game data, or
  filesystem state.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldObjectIdentityBridge_BindValidObject_AcquiresHandle`
- `WorldObjectIdentityBridge_BindRejectsInvalidWorldIdWithoutMutation`
- `WorldObjectIdentityBridge_BindRejectsMissingWorldObjectWithoutMutation`
- `WorldObjectIdentityBridge_BindRejectsInvalidObjectHandleWithoutMutation`
- `WorldObjectIdentityBridge_BindRejectsDuplicateWorldObjectId`
- `WorldObjectIdentityBridge_BindRejectsDuplicateObjectHandle`
- `WorldObjectIdentityBridge_RemoveReleasesHandle`
- `WorldObjectIdentityBridge_ClearReleasesAllHandles`
- `WorldObjectIdentityBridge_StaleGenerationInvalidatesBinding`
- `WorldObjectIdentityBridge_UpdatePathDoesNotGrowWorldStorage`
- `WorldObjectIdentityBridge_NoScriptResourcePackageFileOrGameAdapterDependency`
- `WorldObjectIdentityBridge_NoActorComponentOrTransformHierarchy`
- `WorldObjectIdentityBridge_WorldInstanceCoreRemainsObjectFree`

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
WorldObjectIdentityBridge.h
WorldObjectIdentityBridgeDesc.h
WorldObjectIdentityBinding.h
WorldObjectIdentityResult.h
WorldObjectIdentitySnapshot.h
WorldObjectIdentityStatus.h
```

Suggested source files:

```text
WorldObjectIdentityBridge.cpp
```

Suggested test files:

```text
WorldObjectIdentityBridgeTests.cpp
```

The implementation may update `YuWorld` CMake linkage to include `YuObject`
only for bridge files. It must not move object registry ownership into Kernel,
Platform, Script, Resource, Package, File, or Game Adapter modules.

## Approval Evidence

This gate is approved for implementation after:

1. ENG-041 completed and pushed the `WorldKernelModule` bridge at `86517f8`.
2. ENG-041-QA confirmed World/Kernel dependency boundaries, AGENTS compliance,
   and `229/229` fast-gate test pass.
3. ENG-042-PREFLIGHT completed a read-only architecture preflight recommending
   object identity/lifetime bridge before Script callbacks or actor/component
   expansion.
4. The worktree sequencing check starts from a clean `main...origin/main` state.

## Implementation Guard

The first implementation slice may now begin, but it must stay within the
allowed first-slice paths and tests listed above.

No Script callback policy, actor/component model, transform hierarchy, gameplay,
resource/package/file loading, serialization payload behavior, render/audio/
physics integration, UI, tools, reports, Game Adapter behavior, original-game
runtime service names, or copied UE/Unity API shape may be added in this slice.
