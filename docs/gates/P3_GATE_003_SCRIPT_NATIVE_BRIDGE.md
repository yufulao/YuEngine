# P3-GATE-003: Script Native Bridge

Status: Approved
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙 when implementation exists
Depends on: P3-GATE-001, P3-GATE-002
Related decisions: ADR-0002, ADR-0003, ADR-0005, ADR-0006, ADR-0014, ADR-0015
Source baseline: `32d4045`

## Layer

L4 core asset and script framework.

This gate proves a bounded native script bridge without a VM, bytecode format,
reflection system, world/scene ownership, component model, UI, gameplay, tools,
reports, original-game services, or Game Adapter behavior.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\CoreUObject\Public\UObject\Script.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\CoreUObject\Public\UObject\Class.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`

Responsibility conclusion:

- UE separates object identity, function metadata, script call execution, and
  world/actor lifecycle.
- YuEngine first slice must not copy UObject, Blueprint, macro, reflection, or
  generated-code implementation.
- YuEngine first slice only needs a stable native-call table that later script
  runtimes can call without string lookup in hot paths.

Unity responsibility reference:

- Unity separates scene ownership, GameObject/component lifetime, and script
  component callbacks.
- YuEngine first slice must not copy MonoBehaviour-style APIs or scene callback
  names. It only prepares the native call ABI below future script/world layers.

## Owns

This gate owns the first `YuScript` implementation slice for:

- stable script call IDs;
- fixed native binding registry;
- native call descriptors;
- typed fixed-width value slots for primitive ABI tests;
- explicit call statuses;
- deterministic native call execution;
- bounded registry snapshot and counters;
- no-mutation failure behavior;
- no hidden allocation in measured call paths.

## Does Not Own

This gate does not own:

- script bytecode, parser, compiler, VM, interpreter, JIT, or coroutine model;
- reflection metadata, generated code, properties, object construction, or
  serialization of script values;
- world, scene, actor, entity, component, transform, or update phase ownership;
- Resource, Package, File, RHI, Audio, Input, UI, Physics, Animation, Tools, or
  Game Adapter behavior;
- original TouhouNewWorld service names, script state, title flow, save logic, or
  old backup runtime data;
- JSON reports, oracle output, profiling dashboard, or diagnostics-owned runtime
  behavior.

## Dependencies

Allowed dependencies:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary.

Forbidden dependencies:

- `YuKernel`, `YuObject`, `YuSerialize`, `YuResource`, `YuPackage`, `YuFile`,
  `YuThread`, `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`,
  world/scene, UI, tools, reports, or game adapter modules.

The first slice intentionally does not call the engine service registry. It
stores pre-bound native call entries by stable ID. Later gates may connect this
bridge to kernel services after a separate ownership review.

## Lifecycle

First-slice lifecycle:

1. Setup creates a native registry with fixed binding capacity.
2. Setup registers native call IDs with function pointers and ABI descriptors.
3. Runtime calls use stable call IDs and caller-provided argument/result slots.
4. Runtime failures return explicit statuses and do not mutate registry storage.
5. Snapshot exposes capacity, binding count, successful call count, failed call
   count, and last status.
6. Shutdown is deterministic because the registry owns only fixed storage and
   POD call descriptors.

Failure behavior:

- invalid call ID returns explicit status;
- duplicate call ID returns explicit status and does not replace the existing
  binding;
- capacity overflow returns explicit status and does not mutate binding count;
- null function pointer returns explicit status;
- argument count/type mismatch returns explicit status;
- result count/type mismatch returns explicit status;
- native function failure returns explicit status and records failed call count;
- failed calls must not mutate output slots except for values written by a
  native function before it returns its explicit failure status.

## Inputs

- fixed registry capacity;
- script call IDs;
- native function pointer;
- expected argument count and value types;
- expected result count and value types;
- caller-provided argument slots;
- caller-provided result slots.

## Outputs

- explicit script statuses;
- result value slots;
- registry snapshot;
- successful and failed call counters;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No log text, report text, filesystem data, original-game data, or wall-clock time
may define behavior.

## Performance Constraints

Budget classes:

- setup path for registry construction and binding registration;
- frame path for calling pre-bound native functions.

First-slice bounds:

- native binding capacity: 32;
- maximum argument slots per call: 8;
- maximum result slots per call: 4;
- call ID width: `uint32_t`;
- value payloads are POD fixed-width slots only;
- registry storage may reserve during setup and must not grow during measured
  calls.

Required deterministic signals:

- binding capacity;
- binding count;
- successful call count;
- failed call count;
- last status;
- allocation/accounting status.

Blocking conditions:

- string lookup in the call path;
- unbounded maps or container growth during measured calls;
- dependency on reports, logs, diagnostics, old runtime files, or original-game
  service names;
- script-owned world, object, resource, package, UI, audio, render, or gameplay
  behavior;
- copying UE or Unity public API shape into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `Script_RegisterNativeCall_ReturnsStableId`
- `Script_RegisterDuplicateCall_ReturnsExplicitStatus`
- `Script_RegisterRejectsNullFunction`
- `Script_RegistryCapacityOverflow_DoesNotMutate`
- `Script_InvokeNativeCall_WritesResultDeterministically`
- `Script_InvokeRejectsUnknownCallId`
- `Script_InvokeRejectsArgumentCountMismatch`
- `Script_InvokeRejectsArgumentTypeMismatch`
- `Script_InvokeRejectsResultCountMismatch`
- `Script_NativeFailure_ReturnsExplicitStatus`
- `Script_CallPath_DoesNotGrowRegistryStorage`
- `Script_NoWorldResourcePackageOrGameAdapterDependency`
- `Script_NoHiddenAllocation_UsesYuMemorySignal`
- `Script_SnapshotReportsCountsAndLastStatus`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

## Allowed First Slice

If approved, the first implementation slice may create:

```text
Src/YuEngine/Script/Include/YuEngine/Script/
Src/YuEngine/Script/Src/
Tests/Script/
```

It may update root `CMakeLists.txt` only to add `YuScript`,
`YuScriptTests`, and the required CTest entries.

Suggested public files:

```text
ScriptCallId.h
ScriptConstants.h
ScriptNativeBinding.h
ScriptNativeRegistry.h
ScriptNativeRegistryDesc.h
ScriptSnapshot.h
ScriptStatus.h
ScriptValue.h
ScriptValueType.h
```

Suggested source files:

```text
ScriptNativeRegistry.cpp
```

## Non-Goals

- No VM or bytecode.
- No script parser or compiler.
- No reflection metadata or generated code.
- No UObject, Blueprint, MonoBehaviour, GameObject, or component equivalent.
- No world, scene, actor, entity, transform, or update phase.
- No Resource, Package, File, Object, Serialize, RHI, Audio, Input, UI, Physics,
  Animation, Tools, or Game Adapter dependency.
- No original-game service mapping.
- No report/profiler/oracle/tool output.

## Evidence Inputs

No original-game evidence is required for the first slice.

TouhouNewWorld scripts, services, scenes, saves, resources, old backup runtime
files, old reports, and binary evidence remain future validation inputs only.
They must not be read by P3-GATE-003 fast tests.

## Gate Decision Requested

`APPROVED_FOR_FIRST_SLICE`.

The first implementation task may begin once the implementer confirms the file
scope, dependencies, test names, and verification commands from this gate.
