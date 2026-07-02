# YuEngine Documentation Entry Point

Status: canonical documentation handoff
Owner: Architect
Last major planning sync: `origin/main@96f0bf0eaed6d2847b4128cd31146340ccc07a0d`

## 1. Read This First

This directory contains many historical plans, review notes, RAV records, editor
workflows, and evidence snapshots. They are not all current instructions.

A new team, a restarted agent, or a context-compressed session must start from
this file and the canonical documents below. Do not infer current direction from
older `RAV`, `EDITOR`, `BRIDGE`, or per-feature documents unless one of the
canonical documents explicitly points to them.

## 2. Current Product Target

YuEngine is a small-team native commercial game engine. The current product
reference bar is the shipped TouhouNewWorld class of engine:

- small native runtime binaries;
- no Unity/UE runtime shape as the target architecture;
- shipped-content-scale packed resources as a pressure example, not a hard byte
  target;
- long-session stability around the 20 hour gameplay class;
- package/resource indexes, runtime asset records, diagnostics, and toolchain
  evidence strong enough for a real shipped game.

The goal is not broad UE/Unity feature parity. The goal is a narrower native
engine that the team can ship, patch, diagnose, and maintain.

## 3. Authoritative Current Documents

Read in this order:

| Order | Document | Purpose |
| --- | --- | --- |
| 1 | `docs/YUENGINE_LONG_PLAN_TEAM_EXECUTION.md` | Final target, non-negotiable principles, long-horizon roadmap, current nearest stage, stop conditions |
| 2 | `docs/YUENGINE_L0_L1_EXECUTION_PLAN.md` | L0/L1 execution plan, production target, long-roadmap summary, immediate RTSPINE backlog |
| 3 | `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md` | RuntimeAsset production spine, file/source/cooked contracts, asset target identity before deeper animation |
| 4 | `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md` | Current L1 ownership matrix, state corrections, animation runtime foundation downgraded to `FirstSlice` |
| 5 | `docs/YUENGINE_RUNTIME_VISUAL_EVIDENCE_MATRIX.md` | Runtime visual evidence status and current proof links |

If these documents conflict with older docs, these documents win.

## 4. Current Execution State

At the latest handoff:

- L0/L1 final closure is complete at
  `a4a520ffbdb1049dd4674330854033c61c91d6bb`;
- workspace goal `e427bbd0-2259-4400-9ca0-df0acd5e1dac` is done with
  checkpoint `FINAL_L0L1_DONE_STRICT_XINPUT_PASS_VQ_COMPLETE_2026-06-29`;
- final VQ task `b9e75e70-0dd7-4e3b-a77c-6cda449e2d6a` reports
  `COMPLETE-PASS / FINAL-L0L1-DONE-READY`;
- strict target XInput proof is no longer blocked: DualSenseX 1.4.9 with
  `Xbox 360` emulation exposed the connected DualSense as XInput index `0`,
  `ctest --preset windows-strict-hardware-smoke --output-on-failure` returned
  exit code `0`, `Input_HardwareBridge_PollsXInputGamepad` passed, and strict
  hardware smoke reported `16/16` PASS;
- the previous strict XInput failure cause was environment mapping, not engine
  runtime logic: Windows saw the DualSense HID device, but `XInputGetState(0..3)`
  returned `1167 / ERROR_DEVICE_NOT_CONNECTED` until DualSenseX exposed an
  XInput controller;
- the current repo baseline is clean:
  `HEAD == origin/main == 96f0bf0eaed6d2847b4128cd31146340ccc07a0d`
  after the `bd1c4ce76534ba454deb72eda9db7b0c32fe256b` AFDCA68 docs
  reconcile baseline and the `96f0bf0eaed6d2847b4128cd31146340ccc07a0d`
  WorldSceneAuthoring Asset-edge WorldObject snapshot failure-status
  implementation evidence;
- active workspace tasks, timers, and coordinator goals were empty at the final
  L0/L1 handoff;
- the next mainline is RuntimeAsset production spine continuation, starting from
  the accepted RAV1/RTSPINE evidence and the next-slice boundary in
  `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`;
- scene-animation implementation is complete at
  `f211f7f95299388987ccef00b4d1e8ee6f7bf0c1`;
- scene-animation QA is PASS;
- docs evidence sync is complete at
  `0a9144b0e30cbede56a5dbf04b232f3e5b763802`;
- long-term planning correction is complete at
  `705f8ba94fee8ccbb9330d2c37f14bb47114e0d1`;
- documentation entry cleanup is complete at
  `b0d96b0dece4009e753dd15307235e2e8b8badac`;
- RTSPINE-003 target identity implementation and focused QA are PASS at
  `5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4`;
- RTSPINE-003 focused QA reports the focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS, exact target identity plus
  scene/runtime animation regression discovery `10` rows, execution `10/10`
  PASS, `git diff --check` PASS, added-line hygiene PASS, and boundary scans
  PASS without running broad full CTest;
- RTSPINE-003 docs evidence is synchronized by workspace task
  `d9dc3692-aa12-4f5c-872a-5b7293a92ceb`;
- RTSPINE-003 VQ evidence consistency audit is PASS by workspace task
  `fdd78da4-da12-4956-b6ac-63ff9e377121`;
- RTSPINE-004 animation track target/property binding implementation and focused
  QA are PASS at `ebe9ea35f531aa40133262b701e5e751f8ed9ccf`;
- RTSPINE-004 focused QA task `2e2d5a4e-0bb0-4cf4-bd1b-ab3a87987b7f`
  reports focused `YuRuntimeAssetDataClosedLoopTests` build PASS, exact
  discovery `17` rows, execution `17/17` PASS, `git diff --check` PASS,
  added-line hygiene PASS, and boundary/non-goal scans PASS without running
  broad full CTest;
