# YuEngine RuntimeAsset v0 RAV2 Evidence Matrix

Status: evidence package for RAV2 review
Owner: Architecture / Evidence
Task: #67
Baseline: RAV1 implementation package `origin/main@d1a1b86`
Current accepted anchor: `origin/main@22f8652`
Review gate: task #68

## Purpose

This document records the concrete RAV2 evidence after the RAV1 RuntimeAsset
implementation package.

It is not a claim that YuEngine is complete. It is not editor completion or
full product runtime completion. Device-backed RuntimeAsset visual closure is
accepted only for the current D3D11 target-machine hardware-smoke route listed
below.

The package proves only the next RuntimeAsset-oriented slices:

```text
RAV1 RuntimeAsset implementation package
-> RAV2-A import/cook command and deterministic disk fixtures
-> RAV2-B Resource Browser backend diagnostics contract
-> RAV2-C Preview Host value/session MVP consuming RuntimeAsset graph
-> RAV2-D cooked RuntimeAsset structural visual proof route
```

Task #68 must review this package before any editor-surface implementation wave
is opened.

## Accepted Anchors

| Task | Scope | Accepted anchor | Acceptance source | Status |
| --- | --- | --- | --- | --- |
| #63 RAV2-A | import/cook command contract and deterministic disk fixture generator | `84143d9 Add RuntimeAsset import cook fixture command` | #YuPart:d13c260a msg `9d5ddd48` | PASS |
| #64 RAV2-B | Resource Browser backend data model and import diagnostics contract without UI | `daf9a00 Add Resource Browser diagnostics contract` | #YuPart:b6e4cb01 msg `12fc62a1` | PASS |
| #65 RAV2-C | Engine Preview Host MVP consuming RuntimeAsset graph | `9009eb2 Clarify preview host follow-up wiring` | #YuPart:3802f38c msg `1d935260` | PASS |
| #66 RAV2-D | runtime visual proof from cooked RuntimeAsset records | `97bcd66 Amend RuntimeAsset visual proof style gate` | #YuPart:04bc92ef msg `aad6ff91` | PASS |

## Evidence Classification

| Evidence class | Current state | Notes |
| --- | --- | --- |
| Disk source/cooked RuntimeAsset fixture generation | PASS | #63 writes source `.yu*` and cooked `.racooked` locators through MountTable/File. Suffixes are locators only; internal metadata is type truth. |
| Import/cook command diagnostics | PASS | #63 returns `RuntimeAssetImportCookMissingLayer` and concrete RuntimeAsset/File statuses. |
| Resource Browser diagnostics backend | PASS | #64 validates mounted RuntimeAsset files and projects Resource/Asset/loaded-record diagnostics without mutating Resource/Asset state. |
| Resource Browser UI | NOT IN SCOPE | #64 is backend contract only. No panel, UX, selection, import-settings UI, or editor surface is accepted here. |
| Preview Host value/session layer | PASS | #65 adds `YuPreviewHost` value/session/frame diagnostics and canonical capture route consumption of RuntimeAsset graph outputs. |
| Preview Host wiring to Resource Browser query outputs | PASS | `PreviewHost_ConsumesResourceBrowserImporterCommitOutputs` proves Resource Browser importer commit entries/diagnostics/selection state feed Preview Host viewport build. |
| Preview Host wiring to command/cook outputs | PASS | `PreviewHost_ConsumesResourceBrowserImporterCommitOutputs` combines importer commit selection with `PreviewHostCommandOutputRef` and RHI capture in one frame. |
| ExternalAuthoring bridge rows into Resource Browser commit | PASS | `4e2cfe8` proves actual `BuildExternalAuthoringRuntimeAssetImportBridge` output rows can feed Resource Browser importer commit, then load cooked RuntimeAsset graph through Resource/Asset mutation. |
| Cooked RuntimeAsset structural visual route | PASS | #66 proves cooked records from #63 import/cook outputs can feed RuntimeAsset validation, scene loader + animation sampling, cooked texture/material/shader payload bridges, RenderScene, RenderCore, and RHI capture route in fast gate. |
| Typed scene camera/animation descriptor records | PASS | Scene `cameras=`/`cameraN=` records and animation `clips=`/`tracks=`/`keyframes=` tables are parsed, validated, loaded, sampled, and applied through RuntimeAsset/Animation/World records. |
| D3D11 texture/render-core hardware smoke support | PASS | Existing D3D11 texture sampling and RenderCore drawable-frame texture capture hardware smoke tests pass as supporting evidence. |
| Device-backed RuntimeAsset route final closure | PASS | `RuntimeAssetData_D3D11Hardware_CookedRecordsDriveDeviceBackedVisualProof` directly runs the cooked RuntimeAsset route on D3D11 hardware and verifies RenderScene/RenderCore/RHI submit/present/capture ledger on this machine. |
| Editor surface completion | NOT STARTED | Scene Editor, Animation Editor, UI Editor, Resource Browser UI, native editor shell, and packaging workflow are not complete. |
| Original package parser / Unity / UE importer | NOT IN SCOPE | No original-game package parser or external-authoring importer is accepted by this package. |
| CPU PPM / GDI / manual screenshot as final proof | REJECTED | These remain non-closure evidence. They cannot replace RenderScene/RenderCore/RHI proof or final hardware RuntimeAsset route proof. |

