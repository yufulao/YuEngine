# P3-GATE-014: World Component Resource Binding Bridge

Status: Approved for first slice
Requested decision: `APPROVED_FOR_FIRST_SLICE`
Current decision: `APPROVED_FOR_FIRST_SLICE`
Owner: 八云紫
Reviewers: 八云蓝, 博丽灵梦, 雾雨魔理沙
Depends on: P1-GATE-006, P3-GATE-010, P3-GATE-011, P3-GATE-012
Related decisions: ADR-0002, ADR-0005, ADR-0006, ADR-0009, ADR-0014
Source baseline: `7dc34d7`

## Layer

L5 world runtime adapter over component attachment records and the L4 Resource
identity boundary.

This gate proposes a narrow YuWorld component-resource binding sidecar. The
first slice binds an existing component attachment tuple to an already
registered `ResourceHandle` through a caller-owned `ResourceRegistry`. It
validates the attachment tuple through caller-owned component attachment data
and validates the resource handle through caller-owned resource state. It is not
a component payload store, component behavior system, resource loader, package
resolver, renderer material system, audio scene, editor inspector, save policy,
or Game Adapter feature.

The slice must keep `WorldInstance` ownership unchanged. It must not query or
mutate `WorldInstance` membership. It must not construct resources, load files,
resolve packages, decode payloads, upload render/audio data, dispatch script,
own component lifetime, or copy UE or Unity API shape or source code.

## Reference Boundary

UE5 local reference paths:

- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Components\ActorComponent.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\Engine\AssetManager.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Public\StreamableManager.h`
- `D:\app\Epic Games\UE_5.5\Engine\Source\Runtime\Engine\Classes\GameFramework\Actor.h`

Responsibility conclusion:

- UE component instances may own asset references, participate in construction,
  registration, ticking, streaming, render state, physics state, replication, and
  editor metadata. YuEngine must not copy that public API shape or source code
  in this slice.
- The useful boundary lesson is that component attachment identity, resource
  handle identity, loading policy, upload policy, and component behavior
  lifetime must remain separate.
- YuEngine first slice should prove only deterministic binding between an
  already known component attachment tuple and an already registered resource
  handle.

Unity responsibility reference:

- Unity separates GameObject and Component attachment, asset references,
  runtime loading, renderer/audio consumption, MonoBehaviour lifecycle, scene
  serialization, and editor inspection.
- YuEngine must not copy Unity GameObject, Component, asset reference,
  addressable, or serialized-object API shape. This gate only introduces a
  fixed-capacity binding adapter over YuEngine world component attachment and
  resource handle records.

## Owns

This gate owns a future first slice for:

- a `WorldComponentResourceBindingBridge` adapter;
- explicit world component resource binding statuses;
- POD component resource binding descriptors, records, results, and snapshots;
- binding one existing `(WorldObjectId, WorldComponentTypeId,
  WorldComponentSlotId)` attachment tuple to one already registered
  `ResourceHandle` with an expected `ResourceTypeId`;
- acquiring the resource handle only after all local component attachment and
  resource validation succeeds;
- releasing the acquired handle during successful remove or clear;
- rejecting invalid world object IDs, component type IDs, component slot IDs,
  resource handles, stale resource handles, wrong resource types, duplicate
  component resource bindings, and capacity overflow without mutation;
- rejecting missing attachment tuples without acquiring a resource reference;
- read-only query by component attachment tuple;
- deterministic clear in stored slot order;
- POD counters for capacity, active bindings, acquired handles, released
  handles, cleared bindings, query count, failed operations, last bridge status,
  last resource status, and allocation/accounting status;
- no hidden allocation in measured bind/remove/clear/query paths;
- tests proving `WorldInstance` core remains component-resource-free and
  `YuResource` core remains World-free.

The implementation may use existing component attachment query or export
vocabulary as read-only validation input. It must not expose mutable component
attachment storage and must not introduce component payload ownership.

## Does Not Own

This gate does not own:

- UE-style `Actor`, `ActorComponent`, `SceneComponent`, component construction,
  registration callbacks, ticking, replication, render state, physics state,
  editor details panels, or asset streaming APIs;
- Unity-style GameObject or Component API shape, MonoBehaviour lifecycle,
  prefab serialization, addressables, asset database integration, renderer
  components, or editor inspector behavior;
- component payload storage, component factories, reflection, property bags,
  generated schemas, script object lifetime, behavior dispatch, event routing,
  gameplay services, animation, physics, render scene, audio scene, UI, save
  slots, scene streaming, tools, reports, profiling UI, or Game Adapter
  behavior;
- File IO, VFS, package manifests, package entry resolution, resource discovery,
  resource loading, decoding, upload, cache eviction, hot reload, or
  original-game resource lookup;
- object construction or `YuObject` handle ownership;
- copied UE or Unity public API shape, source code, macro style, reflection
  vocabulary, lifecycle naming, asset path vocabulary, or async-load vocabulary.

## Dependencies

Allowed dependencies for the first slice:

- C++ standard library;
- `YuMemory` only for accepted allocation/accounting vocabulary;
- `YuWorld` public `WorldObjectId`, `WorldComponentAttachment`,
  `WorldComponentQuery`, world status, snapshot, and value vocabulary;
- `YuResource` public `ResourceRegistry`, `ResourceHandle`,
  `ResourceTypeId`, `ResourceStatus`, and snapshot value types.

Forbidden dependencies:

- `YuObject`, `YuScript`, `YuSerialize`, `YuFile`, `YuPackage`, `YuThread`,
  `YuPlatform`, `YuDiagnostics`, `YuRHI`, `YuAudio`, `YuInput`, UI, tools,
  reports, or game adapter modules;
- resource path strings, package keys, file paths, original-game resource names,
  original-game component names, or original-game data as API shape.

The first implementation may add `YuResource` as a declared `YuWorld`
dependency only for `WorldComponentResourceBindingBridge` adapter files. Tests
must prove `WorldInstance.h` and `WorldInstance.cpp` do not include component
resource binding or `YuEngine/Resource`, and `Src/YuEngine/Resource/**` does not
include `YuEngine/World`.

## Lifecycle

First-slice lifecycle:

1. Caller owns and initializes a bounded `ResourceRegistry`.
2. Caller owns a bounded component attachment bridge or read-only query bridge.
3. Caller registers synthetic resource descriptors in the resource registry.
4. Caller creates a `WorldComponentResourceBindingBridge` with fixed binding
   capacity.
5. Caller binds a non-zero `WorldObjectId`, non-zero
   `WorldComponentTypeId`, and non-zero `WorldComponentSlotId` tuple to a valid
   `ResourceHandle` with an expected `ResourceTypeId`.
6. The bridge validates the attachment tuple against caller-owned attachment
   data and validates the resource handle against the caller-owned registry.
7. The bridge acquires the resource handle only after every local validation
   succeeds.
8. The bridge stores the binding only after resource acquire succeeds.
9. Caller may query the component resource binding snapshot by attachment tuple.
10. Caller may remove one binding; the bridge releases the acquired handle and
    clears the binding only when release succeeds.
11. Caller may clear all bindings; the bridge releases each acquired handle in
    deterministic slot order.
12. The bridge never constructs, starts, stops, updates, queries, mutates, or
    owns `WorldInstance`, component payloads, File, Package, renderer, audio,
    script, UI, tools, reports, or Game Adapter state.

Failure behavior:

- null attachment/query bridge pointers return explicit status and do not
  acquire or mutate;
- null resource registry pointers return explicit status and do not mutate;
- invalid world object IDs, component type IDs, or component slot IDs return
  explicit status and do not acquire resources;
- missing attachment tuples return explicit status and do not acquire resources;
- invalid resource handles, stale generation handles, and wrong resource types
  return explicit status and do not mutate binding slots;
- duplicate component resource bindings return explicit status and do not
  acquire a second resource reference;
- binding capacity overflow returns explicit status and does not acquire a
  resource reference;
- remove of a missing component binding returns explicit status and does not
  call `ResourceRegistry::Release`;
- release failure returns explicit status and does not clear the local binding;
- clear failure records explicit status and stops before silently dropping
  unreleased bindings;
- clear releases bindings in deterministic slot order; slots successfully
  released before the first failure may be cleared and counted, while the failed
  slot and every unprocessed slot remain bound;
- after a successful `ResourceRegistry::Release`, the bridge must not perform
  another fallible operation before clearing that local POD binding;
- failed bridge operations do not mutate `WorldInstance`, component attachment
  data, `YuResource` type definitions, or component payload state.

## Inputs

- caller-owned `ResourceRegistry`;
- caller-owned component attachment bridge or component query bridge;
- `WorldObjectId`;
- `WorldComponentTypeId`;
- `WorldComponentSlotId`;
- `ResourceHandle`;
- expected `ResourceTypeId`;
- explicit bridge capacity.

## Outputs

- explicit world component resource binding statuses;
- `ResourceStatus` from the most recent registry operation;
- POD component resource binding snapshot values;
- bridge snapshot counters;
- allocation/accounting signal using accepted `YuMemory` vocabulary.

No file path, package key, resource path string, component class name,
reflection name, script call ID, render handle, physics handle, original-game
name, report text, log text, wall-clock time, pointer value, rendered frame
data, or audio output may define behavior.

## Performance Constraints

Budget classes:

- setup path for bridge construction;
- load/setup path for bind/remove/clear;
- frame path only for read-only binding queries.

First-slice bounds:

- binding storage is fixed capacity and does not grow;
- attachment tuple lookup is bounded by configured attachment/query capacity;
- component resource binding lookup is bounded by configured binding capacity;
- bind/remove/clear use explicit `ResourceRegistry` acquire/release calls and
  do not allocate;
- resource lookup remains handle/generation based, not string/path based;
- no dynamic maps, string keys, global mutable caches, reflection lookup,
  component factories, or hidden registries in measured paths;
- diagnostics or logs are not required for behavior.

Required deterministic signals:

- binding capacity;
- active binding count;
- acquired binding count;
- released binding count;
- cleared binding count;
- query count;
- failed operation count;
- last bridge status;
- last resource status;
- allocation/accounting status.

Blocking conditions:

- File IO, package lookup, resource path lookup, decoder, uploader, renderer,
  audio, script, actor/component behavior, gameplay, save, editor, tools,
  reports, or original-game data required for behavior;
- string lookup, dynamic maps, container growth, component factory lookup, or
  global mutable caches in measured paths;
- modifying `WorldInstance` to include or own component resource bindings or
  `YuResource`;
- modifying `YuResource` core to include or own `YuWorld`;
- copying UE or Unity public API shape or source code into YuEngine.

## Required Tests

Fast gate tests required before the slice can be considered complete:

- `WorldComponentResourceBindingBridge_BindValidAttachmentResource_AcquiresHandle`
- `WorldComponentResourceBindingBridge_BindRejectsNullAttachmentSourceWithoutMutation`
- `WorldComponentResourceBindingBridge_BindRejectsNullRegistryWithoutMutation`
- `WorldComponentResourceBindingBridge_BindRejectsInvalidWorldIdWithoutMutation`
- `WorldComponentResourceBindingBridge_BindRejectsInvalidComponentTypeWithoutMutation`
- `WorldComponentResourceBindingBridge_BindRejectsInvalidComponentSlotWithoutMutation`
- `WorldComponentResourceBindingBridge_BindRejectsMissingAttachmentWithoutMutation`
- `WorldComponentResourceBindingBridge_BindRejectsInvalidResourceHandleWithoutMutation`
- `WorldComponentResourceBindingBridge_BindRejectsStaleResourceHandleWithoutMutation`
- `WorldComponentResourceBindingBridge_BindRejectsTypeMismatchWithoutMutation`
- `WorldComponentResourceBindingBridge_BindRejectsDuplicateComponentBinding`
- `WorldComponentResourceBindingBridge_BindRejectsCapacityOverflowWithoutMutation`
- `WorldComponentResourceBindingBridge_RemoveReleasesHandle`
- `WorldComponentResourceBindingBridge_RemoveRejectsNullRegistryWithoutMutation`
- `WorldComponentResourceBindingBridge_RemoveRejectsMissingBindingWithoutMutation`
- `WorldComponentResourceBindingBridge_RemoveReleaseFailureKeepsBinding`
- `WorldComponentResourceBindingBridge_ClearReleasesAllHandles`
- `WorldComponentResourceBindingBridge_ClearRejectsNullRegistryWithoutMutation`
- `WorldComponentResourceBindingBridge_ClearReleaseFailurePreservesUnreleasedBindings`
- `WorldComponentResourceBindingBridge_BoundResourceCannotRetireUntilReleased`
- `WorldComponentResourceBindingBridge_QueryReturnsStoredBinding`
- `WorldComponentResourceBindingBridge_QueryIsReadOnlyAndBounded`
- `WorldComponentResourceBindingBridge_UpdatePathDoesNotGrowStorage`
- `WorldComponentResourceBindingBridge_SnapshotReportsCountsAndLastStatus`
- `WorldComponentResourceBindingBridge_DoesNotQueryOrMutateWorldInstance`
- `WorldComponentResourceBindingBridge_NoActorComponentPayloadOrLifecycle`
- `WorldComponentResourceBindingBridge_NoFilePackageLoadDecodeUploadOrGameAdapterDependency`
- `WorldComponentResourceBindingBridge_NoRenderPhysicsAudioInputUiToolOrReportDependency`
- `WorldComponentResourceBindingBridge_WorldInstanceCoreRemainsComponentResourceFree`
- `WorldComponentResourceBindingBridge_ResourceCoreRemainsWorldFree`

Expected command family:

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

## Allowed First Slice

With this gate approved for first slice, the first implementation slice may
create or update only:

```text
Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBinding*.h
Src/YuEngine/World/Src/WorldComponentResourceBinding*.cpp
Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachment.h
Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentBridge.h
Src/YuEngine/World/Src/WorldComponentAttachmentBridge.cpp
Src/YuEngine/World/Include/YuEngine/World/WorldComponentQueryBridge.h
Src/YuEngine/World/Src/WorldComponentQueryBridge.cpp
Tests/World/WorldTests.cpp
CMakeLists.txt
```

It may update `YuWorld` CMake linkage to reference `YuResource` only for the
adapter target boundary.

Attachment/query updates are allowed only for minimal const validation helpers
needed by this adapter. This approval does not authorize changes to
`WorldComponentAttachmentSnapshot*` files or snapshot behavior.

It may not modify `WorldInstance` core ownership, `YuResource` core ownership,
`YuObject`, `YuScript`, `YuSerialize`, File, Package, RHI, Audio, Input, UI,
tools, reports, or Game Adapter behavior.

## Review Notes

- 八云蓝 should review mature-engine responsibility separation, Resource
  boundary usage, performance constraints, and AGENTS compliance before
  approval.
- 博丽灵梦 should review fixed-capacity storage, attachment validation,
  acquire/release atomicity, status vocabulary, and no-mutation failure
  requirements.
- 雾雨魔理沙 should review test completeness, CMake/CTest registration
  boundaries, and whether release-failure and missing-attachment tests are
  enforceable before any implementation task is created.