- RTSPINE-004 covers SceneNode `target_id` plus property binding only;
- RTSPINE-005 minimal Step/Linear interpolation implementation and focused QA are
  PASS at `2bfe7e37d36ca711dd706728f21b1e4caecfd3db` /
  `d18f1679ebd389ecec506055764602591f5b9ab6`; focused QA task
  `951a3da8-6b13-4268-960e-407f65c40db7` reports focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS, exact interpolation discovery
  `3`, execution `3/3` PASS, non-Package RuntimeAsset animation whitelist
  `23/23` PASS, diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-006 invalid-target failure model implementation is PASS at
  `96e0c024435f670c39ced019ff825b819a6830a3`; focused QA task
  `6d02c260-936a-456b-917b-5c2802bbb666` reports isolated clean worktree at
  `96e0c024`, focused `YuRuntimeAssetDataClosedLoopTests` build PASS, focused
  RuntimeAsset regex discovery/execution `8/8` PASS, exact new rows `2/2`
  PASS, diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-007 runtime instance mapping implementation is PASS at
  `37a112549190ac2123abcd72b5c688cdfa5b01e5`; focused QA task
  `6b6baf5f-2381-4b9c-89b1-4411fba53d23` reports focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS,
  `RuntimeAssetData_(RuntimeInstanceMapping|AnimationTrackTargetBinding|AnimationFailureModel|RuntimeAnimationTables)`
  discovery/execution `12/12` PASS, exact RuntimeInstanceMapping rows `5/5`
  PASS, commit-level diff/hygiene/boundary PASS, read-only QA with clean final
  repo, and no broad/full CTest;
- RTSPINE-RUNTIMEASSET-MODEL-SKELETON-TARGET-BINDING-U64-001 is VQ-closed at
  `3fa4ef7bd42da8f60bd5ebb3a7f863bd76292c84`; implementation task
  `06724fe5-b2e4-410e-97e7-2b41c195c3a0` reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSET-MODEL-SKELETON-TARGET-BINDING-U64-001-COMMITTED`
  and VQ task `04e2a7a6-eac5-41d2-9624-6e5e952859c4` reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSET-MODEL-SKELETON-TARGET-BINDING-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt`,
  `Src/YuEngine/RuntimeAsset/Src/RuntimeAssetData.cpp`, and
  `Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp`; focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS, focused CTest discovery found
  `17` rows including `RuntimeAssetData_AnimationTrackTargetBindingResolvesModelAndSkeletonTargetFamilies`,
  `RuntimeAssetData_RuntimeInstanceMappingBuildsTargetFamilyRows`, and
  `RuntimeAssetData_RuntimeInstanceMappingResolvesModelAndSkeletonFamilies`,
  focused execution `17/17` PASS, and old unsupported target-family labels are
  absent;
- RTSPINE-RUNTIMEASSETWORLDADAPTER-TARGET-FAMILY-ALIAS-HANDOFF-U64-001 is
  VQ-closed at `296100b3bda25e962c3a3a503f9f78f0160083ce`; implementation task
  `77376606-d3d8-45de-8079-79121593b8e7` reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSETWORLDADAPTER-TARGET-FAMILY-ALIAS-HANDOFF-U64-001-COMMITTED`
  and VQ task `5fb82855-a437-4eb7-b078-373069988b2d` reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSETWORLDADAPTER-TARGET-FAMILY-ALIAS-HANDOFF-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt`,
  `Src/YuEngine/RuntimeAssetWorldAdapter/Src/RuntimeAssetWorldObjectAdapterBridge.cpp`,
  and `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterBridgeTest.cpp`;
  focused `RuntimeAssetWorldObjectAdapter` matrix PASS was `13/13` including
  `RuntimeAssetWorldObjectAdapter_BuildsRestoreRecordsForModelAndSkeletonTargetFamilyAliases`;
  SceneNode behavior and Unknown/default rejection, duplicate, capacity, null,
  no-mutation, and status semantics are preserved;
- RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-TARGET-FAMILY-PROOF-U64-001 is
  VQ-closed at `54e02e049bb6f67fd15ca32d1675f1c61380ae70`; implementation task
  `53b6d5dc-fd17-442c-b18b-9257c4f3650c` reports local commit completion and
  VQ task `8fbe251e-2c14-4786-a48c-5b8b0b6f8e14` reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-TARGET-FAMILY-PROOF-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt` and
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`;
  no `RuntimeAssetWorldObjectRestoreHandoffBridge.cpp` production bridge change
  was needed; focused `RuntimeAssetWorldObjectRestoreHandoff` discovery and
  execution PASS was `5/5`, including
  `RuntimeAssetWorldObjectRestoreHandoff_AppliesModelAndSkeletonTargetFamilyAliases`;
  the older adapter-preflight negative row now uses
  `RuntimeAssetTargetIdentityKind::Unknown` and still proves
  `AdapterBuildFailed`, `UnsupportedTargetKind`, and no output/world mutation;
- RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-ATTACHMENT-BINDING-GATE-PROOF-U64-001
  is VQ-closed at `4d9f244ca373c466478b54b7fbc0dd91bf8b5720`;
  implementation task `3d8c0c2b-987c-4046-8f01-4e04f16f3715` reports
  committed completion and VQ task `4607e700-6bd8-4f0d-a508-ac86b991e7e7`
  reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-ATTACHMENT-BINDING-GATE-PROOF-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt` and
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`;
  production handoff bridge/state files were unchanged; focused
  `YuRuntimeAssetWorldAdapterHandoffTests` build PASS, focused
  `RuntimeAssetWorldObjectRestoreHandoff` discovery and execution PASS was
  `6/6`, including
  `RuntimeAssetWorldObjectRestoreHandoff_CarriesAttachmentAndBindingGateRecordsForTargetAliases`;
  the proof covers non-zero component attachment and component resource binding
  sidecar gate records for ModelNode/SkeletonJoint aliases through the existing
  adapter/world active restore path, with output gates `4/5` as Attachment and
  `6/7` as Binding;
- RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-SIDECAR-ASSEMBLY-RESTORE-U64-001
  is VQ-closed at `f85c67701f2ff90c94c84cdc2761e434524128d8`;
  implementation task `81f4806a-cfc4-464b-a644-b163bfc0459f` reports committed
  completion and VQ task `dac5643f-7225-4ba0-a76b-c063178dfb97` reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-SIDECAR-ASSEMBLY-RESTORE-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt`,
  `Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffState.h`,
  `Src/YuEngine/RuntimeAssetWorldAdapter/Src/RuntimeAssetWorldObjectRestoreHandoffBridge.cpp`,
  and `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`;
  focused builds `YuRuntimeAssetWorldAdapterHandoffTests`,
  `YuRuntimeAssetWorldAdapterTests`, and `YuWorldTests` PASS; focused
  `RuntimeAssetWorldObjectRestoreHandoff` discovery and execution PASS was
  `7/7`, including
  `RuntimeAssetWorldObjectRestoreHandoff_RestoresAttachmentAndBindingSidecarsThroughWorldAssembly`;
  `RuntimeAssetWorldObject(Adapter|RestoreHandoff)` rows pass `20/20`,
  `WorldSceneAssemblyBridge` rows pass `27/27`, and the handoff state records
  `restored_attachment_count` plus `restored_binding_count` from the existing
  `WorldSceneAssemblyBridge`;
- RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-SIDECAR-FAILURE-STATUS-U64-001 is
  VQ-closed at `4587c7d1f204663577950241d4c42a5b72ab03a1`;
  implementation task `ab4eb0f5-0350-49af-8da3-13b4c47dda8b` reports
  committed completion and VQ task `f0c0c54e-32bd-4ee2-9dc2-b8c10c68c59a`
  reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-SIDECAR-FAILURE-STATUS-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt`,
  `Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridge.h`,
  `Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffResult.h`,
  `Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffSnapshot.h`,
  `Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffState.h`,
  `Src/YuEngine/RuntimeAssetWorldAdapter/Src/RuntimeAssetWorldObjectRestoreHandoffBridge.cpp`,
  and `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`;
  `RuntimeAssetWorldObjectRestoreHandoffStatus.h` was allowed but unchanged;
  result/state/snapshot expose `WorldSceneAssemblyStatus` through
  `assembly_status` and `last_assembly_status`; assembly failure still returns
  `RuntimeAssetWorldObjectRestoreHandoffStatus::RestoreFailed`; focused builds
  `YuRuntimeAssetWorldAdapterHandoffTests`, `YuRuntimeAssetWorldAdapterTests`,
  and `YuWorldTests` PASS; focused handoff rows pass `8/8`, adapter plus handoff
  rows pass `21/21`, WorldSceneAssemblyBridge rows pass `27/27`, and
  `RuntimeAssetWorldObjectRestoreHandoff_ExposesSidecarAssemblyFailureStatus`
  verifies no identity/transform/attachment/binding mutation on sidecar assembly
  failure;
- RTSPINE-RUNTIMEASSETDATA-TO-RUNTIMEASSETWORLDADAPTER-HANDOFF-U64-001 is
  VQ-closed at `e512d3990412f90b38aee8469845c44e188dd275`; implementation task
  `150e051b` reports committed completion and VQ task
  `6f086b28-40e3-4574-bac5-33e587b2e91c` reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSETDATA-TO-RUNTIMEASSETWORLDADAPTER-HANDOFF-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt`,
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`,
  and
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectDataHandoffFixtureTest.cpp`;
  no production RuntimeAsset, RuntimeAssetWorldAdapter, World, Resource, docs,
  editor, GameAdapter, UI, or broad File/VFS paths changed; focused build
  `YuRuntimeAssetWorldAdapterHandoffTests` PASS; new data handoff row
  `RuntimeAssetWorldObjectDataHandoff_FeedsRuntimeAssetDataOutputsIntoRestoreHandoff`
  passes `1/1`; RuntimeAssetWorldObject Adapter/Handoff/DataHandoff rows pass
  `22/22`; RuntimeAssetData target/mapping rows pass `14/14`; World active gate
  and object restore rows pass `34/34`; the fixture validates RuntimeAssetData
  `runtime_instance_mappings`, `scene_entities`, and `scene_transforms` feeding
  RuntimeAssetWorldAdapter restore handoff through caller-owned
  World/ObjectRegistry identity and transform destinations; object handles and
  world object ids remain caller-owned runtime identity, not asset-file data;
- RTSPINE-RUNTIMEASSETWORLDADAPTER-WORLD-SCENE-RECORD-STREAM-HANDOFF-U64-001
  is VQ-closed at `088f21eb313be7c0e6ff283af922f23ec335ee09`;
  implementation task `1ea59c46` reports committed completion and VQ task
  `943f29a6-8a24-4a77-8e8a-4366285890b4` reports
  `COMPLETE-PASS / RTSPINE-RUNTIMEASSETWORLDADAPTER-WORLD-SCENE-RECORD-STREAM-HANDOFF-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt`,
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRecordStreamHandoffFixtureTest.cpp`,
  and
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`;
  no docs or production RuntimeAsset, RuntimeAssetWorldAdapter, World,
  Resource, editor, GameAdapter, UI, gameplay, or broad File/VFS paths changed;
  focused build `YuRuntimeAssetWorldAdapterHandoffTests` PASS; new record-stream
  row
  `RuntimeAssetWorldObjectRecordStreamHandoff_FeedsWorldSceneRecordStreamsIntoRestoreHandoff`
  passes `1/1`; RuntimeAssetWorldObject Adapter/Handoff/DataHandoff rows pass
  `22/22`; WorldScene stream/decoded-plan/proof rows pass `143/143`;
  WorldSceneActiveRestoreGate rows pass `4/4`; the fixture proves
  RuntimeAssetWorldAdapter handoff can consume caller-owned World scene
  record-stream and decoded-plan outputs derived from the RuntimeAssetData scene
  output shape through existing in-memory World value-stream, decoded-plan,
  apply-time proof, and active-restore gate records;
- RTSPINE-WORLDSCENEAUTHORING-RUNTIME-EXPORT-TO-RUNTIMEASSETWORLDADAPTER-HANDOFF-U64-001
  is VQ-closed at `f8ef490493f43a97d2958d6a604e598b68f1fcab`;
  implementation task `c1a469a8` reports committed completion and VQ task
  `5c536c0b-7849-436b-a695-e323f8afd339` reports
  `COMPLETE-PASS / RTSPINE-WORLDSCENEAUTHORING-RUNTIME-EXPORT-TO-RUNTIMEASSETWORLDADAPTER-HANDOFF-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt`,
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`,
  and
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`;
  no docs or production RuntimeAsset, RuntimeAssetWorldAdapter, World,
  Resource, editor, GameAdapter, UI, gameplay, or broad File/VFS paths changed;
  focused build `YuRuntimeAssetWorldAdapterHandoffTests` PASS; new authoring
  runtime export handoff row
  `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_FeedsAuthoringRuntimeExportThroughRecordStreamIntoRestoreHandoff`
  passes `1/1`; RuntimeAssetWorldObject handoff label set passes `24/24`;
  WorldSceneAuthoringDocument rows pass `9/9`; WorldScene stream/decoded-plan/proof
  rows pass `143/143`; WorldSceneActiveRestoreGate rows pass `4/4`; the fixture
  proves `WorldSceneAuthoringDocumentValidator::ValidateAndExport` runtime
  export records feed existing WorldScene record-stream, decoded-plan, proof,
  active-gate, and RuntimeAssetWorldAdapter restore handoff plumbing;
- RTSPINE-WORLDSCENEAUTHORING-ATTACHMENT-BINDING-DEPENDENCY-EXPORT-TO-RUNTIMEASSETWORLDADAPTER-HANDOFF-U64-001
  is VQ-closed at `318daeecef8905554bef459e998bd791eafa08bd`;
  implementation task `9184605b` reports committed completion and VQ task
  `97628142-20ed-4e09-ae05-be61f0226c44` reports
  `COMPLETE-PASS / RTSPINE-WORLDSCENEAUTHORING-ATTACHMENT-BINDING-DEPENDENCY-EXPORT-TO-RUNTIMEASSETWORLDADAPTER-HANDOFF-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt` and
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`;
  no docs or production RuntimeAsset, RuntimeAssetWorldAdapter, World,
  Resource, editor, GameAdapter, UI, gameplay, or broad File/VFS paths changed;
  focused build `YuRuntimeAssetWorldAdapterHandoffTests` PASS; new
  attachment/binding/dependency handoff row
  `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_FeedsAttachmentBindingDependencyExportThroughRecordStreamIntoRestoreHandoff`
  passes `1/1`; authoring handoff rows pass `2/2`;
  RuntimeAssetWorldObject handoff rows pass `12/12`; WorldSceneAuthoringDocument
  rows pass `9/9`; WorldScene stream/plan/proof/gate rows pass `88/88`; the
  fixture proves attachment, binding, and dependency runtime export records feed
  existing WorldScene record-stream, decoded-plan, proof, active-gate, and
  RuntimeAssetWorldAdapter attachment/resource-binding sidecar handoff; the
  dependency remains exported data only, with no Resource/File/VFS loading and
  no dependency graph traversal;
