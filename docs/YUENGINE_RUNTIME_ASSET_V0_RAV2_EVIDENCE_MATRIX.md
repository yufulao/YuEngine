# YuEngine RuntimeAsset v0 RAV2 Evidence Matrix

Status: evidence package for RAV2 review
Owner: Architecture / Evidence
Task: #67
Baseline: RAV1 implementation package `origin/main@d1a1b86`
Current accepted anchor: `origin/main@97bcd66`
Review gate: task #68

## Purpose

This document records the concrete RAV2 evidence after the RAV1 RuntimeAsset
implementation package.

It is not a claim that YuEngine is complete. It is not editor completion. It is
not final device-backed RuntimeAsset product visual closure.

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
| Preview Host wiring to Resource Browser query outputs | NOT YET | #65 documents this as later wiring. #64 lower contract is not proof that Preview Host consumes Resource Browser query records. |
| Preview Host wiring to command/cook outputs | NOT YET | #65 documents this as later wiring. #63 command output is not automatically Preview Host input wiring. |
| Cooked RuntimeAsset structural visual route | PASS | #66 proves cooked records from #63 import/cook outputs can feed RuntimeAsset validation, scene loader + animation sampling, cooked texture/material/shader payload bridges, RenderScene, RenderCore, and RHI capture route in fast gate. |
| D3D11 texture/render-core hardware smoke support | PASS | Existing D3D11 texture sampling and RenderCore drawable-frame texture capture hardware smoke tests pass as supporting evidence. |
| Device-backed RuntimeAsset route final closure | NOT CLOSED | #66 explicitly does not prove a hardware device directly running the RuntimeAsset visual route as final product closure. This needs a later explicit task/gate if required. |
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
ctest --preset windows-hardware-smoke -R "RHI_D3D11Hardware_TextureSamplingCaptureBytes|RenderCore_D3D11Hardware_DrawableFramePipelineTextureSamplingCapture" --output-on-failure
```

Result: diff/show/style/build PASS; cooked visual proof tests PASS 2/2;
RuntimeAssetData PASS 55/55; D3D11 hardware smoke PASS 2/2.

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
- exact missing-layer diagnostics cover model, material slot, shader pipeline,
  scene transform, camera, and RHI capture.

Boundary: #66 is fast-gate structural proof plus existing D3D11 texture and
RenderCore hardware smoke support. It is not final device-backed RuntimeAsset
hardware visual product closure and does not implement Resource Browser UX or
Preview Host behavior.

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
