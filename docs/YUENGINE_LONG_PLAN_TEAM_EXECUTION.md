# YuEngine Long Plan And Team Execution

Status: active commercial-engine execution plan
Owner: Architect, lead engineer
Started: 2026-06-11
Current planning checkpoint: `origin/main@96f0bf0eaed6d2847b4128cd31146340ccc07a0d`
Reference product target: `C:\Steam\steamapps\common\TouhouNewWorld`
Canonical entry point: `docs/README.md`

## 0. New Team Handoff Rule

Start from `docs/README.md`, then read this file. This document is the current
long-horizon plan. Older phase, RAV, editor, bridge, preview, resource-browser,
and UI documents are historical/evidence context unless this file or
`docs/README.md` explicitly promotes them back into the active plan.

If context is compacted or a new team takes over, preserve these facts first:

- YuEngine's original target is unchanged: a complete, self-developed native
  commercial game engine for the team's games, not a toy engine, report system,
  asset viewer, or UE/Unity clone.
- UE and Unity are responsibility and workflow references, with Unity editor
  workflow especially important.
- External DCC import is limited to outside-authored data such as model,
  texture, skeleton, animation, and audio. Scene, UI, material-instance, level,
  and gameplay structure are authored inside the YuEngine editor.
- Shader is authored like code or game logic: source, compile, reflect, bind,
  and package through explicit shader/runtime contracts.
- Packaging means asset bundles plus script/logic/shader compile outputs and a
  runnable product package.
- Package, Resource, and RuntimeAsset are the production spine, but they are
  not a byte-size stress goal.
- RuntimeAsset must define asset-internal targets before deeper animation,
  model, scene, or WorldObject binding.
- L0/L1 is complete at
  `a4a520ffbdb1049dd4674330854033c61c91d6bb`; final VQ reports
  `COMPLETE-PASS / FINAL-L0L1-DONE-READY`.
- The current nearest stage is RuntimeAsset production spine continuation.
- The human lead has resumed continuous execution. The coordinator must keep the
  team moving through the active accepted goal until its stop condition is met.
- The architect should preserve context for architecture, dependency control,
  task design, and evidence governance, not spend the main lane on routine
  frontline implementation.

## 1. Final Product Target

YuEngine is the production engine for a small independent team. It is not a
toy engine, a technology demo, a public UE/Unity competitor, an editor-first
research project, or a continuation of old one-off game code. The final target
is a stable native commercial game engine that can ship, patch, diagnose, and
maintain the team's own games.

UE and Unity remain the main references for architecture responsibility and
workflow completeness. Unity is the strongest editor-workflow reference. The
project must not arbitrarily treat Unity/UE-level editor, scene, asset, shader,
UI, package, or runtime responsibilities as optional just because the current
slice is small.

The local TouhouNewWorld package is an observed reference workload:

- native runtime binaries under `bin`;
- no observed Unity runtime markers such as `UnityPlayer.dll` or
  `GameAssembly.dll`;
- packed resource archive surfaces;
- explicit config, shader/filter, database, and resource archive surfaces.

The reference workload is not the project goal and is not a byte-size stress
target. YuEngine must support real-project production use:

- stable long-session play;
- deterministic startup, load, update, render, audio, save, and shutdown;
- high-performance runtime loading from packed assets;
- explicit diagnostics for missing layers and production failures;
- repeatable build, package, validation, and release evidence.

Authoring boundaries are part of the product target:

- external DCC import: model, texture, skeleton, animation, audio, and similar
  outside-authored content;
- engine editor authoring: scene, UI, material instance, level/world structure,
  gameplay-facing object composition, and product packaging;
- shader authoring: code-like source, compile, reflection, binding, and package
  records, not a generic external DCC import path;
- packaging: asset bundles plus script/logic/shader compile outputs and a
  runnable product package.

## 2. Non-Negotiable Architecture Principles

1. Runtime first, editor later.
2. Package, Resource, and RuntimeAsset are the product spine.
3. Assets never bind directly to world instances or editor objects.
4. Runtime assets bind to stable asset-internal targets first.
5. World and scene instances map asset targets to runtime objects later.
6. Reports, screenshots, logs, and viewers are evidence tools, not behavior.
7. Every failure path must name the failing layer and avoid partial mutation.
8. Every direct-main implementation slice must have focused evidence.
9. Compatibility with old game packages is not a design constraint unless a
   separate accepted gate says so.
10. No public plugin ecosystem, marketplace, or generic editor extension work
    belongs in the current L0/L1 execution window.
11. Long-term planning is capability-graph driven, not whole-module
    serialization. A later capability may begin when its exact prerequisites are
    stable; it must not wait for entire upstream modules to be complete, and it
    must not invent temporary lower-layer substitutes.

## 3. Long-Horizon Roadmap

The roadmap is a dependency graph of product capabilities, not a queue that
finishes one whole module before starting the next. Each implementation slice
must name the exact upstream capability it consumes and the exact downstream
capability it unlocks.