- RTSPINE-WORLDSCENEAUTHORING-DEPENDENCY-EXPORT-TO-RESOURCE-EDGE-HANDOFF-U64-001
  is VQ-closed at `f967001c39a53717226127ff67c316c8a3bf2a4a`;
  implementation task `0c9911ab` reports committed completion and VQ task
  `86ce7cfd-627c-445a-ae03-f639d47cec13` reports
  `COMPLETE-PASS / RTSPINE-WORLDSCENEAUTHORING-DEPENDENCY-EXPORT-TO-RESOURCE-EDGE-HANDOFF-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt` and
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`;
  no docs or production RuntimeAsset, RuntimeAssetWorldAdapter, World,
  Resource, editor, GameAdapter, UI, gameplay, broad File/VFS, or Asset Manager
  paths changed; focused build `YuRuntimeAssetWorldAdapterHandoffTests` PASS;
  new Resource edge row
  `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_CommitsExportedDependencyAsCallerOwnedResourceEdge`
  passes `1/1`; authoring handoff rows pass `3/3`; WorldSceneAuthoringDocument
  rows pass `9/9`; Resource and RuntimeAssetData dependency baseline rows pass
  `4/4`; the fixture proves exported
  `WorldSceneAuthoringDependencyRecord.resource_handle` can be committed as a
  caller-owned `ResourceRegistry` dependency edge using a caller-owned
  dependent `ResourceHandle` and exported dependency `ResourceHandle`;
  `stable_resource_id`-only inference is rejected, RuntimeAssetWorldAdapter
  restore handoff remains non-mutating for Resource dependency edges, and no
  Resource/File/VFS loading, dependency graph traversal, or Asset Manager
  dependency-edge follow-through is opened;
- RTSPINE-WORLDSCENEAUTHORING-DEPENDENCY-EXPORT-TO-ASSET-EDGE-HANDOFF-U64-001
  is VQ-closed at `58021419256fc68cd7a84692fd42dbc7a3d0f08e`;
  implementation task `56ab9999` reports committed completion and VQ task
  `b75a7379-ecb4-4024-ad57-1512833b2c5e` reports
  `COMPLETE-PASS / RTSPINE-WORLDSCENEAUTHORING-DEPENDENCY-EXPORT-TO-ASSET-EDGE-HANDOFF-U64-001-VQ-READY`;
  exact implementation scope was `CMakeLists.txt` and
  `Tests/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`;
  no docs or production RuntimeAsset, RuntimeAssetWorldAdapter, World,
  Resource, Asset, editor, GameAdapter, UI, gameplay, broad File/VFS, or Asset
  auto-lookup paths changed; focused build
  `YuRuntimeAssetWorldAdapterHandoffTests` PASS; focused matrix rows pass
  `23/23`; new Asset edge row
  `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_CommitsExportedDependencyAsCallerOwnedAssetEdge`
  passes `1/1`; authoring handoff rows pass `4/4`; Asset baseline rows pass
  `9/9`; RuntimeAssetData dependency/resource-asset edge rows pass `10/10`;
  the fixture proves exported WorldSceneAuthoring dependency rows can be
  committed to `AssetManager` as caller-owned dependency edges using explicit
  caller-owned `AssetHandle` scene/document and dependency records;
  stable-id-only/default-handle inference is rejected, exactly one explicit
  Asset dependency edge is committed/traversable, and RuntimeAssetWorldAdapter
  restore handoff remains non-mutating for Asset dependency edges;
- direct WorldObject/editor object binding remains unopened; the later
  RuntimeAssetWorldAdapter alias/handoff/sidecar gates reuse existing scene
  entity, scene transform, identity-record handoff, and World active restore
  rows and do not authorize editor/GameAdapter/UI/gameplay binding;
- the human lead has resumed execution and requires continuous multi-agent
  coordination until the active accepted goal's stop condition is actually met;
- RTSPINE-008A docs/spec is PASS at
  `ad1a7fb5b3dfa2e1f118103158b640a7111d767f`; the only current
  Package/Resource write lane is RTSPINE-008B Package byte-range/index;
- RTSPINE-008B keeps `archive_byte_offset` and `archive_byte_size` as the
  authoritative pressure byte range, with `byte_offset` and `byte_size` as
  legacy mirrors only, as recorded in
  `docs/gates/RTSPINE_008B_PACKAGE_BYTE_RANGE_LEGACY_MIRROR_DECISION.md`;
- RTSPINE-008C Package artifact hash/dependency integrity implementation and
  focused QA are PASS at `d18f1679ebd389ecec506055764602591f5b9ab6`; focused
  QA task `ba135e38-b73e-4294-b449-97a04b33b982` reports `YuPackageTests`
  build PASS, `^Package_` discovery/execution `35/35` PASS, exact new integrity
  rows `2/2` PASS, diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-008D File/VFS ranged IO implementation is PASS at
  `c67e9710ab39f49ea01f0c194d2e5b44cbf3b97e`; focused QA task
  `aebd28c5-f688-4ccc-abaf-1a3bd61879cb` reports `YuFileTests` build PASS,
  `^File_` discovery/execution `23/23` PASS, ranged subset `4/4` PASS,
  diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-008E Resource payload window/reference budget implementation is PASS
  at `8bb8eff9c98d2a0aa5050c5da6ad94049fa894be`; focused QA task
  `b4fa51c3-aefc-4714-b5d8-062f8a933ac9` reports `YuResourceTests` build
  PASS, Resource window/reference discovery exactly `7` rows, execution `7/7`
  PASS, commit-level diff/hygiene/boundary PASS, and no broad/full CTest;
- RTSPINE-008F Package dependency closure and budgeted load plan implementation
  is PASS at `8509f7e1b6ba15e79c574357a465ddfff4d80e10`; focused QA task
  `4f199c8e-99a4-43b4-a776-8960285ffdaf` reports allowed Package/CMake scope,
  `YuPackageTests` build PASS, exact 008F rows `4/4` PASS, `^Package_`
  focused suite `39/39` PASS, diff/hygiene/scope scan PASS, no broad/full
  CTest, and no QA edits/staging/commits;
- RTSPINE-008G RuntimeAsset packaged validation bridge implementation is PASS
  at `175b6542cf8460b279d1de8a5499e2cbd508c80a`; focused QA task
  `35fdc7a2-c09d-416a-95aa-b4aabdb05d0f` reports focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS, exact packaged validation
  rows `5/5` PASS, adjacent packaged/product rows `8/8` PASS, committed scope
  limited to `CMakeLists.txt`, `RuntimeAssetData.h/.cpp`, and
  `RuntimeAssetDataClosedLoopTests.cpp`, `git diff --check` PASS, and no
  broad/full CTest;
- RTSPINE-008H RuntimeAsset transaction rollback/proof implementation is PASS
  at `1120c3659bf0375f8eb9ef87e042f24c6e5d3ca1`; focused QA task
  `1ec65e79-70f2-4fe5-8f08-6fb0ba2371fd` reports focused
  `YuRuntimeAssetDataClosedLoopTests` build PASS, exact transaction rollback
  row `1/1` PASS, focused rollback/commit/adjacent packaged/product set
  `19/19` PASS, committed scope limited to `CMakeLists.txt`,
  `RuntimeAssetData.h/.cpp`, and `RuntimeAssetDataClosedLoopTests.cpp`,
  `git diff --check` PASS, and no broad/full CTest;
- post-008H RuntimeAsset production-spine follow-through through `50ff335` is
  represented by `7c41265`
  RTSPINE-008I package archive range to RuntimeAsset Resource payload window
  handoff, `e2e8c3c`/`1658639` RuntimeAssetWorldAdapter bridge/handoff,
  `0d2021c` Streaming U64 staging window contract, `bc6d0ee` Resource U64
  payload-window follow-through, `2c93ddf` RuntimeAsset payload logical count
  propagation, `6ac7ff9` Streaming Resource cache payload bridge, `08b1ccd`
  Package payload metadata producer, `35a84c3` legacy artifact compatibility
  fix, `50a654e` Streaming pipeline cache payload consumer, `baae22d`
  rejection/no-mutation coverage fix, `e5cd6ee` Package-to-Streaming artifact
  fixture, `10f7b30` RuntimeAssetData package payload-window consumer,
  `c3cf022` RHI update destination range contract, and `50ff335`
  ResourceUpload destination range consumer;
- the commits above are canonical HEAD implementation/focused-evidence records;
  only the lanes with recorded COMPLETE-PASS VQ in workspace should be cited as
  VQ-closed, and later gates must still run their own scoped VQ before they are
  treated as stage-close proof;
- exact docs-only VQ marker labels for the `50ff335` evidence ledger are
  `RuntimeAssetWorldObjectAdapter`, `RuntimeAssetWorldObjectRestoreHandoff`, and
  `payload_window`;
- current canonical HEAD is `96f0bf0eaed6d2847b4128cd31146340ccc07a0d`;
  `bd1c4ce76534ba454deb72eda9db7b0c32fe256b` records the AFDCA68 docs
  VQ-ready baseline that consumed scout `96c21317-8d03-4a8d-b2e1-a9b3beaa7887`,
  and `96f0bf0eaed6d2847b4128cd31146340ccc07a0d` records implementation and
  focused proof for
  `RTSPINE-WORLDSCENEAUTHORING-ASSET-EDGE-WORLDOBJECT-SNAPSHOT-FAILURE-STATUS-U64-001`;
  implementation task `6b8c3bdd-0398-44ec-a3ea-4adc116d2afe` is COMPLETE-PASS /
  committed, changed only `CMakeLists.txt` and
  `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`, passed
  `YuRuntimeAssetWorldAdapterHandoffTests`, the new
  `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_RejectsAssetEdgeWorldObjectSnapshotFailureWithoutMutation`
  row `1/1`, authoring handoff subset `5/5`, `git diff --check`, and clean
  worktree checks; the proof removes the Model WorldObject before restore,
  expects `GateFailed` / `ProofFailed` / `PlanFailed` with adapter, assembly,
  and restore statuses still successful, preserves identity/transform,
  attachment/binding, and Asset dependency-edge snapshots, and records zero
  accepted/emitted/restored rows with one failed/rejected handoff; VQ guard
  `6df0cd23-9120-42c3-826e-6de352582130` is COMPLETE-PASS /
  `RTSPINE-WORLDSCENEAUTHORING-ASSET-EDGE-WORLDOBJECT-SNAPSHOT-FAILURE-STATUS-U64-001-VQ-READY`,
  so the failure-status row is VQ-ready at `96f0bf0`; read-only scout
  `70161a82-87d2-4408-b8de-cd3ec1af5b00` is COMPLETE-PASS /
  `RTSPINE-CANONICAL-DOCS-HEAD-RECONCILE-96F0BF0-001-NEXT-GATE-MATRIX-READY`
  and records
  `NO_SAFE_WRITE_FRONTIER_REASON / POST-96F0BF0-NARROW-RUNTIMEASSETWORLDADAPTER-SPINE-EXHAUSTED_WITHOUT_BROAD_SCOPE`;
  the successor implementation is the direct
  `RTSPINE-RUNTIMEASSET-WORLDOBJECT-TRANSFORM-APPLICATION-U64-001` gate,
  released as task `f939ed81-da4a-4474-8399-6faf7cc9fde9`; editor/GameAdapter/
  UI/gameplay, broad Resource/File/VFS, Asset auto-lookup, production Asset
  Manager dependency-edge follow-through, and production module gates remain
  separate;
- L0-RES-001 File/VFS loose read/write policy closure is PASS at
  `43cfc18fec4c4c5a5135e4ed15da64c8308247ff`; focused QA task
  `5020f3d6-a492-4138-b81f-c5e80cdd92e2` reports test-only
  `Tests/File/FileTests.cpp` scope, `YuFileTests` build PASS, affected File rows
  `2/2` PASS, `^File_` suite `23/23` PASS, `git diff --check` PASS,
  hygiene PASS, and no broad/full CTest;
- L0-RES-002 Package load-plan/staging baseline closure is PASS at
  `4714199579469a9b1b5e1307b6370fe8f39ce994` without a new implementation
  commit; readiness task `da4f455c` records that the existing `YuPackage`
  manifest/load-plan value contracts and `YuStreaming`
  `PackageResourceStaging*` value/status records already cover the baseline,
  and focused QA task `6aea6396-7af5-43ed-be9a-901e888914d2` reports
  `YuPackageTests` and `YuStreamingTests` build PASS, `^Package_` discovery
  and execution `39/39` PASS, `^Streaming_PackageResourceStaging_` discovery
  and execution `10/10` PASS, a clean read-only QA workspace, and no broad/full
  CTest;
- L0-RES-003 Resource cache/decode chain closure is PASS at
  `8c3a200d813173efe1607e594777afd6f029cc7c`; readiness task `ba6025e8`
  records that existing `YuResource` cache payload, decode plan, decode result,
  decoded payload ownership, release/dependent clear, budget/capacity, and
  no-mutation value/status records cover the baseline; the initial focused QA
  task `085247f3` isolated one decoded-payload capacity status-order issue, fix
  task `abfdb2d1` landed `ResourceRegistry.cpp` only, and focused QA task
  `ca5c3c1b-e61a-4095-8e3c-2e0dfccc2b40` reports `YuResourceTests` build
  PASS, exact `Resource_DecodedPayload_RejectsCapacityOverflow` `1/1` PASS,
  focused cache/decode discovery/execution `65/65` PASS, clean read-only QA,
  and no adjacent/full Resource or broad/full CTest;
- L0-RES-004 Resource residency/upload chain closure is PASS at
  `45f91f6cda02e42f0dce7eae7ff3df6db3616467`; focused QA task
  `2917323c-9869-4a1c-a9fb-67a90b513a23` reports `YuStreamingTests` and
  `YuResourceTests` build PASS, `Streaming_ResourceUpload_` discovery/execution
  `17/17` PASS, `Streaming_ResourceUploadCommit_` discovery/execution `9/9`
  PASS, `Resource_LoadCommit_`/`Resource_Residency_` discovery/execution
  `18/18` PASS, combined focused execution `44/44` PASS, and a clean read-only
  QA workspace; readiness task `d88846fd` records existing
  Resource/Streaming/RHI value/status records for upload queue, upload commit,
  Resource load commit, residency budget/state, pin/unpin/eviction, and
  stale/invalid handle no-mutation; QA did not build or execute `YuRHITests`,
  RHI 38-row dependency execution, adjacent/full Resource, full `^Resource_`,
  or broad/full CTest; RHI primitive evidence remains a dependency note because
  `L0-RHI-003` is not separately table-closed here;
- L0-RES-005 texture bridge to RHI closure is PASS at
  `18030b201c69452a2a7da44fc3d08a4462c3d34f`; readiness task `6fc3d241`
  records that existing `ResourceDecodedTextureBridge` maps decoded texture
  payloads to `ResourceUploadKind::CreateTexture` and sampled texture binding
  values while Resource does not own RHI lifecycle; focused QA task
  `261fa423-28aa-498f-944f-c4eb44cf9d23` reports a read-only clean workspace,
  `YuStreamingTests` focused build PASS, `Streaming_ResourceDecodedTextureBridge_`
  discovery/execution `5/5` PASS, `Streaming_ResourceUpload_.*Texture`
  discovery/execution `2/2` PASS, and RHI texture/sampler/sampling dependency
  discovery count `10`; the RHI dependency set was not executed, RHI closure is
  unchanged, and no adjacent/full Resource/Streaming or broad/full CTest was
  run;
- L0-RES-006 PCM bridge to Audio closure is PASS at
  `804206712988733f38990af6975c67854b16de6a`; readiness task `69ddc757`
  records that existing `AudioResourcePcmPacketImportBridge` maps Resource
  decoded audio metadata into `AudioPcmSamplePacketRequest` values and
  bridge-owned import records while Audio does not parse Resource payloads and
  Resource does not own Audio lifecycle; focused QA task
  `1dbfdaf6-61ff-4ac4-9e47-a2703f2e5a1e` reports a read-only clean workspace,
  `YuAudioResourceTests` focused build PASS,
  `AudioResource_PcmPacketImportBridge_` discovery/execution `8/8` PASS, and
  dependency discovery counts `Audio_PcmSamplePacket_` `13` plus
  `Audio_PcmStreamQueue_` `15`; Audio dependency rows were not executed by this
  lane, `L0-AUD-002` is not table-closed here, and no adjacent/full
  Audio/AudioResource/Resource or broad/full CTest was run;
- L0-RES-007 sample texture/mesh asset path closure is PASS at
  `026f1d06af688ccaa1ff9a421f71ac1ea092cd5a`; readiness task `76377a9a`
  records the lane READY, and focused QA task
  `37d47308-4d38-43d0-85cb-d98f9867b6f8` reports a read-only clean
  workspace, Debug and Release `AssetSmokeDemo` smoke PASS on the current
  machine with `YuAssetSmokeDemo PASS`, `YuAssetSmokeDemo L0_ENGINE PASS`,
  and `YuAssetSmokeDemo L1_PREP PASS`, `YuSampleTests` focused build PASS,
  `Sample_L1VerticalPrep_` discovery/execution `6/6` PASS, dependency
  discovery-only counts `Streaming_ResourceDecodedTextureBridge_` `5`,
  `Streaming_ResourceUpload_.*Texture` `2`, RHI texture/sampler/sampling `10`,
  and RenderCore texture/material/frame/draw/view `60`; generated-output
  hygiene stayed tracked/staged `0`, untracked `0`, with ignored sample/build
  outputs only; L0-SAMPLE-004, L1 sample closure, L0-RHI table closure,
  hardware closure, manual screenshot/listening proof, RenderScene/L1 visual
  implementation, RuntimeAsset/Asset Manager expansion, World/editor/importer,
  UI/GameAdapter/gameplay, material graph, shader compiler pipeline, scene
  loader, old-package compatibility, real codec/parser, Package/Resource public
  API expansion, L0-AUD-005 sample PCM path, L0-AUD-003 callback proof,
  adjacent/full suites, and broad/full CTest remain separate;
- L0-AUD-001 deterministic mixer/test backend closure is PASS at
  `aee81a39d9d9ee063f9f57bc5bab5137d88cbc9f`; readiness task `453eca90`
  records READY, and focused QA task
  `82548add-9a8a-48a7-adf1-ba837608fd07` reports first-slice discovery
  exactly 24 rows, tests `#804` through `#827`, `YuAudioTests` focused build
  PASS, exact 24-row execution `24/24` PASS with `0 failed`, BGM/SE/SFX/music/
  business ID scan `0`, and a clean read-only QA workspace; the executed set
  excluded Callback, PCM packet/stream queue, hardware, sample, and L1 rows, and
  this sync does not close L0-AUD-002, L0-AUD-003, L0-AUD-004, L0-AUD-005,
  L0-RES-006, L0-SAMPLE-006, AudioResource, AudioScene, hardware smoke, sample
  scripts, manual proof, adjacent/full suites, or broad/full CTest;
