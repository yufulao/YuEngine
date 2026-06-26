# YuEngine L1 Runtime Core Matrix

Status: ENG-178A governance document
Baseline: `origin/main@31ebbb2ac77cc422acbecd3209fecf51567c48a0`
Aligned documents:

- `docs/YUENGINE_L0_COMPLETION_MATRIX.md`
- `docs/YUENGINE_BRIDGE_AUDIT.md`

Source plan: `docs/YUENGINE_L0_L1_EXECUTION_PLAN.md` sections 6, 10, 11, and 12
Scope: L1 runtime subsystem ownership, dependencies, forbidden scope, required
evidence, vertical sample linkage, and deferred or environment blockers

## 1. Purpose

This document is the required L1 runtime core matrix from `L1-GOV-001`.
It is a governance document, not implementation proof by itself. It records the
current L1 runtime core state after the L0 completion matrix, bridge audit, and
ENG-176 vertical sample evidence.

State values:

- `Done`: the current repository has value-contract code and reproducible fast
  fixture evidence for this row.
- `FirstSlice`: the boundary exists, but a later acceptance document, hardware
  run, release run, or stronger cross-module evidence must close it.
- `StageClose`: the document and fast evidence exist, but final closure still
  requires stage-close VQ, release validation, or target-machine execution.
- `BlockedByEnv`: the code path exists, but target hardware, local dependency,
  driver, or OS state is required before it can be called complete.
- `Deferred`: accepted as outside the current L1 closure scope.
- `Rework`: known mismatch with the required L1 boundary.

This matrix does not replace `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` or
`docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md`. Those documents now consume
this matrix and must not redefine subsystem boundaries.

## 2. Global Test Route

