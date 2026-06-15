# P3-GATE-016: World Component Resource Binding Restore Bridge

Status: Proposed
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `PENDING_REVIEW`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P1-GATE-006, P3-GATE-011, P3-GATE-012, P3-GATE-014, P3-GATE-015
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0009, ADR-0014, ADR-0015
Source baseline: `4994518`

## Layer

L5 world runtime restore adapter over caller-owned component-resource binding
snapshot records.

This gate proposes a narrow YuWorld restore boundary for records produced by
the approved `WorldComponentResourceBindingSnapshotBridge` read path. The first
slice applies caller-owned POD binding records to an active
`WorldComponentResourceBindingBridge` through explicit caller action. It is not
a snapshot reader, resource loader, package resolver, save-game policy,
component payload store, component behavior system, editor inspector, report
adapter, or Game Adapter feature.

The slice must keep serialized parsing, resource validation, resource acquire,
and active binding mutation as separate responsibilities. Reading serialized
data remains owned by P3-GATE-015. Runtime restore is an explicit apply step
that validates the full caller-owned record set before active mutation.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\AssetManager.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Public\StreamableManager.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Private\World.cpp`

Responsibility conclusion:

- UE may couple component serialization, object ownership, runtime component
  registration, streaming handles, and editor metadata through a broad object
  framework. YuEngine must not copy that public API shape or source code in this
  slice.
- The useful boundary lesson is that serialized asset or resource references,
  runtime resource resolution, active component binding, loading policy, upload
  policy, and component behavior lifetime must remain separate.
- YuEngine first slice should prove only an explicit restore adapter that maps
  already decoded POD records back through the existing binding bridge.

Unity responsibility reference:

- Unity separates serialized component asset references, runtime asset loading,
  component lifecycle, scene object ownership, renderer/audio consumption, and
  editor inspection.
- YuEngine must not copy Unity GameObject, Component, serialized object,
  Addressables, asset database, or inspector API shape. This gate only proposes
  a deterministic restore step over YuEngine component-resource binding records.

## Owns

This gate owns a future first slice for:

- a `WorldComponentResourceBindingRestoreBridge` adapter;
- explicit component-resource binding restore statuses;
- POD restore descriptors, results, state values, and snapshots;
- restoring caller-owned `WorldComponentResourceBinding` or equivalent
  snapshot output records into a caller-owned
  `WorldComponentResourceBindingBridge`;
- validating destination bridge pointers, attachment source pointers, resource
  registry pointers, input record pointers, input record counts, restore
  capacity, world object IDs, component type IDs, component slot IDs, resource
  handles, resource type IDs, duplicate binding tuples, and existing destination
  state before active mutation;
- validating attachment tuples through caller-owned component attachment data;
- validating resource handles through caller-owned `ResourceRegistry` state
  before resource acquisition;
- calling the existing component-resource binding bridge `Bind` path only after
  the full input set passes validation;
- preserving deterministic input order during apply;
- rejecting unsupported non-empty destination restore unless the first slice
  explicitly proves rollback and resource release semantics;
- counters for restore attempts, restored binding count, rejected record count,
  rollback count, failed operation count, last restore status, last binding
  status, last resource status, and allocation/accounting status;
- tests proving no partial active restore on validation failure, no hidden
  allocation, no resource loading, no component payload or lifecycle creep, no
  `WorldInstance` mutation, and no forbidden module dependencies.

If the first slice cannot validate resource handles without mutation using the
current `ResourceRegistry` surface, it may propose one minimal const validation
helper on `ResourceRegistry`. That helper must not include or depend on
`YuWorld`, must not acquire or release resources, and must be covered by tests
that keep `YuResource` core World-free.

## Does Not Own

This gate does not own:

- UE-style `Actor`, `ActorComponent`, `SceneComponent`, asset manager,
  streamable manager, component registration, ticking, replication, render
  state, physics state, editor metadata, or asset streaming APIs;
- Unity-style GameObject or Component API shape, MonoBehaviour lifecycle,
  prefab serialization, serialized object model, addressables, asset database,
  renderer components, audio components, or editor inspector behavior;
- reading serialized streams or parsing snapshot records;
- resource loading, decoding, upload, package lookup, file paths, resource path
  strings, cache eviction, hot reload, or original-game resource lookup;
- component payload serialization, component factories, generated schemas,
  reflection, property bags, script object lifetime, behavior dispatch, event
  routing, gameplay services, animation, physics, render scene, audio scene, UI,
  save slots, scene streaming, tools, reports, profiling UI, or Game Adapter
  behavior;
- object construction or `YuObject` handle ownership;
- copied UE or Unity public API shape, source code, macro style, reflection
  vocabulary, lifecycle naming, asset path vocabulary, async-load vocabulary, or
  editor query vocabulary.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuWorld` public `WorldObjectId`, component attachment, component query,
  component-resource binding, component-resource binding snapshot output,
  status, result, and value vocabulary;