| Capability lane | Required prerequisites | Owns | Must not do |
| --- | --- | --- | --- |
| Package/Resource budget and index | File/VFS ranged IO, package tables, stable hashes, explicit byte/window budgets | bundle layout, mounted lookup, dependency records, no-mutation failures | claim old-package compatibility or byte-size goals |
| RuntimeAsset family contracts | Package/Resource bytes and indexes, asset family ids, dependency tables | source/cooked schemas, validators, loaded records, stable asset-internal identity | bind asset files directly to WorldObject, editor object, raw pointer, display name, or file path |
| Shader capability | RuntimeAsset shader source/cooked records, Package/Resource payload/hash records, RenderCore/RHI shader module and pipeline descriptors, diagnostics | shader source policy, compiler backend selection, reflection, bytecode/hash identity, material binding records | design camera control, viewport input, material graph editor, or DCC importer behavior |
| Modern render pipeline | RHI device/swapchain/command/capture, RenderCore pass/material contracts, RuntimeAsset mesh/material/texture/shader records, scene submission records | frame/pass graph, render queues, resource binding, draw submission, capture evidence | author scenes/UI or hide asset failures inside render code |
| Scene/world authoring | Object/World identity, Transform/Component records, RuntimeAsset target identity, Resource/Asset dependency edges | engine-authored scene files, hierarchy, component data, dependency export, runtime export | import scene files from DCC as the primary authoring path or hard-code preview input |
| UI framework/editor | UI runtime core, Resource font/texture handles, RenderCore UI draw path, input routing when the UI stage reaches interaction | UI document/schema, layout, style, widget tree, hierarchy/inspector, runtime preview | treat UI as external DCC import or game-specific menu code |
| Preview host/editor viewport | editor host lifecycle, RenderScene/RenderCore capture, selected runtime document data, input routing only when this lane reaches interaction | inspectable runtime preview, selection feedback, transform/edit commands | create fake camera/input/audio systems before their product lanes define contracts |
| Script/game logic | Object/World services, reflection/native bridge, package/runtime records, diagnostics | script compile, ABI, lifecycle, runtime calls, failure reporting | implement title gameplay before engine service contracts exist |
| Product package/run | Package builder, RuntimeAsset families, shader/script compile outputs, Resource indexes, release diagnostics | asset bundle outputs, runnable package, validation, crash/patch evidence | add marketplace/plugin ecosystem or unrelated platform tooling |

Example rule: Shader can advance before the whole renderer or whole asset
system is finished, but only through the narrow prerequisites above. A shader
slice may consume current RHI/RenderCore descriptor contracts and
RuntimeAsset/Package records; it may not create temporary camera controls,
hard-coded viewport input, or a pretend material editor to make the slice look
usable.

### Current Short-Term Plan

There is only one current short-term plan. It is not a list of future-stage
mini-plans.

Current short-term scope:

- restore the unchanged product target in canonical docs;
- replace whole-module ordering with the capability dependency graph above;
- freeze the authoring boundary for external DCC import, engine-authored
  scene/UI/material data, shader-as-code, and asset-bundle packaging;
- audit current planning language for temporary lower-layer substitutes such as
  hard-coded preview input, fake camera control, editor-owned runtime behavior,
  or byte-size stress goals;
- release no new feature implementation lane until the current capability
  boundary names its exact prerequisites and non-goals.

Current short-term exit standard:

```text
Canonical planning documents agree on the unchanged product target, the
cross-module capability graph, and the current stage boundary. Future stages
remain long-term architecture only, not active short-term work.
```

### Stage A: L0 Production Foundation

Purpose: make the lower engine reliable enough to run and diagnose real native
runtime work.

Required closure:

- Platform window, event pump, native surface, and shutdown.
- RHI and D3D11 device, swapchain, present, capture, resize, and stale-handle
  failures.
- RenderCore frame packet, draw packet, material values, and graph skeleton.
- File/VFS, package load-plan, resource cache, decode, upload, residency, and
  streaming commit.
- Audio mixer, PCM packet/queue, XAudio2 availability, and callback cost
  discipline.
- Input replay, Win32 input, XInput availability, and command snapshots.
- Diagnostics, memory, and thread cost surfaces.
- One engine-owned lower sample that proves window -> input -> resource ->
  render -> audio -> resize -> shutdown or reports exact blockers.

Exit standard:

```text
Debug/Release/fast/hardware/sample evidence is reproducible, or every missing
hardware path is explicitly graded as an environment blocker.
```

### Stage B: L1 Runtime Core

Purpose: make project-independent runtime systems compose into a real scene.

Required closure:

- RuntimeApp and FrameContext phases.
- Object, component, transform, and scene assembly records.
- Runtime Asset Manager over Resource/Package/File.
- RenderScene over RenderCore/RHI.
- AudioScene over Audio/Resource.
- Input mapping over L0 input devices/actions.
- Serialize/config/save value boundaries.
- Runtime diagnostics counters.
- Pure runtime visual sample with cube, cylinder, cone, shared material,
  multiple texture inputs, camera tween, animation-driven transform, and bounded
  capture output.

Exit standard:

```text
A project-independent scene can load, instantiate, render, play audio, respond
to input, serialize core state, and shut down without project-specific shortcuts.
```

### Stage C: Runtime Asset Production Spine

Purpose: turn source/cooked assets into runtime records that can sustain a
real shipped product without editor-owned runtime behavior.

Required closure:

- Source artifact schema and cooked binary artifact schema.
- Archive/package table directory, offsets, hashes, dependency tables, and
  budget limits.
- Resource index equivalent to a production asset database.
- Runtime asset family contracts for mesh, texture, material, shader, scene,
  camera, model, skeleton, animation, audio, and UI when needed.
- Validator/cook/load stages with no-mutation failures.
- Incremental asset validation and packaging commands.

Exit standard:

```text
The engine can mount, validate, load, and diagnose packed content without loose
test-only construction or editor-owned runtime behavior.
```

### Stage D: Product Runtime Feature Set

Purpose: support the first real team game.

Required closure:

- Scene/world object lifecycle and instance mapping.
- Model, skeleton, animation clips, tracks, curves, and transform application.
- Shader reflection, material binding, texture streaming, and render pipeline
  value contracts.
- Audio source/bus routing and streaming.
- Input maps and replayable command snapshots.
- Save/profile/config persistence policy outside Serialize core.
- Crash-safe shutdown and resource release.

Exit standard:

```text
A representative vertical game slice can run for long sessions with bounded
memory, stable frame pacing, reproducible loads, and actionable diagnostics.
```

### Stage E: Production Tools

Purpose: make content creation efficient only after runtime contracts are firm.

Required closure:

- Importer and cooker.
- Resource browser and validator diagnostics.
- Scene/model/material/animation preview tools.
- Package builder and release validator.
- Performance capture, asset budget reports, and crash triage.

Exit standard:

```text
Artists and designers can produce content through tools that emit the same
runtime contracts proven by the engine.
```

### Stage F: Shipping And Maintenance

Purpose: make the engine and game survive commercial release.

Required closure:

- Release packaging.
- Patch/update compatibility policy for YuEngine packages.
- Long-session soak tests.
- Save compatibility and migration gates.
- Hardware matrix and graded environment failures.
- Versioned crash diagnostics.

Exit standard:

```text
The team can ship, patch, diagnose, and maintain a commercial game without
depending on one-off manual knowledge.
```

## 4. Current State

The old P1/P2/P3 first-slice language is historical. It should no longer drive
new task selection by itself.

Current mainline has meaningful lower-engine and RuntimeAsset evidence:

- Object, Serialize, Resource, Package, RHI, RenderCore, Audio, Input, World,
  and diagnostics have useful value-contract slices.
- RuntimeAsset has disk-backed closed-loop evidence through File/VFS/Resource,
  validators, RenderScene, RenderCore, RHI, package/product run ledgers, shader
  reflection hardening, and scene-animation selected-clip proof.
- RuntimeAsset/Streaming/RHI evidence ledger also includes the post-008H
  payload-window and destination-range chain through
  `origin/main@50ff335fe1ddfea77a72ce20f770baa3028df4a2`.
- Scene-animation implementation and QA are complete at
  `f211f7f95299388987ccef00b4d1e8ee6f7bf0c1`.
- Docs evidence sync is complete at
  `0a9144b0e30cbede56a5dbf04b232f3e5b763802`.

The project is not blocked by lack of proof. It is at risk if proof slices keep
expanding without a production spine.

## 5. Nearest Detailed Stage

The nearest stage is not another broad feature. It is the RuntimeAsset product
spine correction.

### 5.1 Close Current Evidence Gate

Current RTSPINE evidence gate state:

- implementation: done;
- QA: done;
- docs: done;
- scene-animation VQ consistency evidence: verify as regression coverage inside
  the RTSPINE-003 VQ gate instead of reopening a separate write lane;
- RTSPINE-003 target identity implementation: done at
  `origin/main@5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4`;
- RTSPINE-003 focused QA: done, `10/10` focused target identity plus
  scene/runtime animation regression rows PASS;
- RTSPINE-003 docs: synchronized by workspace task
  `d9dc3692-aa12-4f5c-872a-5b7293a92ceb`;
- RTSPINE-003 VQ consistency audit: done / COMPLETE-PASS by workspace task
  `fdd78da4-da12-4956-b6ac-63ff9e377121`.

This gate is closed. RTSPINE-004 may open as the only implementation write lane.
Independent read-only architecture, code-surface scouting, and budget/capacity
audit lanes may run in parallel when they cannot mutate the same evidence gate.

### 5.2 Correct RuntimeAsset Dependency Order

The next planning and implementation queue must use this order:

```text
RuntimeAsset container and family identity
-> package/resource index and dependency tables
-> asset-internal node/model/skeleton target contract
-> animation clip/track/channel binding to those targets
-> Step/Linear interpolation sampler
-> transform application to runtime instance records
-> world object mapping only after instance contracts exist
-> editor/importer authoring surfaces after runtime contracts pass
```

This order is mandatory. Animation must not bind directly to WorldObject,
editor object, scene instance, raw pointer, display name, or file path.

