# YuEngine Bridge Audit

Status: ENG-177B governance document
Baseline: `origin/main@d17d3b4dbd11f2caa96fad3242a9bf09b38daa3d`
Aligned matrix: `docs/YUENGINE_L0_COMPLETION_MATRIX.md` at ENG-177A
Source plan: `docs/YUENGINE_L0_L1_EXECUTION_PLAN.md` sections 10, 11, and 12
Scope: L0/L1 bridge direction, ownership, lifecycle, failures, bounds, tests, and risk

## 1. Purpose

This document is the required bridge audit from `L0-GOV-002`. It is a
governance document, not new implementation proof. It records the current
bridge surfaces that hand values, handles, snapshots, queues, or lifecycle
signals across YuEngine module boundaries.

The audit is aligned to `YUENGINE_L0_COMPLETION_MATRIX.md`:

- `Done` L0 rows may still have bridge risks, but the value-contract tests and
  deterministic failure model exist.
- `BlockedByEnv` rows have implemented code paths and explicit skip/failure
  states, but target hardware or local dependency evidence is still required.
- A `High` risk row must carry an explicit blocker or evidence gap.
- No L0 bridge in this audit is classified as unresolved `High`; the remaining
  L0 gaps are environment-proof gaps from the completion matrix.

## 2. Risk Rules

| Risk | Meaning | Required action |
| --- | --- | --- |
| `Low` | Adjacent value-only handoff with a single owner and deterministic tests | Document and keep |
| `Medium` | Bounded queue, lifecycle-sensitive handoff, hardware environment edge, or cross-owner value contract | Keep tests current; split adapter from policy before adding behavior |
| `High` | Crosses multiple ownership domains while validating or preparing mutation, or has rollback/cache/lifecycle policy that can partially mutate state | Treat as blocker for future expansion until the named gate/evidence remains green |

## 3. L0 Bridge Audit