- L0-AUD-002 PCM packet/stream queue closure is PASS at
  `0de7d7076b73d7d716f6d99dca8ac90ac9974247`; readiness task `821f0e53`
  records READY, and focused QA task `c80e3337-96db-4521-9c0e-b81d5b882efe`
  reports `Audio_PcmSamplePacket_` discovery/execution `13/13`, tests `#840`
  through `#852`, `Audio_PcmStreamQueue_` discovery/execution `15/15`, tests
  `#853` through `#867`, `YuAudioTests` focused build
  PASS, combined exact execution `28/28` PASS with `0 failed`, and a clean
  read-only QA workspace;
  callback rows were discovery-only for later Audio lanes, while hardware,
  sample, AudioResource, AudioScene, L1 rows, adjacent/full suites, and
  broad/full CTest remain separate;
- L0-AUD-003 XAudio2 callback proof closure is PASS at
  `1a1964abbb1ad021d5695ec5ea2e26ee8d5b5f6d`; readiness task `1dec3d24`
  records READY, fast QA task `727479bd-065f-4c6d-9a0f-0cacd2763741`
  reports callback discovery/execution `18/18`, tests `#828` through `#839`
  and `#868` through `#873`, `YuAudioTests` focused build
  PASS, and hardware QA task `fb347834-96a8-4f5c-913d-d3f354e8478e`
  reports hardware/strict hardware discovery `2` rows, tests `#874` through
  `#875`, `YuAudioHardwareSmokeTests` build
  PASS, `windows-hardware-smoke` execution `2/2` PASS, and
  `windows-strict-hardware-smoke` execution `2/2` PASS; this is supported
  hardware evidence with no skip, while callback cost, sample PCM path,
  AudioResource, AudioScene, L1 rows, adjacent/full suites, and broad/full
  CTest remain separate;