## Command Matrix

### #63 RAV2-A Import/Cook Command

Accepted anchor: `origin/main@84143d9`

Focused commands run by architecture:

```powershell
git diff --check d1a1b86..origin/main
git show --check --format=short origin/main
rg -n "\belse\b|\[&\]|\[=\]" Src/YuEngine/RuntimeAsset Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_ImportCookCommand" --output-on-failure
```

Result: diff/show/style/build PASS; ImportCook focused tests PASS 3/3.

Accepted scope:

- `ExecuteRuntimeAssetImportCookCommand`
- `GenerateRuntimeAssetDeterministicDiskFixture`
- deterministic source and cooked disk fixtures
- File/VFS write route through `MountTable`
- RuntimeAssetData validation of command outputs
- explicit missing-layer diagnostics

Boundary: not Resource Browser UI, Preview Host, runtime visual proof, editor
surface, original package parser, Unity/UE importer, or final render closure.

### #64 RAV2-B Resource Browser Diagnostics Backend

Accepted anchor: `origin/main@daf9a00`

Focused commands run by architecture:

```powershell
git diff --check 84143d9..origin/main
git show --check --format=short origin/main
rg -n "\belse\b|\[&\]|\[=\]" Src/YuEngine/ResourceBrowser Tests/ResourceBrowser
cmake --build --preset windows-fast-gate --target YuResourceBrowserDiagnosticsTests -- /v:minimal /m:1
ctest --preset windows-fast-gate -R "ResourceBrowserDiagnostics_" --output-on-failure
```

Result: diff/show/style/build PASS; ResourceBrowser diagnostics tests PASS 3/3.

Accepted scope:

- `YuResourceBrowser` backend diagnostics module
- `BuildResourceBrowserRuntimeAssetDiagnostics`
- RuntimeAsset byte reads through MountTable/File/VFS
- Resource Browser entries and diagnostics from RuntimeAsset validation,
  Resource/Asset snapshots, and loaded records
- Resource/Asset read-only projection behavior

Boundary: not Resource Browser UI, importer expansion, Preview Host, runtime
visual proof, editor completion, original package parser, or suffix/type-truth
logic.

### #65 RAV2-C Preview Host MVP

Accepted anchor: `origin/main@9009eb2`

Implementation commit: `f0ed944 Implement RuntimeAsset preview host MVP`
Docs-only boundary addendum: `9009eb2 Clarify preview host follow-up wiring`

Focused commands run by architecture:

```powershell
git diff --check daf9a00..HEAD
git show --check --format=short HEAD
rg -n "\belse\b|\[&\]|\[=\]" Src/YuEngine/PreviewHost Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "PreviewHost" --output-on-failure
```

Result: diff/show/style/build PASS; PreviewHost focused tests PASS 4/4.

Accepted scope:

- `YuPreviewHost` value/session/frame layer
- RuntimeAsset graph, scene output, loaded file, resource-ref, and scene-entity
  consumption
- bounded diagnostics, hit/selection/transform feedback values
- success route through RenderScene/RenderCore/RHI capture for the canonical
  three-primitive MVP

Boundary: not Resource Browser UX, import/cook owner, editor surface,
source/original package format, runtime visual final proof, or proof that #63
and #64 outputs are already consumed by Preview Host.

### #66 RAV2-D Cooked RuntimeAsset Visual Proof Route

Accepted anchor: `origin/main@97bcd66`

Implementation commit: `55acffa Add cooked RuntimeAsset visual proof route`
Style amend: `97bcd66 Amend RuntimeAsset visual proof style gate`

Focused commands run by architecture:

```powershell
git diff --check 9009eb2..HEAD
git show --check --format=short HEAD
rg -n "\belse\b|\[&\]|\[=\]" Src/YuEngine/RuntimeAsset Tests/RenderScene/RuntimeAssetDataClosedLoopTests.cpp
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_Cooked.*VisualProof" --output-on-failure
ctest --preset windows-fast-gate -R "RuntimeAssetData" --output-on-failure
ctest --preset windows-hardware-smoke -R "RuntimeAssetData_D3D11Hardware_CookedRecordsDriveDeviceBackedVisualProof|RHI_D3D11Hardware_(TextureSamplingCaptureBytes|IndexedStaticMeshCaptureBytes|VisibleTriangleCaptureBytes)|RenderCore_D3D11Hardware_DrawableFramePipelineTextureSamplingCapture" --output-on-failure
```