| Bridge | Direction | Source owner | Destination owner | Lifecycle owner | Failure statuses | Bounds | Tests | Matrix state | Risk | Required action |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `HardwareFrameHost` | Platform/Input/Audio/RenderCore/RHI adapters into one hardware tick | Platform/Input/Audio/RenderCore/RHI modules keep their own internals | Hardware consumes module public contracts | Hardware owns host initialize/tick/shutdown only | `HardwareFrameHostStatus`, `HardwareFrameHostTickResult`, plus mapped module statuses | Tick requests, event output, gamepad polling, audio chunks, render frame storage | `HardwareFrameHost_*`, `HardwareFrameHost_D3D11IntegratedFrameRuns` | `BlockedByEnv` | `Medium` | Keep as orchestrating adapter only; target hardware smoke remains required before product-done claim |
| `InputBridge` and `InputBridgeWindows` | Platform messages and XInput snapshots to Input events | Platform/Win32/XInput source values | Input owns event queue and snapshots | Input owns bridge init/shutdown and event storage | `InputStatus` including `UnsupportedBackend`, `SourceUnavailable`, `FocusLost`, `OutputBufferFull`, `BackendError` | `InputBridgeDesc::MAX_EVENT_CAPACITY`, gamepad user index, drain output capacity | `Input_Bridge*`, `Input_BridgeXInput*`, `HardwareFrameHost_GamepadPollRejectsInvalidIndex` | `Done` and XInput `BlockedByEnv` | `Medium` | Keep platform-native details private; real gamepad smoke may only skip through explicit unavailable status |
| `AudioPcmStreamQueueCallbackBridge` | Audio PCM stream queue to `AudioCallbackDevice` or test submitter | Audio stream queue owns chunks and handles | Audio callback device consumes submitted chunks | Audio owns queue, callback device owns hardware callback | `AudioStatus` including `DeviceUnavailable`, `BufferSubmitFailed`, `CallbackFailed`, `CallbackTimeout` | Scratch chunk storage, stream queue capacity, callback drain count | `Audio_*`, `AudioScene_*`, hardware `Audio`/`XAudio2` smoke | XAudio2 `BlockedByEnv` | `Medium` | Keep callback bridge free of file/resource parsing and unbounded callback work; target XAudio2 proof is still required |
| `AudioResourcePcmPacketImportBridge` | Resource decoded audio result to AudioResource PCM packet record | Resource owns decoded result record | AudioResource owns PCM packet import records | AudioResource owns import handle lifetime; Resource handle remains external | `AudioResourcePcmPacketImportStatus` plus Resource decode status evidence | Import slots, packet IDs, format/sample/frame/byte validation | `AudioResource_PcmPacketImportBridge_*`, `Audio_PcmSamplePacket_*`, `Audio_PcmStreamQueue_*` | `Done` | `Medium` | Keep codec expansion outside this bridge; Audio must not parse Resource payloads |
| `PackageResourceStagingQueue` | Package load-plan record to File async read completion for Streaming | Package owns load-plan record; File owns async read result | Streaming owns staging queue records | Streaming owns pending/completion slots | `PackageResourceStagingStatus`, `ResourceStatus`, `AsyncFileReadStatus` | `MAX_PACKAGE_RESOURCE_STAGING_REQUEST_COUNT`, completion output capacity | `Streaming_PackageResourceStaging_*` | `Done` | `Medium` | Keep package as value plan provider; do not reintroduce old package runtime behavior |
| `ResourceUploadQueue` | Staging completion to RHI upload request/completion | Streaming owns staging completion | RHI owns backend resource creation; Resource receives completion later | Streaming owns upload pending/completion slots | `ResourceUploadStatus`, `ResourceStatus`, `RhiStatus` | `MAX_RESOURCE_UPLOAD_REQUEST_COUNT`, byte range, completion output capacity | `Streaming_ResourceUpload_*` | `Done` | `Medium` | Keep Resource from owning RHI device lifecycle; hardware backend proof stays separate |
| `ResourceUploadCommitQueue` | RHI upload completion to Resource load/residency commit | Streaming owns upload completion | Resource owns registry state transition | Streaming owns commit pending/completion slots | `ResourceUploadCommitStatus`, `ResourceStatus`, load/residency statuses | `MAX_RESOURCE_UPLOAD_COMMIT_REQUEST_COUNT`, completion output capacity | `Streaming_ResourceUploadCommit_*` | `Done` | `Medium` | Keep commit as explicit value transition, not implicit Resource cache mutation |
| `ResourceStreamingPipeline` | Package/File staging through RHI upload to Resource commit | Streaming composes staging, upload, and commit queues | Resource/RHI receive only their public request values | Streaming owns pipeline progress and snapshot | `ResourceStreamingPipelineStatus` plus child queue statuses | One active request, child queue capacities, completion storage | `Streaming_ResourceStreamingPipeline_FixtureBufferReadUploadCommit` | `Done` | `Medium` | Keep as L0 pipeline adapter; do not add Asset or sample policy here |
| `ResourceDecodedTextureBridge` | Resource decoded texture payload to Streaming/RHI upload request | Resource owns decoded payload and sampled texture metadata | Streaming/RHI consume upload request; RenderCore consumes texture binding values | Streaming bridge owns scratch/result snapshot only | `ResourceDecodedTextureBridgeStatus`, `ResourceStatus`, upload statuses | Scratch byte storage, sampled texture slot, texture byte count | `Streaming_ResourceDecodedTextureBridge_*`, `Streaming_ResourceUpload_.*Texture` | `Done` | `Medium` | Keep decoded payload ownership with Resource; no RHI lifecycle ownership in Resource |
| `RenderSwapchainFramePipeline` | RenderCore clear/present/capture request to RHI swapchain | RenderCore owns frame request values | RHI owns swapchain and backend handles | RenderCore owns frame records; RHI owns device/swapchain lifecycle | `RenderSwapchainFramePipelineStatus`, `RhiStatus` | `MAX_RENDER_SWAPCHAIN_FRAME_RECORDS`, command capacity, capture storage | `RenderCore_SwapchainFramePipeline_*`, D3D11 swapchain hardware smoke | RHI hardware `BlockedByEnv` | `Medium` | Keep swapchain lifecycle in RHI; target D3D11 proof remains required |
| `RenderDrawableFramePipeline` | RenderCore material/view/frame packets to RHI drawable frame | RenderCore owns packet/material/view values | RHI owns backend draw execution | RenderCore owns frame records; RHI owns device resources | `RenderDrawableFramePipelineStatus`, material/view/frame packet statuses, `RhiStatus` | `MAX_RENDER_DRAWABLE_FRAME_RECORDS`, command/pass/packet capacities | `RenderCore_DrawableFramePipeline_*`, D3D11 drawable smoke | RHI hardware `BlockedByEnv` | `Medium` | Keep material and view policy value-only; no scene traversal or native public leak |
| `L0EngineEvidence` sample helper | Sample command gathers L0 module proof values | Platform/RHI/Resource/Audio/Input modules own engine behavior | Sample owns evidence aggregation | Sample owns local evidence result only | Boolean result plus explicit sample evidence flags | Caller-provided result storage and sample fixture bounds | `Sample_*`, `RunAssetSmokeDemo.ps1` debug/release routes | Engine sample `BlockedByEnv` | `Medium` | Keep generated artifacts ignored; sample proof must not replace module tests |