- `YuResource` public `ResourceRegistry`, `ResourceHandle`, `ResourceTypeId`,
  `ResourceStatus`, and a minimal const validation helper if review approves it.

Forbidden dependencies:

- `YuObject`, `YuScript`, `YuSerialize`, `YuFile`, `YuPackage`, `YuThread`,
  `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools,
  reports, or game adapter modules;
- File IO, package lookup, resource path strings, original-game resource names,
  original-game component names, or original-game data as API shape;
- loading, decoding, uploading, streaming, or cache eviction during restore.

The first implementation may add only restore adapter files under `YuWorld`,
minimal validation/export support on the existing component-resource binding
bridge or resource registry if review approves the need, tests, and CMake/CTest
registration. It must not modify snapshot read files, attachment snapshot files,
`WorldInstance` core ownership, or make `YuResource` depend on `YuWorld`.

## Lifecycle

First-slice restore lifecycle:

1. Caller reads serialized component-resource binding records through the
   already approved snapshot bridge into caller-owned POD records.
2. Caller owns a destination `WorldComponentResourceBindingBridge`.
3. Caller owns a component attachment source or query source.
4. Caller owns a `ResourceRegistry` that already contains the referenced
   resource handles.
5. Caller calls the restore bridge with the destination, attachment source,
   resource registry, input record buffer, and input record count.
6. The adapter validates every input and every record before active mutation.
7. The adapter rejects duplicate binding tuples before active mutation.
8. The adapter rejects non-empty destination state unless the approved first
   slice explicitly includes rollback tests.
9. The adapter applies records in deterministic order through the existing
   `WorldComponentResourceBindingBridge::Bind` path.
10. The binding bridge remains the only owner of resource acquire semantics.
11. The adapter returns explicit restore result and counters.

Failure behavior:

- null destination bridge, attachment source, resource registry, input pointer,
  or count pointer returns explicit status and does not mutate;
- invalid world object IDs, component type IDs, component slot IDs, resource
  handles, or resource type IDs return explicit status and does not mutate;
- missing attachment tuples return explicit status and does not acquire resources;
- stale resource handles, type mismatches, reference count overflow risk, and
  resource validation failures return explicit status and does not mutate;
- duplicate input records return explicit status and does not mutate;
- destination capacity overflow returns explicit status and does not mutate;
- unsupported non-empty destination returns explicit status and does not mutate;
- if a later review accepts rollback support, rollback failure must return a
  distinct explicit status and preserve evidence counters;
- failed operations do not mutate `WorldInstance`, serialized buffers,
  component payload data, package/file state, renderer/audio state, tools,
  reports, or Game Adapter state.

## Inputs

- destination component-resource binding bridge;
- caller-owned component attachment or query source;
- caller-owned `ResourceRegistry`;
- caller-owned component-resource binding snapshot output records;
- input record count;
- explicit restore descriptor.

## Outputs

- explicit world component-resource binding restore statuses;
- POD restore result values;
- restored active component-resource binding records;
- bridge snapshot counters;
- last component-resource binding status;
- last resource status;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No component class name, reflection name, file path, package key, resource path,
script call ID, render handle, physics handle, original-game name, report text,
log text, wall-clock time, pointer value, rendered frame data, or audio output
may define behavior.

## Performance Constraints

Budget classes:

- setup path for restore bridge construction;
- load/setup path for restore apply calls.

First-slice bounds:

- restore state is fixed size and does not grow;
- restore validation scans are bounded by configured input record count and
  destination binding capacity;
- restore calls do not allocate;
- restore calls do not load, decode, upload, stream, resolve packages, or query
  resource paths;
- no dynamic maps, string keys, global mutable caches, reflection lookup,
  component factories, or hidden registries in measured paths;
- diagnostics or logs are not required for behavior.

Required deterministic signals:

- restore attempt count;
- restored binding count;
- rejected record count;
- rollback count;
- failed operation count;
- last restore status;
- last binding status;
- last resource status;
- allocation/accounting status.

Blocking conditions:

- requiring stream parsing inside the restore bridge;
- requiring resource loading, package lookup, file IO, decoder, uploader,
  renderer, audio, script, actor/component behavior, gameplay, save, editor,
  tools, reports, or original-game data for behavior;
- string lookup, dynamic maps, container growth, component factory lookup, or
  global mutable caches in measured paths;
- mutating active bindings before complete input validation;
- modifying `WorldInstance` to include restore, component-resource binding, or
  `YuResource` storage;
- modifying `YuResource` core to include or own `YuWorld`;
- copying UE or Unity public API shape or source code into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldComponentResourceBindingRestoreBridge_RestoresRecordsInInputOrder`
- `WorldComponentResourceBindingRestoreBridge_RestoresEmptyInputWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsNullDestinationWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsNullAttachmentSourceWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsNullRegistryWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsNullInputWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsInvalidWorldIdWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsInvalidComponentTypeWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsInvalidComponentSlotWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsMissingAttachmentWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsInvalidResourceHandleWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsStaleResourceHandleWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsResourceTypeMismatchWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsDuplicateInputWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsDestinationCapacityOverflowWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_RejectsNonEmptyDestinationWithoutMutation`
- `WorldComponentResourceBindingRestoreBridge_AcquiresOnlyAfterPreflight`
- `WorldComponentResourceBindingRestoreBridge_ResourceAcquireFailureDoesNotPartiallyRestore`
- `WorldComponentResourceBindingRestoreBridge_SnapshotReportsCountsAndLastStatus`
- `WorldComponentResourceBindingRestoreBridge_NoActorComponentPayloadOrLifecycle`
- `WorldComponentResourceBindingRestoreBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency`
- `WorldComponentResourceBindingRestoreBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency`
- `WorldComponentResourceBindingRestoreBridge_WorldInstanceCoreRemainsRestoreFree`
- `WorldComponentResourceBindingRestoreBridge_ResourceCoreRemainsWorldFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

## Allowed First Slice

If this gate is approved for first slice, the first implementation slice may
create or update only:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingRestore*.h
Src/YuEngine/World/Src/WorldComponentResourceBindingRestore*.cpp
Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingBridge.h
Src/YuEngine/World/Src/WorldComponentResourceBindingBridge.cpp
Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceRegistry.h
Src/YuEngine/Resource/Src/ResourceRegistry.cpp
Tests/World/WorldTests.cpp
Tests/Resource/ResourceTests.cpp
CMakeLists.txt
```

Resource files are allowed only for a minimal const handle/type validation helper
needed to preflight restore without mutation. This approval does not authorize
resource loading, package lookup, file IO, cache state, async operations, or any
`YuWorld` dependency in `YuResource`.

The `WorldComponentResourceBindingBridge` files are allowed only for minimal
const inspection or validation helpers needed by restore preflight. This
approval does not authorize changes to `WorldComponentResourceBindingSnapshot*`,
`WorldComponentAttachmentSnapshot*`, component payload storage, serialized
parsing, or attachment ownership behavior.

It may not modify `WorldInstance` core ownership, `YuObject`, `YuScript`,
`YuSerialize`, File, Package, RHI, Audio, Input, UI, tools, reports, or Game
Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine responsibility separation, restore versus
  snapshot read boundaries, Resource validation usage, performance constraints,
  and AGENTS compliance before approval.
- 博丽灵梦 should review implementability of validation-before-mutation,
  optional ResourceRegistry const validation, rollback requirements, public
  status vocabulary, and no-mutation failure requirements.
- 雾雨魔理沙 should review test completeness, CMake/CTest registration
  boundaries, and whether non-empty destination, stale resource, acquire
  failure, no partial restore, and no forbidden dependency tests are enforceable
  before any implementation task is created.
