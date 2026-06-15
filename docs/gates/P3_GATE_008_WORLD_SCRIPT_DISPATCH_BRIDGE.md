# P3-GATE-008: World Script Dispatch Bridge

Status: Approved
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-003, P3-GATE-004, P3-GATE-006, P3-GATE-007
Related decisions: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0014, ADR-0015
Source baseline: `a2eb08b`
Review baseline: `8cfa2dc`
Approval baseline: `d49969e`

## Layer

L5 world runtime adapter over the L4 script native bridge.

This gate proposes a narrow adapter that lets a world phase dispatch a stable
native script call ID. It must keep `WorldInstance` script-free and
must keep `ScriptNativeRegistry` world-free. The adapter owns only the bridge
between existing public data contracts.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\CoreUObject\Public\UObject\Script.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\CoreUObject\Public\UObject\Class.h`

Responsibility conclusion:

- UE separates world lifetime, actor/component lifetime, script metadata, and
  event execution. YuEngine must preserve that separation and must not copy UE
  public API, macros, reflection layout, generated-code flow, or tick API shape.
- YuEngine first slice should only prove deterministic dispatch from a known
  world phase to a non-zero native call ID. Registry availability is reported at
  dispatch time through `ScriptNativeRegistry::Invoke`.
- Actor-like script callbacks, construction scripts, reflection events, and
  component tick scheduling remain future gates.

Unity responsibility reference:

- Unity separates Scene ownership, GameObject/Component lifetime, and managed
  script callbacks.
- YuEngine must not copy MonoBehaviour callback names or scene/component API
  shape. This gate only introduces an adapter that can call a native script
  function from a world phase using explicit fixed slots.

## Owns

This gate owns a future first slice for:

- a fixed-capacity world-script dispatch bridge;
- explicit world-script dispatch statuses;
- binding a `WorldUpdatePhase` to a stable `ScriptCallId`;
- deterministic dispatch order over existing `WorldPhaseTrace` records;
- caller-provided argument and result slot buffers passed to
  `ScriptNativeRegistry`;
- bind-time validation limited to local phase and non-zero call ID checks;
- dispatch-time mapping of `ScriptNativeRegistry::Invoke` statuses, including
  unknown call IDs;
- no-mutation behavior for invalid binding, duplicate binding, capacity
  overflow, invalid trace buffers, and script call failures;
- bounded dispatch snapshot and counters;
- no hidden allocation in measured dispatch paths;
- explicit tests proving `WorldInstance` core files remain script-free.

## Does Not Own

This gate does not own:

- script VM, bytecode, parser, compiler, coroutine, reflection, property, or
  generated-code systems;
- actor, component, entity, scene graph, transform hierarchy, prefab, gameplay,
  UI callback, or original-game adapter behavior;
- object construction, object ownership, serialization payloads, resource
  loading, package loading, file IO, save/load, reports, tools, or profiling UI;
- render, audio, physics, animation, input, networking, replication, or editor
  behavior;
- copied UE or Unity API shape, source code, macro vocabulary, or callback
  names.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuWorld` public phase trace and lifecycle value types;
- `YuScript` public native registry, call ID, value slot, and status types.

Forbidden dependencies:

- `YuObject` ownership or object construction behavior;
- `YuSerialize` payload encoding or save/load behavior;
- `YuResource`, `YuPackage`, `YuFile`, `YuThread`, `YuPlatform`,
  `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools, reports, or game
  adapter modules;
- old runtime files, original-game script services, or original-game data as API
  shape.

The first implementation may add `YuScript` as a declared `YuWorld` dependency
only for `WorldScriptDispatchBridge` adapter files. Tests must prove
`WorldInstance.h` and `WorldInstance.cpp` do not include `YuEngine/Script`.

## Lifecycle

First-slice lifecycle:

1. Setup creates a dispatch bridge with fixed binding capacity.
2. Setup binds world phases to non-zero `ScriptCallId` values.
3. Runtime receives existing `WorldPhaseTrace` records after a world update.
4. Runtime dispatches matching phase records in trace order.
5. Runtime passes caller-provided argument and result slot buffers into
   `ScriptNativeRegistry::Invoke`.
6. One `DispatchTrace` call forwards the same caller-provided slot buffers to
   every bound phase it dispatches. Per-phase argument context ownership is a
   future gate.
7. Unbound phases are skipped and counted, not treated as script failures.
8. Failed script calls stop the current dispatch immediately and return explicit
   world-script dispatch status.
9. Snapshot exposes binding capacity, binding count, dispatch count, skipped
   phase count, failed dispatch count, last bridge status, and last script
   status.

Failure behavior:

- invalid phase binding returns explicit status and does not mutate binding
  count;
- zero or otherwise invalid call ID returns explicit status and does not mutate
  binding count;
- duplicate phase binding returns explicit status and does not replace the
  existing binding;
- capacity overflow returns explicit status and does not mutate binding count;
- null trace buffer with non-zero trace count returns explicit status and does
  not dispatch;
- invalid caller slot buffers return explicit status and do not dispatch. Null
  argument or result buffers are valid only when the matching count is zero;
- call IDs that are not registered in `ScriptNativeRegistry` are not rejected at
  bind time. They are reported during dispatch through the returned script
  status, then mapped to an explicit bridge status;
- native call failure maps to explicit bridge status and records the script
  status;
- failed dispatch does not mutate binding storage.

## Inputs

- fixed dispatch binding capacity;
- world update phases;
- stable script call IDs;
- world phase trace pointer and trace count;
- caller-provided argument slots;
- caller-provided argument count;
- caller-provided result slots;
- caller-provided result count.

## Outputs

- explicit world-script dispatch statuses;
- script status from the most recent native call;
- dispatch snapshot;
- dispatch counters;
- skipped phase count;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No log text, report text, filesystem data, original-game data, wall-clock time,
or rendered frame data may define behavior.

## Performance Constraints

Budget classes:

- setup path for bridge construction and phase binding;
- frame path for phase trace dispatch.

First-slice bounds:

- default dispatch binding capacity: `WORLD_UPDATE_PHASE_COUNT`;
- maximum dispatch binding capacity: 8;
- maximum traced phases per dispatch: `MAX_WORLD_PHASE_TRACE_COUNT`;
- phase binding key: `WorldUpdatePhase`;
- script call key: `ScriptCallId`;
- argument and result slots must be caller-provided;
- bridge storage must be fixed capacity and must not grow during measured
  dispatch.

Required deterministic signals:

- binding capacity;
- binding count;
- dispatched call count;
- skipped phase count;
- failed dispatch count;
- last bridge status;
- last script status;
- allocation/accounting status.

Blocking conditions:

- string lookup in the measured dispatch path;
- unbounded maps or container growth during measured dispatch;
- modifying `WorldInstance` to include or own `YuScript`;
- modifying `ScriptNativeRegistry` to include or own `YuWorld`;
- dispatching Actor/Component/gameplay/UI callbacks;
- requiring logs, reports, diagnostics, file IO, resources, packages, save data,
  or original-game services for behavior;
- copying UE or Unity public API shape into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldScriptDispatchBridge_BindPhaseCall_ReturnsStableBinding`
- `WorldScriptDispatchBridge_BindRejectsInvalidCallIdWithoutMutation`
- `WorldScriptDispatchBridge_BindRejectsDuplicatePhaseWithoutMutation`
- `WorldScriptDispatchBridge_BindRejectsCapacityOverflowWithoutMutation`
- `WorldScriptDispatchBridge_DispatchTraceInvokesPhasesInTraceOrder`
- `WorldScriptDispatchBridge_DispatchSkipsUnboundPhase`
- `WorldScriptDispatchBridge_DispatchRejectsInvalidTraceBuffer`
- `WorldScriptDispatchBridge_DispatchRejectsInvalidSlotBuffers`
- `WorldScriptDispatchBridge_DispatchPropagatesScriptFailure`
- `WorldScriptDispatchBridge_DispatchPathDoesNotGrowStorage`
- `WorldScriptDispatchBridge_SnapshotReportsCountsAndLastStatus`
- `WorldScriptDispatchBridge_NoActorComponentSceneGraphOrGameAdapterDependency`
- `WorldScriptDispatchBridge_NoResourcePackageFileSerializeOrObjectOwnershipDependency`
- `WorldScriptDispatchBridge_WorldInstanceCoreRemainsScriptFree`
- `WorldScriptDispatchBridge_ScriptRegistryCoreRemainsWorldFree`

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
`WorldScriptDispatchBridge` source file, declare the `YuWorld` to `YuScript`
adapter dependency, and register the required CTest entries.

Suggested public files:

```text
WorldScriptDispatchBinding.h
WorldScriptDispatchBridge.h
WorldScriptDispatchBridgeDesc.h
WorldScriptDispatchConstants.h
WorldScriptDispatchResult.h
WorldScriptDispatchSnapshot.h
WorldScriptDispatchStatus.h
```

Suggested source files:

```text
WorldScriptDispatchBridge.cpp
```

Suggested test file:

```text
Tests/World/WorldTests.cpp
```

The implementation must not create a new module, public script callback names,
actor/component files, gameplay files, tools, reports, or adapter files outside
the allowed first-slice paths.

## Non-Goals

- No VM or bytecode.
- No script parser, compiler, coroutine, reflection, or generated-code path.
- No Actor, Component, GameObject, MonoBehaviour, Blueprint, UObject, or
  equivalent public API.
- No scene graph, transform hierarchy, prefab, save/load, or gameplay behavior.
- No Resource, Package, File, Object ownership, Serialize payload, RHI, Audio,
  Input, UI, Physics, Animation, Tools, reports, or Game Adapter dependency.
- No original-game service mapping.
- No report/profiler/oracle/tool output.

## Evidence Inputs

No original-game evidence is required for the first slice.

UE5 and Unity are used only for responsibility separation. Their public API
shape, macros, reflection system, generated code, callback names, and source
implementation must not be copied.

## Gate Decision

`APPROVED_FOR_FIRST_SLICE`.

The first implementation task may begin with the dependency boundary, file
scope, test names, and verification commands from this gate.
