# P3-GATE-005: World Kernel Module Bridge

Status: Approved
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙 when implementation exists
Depends on: P3-GATE-004
Related decisions: ADR-0003, ADR-0005, ADR-0006, ADR-0014
Source baseline: `3b6ee1c`

## Layer

L5 runtime integration.

This gate approves the first bridge between the bounded `YuWorld` lifecycle
fixture and the existing Kernel module lifecycle. `YuWorld` is now stable as an
isolated lifecycle fixture, but it is not yet driven by the engine runtime loop.
The first integration slice must connect world start, update, and stop to the
Kernel `IModule` contract without turning the world into a gameplay, actor,
component, script, scene, or resource system.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Private\World.cpp`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\Engine.h`

Responsibility conclusion:

- UE keeps engine loop ownership, world tick dispatch, actor lifecycle, component
  tick, and gameplay callbacks as separate responsibilities. YuEngine must
  preserve that separation and must not copy UE API names, reflection macros, or
  generated-code patterns.
- The first YuEngine bridge may map Kernel module lifecycle calls to a bounded
  `WorldInstance`. It must not create actor or component ownership.

Unity responsibility reference:

- Unity separates the player loop, scene ownership, GameObject lifetime,
  Component data, and MonoBehaviour callback semantics.
- YuEngine may follow the responsibility separation, but it must not copy
  Unity callback names or scene APIs. Script callback attachment remains a
  future adapter gate.

## Owns

This gate owns the first implementation slice for:

- a small `WorldKernelModule` adapter implementing Kernel `IModule`;
- deterministic mapping from Kernel `Start` to `WorldInstance::Start`;
- deterministic mapping from Kernel `Update` to `WorldInstance::Update`;
- deterministic mapping from Kernel `Shutdown` to `WorldInstance::Stop`;
- optional publication of a typed `WorldInstance` service through
  `ServiceRegistry`;
- explicit Kernel failure results when World start/update/stop fails;
- fixed frame-duration mapping owned by adapter descriptor data;
- tests proving the adapter can be driven through Kernel and HeadlessHost;
- dependency scans that keep `WorldInstance` independent from Kernel while
  allowing only the adapter files to include Kernel headers.

## Does Not Own

This gate does not own:

- script callback policy or `YuScript` integration;
- actor, component, transform hierarchy, prefab, scene graph, or gameplay object
  model;
- resource/package/file loading or serialization payload behavior;
- render, audio, physics, animation, input, UI, tools, reports, or Game Adapter
  behavior;
- original TouhouNewWorld runtime service names, scene facts, title flow, save
  logic, or backup data;
- copied UE or Unity public API shape.

## Dependencies

Allowed implementation dependencies for the first slice:

- C++ standard library;
- `YuWorld`;
- `YuKernel` only for `IModule`, `KernelResult`, `KernelStatus`, and
  `ServiceRegistry`;
- `YuMemory` only through the existing `YuWorld` accounting vocabulary.

Allowed test-only dependencies:

- `YuPlatform` only to prove `HeadlessHost -> KernelHostRuntime -> EngineKernel
  -> WorldKernelModule -> WorldInstance` can run deterministically.

Forbidden dependencies:

- `YuScript`, `YuSerialize`, `YuResource`, `YuPackage`, `YuFile`, `YuThread`,
  `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools, reports, or game
  adapter modules in implementation files;
- gameplay classes, old runtime files, original-game data, script-owned world
  state, actor/component/transform hierarchy, or copied UE/Unity API names.

The existing `WorldInstance` core files must remain Kernel-free. Only files
named for the Kernel bridge may include `YuEngine/Kernel/*`.

## Lifecycle Mapping

First-slice adapter lifecycle:

1. Construction receives a `WorldInstance` reference and fixed duration mapping.
2. `Name` returns a stable module name owned by YuEngine.
3. `Dependencies` and `RequiredServices` are empty for the first slice.
4. `PublishedServices` may expose one world service id for later modules.
5. `Start` registers the world service during Kernel's registration window and
   calls `WorldInstance::Start`.
6. `Update` calls `WorldInstance::Update` using the Kernel frame index and the
   adapter-owned fixed/frame duration mapping.
7. `Shutdown` calls `WorldInstance::Stop`.
8. Any failed World status maps to an explicit Kernel failure result and does
   not add new hidden runtime behavior.

Failure behavior:

- null world references are not allowed by constructor shape;
- failed service publication returns an explicit Kernel startup failure;
- invalid World start state returns explicit Kernel startup failure;
- invalid World update state or time step returns explicit Kernel update
  failure and lets Kernel perform its deterministic teardown;
- invalid World stop state returns explicit Kernel shutdown failure;
- failed operations update World's last status through existing World APIs.

## Inputs

- `WorldInstance` reference;
- stable module name;
- stable world service id;
- fixed step duration;
- frame delta duration;
- Kernel frame index and tick duration.

## Outputs

- Kernel lifecycle trace entries;
- World snapshot changes;
- optional registered World service pointer;
- explicit Kernel failure result and message.

No log text, report text, filesystem data, original-game data, wall-clock time,
rendered frame data, or script callback may define behavior.

## Performance Constraints

First-slice bounds:

- no new storage growth in the World update path;
- no string lookup in measured World update behavior;
- no allocation required by `WorldInstance::Update`;
- service registration is setup path only;
- adapter tests must prove World object capacity and phase trace capacity do not
  change across Kernel-driven ticks.

Blocking conditions:

- modifying `WorldInstance` to include Kernel headers;
- Kernel depending on actor/component/transform, Script, Resource, Package, File,
  render, audio, physics, UI, tools, reports, or Game Adapter;
- hiding Script callbacks in Kernel update;
- copying UE or Unity public API shape.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldKernelModule_StartPublishesWorldService`
- `WorldKernelModule_UpdateTicksWorldInKernelOrder`
- `WorldKernelModule_ShutdownStopsWorld`
- `WorldKernelModule_StartFailurePropagatesExplicitStatus`
- `WorldKernelModule_UpdateFailureTriggersKernelTeardown`
- `WorldKernelModule_HeadlessHostDrivesWorldDeterministically`
- `WorldKernelModule_UpdatePathDoesNotGrowWorldStorage`
- `WorldKernelModule_NoScriptResourcePackageFileOrGameAdapterDependency`
- `WorldKernelModule_NoActorComponentOrTransformHierarchy`
- `WorldKernelModule_CoreWorldInstanceRemainsKernelFree`

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
WorldKernelModule.h
WorldKernelModuleDesc.h
WorldServiceIds.h
```

Suggested source files:

```text
WorldKernelModule.cpp
```

Suggested test files:

```text
WorldKernelModuleTests.cpp
```

The implementation may update `YuWorld` CMake linkage to include `YuKernel`
only for adapter files. It must not move World runtime ownership into Kernel or
Platform modules.

## Approval Evidence

This gate is approved for implementation after:

1. ENG-040 completed and pushed the `YuWorld` lifecycle first slice at
   `3b6ee1c`.
2. ENG-040-QA confirmed World dependencies, AGENTS compliance, fixed-capacity
   behavior, and `219/219` fast-gate test pass.
3. The worktree sequencing check starts from a clean `main...origin/main` state.

## Implementation Guard

The first implementation slice may now begin, but it must stay within the
allowed first-slice paths and tests listed above.

No Script callback policy, actor/component model, transform hierarchy, gameplay,
resource/package/file loading, render/audio/physics integration, UI, tools,
reports, Game Adapter behavior, or copied UE/Unity API shape may be added in this
slice.
