# P3-GATE-017: World Scene Assembly Snapshot Restore Coordinator

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P3-GATE-004, P3-GATE-006, P3-GATE-007, P3-GATE-009, P3-GATE-011, P3-GATE-013, P3-GATE-014, P3-GATE-015, P3-GATE-016
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0009, ADR-0014, ADR-0015
Source baseline: `5c31c7c`
Review closure: ENG-067F, ENG-067G, and ENG-067H PASS; ENG-067I readiness complete at `dc8bb9d`

## Layer

L5 world runtime scene-assembly coordinator over caller-owned world sidecar
records.

This gate proposes a narrow coordinator that groups the already approved world
sidecar records into one deterministic assembly contract. The first slice must
not become a scene loader, save-game system, object factory, component payload
store, resource loader, render scene, audio scene, UI framework, tool adapter,
report generator, or Game Adapter feature.

The useful first slice is an explicit boundary that validates caller-owned
assembly inputs before any active mutation. This first slice coordinates only
caller-owned component attachment records and caller-owned component-resource
binding records, then restores them through existing bridges only after full
preflight succeeds. `WorldSnapshot`, world transform snapshot values, object
identity restore, transform restore, and manifest streams are not first-slice
active restore inputs.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Private\World.cpp`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\World.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`

Responsibility conclusion:

- UE may combine actor construction, component registration, resource
  references, serialization, streaming, editor metadata, and gameplay lifecycle
  in broad runtime types. YuEngine must not copy that public API shape or source
  code in this slice.
- The useful boundary lesson is that a scene assembly step must separate
  decoded records, validation, active sidecar mutation, resource acquisition,
  object ownership, and component behavior lifetime.
- YuEngine first slice should prove only a deterministic coordinator over
  existing sidecar records. It must not define a general scene asset format.

Unity responsibility reference:

- Unity separates scene files, GameObject/component ownership, asset references,
  runtime asset loading, renderer/audio consumption, and editor inspection.
- YuEngine must not copy Unity Scene, GameObject, Component, prefab,
  Addressables, asset database, or inspector API shape. This gate only proposes
  a bounded scene-assembly coordinator over YuEngine world sidecar records.

## Owns

This gate owns a future first slice for:

- a `WorldSceneAssemblyBridge` or equivalent coordinator;
- explicit scene-assembly statuses, descriptors, results, state values, and
  snapshots;
- caller-owned assembly input records that describe which already decoded
  sidecar record buffers belong to one assembly operation;
- deterministic validation of component attachment record counts, component
  resource binding record counts, world object IDs, component type IDs,
  component slot IDs, resource handles, resource type IDs, and duplicate tuples;
- validating that every component-resource binding record has a matching
  component attachment tuple in the same assembly input set;
- validating destination component attachment and component-resource binding
  bridge state before mutation;
- validating projected resource acquire counts through caller-owned
  `ResourceRegistry` state before active mutation;
- applying component attachments before component-resource bindings only after
  the full assembly preflight succeeds;
- preserving deterministic input order during active restore;
- rejecting unsupported non-empty destinations unless the first slice explicitly
  proves rollback and release semantics;
- counters for assembly attempt count, restored attachment count, restored
  binding count, rejected record count, rollback count, failed operation count,
  last assembly status, last attachment status, last binding status, last
  resource status, and allocation/accounting status;
- explicit status vocabulary for invalid destination, invalid input, invalid
  world object ID, invalid component type ID, invalid component slot ID, invalid
  resource handle, stale resource handle, invalid resource type ID, resource type
  mismatch, projected resource acquire overflow, missing attachment for binding,
  duplicate attachment input, duplicate binding input, attachment capacity
  exceeded, binding capacity exceeded, destination not empty, attachment apply
  failure, binding restore failure, resource acquire failure, and rollback
  failure if rollback is approved;
- tests proving no partial active assembly on validation failure, no hidden
  allocation, no `WorldInstance` ownership creep, no component payload or
  lifecycle creep, no resource loading, and no forbidden module dependencies.

If the first slice cannot preflight component attachment destination state
without mutation using the current `WorldComponentAttachmentBridge` surface, it
may propose one minimal const validation helper on that bridge, such as
`ValidateRestoreDestination(required_attachment_count)`. That helper must not
include or depend on Object, Resource, Script, Serialize, File, Package, render,
audio, physics, UI, tools, reports, or Game Adapter modules.

If the first slice cannot prove component-resource binding preflight through
the current `WorldComponentResourceBindingBridge` and `ResourceRegistry`
surfaces, it must stop at `NEEDS_ARCHITECTURE` instead of restoring partially.
The current proposal expects assembly-local preflight using existing public
binding destination validation and `ResourceRegistry::ValidateAcquire`. A new
public preflight API on `WorldComponentResourceBindingRestoreBridge` is not
authorized unless the gate is amended again.

## Does Not Own

This gate does not own:

- UE-style Actor, ActorComponent, SceneComponent, Level, World partition,
  streaming level, replication, ticking, render state, physics state, editor
  metadata, or asset streaming APIs;
- Unity-style Scene, GameObject, Component, MonoBehaviour, prefab serialization,
  serialized object model, Addressables, asset database, renderer components,
  audio components, or editor inspector behavior;
- reading original-game files, package lookup, resource path strings, resource
  loading, decoding, upload, cache eviction, hot reload, or save-slot policy;
- component payload serialization, generated schemas, reflection, property bags,
  script object lifetime, behavior dispatch, event routing, gameplay services,
  animation, physics, render scene, audio scene, UI, scene streaming, tools,
  reports, profiling UI, or Game Adapter behavior;
- object construction, object destruction, or `YuObject` handle ownership;
- `WorldInstance` object reconstruction, object identity binding restore, or
  world transform record restore;
- modifying `WorldInstance` core ownership to contain component attachment,
  component-resource binding, resource, serialize, or scene assembly storage;
- copied UE or Unity public API shape, source code, macro style, reflection
  vocabulary, lifecycle naming, asset path vocabulary, async-load vocabulary, or
  editor query vocabulary.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuWorld` public `WorldObjectId`, component attachment, component attachment
  snapshot records, component query, component-resource binding,
  component-resource binding snapshot records, component-resource binding
  restore, status, result, and value vocabulary;
- `YuResource` public `ResourceRegistry`, `ResourceHandle`, `ResourceTypeId`,
  `ResourceStatus`, and const projected-acquire validation.

Forbidden dependencies:

- `YuObject`, `YuScript`, `YuSerialize`, `YuFile`, `YuPackage`, `YuThread`,
  `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools,
  reports, or game adapter modules;
- File IO, package lookup, resource path strings, original-game resource names,
  original-game component names, or original-game data as API shape;
- loading, decoding, uploading, streaming, cache eviction, object construction,
  component factory lookup, scene streaming, or save policy during assembly.

The first implementation may add only scene assembly coordinator files under
`YuWorld`, minimal const validation support on existing world sidecar
bridges if review approves the need, `Tests/World` coverage, and CMake/CTest
registration. It must not modify `WorldInstance` core ownership, `YuObject`,
`YuScript`, `YuResource` core ownership beyond already approved const
validation usage, File, Package, RHI, Audio, Input, UI, tools, reports, or Game
Adapter behavior.

## Lifecycle

First-slice assembly lifecycle:

1. Caller owns already decoded world sidecar record buffers.
2. Caller owns a destination `WorldComponentAttachmentBridge`.
3. Caller owns a destination `WorldComponentResourceBindingBridge`.
4. Caller owns a `ResourceRegistry` that already contains referenced resource
   handles.
5. Caller calls the scene assembly coordinator with explicit input counts,
   destination bridges, resource registry, and descriptor.
6. The coordinator validates every destination and every record before active
   mutation.
7. The coordinator rejects duplicate attachment tuples and duplicate binding
   tuples before active mutation.
8. The coordinator rejects resource binding records that do not have a matching
   attachment tuple in the same assembly.
9. The coordinator validates projected resource acquire counts before active
   mutation.
10. The coordinator restores attachment records in deterministic input order.
11. The coordinator restores component-resource binding records in deterministic
   input order through the approved binding restore path.
12. The coordinator returns explicit assembly result and counters.

Failure behavior:

- null destination bridges, resource registry, or input record pointers return
  explicit status and do not mutate;
- invalid input record counts return explicit status and do not mutate;
- invalid world object IDs, component type IDs, component slot IDs, resource
  handles, or resource type IDs return explicit status and do not mutate;
- duplicate attachment or binding records return explicit status and do not
  mutate;
- binding records missing matching attachment records return explicit status and
  do not mutate;
- destination capacity overflow returns explicit status and does not mutate;
- unsupported non-empty destinations return explicit status and do not mutate;
- stale resource handles, type mismatches, projected acquire overflow risk, and
  resource validation failures return explicit status and do not mutate;
- if a later review accepts rollback support, rollback failure must return a
  distinct explicit status and preserve evidence counters;
- failed operations do not mutate `WorldInstance`, serialized buffers,
  component payload data, package/file state, renderer/audio state, tools,
  reports, or Game Adapter state.

## Inputs

- destination component attachment bridge;
- destination component-resource binding bridge;
- caller-owned `ResourceRegistry`;
- caller-owned component attachment records;
- caller-owned component-resource binding records;
- explicit attachment and binding record counts passed by value;
- explicit scene assembly descriptor.

## Outputs

- explicit world scene assembly statuses;
- POD scene assembly result values;
- restored active component attachment records;
- restored active component-resource binding records;
- bridge snapshot counters;
- last component attachment status;
- last component-resource binding status;
- last resource status;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No component class name, reflection name, file path, package key, resource path,
script call ID, render handle, physics handle, original-game name, report text,
log text, wall-clock time, pointer value, rendered frame data, or audio output
may define behavior.