## 3.1 L0-RES-005 Evidence Sync

L0-RES-005 texture bridge to RHI closure is PASS at
`18030b201c69452a2a7da44fc3d08a4462c3d34f`. Readiness task `6fc3d241`
records that existing `ResourceDecodedTextureBridge` maps decoded texture
payloads to `ResourceUploadKind::CreateTexture` and sampled texture binding
values while Resource does not own RHI lifecycle.

Focused QA task `261fa423-28aa-498f-944f-c4eb44cf9d23` reports a read-only
clean workspace, `YuStreamingTests` focused build PASS,
`Streaming_ResourceDecodedTextureBridge_` discovery/execution `5/5` PASS,
`Streaming_ResourceUpload_.*Texture` discovery/execution `2/2` PASS, and RHI
texture/sampler/sampling dependency discovery count `10`. The RHI dependency
set was not executed, RHI closure is unchanged, and this sync does not claim
RuntimeAsset bridge/CMake cross-proof, RenderScene/RHI implementation,
World/editor/importer, old-package compatibility, real codec/parser,
Package/Resource public API expansion, L0-RES-006 PCM bridge, L0-RES-007 sample
texture/mesh path, adjacent/full Resource/Streaming, screenshots/reports/manual
proof, or broad/full CTest completion.

## 3.2 L0-RES-006 Evidence Sync

L0-RES-006 PCM bridge to Audio closure is PASS at
`804206712988733f38990af6975c67854b16de6a`. Readiness task `69ddc757`
records that existing `AudioResourcePcmPacketImportBridge` maps Resource
decoded audio metadata into `AudioPcmSamplePacketRequest` values and
bridge-owned import records while Audio does not parse Resource payloads and
Resource does not own Audio lifecycle.

Focused QA task `1dbfdaf6-61ff-4ac4-9e47-a2703f2e5a1e` reports a read-only
clean workspace, `YuAudioResourceTests` focused build PASS,
`AudioResource_PcmPacketImportBridge_` discovery/execution `8/8` PASS, and
dependency discovery counts `Audio_PcmSamplePacket_` `13` plus
`Audio_PcmStreamQueue_` `15`. Audio dependency rows were not executed by this
lane, `L0-AUD-002` is not table-closed here, and this sync does not claim
L0-RES-007 sample path, L0-AUD-003 callback proof, L0-AUD-005 sample PCM path,
RuntimeAsset or Asset Manager expansion, RenderScene/RHI, World/editor/importer,
codec/parser, Package/Resource public API expansion, screenshots/reports/manual
listening proof, adjacent/full suites, or broad/full CTest completion.