Focused L1 runtime validation:

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate -R "^(Kernel_RuntimeApp|Object_|WorldObject|WorldTransform|WorldComponent|WorldResource|WorldIdentityBaseline|WorldScene|Asset_|RenderScene_|AudioScene_|Input_CommandMapper|Serialize_|Script_RuntimePhaseDispatch|Diagnostics_RuntimeCounters|Diagnostics_OverlayHook|Sample_L1Vertical)" --output-on-failure
```

Filtered integration validation:

```powershell
ctest --preset windows-dev-gate --output-on-failure
ctest --preset windows-l0-gate --output-on-failure
```

Stage-close validation:

```powershell
ctest --preset windows-fast-gate --output-on-failure
ctest --preset windows-release-gate --output-on-failure
ctest --preset windows-hardware-smoke --output-on-failure
ctest --preset windows-strict-hardware-smoke --output-on-failure
```

Hardware rows from the L0 matrix remain `BlockedByEnv` unless the target
production Windows machine proves D3D11, XAudio2, XInput, and integrated
HardwareFrameHost paths. L1 value-contract tests may consume explicit
unavailable statuses, but they must not count missing hardware as a runtime
success.

## 3. Runtime Core Matrix

| Subsystem | Backlog IDs | Owner / responsibility | Dependencies | Allowed L1 dependencies | State | Required tests / evidence | Vertical sample linkage | Deferred / env blockers | Forbidden scope | Next action |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Governance and scope freeze | `L1-GOV-001..003` | Architecture owner defines L1 rows, freezes broad World bridge expansion, and decides L1 vertical sample scope | `L0-GOV-001`, `L0-GOV-002`, `L0-SAMPLE-008` or documented blocker | All L1 subsystem rows may reference this matrix | FirstSlice | This document exists and is accepted; follow-on docs 4/5 consume it | Gates `L1-SAMPLE-001..012` acceptance docs plus `docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md` | `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`, `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md`, and `docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md` exist; stage-close command evidence remains, including runtime visual scene proof | New broad World/GameAdapter/UI/gameplay tasks, old package runtime proof, editor/Web/UI/input dependency for pure runtime visual proof | Keep governance rows synchronized with sample acceptance docs and the runtime visual foundation ladder |
| Kernel / game loop | `L1-KERN-001`, `L1-KERN-004` | Kernel/runtime owner owns `RuntimeApp` lifecycle, zero-world loop, shutdown, and L0 status propagation | Kernel services, L0 FrameHost failure values | `FrameContext`, diagnostics sink values, input snapshot values | Done | `Kernel_RuntimeAppZeroWorld_FixedFrameLoop`, `Kernel_RuntimeAppUpdateFailure_PropagatesAndShutsDown` | `L1-SAMPLE-002` boots runtime and enters fixed-frame loop | Target hardware failures remain lower L0 statuses | Gameplay update policy, scene loading, UI/title behavior, old runtime service state | Keep RuntimeApp project-independent and use focused Kernel tests in VQ |
| Frame context / time | `L1-KERN-002`, `L1-KERN-003` | Runtime owner defines explicit frame phases and caller-owned `FrameContext` values | RuntimeApp phase trace, Input command snapshot, Diagnostics channel | World update, Resource commit, RenderScene, AudioScene, diagnostics counters | Done | `Kernel_RuntimeAppFrameContext_ExposesValueContract`; phase consumers in Sample L1 tests | `L1-SAMPLE-002`, `L1-SAMPLE-005`, and diagnostics sample validation route | None for value-contract tests | Global mutable business state, project-specific pause policy, hidden singleton time service | Keep frame phases as values consumed by L1 rows |
| Object registry | `L1-OBJ-001`, `L1-OBJ-004`, `L1-OBJ-005` | Object owner owns generation handles, acquire/release, retire, destroy invalidation, and hot-path query smoke | Current `YuObject`; World object identity bridge | World identity baseline, scene restore gates | Done | `Object_*`, `Object_HotPathValidateSmoke_DoesNotGrowRuntimeCounts`, `WorldIdentityBaseline_*` | `L1-SAMPLE-003` deterministic object graph and `L1-SAMPLE-009` cleanup proof | None for fixture tests | World owning Object internals, script callbacks, gameplay lifecycle | Keep object lifetime evidence current before active restore expansion |
| Component identity | `L1-OBJ-002`, `L1-OBJ-005` | World/component bridge owner owns component type, slot, attachment, query, and snapshot records | World object IDs, component attachment bridge | Object handles, scene assembly, Serialize value streams | Done | `WorldComponentAttachmentBridge_*`, `WorldComponentQueryBridge_*`, `WorldComponentAttachmentSnapshotBridge_*`, `WorldIdentityBaseline_*` | `L1-SAMPLE-003` component record creation and cleanup | None for fixture tests | Actor behavior lifecycle, reflection, script callbacks, render/audio behavior | Keep component bridge value-only and bounded |
| Transform records | `L1-OBJ-003`, `L1-OBJ-005` | World/transform bridge owner owns POD transform records and query/update storage | World object IDs, object lifetime validation | RenderScene value references, scene manifest streams | Done | `WorldTransformBridge_*`, `WorldSceneObjectTransformManifestStreamBridge_*`, `WorldSceneObjectTransformRestoreBridge_*`, `WorldIdentityBaseline_*` | `L1-SAMPLE-003` deterministic transform records and `L1-SAMPLE-006` render submission | Transform hierarchy policy is deferred | Transform hierarchy, physics, scene graph policy, animation/skinning | Keep transform as POD sidecar until a separate hierarchy gate exists |
| World lifecycle | `L1-WORLD-001`, `L1-WORLD-004`, `L1-WORLD-005` | World owner owns deterministic create/update/destroy, active restore gate, and cleanup policy | Object registry, component/transform bridges, Resource handles | Serialize streams, Script phase dispatch, RuntimeApp phases | Done | `WorldIdentityBaseline_*`, `WorldSceneActiveRestoreGateBridge_*`, `WorldSceneObjectTransformRestoreBridge_*` | `L1-SAMPLE-002`, `L1-SAMPLE-008`, and `L1-SAMPLE-009` | Future broader active mutation stays gated by validation-before-mutation evidence | GameAdapter semantics, scene file loading as first step, hidden partial mutation | Treat restore gate as the expansion control point |
| Scene assembly | `L1-WORLD-002`, `L1-WORLD-003`, `L1-WORLD-005` | World scene owner owns caller-provided assembly records, decoded restore plan, apply-time proof, and rollback status | Object/component/transform records, Resource handles, Serialize streams | RenderScene, AudioScene, Asset binding values | Done | `WorldSceneAssemblyBridge_*`, `WorldSceneAssemblyManifestStreamBridge_*`, `WorldSceneDecodedRestorePlanBridge_*`, `WorldSceneApplyTimeRestoreProofBridge_*`, `WorldSceneRecordValueStreamBridge_*` | `L1-SAMPLE-001`, `L1-SAMPLE-003`, `L1-SAMPLE-008` | Active restore expansion requires proof/gate tests to stay green | Resource loading inside restore, object construction by Serialize, save-game policy | Keep scene assembly as explicit value records |
| Asset manager | `L1-ASSET-001..005` | Asset owner owns runtime asset handles, states, bounded dependencies, ready records, and release semantics | L0 Resource, Streaming, AudioResource, Resource residency/upload rows | RenderScene, AudioScene, vertical sample manifest | Done | `Asset_RegisterRuntimeAsset_ReturnsStableHandleAndState`, `Asset_DependenciesTraverseBoundedAndRejectCycle`, `Asset_TextureReadyRecordUsesStreamingResultWithoutOwningDevice`, `Asset_AudioReadyRecordUsesImportRecordWithoutOwningDevice`, `Asset_RuntimeFixture*` | `L1-SAMPLE-004`, `L1-SAMPLE-006`, `L1-SAMPLE-007` | D3D11/XAudio2 hardware proof remains L0 `BlockedByEnv` | Owning RHI/Audio devices, editor import UX, old package compatibility, gameplay asset semantics | Keep Asset over Resource as a runtime value layer |
| Render scene | `L1-RSCENE-001..006`, `L1-CAMERA-001..003`, `L1-GEOM-001`, `L1-MAT-001` | RenderScene owner owns render entity/camera records, full-list visibility, RenderCore packet assembly, multi-entity runtime visual submission, and failure statuses | Object/transform records, Asset handles, L0 RenderCore/RHI, runtime camera/geometry/material/resource records | RuntimeApp frame context, vertical sample route, runtime visual foundation route | Done | Value-contract rows plus runtime visual and RuntimeAsset rows pass: `RenderScene_L1Sample016EmitsPerspective3DPrimitiveCameraTweenArtifacts`, `RenderScene_L1Sample018BlendsTransparentRuntimePanel`, `RenderScene_L1Sample019RendersTexturedGlassEmissiveMetalMaterials`, `RuntimeAssetData_D3D11Hardware_CookedRecordsDriveDeviceBackedVisualProof` | `L1-SAMPLE-006` submits render scene through value contracts; `L1-SAMPLE-011/012` and RuntimeAssetData now consume the runtime visual foundation ladder through RenderScene/RenderCore/RHI | Missing camera, model geometry, material texture slots, shader/pipeline, RenderScene multi-entity, RenderCore multi-draw, capture, or resource resolution remains an exact failure layer, not `BlockedByEnv` | D3D11 includes in public L1 headers, gameplay scene policy, lighting, animation/skinning, UI rendering, editor viewport, standalone D3D sample bypass | Keep runtime visual and RuntimeAsset focused tests green while product sample closure continues |
| Audio scene | `L1-ASCENE-001..005` | AudioScene owner owns sound asset binding, source records, fixed bus routing, queue request submission, and failure states | Asset manager, AudioResource import records, L0 Audio PCM queue | RuntimeApp frame context, vertical sample route | Done | `AudioScene_PlayingSourceBuildsPcmQueueRequest`, `AudioScene_BusRoutingMapsFixedBusIdsToQueueIds`, `AudioScene_MissingBackendReturnsExplicitStatus`, `AudioScene_NoNativeOrUpperDependency` | `L1-SAMPLE-007` submits audio or explicit unavailable status | XAudio2 callback proof remains L0 hardware `BlockedByEnv` | BGM/SE project service IDs, UI volume menu, DSP graph, audio editor, blocking IO | Keep unavailable backend explicit and non-fatal for L1 sample |
| Input mapping | `L1-INPUT-001..004` | Input owner owns action map, input context, frame-indexed command snapshot, keyboard/XInput fixture behavior | L0 Input replay/action snapshots, FrameContext frame index | RuntimeApp frame context, vertical sample state update | Done | `Input_CommandMapper_KeyboardBuildsFrameCommandSnapshot`, `Input_CommandMapper_XInputStyleAxisBuildsCommandSnapshot`, `Input_CommandMapper_KeyboardAndXInputFixturesStayRuntimeOnly`, focus/invalid event tests | `L1-SAMPLE-005` maps input to command snapshot only | Real XInput gamepad smoke remains L0 hardware `BlockedByEnv` | Gameplay command execution, UI navigation policy, text input framework, World/Script callbacks | Keep command snapshot replayable and hardware skips documented |
| Serialization / save boundary | `L1-SER-001`, `L1-SER-002`, `L1-SER-004` | Serialize owner owns value streams, scene assembly record roundtrip, version/status handling, and save/profile boundary | `YuSerialize`, World scene record value streams | Runtime config, sample reload, external File/tool persistence later | Done | `Serialize_*`, `WorldSceneRecordValueStreamBridge_*`, `WorldSerializeSnapshotBridge_*`, sample roundtrip test | `L1-SAMPLE-008` serializes and reloads sample state | File persistence/profile UX is deferred outside Serialize core | Save-game business policy, original save compatibility, object construction in Serialize, File/Package dependency in Serialize core | Keep persistence policy outside Serialize and document sample acceptance separately |
| Config / profile boundary | `L1-SER-003`, `L1-SER-004` | Runtime/config owner owns caller-owned runtime config records and unsupported-version behavior | RuntimeApp value records, Serialize core | Diagnostics and sample validation route may read values | Done | `Serialize_RuntimeConfigStream_RoundTripsCallerOwnedConfigBoundary`, `Serialize_RuntimeConfigStream_RejectsUnsupportedVersionWithoutMutation`, `Serialize_RuntimeConfigStream_KeepsPersistencePolicyOutsideCore` | Supports `L1-SAMPLE-010` validation route documentation | Actual user profile persistence remains deferred | File/Package dependency inside Serialize, product settings UI, old save/profile compatibility | Keep config as value stream until a separate persistence gate |
| Script native bridge runtime adapter | `L1-SCRIPT-001..003` | Script owner owns native call registry, caller-owned value slots, runtime phase dispatch adapter, and explicit failure states | `YuScript`, World phase trace values | World phase dispatch adapter, RuntimeApp phases | Done | `Script_RuntimePhaseDispatch_*`, `WorldScriptDispatchBridge_*` | Not required by the current project-independent vertical sample; available for future script-driven sample rows | VM/bytecode evidence is deferred | Reflection, original-game service state, script-owned World/Resource/File access, gameplay script semantics | Keep adapter narrow and do not make World core depend on Script core |
| Animation runtime foundation | `L1-ANIM-001..005` | Animation owner owns bounded runtime clip/track/keyframe records, interpolation sampler, FrameContext time sampling, transform application, and failure states | FrameContext time, Object/Transform records | RenderScene consumes sampled transform values; vertical visual sample may use animation output | Done | RuntimeAssetData camera/animation descriptor tests pass: scene `cameras=`/`cameraN=` records, animation `clips=`/`tracks=`/`keyframes=` tables, dependency validators, disk sampling, and transform apply are covered by the focused RuntimeAssetData command set | `L1-SAMPLE-011` and RuntimeAsset visual routes can drive transforms through runtime animation records; failures report exact animation/data layers | None as environment blocker; future richer animation authoring remains out of scope | Editor timeline, gameplay state machine, skeletal skinning, blend tree, montage graph, physics, UI animation authoring | Keep descriptor/validator/sampler tests in stage-close VQ |
| Runtime diagnostics | `L1-DIAG-001..003` | Diagnostics owner owns bounded runtime counters, disabled equivalence, and optional overlay hook proposal | FrameContext, runtime subsystem counters, Diagnostics channel | Sample validation route, tooling plane only | Done | `Diagnostics_RuntimeCounters_*`, `Diagnostics_OverlayHookProposalStaysOptionalToolingPlane`, `Diagnostics_OverlayHookRejectsRuntimeDependency`, `Diagnostics_OverlayHookDisabledDoesNotChangeRuntimeValues` | `L1-SAMPLE-010` validation route and sample diagnostics counters | Visual overlay implementation is deferred tooling, not runtime dependency | Report JSON as core API, unbounded logging, overlay required for correctness | Keep diagnostics optional and behavior-equivalent when disabled |
| L1 vertical sample prep | `L1-SAMPLE-001..012`, `L1-VIS-001..006` | Sample owner aggregates L1 value-contract proof for manifest, runtime boot, object graph, asset/render/audio routes, input, serialize, cleanup, validation, and pure runtime visual scene closure | All L1 subsystem rows above plus L0 explicit hardware statuses and the runtime visual foundation ladder from camera/geometry/material/animation through RenderScene/RenderCore/RHI/capture | RuntimeApp, World, Asset, Input, RenderScene, AudioScene, Serialize, Diagnostics, runtime camera/material/resource/animation records | StageClose | Existing value rows pass, runtime visual foundation rows pass through RVF-016/018/019, and RuntimeAssetData device-backed visual/product-run rows prove disk-loaded data through RenderScene/RenderCore/RHI; remaining stage-close evidence is sample command/release audit and strict XInput target hardware proof | This is the vertical sample linkage row; visual closure now has checked-in focused tests and D3D11 RuntimeAsset hardware smoke evidence | `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md` and `docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md` exist; real XInput remains L0 `BlockedByEnv`, and release/native sample runs remain stage-close evidence | TouhouNewWorld business logic, old package runtime parsing, UI/GameAdapter/gameplay, editor/Web/UI/input dependency, manual-only screenshot proof, committed generated output | Continue with release/sample command audit and strict XInput grading without reopening runtime visual foundations |

## 4. Dependency And Scope Rules

The current L1 dependency direction is:

```text
RuntimeApp / FrameContext
    -> World/Object/Component/Transform value records
    -> Asset Manager over L0 Resource/Streaming/AudioResource
    -> Input command snapshots
    -> RenderScene and AudioScene contract queues
    -> Camera, geometry, material, animation, and capture foundation floors
    -> runtime visual scene sample through resource/material/model, camera,
       animation, and capture values
    -> Serialize/config value streams
    -> bounded Diagnostics counters
    -> project-independent L1 vertical sample
