# P3-GATE-004: World Lifecycle Fixture

Status: Prepared
Requested decision: `NEEDS_SCRIPT_STABILITY`
Current decision: `NEEDS_SCRIPT_STABILITY`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙 when implementation exists
Depends on: P3-GATE-001, P3-GATE-002, P3-GATE-003
Related decisions: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0014, ADR-0015
Source baseline: `5ef0037`

## Layer

L5 runtime world systems.

This gate prepares a bounded `YuWorld` lifecycle fixture. It does not approve
implementation yet because the Script first slice is still in flight. The gate
defines the minimum world ownership, lifecycle, update phases, performance
signals, and tests needed before a later implementation handoff can be safely
created.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\WorldSettings.h`

Responsibility conclusion:

- UE separates world ownership, loaded levels, actor lifetime, actor components,
  and tick execution. YuEngine must keep those responsibilities separate and must
  not copy UE public API, macro, reflection, or generated-code implementation.
- A world-level object should own lifecycle state, fixed update phase order,
  bounded runtime entries, and snapshots. It should not own component behavior,
  render objects, physics bodies, audio voices, UI screens, or gameplay rules in
  this first slice.
- Actor-like or component-like models are future gates. The first slice only
  proves deterministic world state transitions and phase dispatch.

Unity responsibility reference:

- Unity separates Scene ownership, GameObject lifetime, Component data, and
  script callback behavior.
- YuEngine must not copy MonoBehaviour-style callback names or scene APIs. It
  may use the responsibility split as a reminder that scene ownership and script
  execution must remain separate contracts.

## Owns

This gate owns the future first `YuWorld` implementation slice for:

- a fixed-capacity world instance;
- explicit world lifecycle states;
- explicit world statuses;
- stable world object IDs for fixture entries;
- deterministic object registration and removal;
- deterministic update phase order;
- caller-visible phase trace or snapshot for tests;
- lifecycle counters and last-status snapshot;
- bounded capacity behavior and no-mutation failures;
- no hidden allocation in measured update paths;
- no direct dependency on gameplay, UI, rendering, audio, physics, resources, or
  original-game adapters.

## Does Not Own

This gate does not own:

- script VM, bytecode, parser, coroutine, or script callback policy;
- actor, entity-component, transform hierarchy, prefab, or full scene graph;
- streaming levels, partitioning, visibility sets, replication, networking, or
  save/load;
- render, audio, physics, animation, input, UI, tools, reports, or Game Adapter
  behavior;
- original TouhouNewWorld runtime service names, scene facts, title flow, save
  logic, or backup data;
- resource/package/file loading;
- object construction, reflection, or serialization payload behavior;
- copied UE or Unity API shape.

## Dependencies

Allowed dependencies for a later first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuObject` only after P3-GATE-001 remains stable and only for opaque object
  identity vocabulary if the implementation proposal proves it needs that link.

Forbidden dependencies:

- `YuScript` until ENG-039A is complete, reviewed, and the gate is updated;
- `YuSerialize`, `YuResource`, `YuPackage`, `YuFile`, `YuThread`, `YuPlatform`,
  `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools, reports, or game
  adapter modules;
- gameplay classes, old runtime files, original-game data, or script-owned world
  state.

The first implementation handoff must choose either a local POD `WorldObjectId`
or an approved opaque `YuObject` identity adapter. It must not mix both without a
separate architecture review.

## Lifecycle

First-slice lifecycle states:

```text
Created -> Starting -> Running -> Stopping -> Stopped
```

`Failed` is an explicit terminal state for setup failures that cannot preserve a
valid world instance.

First-slice lifecycle:

1. Setup creates a world instance with fixed object capacity and fixed phase
   trace capacity.
2. Setup registers fixture objects before the world starts.
3. Start transitions `Created` to `Starting`, validates entries, and then
   transitions to `Running`.
4. Update is allowed only in `Running`.
5. Update executes phases in fixed order.
6. Update records phase trace and counters in caller-visible snapshot data.
7. Stop transitions `Running` to `Stopping`, clears active entries, and then
   transitions to `Stopped`.
8. Destroy is deterministic because the first slice owns only bounded storage and
   POD fixture entries.

Update phases:

```text
BeginFrame
FixedStep
FrameStep
EndFrame
```

The phase names are YuEngine gate vocabulary. They are not UE or Unity callback
names. Future script integration may attach to a phase only through a separate
adapter gate after the Script bridge is stable.

Failure behavior:

- invalid capacity returns explicit status and does not create a running world;
- duplicate world object ID returns explicit status and does not replace the
  existing entry;
- capacity overflow returns explicit status and does not mutate object count;
- update before start returns explicit status and does not run phases;
- update after stop returns explicit status and does not run phases;
- invalid time step returns explicit status and does not mutate frame counters;
- disabled entries remain registered but are skipped by measured update;
- failed operations update last status but keep the previous valid snapshot.

## Inputs

- fixed world object capacity;
- fixed phase trace capacity;
- world object IDs;
- world object enabled state;
- frame index;
- fixed step duration;
- frame delta duration.

## Outputs

- explicit world statuses;
- world lifecycle state;
- object capacity and active count;
- frame count;
- phase execution count;
- skipped object count;
- last status;
- deterministic phase trace;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No log text, report text, filesystem data, original-game data, wall-clock time,
or rendered frame data may define behavior.

## Performance Constraints

Budget classes:

- setup path for world construction and fixture object registration;
- frame path for phase execution and snapshot update.

First-slice bounds:

- world object capacity: 64;
- update phase count: 4;
- phase trace capacity: 32 records;
- world object ID width: `uint32_t`;
- frame index width: `uint64_t`;
- time values use fixed-width integer durations;
- storage may reserve during setup and must not grow during measured updates.

Required deterministic signals:

- object capacity;
- active object count;
- frame count;
- phase execution count;
- skipped object count;
- last status;
- allocation/accounting status.

Blocking conditions:

- string lookup in the measured update path;
- unbounded maps or container growth during measured updates;
- logging, reporting, diagnostics, wall-clock, or file IO required for behavior;
- hidden dependency on Script, Resource, Package, File, Serialize, UI, render,
  audio, physics, tools, reports, or Game Adapter modules;
- actor/component/transform hierarchy in the first slice;
- copying UE or Unity public API shape into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `World_CreateWithFixedCapacity_ReportsSnapshot`
- `World_StartStop_RunsDeterministicLifecycle`
- `World_UpdateRunsPhasesInFixedOrder`
- `World_UpdateBeforeStart_ReturnsExplicitStatus`
- `World_UpdateAfterStop_ReturnsExplicitStatus`
- `World_RegisterDuplicateObject_DoesNotMutate`
- `World_RegisterOverflow_DoesNotMutate`
- `World_DisabledObject_IsSkipped`
- `World_UpdatePath_DoesNotGrowStorage`
- `World_StopClearsActiveEntries`
- `World_NoScriptResourcePackageFileOrGameAdapterDependency`
- `World_NoActorComponentOrTransformHierarchy`
- `World_SnapshotReportsCountsAndLastStatus`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

## Allowed First Slice

If this gate is later updated to `APPROVED_FOR_FIRST_SLICE`, the first
implementation slice may create:

```text
Src/YuEngine/World/Include/YuEngine/World/
Src/YuEngine/World/Src/
Tests/World/
```

It may update root `CMakeLists.txt` only to add `YuWorld`, `YuWorldTests`, and
the required CTest entries.

Suggested public files:

```text
WorldConstants.h
WorldDesc.h
WorldInstance.h
WorldLifecycleState.h
WorldObjectDesc.h
WorldObjectId.h
WorldSnapshot.h
WorldStatus.h
WorldUpdatePhase.h
WorldPhaseTrace.h
```

Suggested source files:

```text
WorldInstance.cpp
```

Suggested test files:

```text
WorldLifecycleTests.cpp
WorldPerformanceTests.cpp
```

## Implementation Guard

This gate is prepared but not approved for implementation.

Implementation remains blocked until:

1. ENG-039A completes the `YuScript` first slice.
2. QA confirms Script dependencies, hot paths, and tests.
3. Architecture updates this gate from `NEEDS_SCRIPT_STABILITY` to
   `APPROVED_FOR_FIRST_SLICE`.
4. A clean worktree sequencing check confirms no overlap with active Script
   implementation files.

No `YuWorld` source target, CMake target, or test target should be added before
those conditions are satisfied.