## 3.3 L0-RES-007 Evidence Sync

L0-RES-007 sample texture/mesh asset path closure is PASS at
`026f1d06af688ccaa1ff9a421f71ac1ea092cd5a`. Readiness task `76377a9a`
records the lane READY.

Focused QA task `37d47308-4d38-43d0-85cb-d98f9867b6f8` reports a read-only
clean workspace, Debug and Release `AssetSmokeDemo` smoke PASS on the current
machine with `YuAssetSmokeDemo PASS`, `YuAssetSmokeDemo L0_ENGINE PASS`, and
`YuAssetSmokeDemo L1_PREP PASS`, `YuSampleTests` focused build PASS,
`Sample_L1VerticalPrep_` discovery/execution `6/6` PASS, and dependency
discovery-only counts `Streaming_ResourceDecodedTextureBridge_` `5`,
`Streaming_ResourceUpload_.*Texture` `2`, RHI texture/sampler/sampling `10`,
and RenderCore texture/material/frame/draw/view `60`. Generated-output hygiene
stayed tracked/staged `0`, untracked `0`, with ignored sample/build outputs
only.

This evidence keeps L0-SAMPLE-004, L1 sample closure, L0-RHI table closure,
hardware closure, manual screenshot/listening proof, RenderScene/L1 visual
implementation, RuntimeAsset/Asset Manager expansion, World/editor/importer,
UI/GameAdapter/gameplay, material graph, shader compiler pipeline, scene loader,
old-package compatibility, real codec/parser, Package/Resource public API
expansion, L0-AUD-005 sample PCM path, L0-AUD-003 callback proof,
adjacent/full suites, and broad/full CTest outside this closure.

## 3.4 L0-AUD-001 Evidence Sync

L0-AUD-001 deterministic mixer/test backend closure is PASS at
`aee81a39d9d9ee063f9f57bc5bab5137d88cbc9f`. Readiness task `453eca90`
records READY.

Focused QA task `82548add-9a8a-48a7-adf1-ba837608fd07` reports first-slice
discovery exactly 24 rows, tests `#804` through `#827`, `YuAudioTests` focused
build PASS, exact 24-row execution `24/24` PASS with `0 failed`, BGM/SE/SFX/
music/business ID scan `0`, and a clean read-only QA workspace. The executed
set excluded Callback, PCM packet/stream queue, hardware, sample, and L1 rows.

This evidence keeps L0-AUD-002 PCM packet/stream queue, L0-AUD-003 XAudio2
callback proof, L0-AUD-004 callback cost proof, L0-AUD-005 sample PCM path,
L0-RES-006, L0-SAMPLE-006, AudioResource, AudioScene, hardware smoke, sample
scripts, screenshots/reports/manual listening proof, adjacent/full suites,
RuntimeAsset/Asset Manager, RenderScene/RHI, World/editor/importer,
UI/GameAdapter/gameplay, material graph, shader compiler pipeline, scene loader,
old-package compatibility, real codec/parser, Package/Resource public API
expansion, and broad/full CTest outside this closure.

## 4. L1 Runtime Bridge Audit