## Performance Constraints

Budget classes:

- setup path for scene assembly bridge construction;
- load/setup path for scene assembly restore calls.

First-slice bounds:

- scene assembly state is fixed size and does not grow;
- assembly validation scans are bounded by configured attachment and binding
  record counts;
- assembly calls do not allocate;
- assembly calls do not load, decode, upload, stream, resolve packages, query
  resource paths, construct objects, or dispatch component behavior;
- no dynamic maps, string keys, global mutable caches, reflection lookup,
  component factories, scene streaming registries, or hidden registries in
  measured paths;
- diagnostics or logs are not required for behavior.

Required deterministic signals:

- assembly attempt count;
- restored attachment count;
- restored binding count;
- rejected record count;
- rollback count;
- failed operation count;
- last assembly status;
- last component attachment status;
- last component-resource binding status;
- last resource status;
- allocation/accounting status.

Blocking conditions:

- requiring stream parsing as the only way to restore active assembly state;
- requiring object construction, resource loading, package lookup, file IO,
  decoder, uploader, renderer, audio, script, actor/component behavior,
  gameplay, save, editor, tools, reports, or original-game data for behavior;
- string lookup, dynamic maps, container growth, component factory lookup, scene
  streaming registries, or global mutable caches in measured paths;
- mutating active attachments before complete assembly input validation;
- mutating active bindings before complete assembly input validation;
- modifying `WorldInstance` to include scene assembly, component attachment,
  component-resource binding, serialize, or resource storage;
- modifying `YuResource` core to include or own `YuWorld`;
- copying UE or Unity public API shape or source code into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldSceneAssemblyBridge_RestoresAttachmentAndBindingRecordsInInputOrder`
- `WorldSceneAssemblyBridge_RestoresEmptyAssemblyWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsNullAttachmentDestinationWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsNullBindingDestinationWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsNullRegistryWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsNullAttachmentInputWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsNullBindingInputWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsInvalidAttachmentRecordWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsInvalidBindingRecordWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsMissingAttachmentForBindingWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsDuplicateAttachmentInputWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsDuplicateBindingInputWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsAttachmentCapacityOverflowWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsBindingCapacityOverflowWithoutMutation`
- `WorldSceneAssemblyBridge_RejectsNonEmptyDestinationsWithoutMutation`
- `WorldSceneAssemblyBridge_ValidatesResourceHandlesBeforeMutation`
- `WorldSceneAssemblyBridge_BindingPreflightFailureDoesNotRestoreAttachments`
- `WorldSceneAssemblyBridge_ResourceAcquireFailureDoesNotPartiallyAssemble`
- `WorldSceneAssemblyBridge_RestorePathDoesNotGrowStorage`
- `WorldSceneAssemblyBridge_NoHiddenAllocation_UsesYuMemorySignal`
- `WorldSceneAssemblyBridge_SnapshotReportsCountsAndLastStatus`
- `WorldSceneAssemblyBridge_NoActorComponentPayloadOrLifecycle`
- `WorldSceneAssemblyBridge_NoObjectScriptSerializeThreadPlatformDiagnosticsDependency`
- `WorldSceneAssemblyBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency`
- `WorldSceneAssemblyBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency`
- `WorldSceneAssemblyBridge_WorldInstanceCoreRemainsAssemblyFree`
- `WorldSceneAssemblyBridge_ResourceCoreRemainsWorldFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-fast-gate -N -R "WorldSceneAssemblyBridge"
```

## Allowed First Slice

If this gate is approved for first slice, the first implementation slice may
create or update only:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssembly*.h
Src/YuEngine/World/Src/WorldSceneAssembly*.cpp
Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentBridge.h
Src/YuEngine/World/Src/WorldComponentAttachmentBridge.cpp
Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingBridge.h
Src/YuEngine/World/Src/WorldComponentResourceBindingBridge.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

`WorldComponentAttachmentBridge` and
`WorldComponentResourceBindingBridge` files are allowed only for minimal const
inspection or destination-validation helpers needed by assembly preflight. This
approval does not authorize changes to snapshot stream formats, `WorldInstance`
core ownership, `YuResource` ownership, component payload storage, serialized
parsing, object construction, resource loading, package lookup, File, RHI,
Audio, Input, UI, tools, reports, or Game Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine responsibility separation, scene assembly
  versus scene loading/save boundaries, performance constraints, dependency
  direction, and AGENTS compliance before approval.
- 博丽灵梦 should review whether existing public surfaces are sufficient for
  validation-before-mutation, whether minimal const helpers are required, and
  whether rollback can be avoided by full preflight.
- 雾雨魔理沙 should review test completeness, CMake/CTest registration
  boundaries, and whether no-partial-assembly and forbidden dependency tests are
  enforceable before any implementation task is created.