- L0-AUD-004 Audio callback cost proof closure is PASS at
  `34093cf83ece469c75baad01e8a99b0e426e3d4e`; readiness task `a1b9ed42`
  records the pre-fix callback handler lock/CV path as NEEDS-IMPLEMENTATION,
  implementation task `bf2b5bc2` lands the single-file
  `Src/YuEngine/Audio/Src/AudioCallbackDeviceWindows.cpp` fix, and focused
  proof QA task `39cca45c` reports commit scope limited to that production
  file, callback hot-path static scan `0` forbidden operations across
  `VoiceCallback` lines `121`-`166` and handler lines `261`-`282`,
  `YuAudioTests` plus `YuAudioHardwareSmokeTests` focused builds
  PASS, fast callback/bridge execution `18/18` PASS, hardware callback
  execution `2/2` PASS, strict hardware callback execution `2/2` PASS,
  `git diff --check HEAD^..HEAD` PASS, and clean read-only QA at
  `HEAD == origin/main == 34093cf`; callback hot path records bounded atomic
  pending events, while lock/CV merge, `WaitForCompletedCallbacks`, and
  `DrainCompletions` stay in non-callback API paths; sample PCM path,
  AudioResource, AudioScene, L1 rows, adjacent/full suites, and broad/full CTest
  remain separate;