| Bridge | Direction | Source owner | Destination owner | Lifecycle owner | Failure statuses | Bounds | Tests | Risk | Required action |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `RuntimeApp` | Kernel lifecycle to explicit runtime frame phases and `FrameContext` | Kernel owns module lifecycle | RuntimeApp owns zero-world frame loop values | RuntimeApp owns run state; Kernel owns services/modules | `RuntimeAppStatus`, `KernelStatus` | Fixed frame count, phase trace storage | `Kernel_RuntimeApp*` | `Low` | Keep project behavior outside RuntimeApp |
| `WorldObjectIdentityBridge` | World object IDs to Object handles | World owns world object ID space | Object owns handle acquire/release | Bridge owns binding records; Object owns handle lifetime | `WorldObjectIdentityStatus`, `ObjectStatus` | Bridge capacity and handle generation | `WorldObjectIdentityBridge_*` | `Medium` | Keep World core object-free; maintain no-mutation failure tests |
| `WorldTransformBridge` | World object IDs to transform sidecar records | World owns object IDs | World transform bridge owns transform records | Bridge owns transform sidecar lifetime | `WorldTransformStatus` | Bridge capacity and output query storage | `WorldTransformBridge_*` | `Low` | Keep hierarchy/culling policy deferred |
| `WorldComponentAttachmentBridge` | World object IDs to component attachment records | World owns object IDs | Component attachment bridge owns component slots | Bridge owns attachment storage | `WorldComponentAttachmentStatus` | Attachment bridge capacity and query output storage | `WorldComponentAttachmentBridge_*` | `Low` | Keep behavior/lifecycle out of component records |
| `WorldComponentQueryBridge` | Component attachment records to bounded query outputs | Attachment bridge owns source storage | Caller owns output buffer | Query bridge owns only snapshot counters | `WorldComponentQueryStatus` | Output capacity and source bridge capacity | `WorldComponentQueryBridge_*` | `Low` | Keep read-only and bounded |
| `WorldResourceBindingBridge` | World object IDs to Resource handles | World owns object IDs | Resource owns handles and acquire/release semantics | Binding bridge owns world-to-resource binding records | `WorldResourceBindingStatus`, `ResourceStatus` | Bridge capacity and handle generation | `WorldResourceBindingBridge_*` | `Medium` | Keep File/Package/load/decode/upload outside this bridge |
| `WorldComponentResourceBindingBridge` | Component attachments to Resource handles | Attachment bridge owns component records | Resource owns handles and acquire/release semantics | Component binding bridge owns binding records | `WorldComponentResourceBindingStatus`, `ResourceStatus` | Bridge capacity, attachment source, handle generation | `WorldComponentResourceBindingBridge_*` | `Medium` | Keep binding release failures explicit; no Resource loading here |
| `WorldComponentAttachmentSnapshotBridge` | Component attachment records to Serialize value stream and back | Attachment bridge owns source/destination records | Serialize owns reader/writer values | Snapshot bridge owns no durable storage | `WorldComponentAttachmentSnapshotStatus`, `SerializeStatus` | Record count, version, destination capacity | `WorldComponentAttachmentSnapshotBridge_*` | `Medium` | Keep restore no-mutation failures covered |
| `WorldComponentResourceBindingSnapshotBridge` | Component resource binding records to Serialize value stream and back | Binding bridge owns source records | Serialize owns stream values; caller owns output buffers | Snapshot bridge owns no durable storage | `WorldComponentResourceBindingSnapshotStatus`, `SerializeStatus` | Record count, output capacity, version | `WorldComponentResourceBindingSnapshotBridge_*` | `Medium` | Keep resource acquisition out of snapshot phase |
| `WorldComponentResourceBindingRestoreBridge` | Serialized binding records to destination component binding bridge | Serialize/caller owns input records | Component binding bridge and Resource registry receive validated restore | Restore bridge owns apply transaction only | `WorldComponentResourceBindingRestoreStatus`, `ResourceStatus` | Input count, destination capacity, rollback path | `WorldComponentResourceBindingRestoreBridge_*` | `High` | Future expansion is blocked unless rollback/failure tests remain green; current evidence covers no-mutation and rollback failure states |
| `WorldSerializeSnapshotBridge` | World lifecycle trace/snapshot values to Serialize stream | World owns trace/snapshot values | Serialize owns reader/writer contract | Bridge owns no durable storage | `WorldSerializeSnapshotStatus`, `SerializeStatus` | Trace capacity, stream version, output storage | `WorldSerializeSnapshotBridge_*` | `Medium` | Keep File/Package/save policy outside Serialize core |
| `WorldScriptDispatchBridge` | World phase trace to Script call IDs | World owns phase trace values | Script owns native call registry and value slots | Bridge owns phase binding table | `WorldScriptDispatchStatus`, `ScriptStatus` | Binding capacity, trace capacity, argument/result buffers | `WorldScriptDispatchBridge_*` | `Medium` | Keep Script registry free of World ownership and World core free of Script ownership |
| `ScriptRuntimePhaseDispatchAdapter` | Runtime phase trace to Script native calls | Runtime phase trace caller owns trace values | Script registry owns native call dispatch | Adapter owns phase binding table | `ScriptRuntimePhaseDispatchStatus`, `ScriptStatus` | Binding capacity, trace capacity, argument/result buffers | `Script_RuntimePhaseDispatch_*` | `Medium` | Keep it as adapter only; no gameplay script semantics |
| `WorldIdentityBaseline` | Object, World, identity, transform, and component baseline fixture | Object/World/bridge modules own primitives | Baseline fixture composes first-slice L1 identity records | Baseline owns fixture records and rollback sequence | `WorldIdentityBaselineStatus` plus child bridge statuses | Baseline object capacity and component slot constraints | `WorldIdentityBaseline_*` | `High` | Treat as controlled L1 fixture, not a new owner; future object lifecycle expansion is blocked unless rollback/no-mutation tests remain green |
| `WorldSceneAssemblyBridge` | Attachment and resource-binding input records to destination bridges | Caller owns assembly input records | Attachment/resource-binding bridges own destination storage | Assembly bridge owns transaction/rollback only | `WorldSceneAssemblyStatus`, child bridge statuses | Input count, destination capacity, rollback path | `WorldSceneAssemblyBridge_*` | `High` | No active scene mutation expansion until rollback and destination-not-empty tests remain green |
| `WorldSceneAssemblyManifestStreamBridge` | Scene assembly attachment/binding records to Serialize stream and back | Caller owns scene assembly value records | Serialize owns stream; caller owns output buffers | Manifest stream bridge owns no durable storage | `WorldSceneAssemblyManifestStreamStatus`, `SerializeStatus` | Input/output count, version, output capacity | `WorldSceneAssemblyManifestStreamBridge_*` | `Medium` | Keep as value-stream bridge; no active restore here |
| `WorldSceneObjectTransformManifestStreamBridge` | Object identity and transform records to Serialize stream and back | Caller owns identity/transform values | Serialize owns stream; caller owns outputs | Manifest stream bridge owns no durable storage | `WorldSceneObjectTransformManifestStreamStatus`, `SerializeStatus` | Input/output count, version, output capacity | `WorldSceneObjectTransformManifestStreamBridge_*` | `Medium` | Keep object creation and transform apply outside manifest stream |
| `WorldSceneRecordValueStreamBridge` | Scene identity/transform/attachment/binding records to one value stream | Caller owns scene record arrays | Serialize owns stream; caller owns outputs | Value stream bridge owns no durable storage | `WorldSceneRecordValueStreamStatus`, child stream statuses | Input/output counts, version, output capacity | `WorldSceneRecordValueStreamBridge_*` | `Medium` | Keep save/profile policy outside this bridge |
| `WorldSceneDecodedRestorePlanBridge` | Validated decoded scene records to restore plan records | Caller/Serialize owns decoded records | Restore coordinator consumes plan values | Plan bridge owns scratch/output only | `WorldSceneDecodedRestorePlanStatus`, Object/Resource statuses | Plan output, destination capacities, duplicate/missing checks | `WorldSceneDecodedRestorePlanBridge_*` | `High` | Future active restore remains blocked until plan preflight tests cover object/resource mismatch and capacity failures |
| `WorldSceneApplyTimeRestoreProofBridge` | Restore plan values to apply-time proof slices | Restore plan bridge owns plan output values | Active restore gate consumes proof/slice outputs | Proof bridge owns scratch/output only | `WorldSceneApplyTimeRestoreProofStatus` | Plan scratch, proof output, slice output, destination capacities | `WorldSceneApplyTimeRestoreProofBridge_*` | `High` | Keep as proof-only; mutation remains blocked without valid proof |
| `WorldSceneObjectTransformRestoreBridge` | Identity/transform records to Object/World/transform destination bridges | Caller owns identity/transform input records | Object, identity bridge, and transform bridge receive apply calls | Restore bridge owns apply transaction/rollback | `WorldSceneObjectTransformRestoreStatus`, Object/World/transform statuses | Input count, destination capacities, rollback path | `WorldSceneObjectTransformRestoreBridge_*` | `High` | Future expansion is blocked unless object acquire/rollback failure evidence remains current |
| `WorldSceneActiveRestoreGateBridge` | Plan/proof/slices to active restore gate decision | Plan/proof bridges own validation artifacts | Active restore caller receives gate output | Gate bridge owns validation only; destination bridges own mutation if later allowed | `WorldSceneActiveRestoreGateStatus` | Plan/proof/slice scratch, gate output, destination capacities | `WorldSceneActiveRestoreGateBridge_*` | `High` | This is the explicit blocker gate: active restore mutation must not expand unless validation-before-mutation evidence remains green |
| `AssetManager` | Resource/Streaming/AudioResource ready records to runtime asset states | Resource/Streaming/AudioResource own lower records | Asset owns runtime handles, state, dependencies, and ready records | Asset owns asset record lifetime; lower modules own device/resource lifetimes | `AssetStatus`, Resource/Streaming/AudioResource statuses | Asset capacity, dependency traversal output, handle generations | `Asset_*` | `Medium` | Keep RHI/Audio device ownership outside Asset |
| `AssetRuntimeFixture` | Synthetic Asset path over Resource/Streaming/AudioResource records | AssetManager and lower modules own their records | Fixture owns only validation run result | Fixture owns no durable runtime state | `AssetRuntimeFixtureStatus` | Dependency output capacity and fixture request storage | `Asset_RuntimeFixture*` | `Medium` | Keep fixture as proof path only; no sample/game policy here |
| `RenderSceneContractQueue` | L1 render scene records to RenderCore packet values | World/Asset own object and asset handle values | RenderCore owns view/draw/material packet contracts | RenderScene owns submission result/snapshot only | `RenderSceneStatus` | Entity/camera/output packet capacities | `RenderScene_*` | `Medium` | Keep native and RHI details out of RenderScene |
| `AudioSceneContractQueue` | L1 audio source records to Audio PCM stream queue requests | Asset/AudioResource own sound/packet records | Audio owns PCM queue request contract | AudioScene owns submission result/snapshot only | `AudioSceneStatus` | Source count, bus IDs, output request capacity | `AudioScene_*` | `Medium` | Missing backend must stay explicit; no XAudio2 details here |
| `InputCommandMapper` | L0 Input events to L1 frame-indexed command snapshots | Input owns device events | Runtime/input layer owns command snapshots | Mapper owns action map/context state | `InputStatus` | Binding capacity, event span, command snapshot capacity | `Input_CommandMapper_*` | `Low` | Keep gameplay command semantics outside mapper |
| `RuntimeDiagnosticsCounterRecorder` | Runtime counter values to Diagnostics channel | Runtime owner produces counter values | Diagnostics owns channel and counter registry | Recorder owns no durable behavior state | `DiagnosticsStatus`, `RuntimeDiagnosticsRecordResult` | Counter ID set and channel capacity | `Diagnostics_RuntimeCounters_*` | `Low` | Diagnostics-disabled path must stay behavior-equivalent |
| `RuntimeDiagnosticsOverlayHook` | Optional diagnostics proposal to tooling-plane validation | Runtime diagnostics produces proposal values | Tooling/debug overlay consumes optional hook | Hook owns no runtime dependency | `RuntimeDiagnosticsOverlayHookStatus` | Proposal capacity and disabled state | `Diagnostics_OverlayHook*` | `Medium` | Treat as tooling plane only; runtime dependency is rejected by tests |
| `L1VerticalSamplePrep` | Sample manifest through RuntimeApp/World/Asset/Input/RenderScene/AudioScene/Serialize evidence | L1 modules own their value contracts | Sample owns validation aggregation | Sample owns prep result only | Boolean result plus sample route flags | Fixed sample object graph, route output values, cleanup counters | `Sample_L1VerticalPrep_*` | `Medium` | Keep sample project-independent and value-contract based; no UI/GameAdapter/gameplay expansion |