Result: diff/show/style/build PASS; cooked visual proof tests PASS 2/2;
RuntimeAssetData PASS 55/55; D3D11 hardware smoke PASS 5/5 on the current
target machine.

Accepted scope:

- `RuntimeAssetLoadedFile` carries validator-derived cooked/source metadata,
  mesh geometry kind, payload/table/hash/texture/shader counts;
- `BuildRuntimeAssetCookedVisualProofRoute` consumes cooked scene, mesh,
  material, texture, shader/program, scene output, animation sampling, and RHI
  inputs;
- cooked records from #63 command outputs are validated through
  File/VFS/RuntimeAsset before the visual route;
- scene transform comes from scene loader and animation sampling;
- route builds RenderScene geometry/material/entities and uses the
  RenderScene three-primitive capture route;
- `RuntimeAssetData_D3D11Hardware_CookedRecordsDriveDeviceBackedVisualProof`
  swaps the route from Null/fast-gate RHI to a D3D11 device and verifies
  submitted indexed draw count, present count, capture count, capture bytes,
  and capture extent;
- exact missing-layer diagnostics cover model, material slot, shader pipeline,
  scene transform, camera, and RHI capture.

Boundary: #66 plus the current hardware-smoke run closes the device-backed
RuntimeAsset visual route for this D3D11 target machine. It does not implement
Resource Browser UX, Preview Host UI, editor UX, original package parsing, or a
guarantee that machines without D3D11/display support pass instead of reporting
the configured hardware skip.

### Main Trunk Device-backed RuntimeAsset Hardware Closure

Anchor base: `origin/main@30be48c`

Focused command run by architecture:

```powershell
ctest --preset windows-hardware-smoke -R "RuntimeAssetData_D3D11Hardware_CookedRecordsDriveDeviceBackedVisualProof|RHI_D3D11Hardware_(TextureSamplingCaptureBytes|IndexedStaticMeshCaptureBytes|VisibleTriangleCaptureBytes)|RenderCore_D3D11Hardware_DrawableFramePipelineTextureSamplingCapture" --output-on-failure
```

Result: D3D11/RHI/RenderCore/RuntimeAsset device-backed focused hardware smoke
PASS 5/5.

Accepted scope:

- RHI D3D11 visible triangle, indexed static mesh, and texture sampling capture
  all pass;
- RenderCore D3D11 drawable frame pipeline texture sampling capture passes;
- cooked RuntimeAsset scene records drive the RuntimeAsset visual route on the
  D3D11 device;
- the RuntimeAsset route verifies runtime-loaded records, shader pipeline,
  cooked material slots, animation transforms, RenderScene routing, RenderCore
  RHI capture routing, submitted draw count, present count, capture count, and
  capture extent.

Boundary: hardware-smoke tests keep `SKIP_RETURN_CODE 77`; on an unsupported
machine this must be reported as hardware environment skip/blocker, not counted
as semantic runtime/data closure failure and not silently called PASS.

### Main Trunk Typed Camera/Animation Descriptor Closure

Anchor base: `origin/main@22f8652`

Focused command run by architecture:

```powershell
ctest --preset windows-fast-gate -R "RuntimeAssetData_(SceneCameraAnimationDependencyValidatorRejectsTypeMismatchWithoutMutation|AnimationDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs|DiskAnimationSamplingFeedsSceneTransforms|SceneAnimationLoaderLoadsBoundedNEntityScene|SceneAnimationLoaderRejectsEntityCapacityOverflowWithoutMutation|SceneAnimationLoaderRejectsMissingRefsWithoutMutation|SceneAnimationLoaderRejectsInvalidRecordsWithoutMutation|SceneAnimationLoaderPathIndependentSceneAnimationDetection|ShaderSceneAnimationRequireSourceSchema)" --output-on-failure
```

Result: RuntimeAsset camera/animation descriptor focused tests PASS 9/9.

Accepted scope:

- scene source supports typed `cameras=` and `cameraN=` records;
- bounded scene loader validates entity camera indices and typed dependency
  refs before output mutation;
- animation source supports bounded `clips=`, `tracks=`, and `keyframes=`
  tables;
- animation tracks validate target entity refs, channel, interpolation, keyframe
  ranges, time bounds, and target availability;
- Animation sampler and transform apply feed RuntimeAsset scene transform output
  records consumed by RenderScene/RuntimeAsset visual routes.

Boundary: this does not add an editor animation timeline, external authoring
importer, or original package animation parser.

### Main Trunk ExternalAuthoring -> ResourceBrowser Commit Closure

Anchor: `origin/main@4e2cfe8`

Focused commands run by architecture:

```powershell
cmake --build --preset windows-fast-gate --target YuResourceBrowserDiagnosticsTests -- /v:minimal
ctest --preset windows-fast-gate -R "ResourceBrowserImporterCommitWorkflow_(CommitsExternalAuthoringBridgeRowsThroughRuntimeAssetGraph|CommitsExternalManifestReadyRuntimeAssetGraph|RejectsMissingPayloadWithoutMutation|RejectsInvalidDependencyWithoutMutation)$" --output-on-failure
ctest --preset windows-fast-gate -R "ExternalAuthoringBridge_AcceptsManifestAndEmitsRuntimeAssetImportCookInputs|RuntimeAssetData_ImportCookCommand" --output-on-failure
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
```

Result: target build PASS; ResourceBrowser importer commit focused tests PASS
4/4; ExternalAuthoring/RuntimeAsset import-cook focused tests PASS 4/4; full
fast gate PASS 1429/1429.

Accepted scope:

- actual `BuildExternalAuthoringRuntimeAssetImportBridge` manifest/payload
  output rows feed `BuildResourceBrowserImporterCommitWorkflow`;
- the bridge-emitted `RuntimeAssetImportCookCommandRequest` writes deterministic
  source/cooked fixtures through File/VFS;
- Resource Browser validates the external manifest boundary, then loads cooked
  RuntimeAsset graph and commits Resource/Asset records;
- test-only linkage verifies the integration without adding a production
  `YuResourceBrowser -> YuExternalAuthoring` dependency.

Boundary: this is still not a Unity/Unreal/DCC importer, Resource Browser UI,
Preview Host UI, original package parser, or final hardware product proof.

### Main Trunk ResourceBrowser -> PreviewHost Importer Commit Closure

Anchor base: `origin/main@45e1470`

Focused commands run by architecture:

```powershell
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "PreviewHost_ConsumesResourceBrowserImporterCommitOutputs" --output-on-failure
ctest --preset windows-fast-gate -R "PreviewHost_(ConsumesResourceBrowserImporterCommitOutputs|BuildsViewportSessionSurfaceFromResourceBrowserSelection|ConsumesImportCookCommandOutputs|ConsumesRuntimeAssetGraphAndCapturesThroughRhi)|ResourceBrowserImporterCommitWorkflow_(CommitsExternalAuthoringBridgeRowsThroughRuntimeAssetGraph|CommitsExternalManifestReadyRuntimeAssetGraph)" --output-on-failure
git diff --check
rg -n "\t|\belse\b|\[&\]|\[=\]" Tests\RenderScene\RuntimeAssetDataClosedLoopTests.cpp CMakeLists.txt
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

Result: target build PASS; new PreviewHost importer-commit focused test PASS
1/1; adjacent PreviewHost/ResourceBrowser focused tests PASS 6/6; diff/style
scan PASS; full fast gate PASS 1430/1430.

Accepted scope:

- Resource Browser importer commit loads cooked RuntimeAsset graph and commits
  Resource/Asset records before Preview Host consumption;
- Resource Browser entries, diagnostics, surface rows, and selection state feed
  `PreviewHost::BuildViewportSessionSurface`;
- the same frame also consumes `PreviewHostCommandOutputRef` from the
  import/cook result, loaded files, scene refs, scene entities, material slots,
  geometry records, and RHI device;
- Preview Host submits RenderScene frame and captures through RenderCore/RHI.

Boundary: this is a runtime/data contract closure. It is not Resource Browser UI,
Preview Host UI, editor UX, external authoring tool import fidelity, original
package parsing, or final hardware product proof.

## Required #68 Review Questions

Task #68 should explicitly answer:

1. Do #63 through #66 have sufficient commits, files, commands, and message
   anchors to support RAV2 PASS?
2. Does any accepted row rely on suffixes as type truth?
3. Does any accepted row substitute CPU PPM, GDI, manual screenshots, or
   standalone samples for RenderScene/RenderCore/RHI evidence?
4. Does #66 overclaim device-backed RuntimeAsset hardware visual closure?
5. Does #65 overclaim editor surface or Resource Browser/command output wiring?
6. Does the package open only the next explicit task batch, rather than calling
   YuEngine or its editors complete?

## PASS / AMEND Rule For #68

PASS may allow creation of the next explicit RuntimeAsset/editor-adjacent task
batch. PASS must not claim full engine completion, all required editor
completion, final hardware RuntimeAsset visual closure, or packaging/product
completion.

AMEND is required if any accepted row:

- treats `.yu*` / `.racooked` suffixes as type truth;
- treats Resource Browser backend diagnostics as UI completion;
- treats Preview Host MVP as editor surface completion;
- treats fast-gate/NullRHI structural proof as final device-backed visual
  product closure;
- uses CPU PPM, GDI, manual screenshot, or standalone sample proof as closure;
- omits the exact command, commit, or message anchor needed to reproduce the
  evidence.