```

The direction must not be inverted. Lower modules must not depend on World, UI,
Script, Project, or GameAdapter types. Bridge adapters may coordinate public
values, but they must not own both sides' lifecycle.

## 5. Deferred And Environment Blockers

| Blocker | Classification | Effect on this matrix | Required follow-up |
| --- | --- | --- | --- |
| Target D3D11 device/swapchain/draw proof | `PASS on current target` | L1 RenderScene and RuntimeAsset device-backed rows have current hardware evidence; unsupported machines must still report explicit skips/statuses | Keep `windows-hardware-smoke` in final audit and rerun strict target hardware where configured |
| Target XAudio2 callback proof | `PASS on current target` | L1 AudioScene can submit queue requests and L0 hardware callback proof is current on this machine | Keep hardware audio smoke in final audit |
| Target XInput gamepad proof | `BlockedByEnv` | L1 Input mapping fixtures can pass, but real gamepad smoke remains L0 hardware evidence | Run XInput hardware smoke with a connected target gamepad |
| Sample Ogg/Vorbis local dependency | `BlockedByEnv` | L1 value tests are not blocked; sample command acceptance must grade the local dependency explicitly | ENG-178B/C should document Debug/Release/sample-smoke skip rules |
| L0 sample acceptance command proof | `StageClose` | The L0 sample acceptance document exists; final closure still requires recorded Debug/Release sample command evidence and generated-output hygiene | Run the required rows from `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` during VQ or final audit |
| L1 vertical sample release proof | `StageClose` | The L1 vertical sample acceptance document exists; fast value rows pass, while release/native proof remains stage-close evidence | Run the Release L1 sample route and native sample rows from `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md` and `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` |
| L1 runtime visual foundation proof | `PASS on current target` | The shallow-to-deep camera, geometry, scene placement, material/texture, shader/pipeline, animation interpolation, RenderScene, RenderCore/RHI, capture, material blend, and missing-layer diagnostic floors have focused test evidence; remaining closure work is sample/release packaging and XInput hardware grading | Keep RVF/RuntimeAsset focused tests in final audit and do not use viewer/manual image proof as acceptance |

## 6. L1 Closure Candidate Gate

The repository may enter L1 closure-candidate review only when all of these are
true:

1. This matrix is committed and accepted.
2. `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md` is committed and accepted.
3. `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md` is committed and accepted.
4. Runtime loop, FrameContext, Object, Component, Transform, World, Asset,
   RenderScene, AudioScene, Input, Serialize, Script adapter, and Diagnostics
   rows stay green in focused L1 tests.
5. `windows-dev-gate` and `windows-l0-gate` pass after the docs are accepted,
   or any failure is documented as unrelated and blocking.
6. L1 vertical sample evidence passes through value-contract tests and has a
   documented Debug/Release/sample-smoke route.
7. The runtime visual foundation ladder passes from shallow to deep: camera
   capture, geometry/model, scene placement, material texture slots,
   shader/pipeline binding, animation interpolation, transform application,
   RenderScene/RenderCore/RHI draw, orbit capture, and missing-layer
   diagnostics.
8. The pure runtime visual scene route captures the required cube/cylinder/cone
   orbit set, or reports the exact missing runtime layer with no editor, Web,
   UI, or input dependency.
9. Hardware smoke has no unexpected failure; environment skips are graded by
   the L0 matrix and the sample acceptance documents.

## 7. Forbidden Expansion Freeze

Until required documents 3/4/5 pass verification:

- Do not start new broad World, GameAdapter, UI, or gameplay implementation
  tasks.
- Do not use old package runtime compatibility as runtime or sample proof.
- Do not expose D3D11, XAudio2, XInput, Win32, or other backend-native types
  through public L1 headers.
- Do not require report JSON, capture files, manual screenshots, or manual
  audio listening for runtime correctness.
- Do not count hardware absence as `Done`.
- Do not make diagnostics, debug overlay, scripts, or sample helpers control
  core runtime behavior.