## 5. High-Risk Rows And Evidence Gaps

| Row | Classification | Evidence gap or blocker | Current containment |
| --- | --- | --- | --- |
| L0 hardware bridges (`HardwareFrameHost`, XAudio2 callback, XInput, D3D11 frame pipelines) | `Medium` architecture risk, `BlockedByEnv` proof state | Target production hardware must run `windows-hardware-smoke` and `windows-strict-hardware-smoke` | Public contracts are value/status based and have fast fixture tests |
| `WorldComponentResourceBindingRestoreBridge` | `High` L1 restore bridge | Future restore expansion is blocked unless rollback/resource-acquire failure tests stay green | Current tests cover destination-not-empty, rollback, no-mutation, and bounded inputs |
| `WorldIdentityBaseline` | `High` L1 fixture bridge | Future Object/World lifecycle expansion is blocked unless baseline create/query/destroy tests stay green | Current fixture owns records and child bridges retain primitive ownership |
| `WorldSceneAssemblyBridge` | `High` L1 scene assembly bridge | Future active scene mutation is blocked unless rollback and destination capacity evidence stay green | Current bridge applies only validated records to destination bridges |
| `WorldSceneDecodedRestorePlanBridge` | `High` L1 plan bridge | Future active restore is blocked unless duplicate/missing/object/resource mismatch proof stays green | Current bridge emits plan records and does not mutate destinations |
| `WorldSceneApplyTimeRestoreProofBridge` | `High` L1 proof bridge | Future active restore is blocked unless apply-time proof and slice evidence stay green | Current bridge produces proof/slice outputs before mutation |
| `WorldSceneObjectTransformRestoreBridge` | `High` L1 restore bridge | Future object/transform apply expansion is blocked unless object acquire and rollback evidence stays green | Current bridge owns apply transaction and maps explicit failures |
| `WorldSceneActiveRestoreGateBridge` | `High` L1 gate bridge | Active restore coordinator mutation remains blocked without validation-before-mutation proof | Current gate validates plan/proof/slices/capacity before any active mutation path |

## 6. Forbidden Scope Check

This audit did not add implementation scope. The bridge rows above must keep
these boundaries:

- No World/GameAdapter/UI/gameplay bridge is accepted as L0 or L1 closure proof.
- No old package runtime compatibility path is accepted as Resource or sample
  proof.
- No report JSON, capture file, manual screenshot, or manual audio listening is
  accepted as runtime behavior transport.
- Public bridge headers must not expose D3D11, XAudio2, XInput, Win32, or other
  backend-native types unless the owning module explicitly marks the bridge as
  platform-private.
- Hardware `BlockedByEnv` rows require explicit unavailable/skip statuses; they
  must not be rewritten as `Done` from local absence of hardware.

## 7. Required Next Documents

After this audit and `YUENGINE_L0_COMPLETION_MATRIX.md` pass VQ, the remaining
governance documents from the execution plan stay gated in order:

1. `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`
2. `docs/YUENGINE_L0_SAMPLE_ACCEPTANCE.md`
3. `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md`