### 5.3 Immediate Backlog

| ID | Work item | Acceptance |
| --- | --- | --- |
| RTSPINE-001 | Final VQ for scene-animation evidence gate | Implementation, QA, docs, `origin/main`, and evidence matrices agree; no next lane opens before PASS |
| RTSPINE-002 | RuntimeAsset production target addendum | Long-term target, TouhouNewWorld package reference, and no-compatibility policy are written into planning docs |
| RTSPINE-003 | Asset-internal target identity contract | PASS at `origin/main@5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4`: scene node, model node, and skeleton joint target IDs are bounded output records independent from WorldObject; duplicate id, missing parent, and capacity overflow fail without mutation |
| RTSPINE-004 | Animation track target binding contract | Tracks bind to target ID plus property, not world instance or editor object |
| RTSPINE-005 | Minimal interpolation contract | PASS at `origin/main@2bfe7e37d36ca711dd706728f21b1e4caecfd3db` with focused QA at `origin/main@d18f1679ebd389ecec506055764602591f5b9ab6`: Step and Linear scalar/vector/transform sampling pass fixed-time tests with no hidden global time; unsupported interpolation and sample output capacity fail without mutation |
| RTSPINE-006 | Invalid target failure contract | PASS at `origin/main@96e0c024435f670c39ced019ff825b819a6830a3`; target-family mismatch and sample failure diagnostics fail without output mutation, and focused QA task `6d02c260-936a-456b-917b-5c2802bbb666` reports focused RuntimeAsset regex `8/8` PASS plus exact new rows `2/2` PASS without broad/full CTest |
| RTSPINE-007 | Runtime instance mapping contract | PASS at `origin/main@37a112549190ac2123abcd72b5c688cdfa5b01e5`: asset target records map to caller-owned runtime instance rows for scene entities before any WorldObject/editor binding; focused QA task `6b6baf5f-2381-4b9c-89b1-4411fba53d23` reports exact RuntimeInstanceMapping rows `5/5` PASS without broad/full CTest |
| RTSPINE-008 | Package/resource index budget and capacity gate | 008A spec, 008B byte-range/index, 008C Package hash/dependency integrity, 008D File/VFS ranged IO, 008E Resource payload window/reference budget, 008F Package dependency closure/budgeted load plan, 008G RuntimeAsset packaged validation bridge, 008H RuntimeAsset transaction rollback/proof, and 008I Package archive range to RuntimeAsset Resource payload-window handoff are represented in current HEAD evidence; cite VQ-closed status only for lanes with workspace COMPLETE-PASS VQ |
| RTSPINE-009 | Payload-window and destination-range follow-through ledger | The `50ff335` ledger records `e2e8c3c`/`1658639` RuntimeAssetWorldAdapter bridge/handoff with exact marker aliases `RuntimeAssetWorldObjectAdapter` and `RuntimeAssetWorldObjectRestoreHandoff`, `0d2021c` Streaming U64 staging, `bc6d0ee` Resource U64 `payload_window`, `2c93ddf` RuntimeAsset payload logical count, `6ac7ff9` Streaming cache payload bridge, `08b1ccd`/`35a84c3` Package payload metadata and legacy compatibility, `50a654e`/`baae22d` Streaming pipeline cache payload consumer plus rejection/no-mutation coverage, `e5cd6ee` Package-to-Streaming artifact fixture, `10f7b30` RuntimeAssetData package payload-window consumer, `c3cf022` RHI update destination range contract, and `50ff335` ResourceUpload destination range consumer; this is implementation/focused evidence until a scoped VQ result accepts the individual lane |
| RTSPINE-010 | ModelNode/SkeletonJoint target-family binding | VQ-closed at `origin/main@3fa4ef7bd42da8f60bd5ebb3a7f863bd76292c84`: implementation task `06724fe5-b2e4-410e-97e7-2b41c195c3a0` is COMPLETE-PASS / committed, VQ task `04e2a7a6-eac5-41d2-9624-6e5e952859c4` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt`, `RuntimeAssetData.cpp`, and `RuntimeAssetDataClosedLoopTests.cpp`, focused discovery found `17` rows including new Model/Skeleton target-family rows, execution `17/17` PASS, and old unsupported target-family labels are absent |
| RTSPINE-011 | RuntimeAssetWorldAdapter target-family alias handoff | VQ-closed at `origin/main@296100b3bda25e962c3a3a503f9f78f0160083ce`: implementation task `77376606-d3d8-45de-8079-79121593b8e7` is COMPLETE-PASS / committed, VQ task `5fb82855-a437-4eb7-b078-373069988b2d` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt`, `RuntimeAssetWorldObjectAdapterBridge.cpp`, and `RuntimeAssetWorldObjectAdapterBridgeTest.cpp`, focused RuntimeAssetWorldObjectAdapter matrix reports `13/13` PASS including Model/Skeleton alias handoff, and SceneNode plus invalid/no-mutation semantics are preserved |
| RTSPINE-012 | RuntimeAssetWorldAdapter handoff target-family proof | VQ-closed at `origin/main@54e02e049bb6f67fd15ca32d1675f1c61380ae70`: implementation task `53b6d5dc-fd17-442c-b18b-9257c4f3650c` is COMPLETE-PASS / committed, VQ task `8fbe251e-2c14-4786-a48c-5b8b0b6f8e14` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt` and `RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`, focused RuntimeAssetWorldObjectRestoreHandoff discovery/execution reports `5/5` PASS including Model/Skeleton alias restore handoff, the Unknown negative row preserves no-mutation failure semantics, and no production bridge source was changed |
| RTSPINE-013 | RuntimeAssetWorldAdapter handoff attachment/resource binding sidecar proof | VQ-closed at `origin/main@4d9f244ca373c466478b54b7fbc0dd91bf8b5720`: implementation task `3d8c0c2b-987c-4046-8f01-4e04f16f3715` is COMPLETE-PASS / committed, VQ task `4607e700-6bd8-4f0d-a508-ac86b991e7e7` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt` and `RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`, production handoff bridge/state files were unchanged, focused RuntimeAssetWorldObjectRestoreHandoff discovery/execution reports `6/6` PASS including `RuntimeAssetWorldObjectRestoreHandoff_CarriesAttachmentAndBindingGateRecordsForTargetAliases`, and the proof covers non-zero component attachment plus component resource binding sidecar gate records for ModelNode/SkeletonJoint aliases through the existing adapter/world active restore path |
| RTSPINE-014 | RuntimeAssetWorldAdapter handoff sidecar assembly restore | VQ-closed at `origin/main@f85c67701f2ff90c94c84cdc2761e434524128d8`: implementation task `81f4806a-cfc4-464b-a644-b163bfc0459f` is COMPLETE-PASS / committed, VQ task `dac5643f-7225-4ba0-a76b-c063178dfb97` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt`, `RuntimeAssetWorldObjectRestoreHandoffState.h`, `RuntimeAssetWorldObjectRestoreHandoffBridge.cpp`, and `RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`, focused builds `YuRuntimeAssetWorldAdapterHandoffTests`, `YuRuntimeAssetWorldAdapterTests`, and `YuWorldTests` PASS, focused RuntimeAssetWorldObjectRestoreHandoff discovery/execution reports `7/7` PASS including `RuntimeAssetWorldObjectRestoreHandoff_RestoresAttachmentAndBindingSidecarsThroughWorldAssembly`, RuntimeAssetWorldObjectAdapter and RuntimeAssetWorldObjectRestoreHandoff rows report `20/20` PASS, WorldSceneAssemblyBridge rows report `27/27` PASS, and direct WorldObject/editor/GameAdapter/UI plus broader Resource/File/VFS closure remains out of scope |
| RTSPINE-015 | RuntimeAssetWorldAdapter handoff sidecar failure status | VQ-closed at `origin/main@4587c7d1f204663577950241d4c42a5b72ab03a1`: implementation task `ab4eb0f5-0350-49af-8da3-13b4c47dda8b` is COMPLETE-PASS / committed, VQ task `f0c0c54e-32bd-4ee2-9dc2-b8c10c68c59a` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt`, `RuntimeAssetWorldObjectRestoreHandoffBridge.h`, `RuntimeAssetWorldObjectRestoreHandoffResult.h`, `RuntimeAssetWorldObjectRestoreHandoffSnapshot.h`, `RuntimeAssetWorldObjectRestoreHandoffState.h`, `RuntimeAssetWorldObjectRestoreHandoffBridge.cpp`, and `RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`, `RuntimeAssetWorldObjectRestoreHandoffStatus.h` was allowed but unchanged, result/state/snapshot expose `WorldSceneAssemblyStatus`, handoff failure remains `RestoreFailed`, focused handoff rows report `8/8` PASS including `RuntimeAssetWorldObjectRestoreHandoff_ExposesSidecarAssemblyFailureStatus`, adapter plus handoff rows report `21/21` PASS, WorldSceneAssemblyBridge rows report `27/27` PASS, and direct WorldObject/editor/GameAdapter/UI plus broader Resource/File/VFS closure remains out of scope |
| RTSPINE-016 | RuntimeAssetData to RuntimeAssetWorldAdapter handoff fixture | VQ-closed at `origin/main@e512d3990412f90b38aee8469845c44e188dd275`: implementation task `150e051b` is COMPLETE-PASS / committed, VQ task `6f086b28-40e3-4574-bac5-33e587b2e91c` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt`, `RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`, and `RuntimeAssetWorldObjectDataHandoffFixtureTest.cpp`, no production RuntimeAsset/RuntimeAssetWorldAdapter/World/Resource or docs/editor/GameAdapter/UI/broad File/VFS paths changed, focused `YuRuntimeAssetWorldAdapterHandoffTests` build PASS, new data handoff row `1/1` PASS, RuntimeAssetWorldObject Adapter/Handoff/DataHandoff rows report `22/22` PASS, RuntimeAssetData target/mapping rows report `14/14` PASS, World active gate/object restore rows report `34/34` PASS, and the fixture proves RuntimeAssetData `runtime_instance_mappings`, `scene_entities`, and `scene_transforms` feed RuntimeAssetWorldAdapter restore handoff through caller-owned World/ObjectRegistry identity and transform destinations without writing object handles/world object ids into asset files |
| RTSPINE-017 | RuntimeAssetWorldAdapter World scene record-stream handoff | VQ-closed at `origin/main@088f21eb313be7c0e6ff283af922f23ec335ee09`: implementation task `1ea59c46` is COMPLETE-PASS / committed, VQ task `943f29a6-8a24-4a77-8e8a-4366285890b4` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt`, `RuntimeAssetWorldObjectRecordStreamHandoffFixtureTest.cpp`, and `RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`, no docs or production RuntimeAsset/RuntimeAssetWorldAdapter/World/Resource paths changed, focused `YuRuntimeAssetWorldAdapterHandoffTests` build PASS, new record-stream row `1/1` PASS, RuntimeAssetWorldObject Adapter/Handoff/DataHandoff rows report `22/22` PASS, WorldScene stream/decoded-plan/proof rows report `143/143` PASS, WorldSceneActiveRestoreGate rows report `4/4` PASS, and the fixture proves RuntimeAssetWorldAdapter handoff can consume caller-owned World scene record-stream and decoded-plan outputs through existing in-memory World value-stream/proof records without opening direct WorldObject/editor/GameAdapter/UI/gameplay or broader Resource/File/VFS |
| RTSPINE-018 | WorldSceneAuthoring runtime export to RuntimeAssetWorldAdapter handoff | VQ-closed at `origin/main@f8ef490493f43a97d2958d6a604e598b68f1fcab`: implementation task `c1a469a8` is COMPLETE-PASS / committed, VQ task `5c536c0b-7849-436b-a695-e323f8afd339` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt`, `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`, and `RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`, no docs or production RuntimeAsset/RuntimeAssetWorldAdapter/World/Resource paths changed, focused `YuRuntimeAssetWorldAdapterHandoffTests` build PASS, new authoring runtime export handoff row `1/1` PASS, RuntimeAssetWorldObject handoff label set reports `24/24` PASS, WorldSceneAuthoringDocument rows report `9/9` PASS, WorldScene stream/decoded-plan/proof rows report `143/143` PASS, WorldSceneActiveRestoreGate rows report `4/4` PASS, and the fixture proves `WorldSceneAuthoringDocumentValidator::ValidateAndExport` runtime export records feed existing WorldScene record-stream, decoded-plan, proof, active-gate, and RuntimeAssetWorldAdapter restore handoff plumbing without opening direct WorldObject/editor/GameAdapter/UI/gameplay or broader Resource/File/VFS |
| RTSPINE-019 | WorldSceneAuthoring attachment/binding/dependency export to RuntimeAssetWorldAdapter handoff | VQ-closed at `origin/main@318daeecef8905554bef459e998bd791eafa08bd`: implementation task `9184605b` is COMPLETE-PASS / committed, VQ task `97628142-20ed-4e09-ae05-be61f0226c44` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt` and `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`, no docs or production RuntimeAsset/RuntimeAssetWorldAdapter/World/Resource paths changed, focused `YuRuntimeAssetWorldAdapterHandoffTests` build PASS, new `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_FeedsAttachmentBindingDependencyExportThroughRecordStreamIntoRestoreHandoff` row reports `1/1` PASS, authoring handoff rows report `2/2` PASS, RuntimeAssetWorldObject handoff rows report `12/12` PASS, WorldSceneAuthoringDocument rows report `9/9` PASS, WorldScene stream/plan/proof/gate rows report `88/88` PASS, and dependency remains exported data only with no Resource/File/VFS loading, no dependency graph traversal, and no direct WorldObject/editor/GameAdapter/UI/gameplay or broader Resource/File/VFS gate opened |
| RTSPINE-020 | WorldSceneAuthoring dependency export to Resource edge handoff | VQ-closed at `origin/main@f967001c39a53717226127ff67c316c8a3bf2a4a`: implementation task `0c9911ab` is COMPLETE-PASS / committed, VQ task `86ce7cfd-627c-445a-ae03-f639d47cec13` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt` and `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`, no docs or production RuntimeAsset/RuntimeAssetWorldAdapter/World/Resource paths changed, focused `YuRuntimeAssetWorldAdapterHandoffTests` build PASS, new `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_CommitsExportedDependencyAsCallerOwnedResourceEdge` row reports `1/1` PASS, authoring handoff rows report `3/3` PASS, WorldSceneAuthoringDocument rows report `9/9` PASS, Resource and RuntimeAssetData dependency baseline rows report `4/4` PASS, exported `WorldSceneAuthoringDependencyRecord.resource_handle` can be committed as a caller-owned `ResourceRegistry` dependency edge using caller-owned dependent and exported dependency `ResourceHandle` values, `stable_resource_id`-only inference is rejected, RuntimeAssetWorldAdapter restore handoff remains non-mutating for Resource dependency edges, and no direct WorldObject/editor/GameAdapter/UI/gameplay, broad Resource/File/VFS loading/decoding/dependency traversal, or Asset Manager dependency-edge follow-through gate is opened |
| RTSPINE-021 | WorldSceneAuthoring dependency export to Asset edge handoff | VQ-closed at `origin/main@58021419256fc68cd7a84692fd42dbc7a3d0f08e`: implementation task `56ab9999` is COMPLETE-PASS / committed, VQ task `b75a7379-ecb4-4024-ad57-1512833b2c5e` is COMPLETE-PASS / VQ-READY, changed paths were exactly `CMakeLists.txt` and `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`, no docs or production RuntimeAsset/RuntimeAssetWorldAdapter/World/Resource/Asset paths changed, focused `YuRuntimeAssetWorldAdapterHandoffTests` build PASS, focused matrix rows report `23/23` PASS, new `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_CommitsExportedDependencyAsCallerOwnedAssetEdge` row reports `1/1` PASS, authoring handoff rows report `4/4` PASS, Asset baseline rows report `9/9` PASS, RuntimeAssetData dependency/resource-asset edge rows report `10/10` PASS, exported WorldSceneAuthoring dependency rows can be committed as caller-owned `AssetManager` dependency edges using explicit caller-owned `AssetHandle` scene/document and dependency records, stable-id-only/default-handle inference is rejected, exactly one explicit Asset dependency edge is committed and traversable, RuntimeAssetWorldAdapter restore handoff remains non-mutating for Asset dependency edges, and no direct WorldObject/editor/GameAdapter/UI/gameplay, broad Resource/File/VFS, Asset auto-lookup, or production Asset Manager dependency-edge follow-through gate is opened |
| RTSPINE-022 | WorldSceneAuthoring Asset-edge WorldObject snapshot handoff | Implementation/focused evidence at `origin/main@bcfd6eaad3fc198eb4dbba4e31e49c1eed68c0db`: implementation task `e5b2a316-da0b-438f-8073-9315b362d304` is COMPLETE-PASS / committed, changed path was exactly `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`, `CMakeLists.txt` was inspected but unchanged, no docs or production RuntimeAsset/RuntimeAssetWorldAdapter/World/Resource/Asset paths changed, focused `YuRuntimeAssetWorldAdapterHandoffTests` build PASS, exact Asset-edge row PASS, authoring handoff rows report `4/4` PASS, same-module RuntimeAssetWorldObject/RuntimeAssetWorldAdapter focused matrix reports `27/27` PASS, Asset baseline rows report `9/9` PASS, RuntimeAssetData dependency/resource/package superset reports `34/34` PASS, `git diff --check` and commit path audit PASS, and the test proves the caller-owned Asset-edge handoff restores WorldObject identity and transform destinations with matching identity/transform snapshot counts and RuntimeAssetWorldAdapter restored identity/transform counts; this is not yet a VQ-closed gate, and `origin/main@afdca68851a4bd88762400101e896c238b37fbfd` only adds `.gitignore` worktree isolation infrastructure |
| RTSPINE-023 | WorldSceneAuthoring Asset-edge WorldObject snapshot failure status | VQ-ready at `origin/main@96f0bf0eaed6d2847b4128cd31146340ccc07a0d`: implementation task `6b8c3bdd-0398-44ec-a3ea-4adc116d2afe` is COMPLETE-PASS / committed and VQ task `6df0cd23-9120-42c3-826e-6de352582130` is COMPLETE-PASS / `RTSPINE-WORLDSCENEAUTHORING-ASSET-EDGE-WORLDOBJECT-SNAPSHOT-FAILURE-STATUS-U64-001-VQ-READY`; changed paths were exactly `CMakeLists.txt` and `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoffFixtureTest.cpp`; no docs or production RuntimeAsset/RuntimeAssetWorldAdapter/World/Resource/Asset paths changed; focused `YuRuntimeAssetWorldAdapterHandoffTests` build PASS; new `RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_RejectsAssetEdgeWorldObjectSnapshotFailureWithoutMutation` row reports `1/1` PASS; authoring handoff subset reports `5/5` PASS; `git diff --check` and clean worktree checks PASS; and the test proves a missing Model WorldObject in the caller-owned Asset-edge handoff returns `GateFailed` / `ProofFailed` / `PlanFailed` without mutating identity/transform, attachment/binding, or Asset dependency-edge destinations |

### 5.4 Forbidden Work In This Stage

- no editor-first animation timeline;
- no gameplay animation state machine;
- no blend tree, montage, skeletal skinning, or physics coupling;
- no WorldObject direct binding inside asset files;
- no old TouhouNewWorld package parser as L0/L1 proof;
- no broad shader/material work as a substitute for target identity;
- no direct WorldObject/editor binding beyond caller-owned scene-entity rows and
  the VQ-closed narrow RuntimeAssetWorldAdapter alias/handoff plus handoff
  target-family proof evidence;
- no broader Resource/File/VFS expansion beyond the narrow Resource/Streaming
  payload-window and destination-range follow-through already represented at
  `50ff335` before its own gate is released.

## 6. Team Execution Model

The lead architect owns order, dependency, and stop conditions.

Specialist agents execute only when a task has:

- exact scope;
- exact files or module surface;
- exact non-goals;
- expected evidence;
- failure/blocker policy.

### 6.1 Continuous Multi-Agent Execution

The coordinator must not stop after a single task while accepted work remains.
The default behavior is to keep the goal active, set timers, and keep assigning
the next independent work surface until the stop condition is met or a real
blocker is recorded.

Every shared task must include:

- AI ETA;
- owner and role fit;
- exact scope;
- exact read/write surface;
- non-goals;
- expected evidence;
- stale-owner timeout and reroute rule.

The architect must avoid routine frontline implementation unless the work is a
small architecture patch, an unblocker, or there is no viable owner. This keeps
the architect's context budget available for global dependency control, risk
triage, task decomposition, and evidence review.

### 6.2 True Parallelism Rule

Parallel work is useful only when it reduces calendar time without corrupting
contracts. A lane can run in parallel when it is read-only, writes to disjoint
files, or owns an isolated module boundary with no shared-contract mutation.

Do not split one serial dependency into fake parallel work such as separate
"test 1", "test 2", and "full test" lanes when each lane must wait for the same
implementation or produces duplicate evidence. That burns tokens and does not
increase throughput.

Allowed parallel lanes in the current stage:

- VQ evidence consistency audit for the completed scene-animation gate;
- read-only RuntimeAsset target-identity design surface;
- read-only package/resource index budget/capacity audit;
- read-only implementation-surface scouting for the next RTSPINE contracts;
- read-only quality and risk audit that identifies safe implementation splits.

Forbidden parallel lanes in the current stage:

- any write lane that changes RuntimeAsset contracts before VQ accepts the
  current evidence gate;
- editor, UI, gameplay, or WorldObject binding expansion;
- duplicate QA lanes that only repeat the same focused evidence without a new
  risk question;
- broad full-suite testing without an explicit shared-contract or release
  reason.

QA and evidence tasks may run in parallel only when they are read-only or when
their write surfaces are disjoint. Implementation lanes do not overlap if they
touch shared contracts.

## 7. Stop Conditions

Stop and return to architecture when any of these happen:

- a lower module depends upward on World, UI, Script, Project, GameAdapter, or
  editor types;
- a runtime asset references world instance IDs or editor object IDs;
- a feature bypasses Package/Resource/RuntimeAsset and claims production asset
  proof;
- a report, viewer, screenshot, or log becomes required for runtime behavior;
- old game package compatibility shapes the first production contract;
- a bridge owns both sides' lifecycle;
- hot paths allocate, grow containers, format strings, or perform file IO
  without an accepted gate exception;
- a new implementation lane opens before the active evidence gate closes;
- stage evidence cannot name exact commands, commits, and failure statuses.

## 8. Summary

YuEngine must become a compact native production engine for the team, with the
same kind of reliability and asset/toolchain discipline as the TouhouNewWorld
class of shipped package. The spine is:

```text
Package -> Resource -> RuntimeAsset -> Runtime records -> World/Scene instance
mapping -> Render/Audio/Input/Save -> Tools -> Shipping
```

The next correct move is not feature breadth. The current RuntimeAsset
production spine has VQ-closed Asset-edge handoff evidence through `5802141`,
implementation/focused WorldObject snapshot evidence through `bcfd6ea`, the
AFDCA68 docs baseline at `bd1c4ce`, and implementation/focused failure-status
evidence at `96f0bf0`. Task `6b8c3bdd-0398-44ec-a3ea-4adc116d2afe` added the
tests-only
`RuntimeAssetWorldObjectAuthoringRuntimeExportHandoff_RejectsAssetEdgeWorldObjectSnapshotFailureWithoutMutation`
row and passed focused `YuRuntimeAssetWorldAdapterHandoffTests`, the new row
`1/1`, authoring subset `5/5`, `git diff --check`, and clean worktree checks.
The proof removes the Model WorldObject before restore, expects `GateFailed` /
`ProofFailed` / `PlanFailed`, preserves caller-owned destinations and Asset
dependency edges, and records zero accepted/emitted/restored rows plus one
failed/rejected handoff. VQ task `6df0cd23-9120-42c3-826e-6de352582130` and
next-gate scout `70161a82-87d2-4408-b8de-cd3ec1af5b00` are COMPLETE-PASS. The
scout records
`NO_SAFE_WRITE_FRONTIER_REASON / POST-96F0BF0-NARROW-RUNTIMEASSETWORLDADAPTER-SPINE-EXHAUSTED_WITHOUT_BROAD_SCOPE`;
the successor implementation is
`RTSPINE-RUNTIMEASSET-WORLDOBJECT-TRANSFORM-APPLICATION-U64-001`, released as
task `f939ed81-da4a-4474-8399-6faf7cc9fde9`. Editor, GameAdapter/UI, broad
Resource/File/VFS, Asset auto-lookup, production Asset Manager
dependency-edge follow-through, and gameplay work remain outside this closure.