- L0-AUD-005 sample PCM path closure is PASS at
  `e14869b9138c750152b7e0ea16f466fd4101a8a8`; readiness task `5270aafd`
  records the existing sample path as READY: synthetic audio resource evidence
  flows through `AssetAudioReadyRecord` and
  `AudioSceneContractQueue::SubmitSourceUpdates` into
  `AudioPcmStreamQueueRequest`, with explicit `BackendUnavailable` status.
  Focused QA task `e4e6cece` reports `YuSampleTests` build
  PASS, `YuAudioSceneTests` build
  PASS, focused regex execution only `8/8` PASS with `0 failed` and
  `0 skipped/not-run`, and clean read-only QA at
  `HEAD == origin/main == e14869b`; hardware output/listening, sample
  scripts/manual proof, L0-SAMPLE-006, AudioResource closure, AudioScene closure,
  L1 ASCENE rows, L1-SAMPLE-007, adjacent/full suites, broad/full CTest, and
  Render/RHI/World/UI/material/shader/scene/importer/package expansion remain
  separate;
- direct WorldObject/editor/GameAdapter binding and broad Resource/File/VFS
  expansion remain blocked until their own gates are released; the current HEAD
  only closes the narrow RuntimeAssetWorldAdapter handoff chain through
  RuntimeAssetData output fixture proof plus the Resource/Streaming
  payload-window, RHI destination-range, and ResourceUpload destination-range
  evidence chain listed above.

Live workspace state is still authoritative for task ownership and current
status. This file records the handoff baseline, not a replacement for the task
board.

## 5. Current Nearest Stage

L0/L1 is closed. The next stage is RuntimeAsset production spine continuation.
It is not broad feature expansion, editor-first work, or product-specific L2
integration.

Required order:

```text
RuntimeAsset container and family identity
-> package/resource index and dependency tables
-> asset-internal scene node / model node / skeleton joint targets
-> animation track/channel binding to target plus property
-> Step/Linear interpolation
-> sampled transform application to runtime instance records
-> ModelNode/SkeletonJoint target-family binding VQ-closed at `3fa4ef7`
-> RuntimeAssetWorldAdapter ModelNode/SkeletonJoint alias handoff VQ-closed at `296100b`
-> RuntimeAssetWorldAdapter handoff target-family proof VQ-closed at `54e02e0`
-> RuntimeAssetWorldAdapter handoff attachment/resource binding sidecar proof VQ-closed at `4d9f244`
-> RuntimeAssetWorldAdapter handoff sidecar assembly restore VQ-closed at `f85c677`
-> RuntimeAssetWorldAdapter handoff sidecar failure status VQ-closed at `4587c7d`
-> RuntimeAssetData to RuntimeAssetWorldAdapter handoff fixture VQ-closed at `e512d39`
-> RuntimeAssetWorldAdapter World scene record stream handoff VQ-closed at `088f21e`
-> WorldSceneAuthoring runtime export to RuntimeAssetWorldAdapter handoff VQ-closed at `f8ef490`
-> WorldSceneAuthoring attachment/binding/dependency export handoff VQ-closed at `318daee`
-> WorldSceneAuthoring dependency export to Resource edge handoff VQ-closed at `f967001`
-> WorldSceneAuthoring dependency export to Asset edge handoff VQ-closed at `5802141`
-> WorldSceneAuthoring Asset-edge WorldObject snapshot handoff implemented as
   focused evidence at `bcfd6ea`
-> worktree isolation baseline at `afdca68`
-> AFDCA68 docs reconcile baseline committed at `bd1c4ce`
-> Asset-edge WorldObject snapshot failure/no-mutation implementation evidence
   landed at `96f0bf0`; VQ guard `6df0cd23` is COMPLETE-PASS / VQ-READY
-> Resource/Streaming payload-window follow-through as narrow evidence already
   landed through `50ff335`
-> read-only scout `70161a82` marks the narrow RuntimeAssetWorldAdapter spine
   exhausted and releases direct RuntimeAsset WorldObject transform application
   as task `f939ed81`
-> editor/GameAdapter binding or broader Resource/File/VFS expansion only after
   their own gates are released
-> editor/importer authoring surfaces after runtime contracts pass
```

Animation must not bind directly to WorldObject, editor object, raw pointer,
display name, or file path.

## 6. Historical Documents Policy

Historical docs stay in the repository because they contain evidence and design
context. They do not automatically direct future work.

Treat these categories as historical unless a canonical document explicitly
references them:

- `*_RAV*.md`
- `*_EDITOR_*.md`
- old bridge audits and queue documents;
- old phase documents;
- old preview/resource-browser/UI workflow documents;
- old per-gate plans that predate the current production-spine correction.

Do not delete evidence documents casually. If the team decides to hard-clean the
directory later, do it as a separate docs archival task with a table mapping
each removed or moved document to the canonical document that replaces it.

## 7. Stop Conditions

Stop and return to architecture if any future work:

- bypasses Package/Resource/RuntimeAsset and claims production asset proof;
- binds animation or scene data directly to WorldObject or editor ids;
- treats reports, screenshots, viewers, or logs as runtime behavior;
- opens a new implementation lane before the active evidence gate closes;
- uses old TouhouNewWorld compatibility to define first-class L0/L1 contracts;
- expands editor/UI/gameplay before runtime contracts are stable.

## 8. Continuous Execution Rule

The coordinator must continue driving the active accepted plan until its stop
condition is met. Do not stop after one task, and do not wait for the human lead
to say "continue" while accepted work remains. L0/L1 is already closed; the
current continuation lane is RuntimeAsset production spine work unless a newer
workspace goal explicitly changes the active lane.

Required execution discipline:

- keep the architect on coordination, dependency control, design decisions, and
  evidence governance rather than routine frontline implementation;
- split real parallel work only when tasks have independent read or write
  surfaces and do not depend on each other;
- do not create fake parallel QA lanes that only serialize one dependency chain
  or burn review tokens without increasing throughput;
- every shared task must state scope, non-goals, expected evidence, AI ETA, and
  stale-owner timeout behavior;
- set workspace timers for owner checkpoints and reroute stale work instead of
  waiting indefinitely;
- focused evidence is the default; broad full-suite testing is reserved for
  explicit shared-contract, release, or high-risk decisions;
- never open a write lane that can invalidate an active evidence gate.

The current safe pattern is: keep VQ read-only, immediately release the next
non-conflicting lane once its dependency closes, and keep unrelated read-only
architecture, prep, and pressure-audit lanes moving in parallel.
