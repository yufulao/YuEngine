# L1-GATE: Runtime Asset Data Closed Loop

Status: RuntimeAsset module and first cook/decode slices implemented
Requested decision: `FIRST_SLICE_CONTINUE`
Current decision: `RUNTIME_ASSET_MODULE_SLICE_IMPLEMENTED_WITH_PRODUCTION_GAPS`
Owner: Architecture
Task: #73
Related plan: `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
Production-gap closure: `docs/YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md`
Depends on:

- task #71 `RVF Recovery 0: restore clean windows-fast-gate configure/build/test`
- task #72 `RVF Recovery 1: produce runtime visual evidence matrix for current RVF implementation`
- File/VFS, Resource, Package, Asset, RenderScene, RenderCore, RHI, World,
  Animation, and runtime visual foundation contracts already accepted or
  explicitly blocked by name

## Purpose

Define the L1 gate record for a disk-backed runtime asset/data closed loop. This
gate exists so generated runtime visual assets become stable YuEngine data
instead of test-only in-memory structures, CPU helper images, or viewer output.

The required closed loop is:

```text
fixture generator writes disk files
-> File/VFS/Resource reads those files
-> validator/cook/load produces runtime records/resources
-> RenderScene builds a multi-entity frame from loaded scene data
-> RenderCore/RHI renders and captures the cube/cylinder/cone scene
```

This gate now records the first smoke, validator, and `YuRuntimeAsset` module
implementation slices. It still defines the acceptance shape for the remaining
production validator/cook/load slices.

## Layer

L1 runtime data contract over lower File/VFS/Resource/RenderCore/RHI paths and
L1 Asset/RenderScene/World/Animation runtime records.

The gate is below editor, Web, UI, input, gameplay, and Game Adapter work. It is
above individual lower-engine fixtures because it requires a cross-module data
graph loaded from disk and consumed by the runtime render path.

## Current Reality

Current RVF work proves several useful runtime value floors:

- RenderScene owns camera, primitive geometry, runtime material, frame, orbit,
  and missing-layer value routes;
- Animation has runtime clip/track/keyframe sampling and transform application
  value tests;
- RenderCore and RHI have lower-engine fixture, material binding, graph,
  pipeline, draw, capture, and swapchain evidence;
- the RVF tests exercise a cube/cylinder/cone route and emit helper image
  artifacts.

Those facts did not prove a runtime asset/data contract by themselves. The
RuntimeAssetData slices now load the canonical scene from generated disk files
through File/VFS, `YuRuntimeAsset`, Resource, Asset, RenderScene, RenderCore, and
RHI before capture. They also reject unsupported versions, invalid mesh bounds,
and missing/duplicate scene dependencies before runtime output mutation.
`YuRuntimeAsset` now stores cache payloads, deterministic decoded payload records
for mesh/material/texture, and Resource/Asset dependency edges. The remaining
work is the full production cook/load API, decoded texture upload consumption by
RenderScene materials, shader bytecode/program ownership, and disk animation
sampling contract.

## Owns

This gate owns the proposal for:

- runtime mesh data format records for cube, cylinder, cone, and later imported
  meshes;
- runtime material data format records with shader/program reference, constants,
  render state, and texture/sampler slots;
- texture descriptor and payload-reference records with format, extent, mips,
  color space, byte ranges, payload hashes, and sampler references;
- shader/program descriptor records with stage references, input layout,
  constant ranges, texture slots, and required semantics;
- scene data records with scene id, entity ids, transforms, mesh refs, material
  refs, camera refs, animation refs, and dependency graph;
- animation clip and sampled-transform records, either inside the first slice or
  as an explicit same-gate blocker;
- common file-header/version/bounds/dependency/hash/coordinate/status/validator
  rules for every family;
- deterministic fixture generator output to disk with repeated-run hash
  equality;
- validator, cook, loader, and no-mutation failure behavior over files read
  through approved File/VFS/Resource paths;
- runtime output records/resources consumed by RenderScene, RenderCore, and RHI;
- canonical cube/cylinder/cone scene capture through the loaded data graph;
- exact missing-layer diagnostics when any data, dependency, loader, runtime,
  RenderScene, RenderCore, RHI, capture, or animation layer is absent.

## Does Not Own

This gate does not own:

- editor preview host sessions, Web editor panels, UI editor behavior, input
  routes, viewport overlays, or native editor shells;
- old TouhouNewWorld package compatibility, original package parsers, original
  renderer layout, old report schemas, old runtime services, or Game Adapter
  behavior;
- material graph authoring, shader source tooling, shader compiler policy,
  asset database UX, hot reload, cache persistence, remote/cloud cook, or
  editor import workflow;
- real D3D11 hardware availability as a required default fast-gate condition;
- CPU semantic PPM/image artifact generation, GDI/software raster viewers,
  screenshots, reports, logs, sleeps, manual visual inspection, or generated
  artifacts as final acceptance;
- widening lower modules so File, Resource, RHI, RenderCore, or RenderScene own
  each other's lifecycle.

## Data Contract Requirements

Every approved file family must specify:

| Requirement | Required content |
| --- | --- |
| Header/version | magic, version, byte order, compatible/minimum version policy |
| Family identity | internal kind/schema metadata; no production dependency on `.yu*` suffixes |
| Bounds | record counts, byte counts, string/path length, alignment, extent, vertex/index limits |
| Dependencies | typed refs, dependency list order, missing/duplicate/type mismatch status |
| Identity/hash/size | deterministic asset id, source hash, payload hash, total file size |
| Coordinates | units, handedness, axis convention, winding, transform order, UV origin |
| Status/errors | unsupported version, invalid header, invalid size, invalid dependency, hash mismatch, duplicate id, capacity exceeded, budget exceeded |
| Validator | no-mutation failure behavior, output capacity behavior, diagnostics bounds |
| Cook/load | deterministic runtime id, dependency resolution, Resource state transition, runtime output ownership |

The current `.yu*` names are smoke-fixture names only. Production source and
authoring data may use AI-/human-readable schema text or manifests, but type
identity must come from internal metadata rather than suffixes. Runtime/cook/
export data should be high-performance binary with internal magic, version,
kind, hash, dependency, and table metadata. The first implementation may choose
a compact custom test format, but it must still carry all of the rows above and
must not optimize for external ecosystem, plugin-marketplace, or commercial-
engine compatibility over YuEngine runtime cleanliness.

## Required Closed-Loop Proof

The implementation must prove the following in order:

1. `RuntimeAssetData_GeneratorWritesDeterministicFilesAndHashes` writes the
   fixture graph to disk twice and verifies byte size/hash equality.
2. `RuntimeAssetData_FormatHeaderRejectsUnsupportedVersion` rejects invalid
   versions before mutation.
3. `RuntimeAssetData_ValidatorRejectsInvalidBoundsWithoutOutputs` rejects bad
   counts, sizes, or bounds without partial runtime records.
4. `RuntimeAssetData_DependencyGraphRejectsMissingAndDuplicateRefs` validates
   typed refs before cook/load mutation.
5. `RuntimeAssetData_LoaderUsesFileResourcePathNotInMemoryStructs` proves the
   loader reads through File/VFS/Resource or an approved bridge over them.
6. `RuntimeAssetData_SceneReferencesMeshMaterialTextureShader` proves the scene
   file references mesh, material, texture, shader/program, camera, and
   animation data by typed ids.
7. `RuntimeAssetData_LoadCreatesRenderSceneRuntimeRecords` proves loaded data
   becomes RenderScene geometry, material, camera, entity, and frame records.
8. `RuntimeAssetData_CookStoresDecodedPayloadsForMeshMaterialTexture` proves
   mesh/material/texture families produce Resource decoded payload records.
9. `RuntimeAssetData_LoadRegistersResourceAndAssetDependencyEdges` proves scene
   dependencies are represented in both Resource and Asset dependency graphs.
10. `RuntimeAssetData_RenderClosedLoop_CapturesCubeCylinderConeThroughRhi`
   executes through RenderCore/RHI and captures the canonical scene, or reports
   `BlockedByEnv` only for target display/D3D11 constraints.
11. `RuntimeAssetData_CpuPpmOracleDoesNotBypassRhiRenderCore` proves CPU image
   helpers run only after the RHI/RenderCore capture source exists.
12. `RuntimeAssetData_DoesNotDependOnEditorWebUiInputOrGdiViewer` proves no
    editor, Web, UI, input, GDI viewer, or software raster dependency is part of
    the closed-loop proof.

Current first-slice status:

| Proof | Status |
| --- | --- |
| 1. deterministic disk files | PASS |
| 2. unsupported-version no-mutation validator | PASS |
| 3. invalid-bounds no-output validator | PASS |
| 4. dependency missing/duplicate validator | PASS |
| 5. loader uses File/VFS/Resource | PASS |
| 6. scene references mesh/material/texture/shader | PASS for smoke; camera/animation are checked as scene tokens and need fuller typed descriptor records |
| 7. loaded data creates RenderScene runtime records | PASS |
| 8. mesh/material/texture decoded payload records | PASS |
| 9. Resource/Asset dependency edges | PASS |
| 10. RenderCore/RHI capture | PASS |
| 11. CPU oracle guard | PASS |
| 12. no editor/Web/UI/input/GDI viewer dependency | PASS |

## Candidate First Slice

This gate recommends this remaining slice shape:

| ID | Work item | Acceptance direction |
| --- | --- | --- |
| RADC-001 | Common runtime data header and validator vocabulary | shared header/version/bounds/hash/status/no-mutation rules |
| RADC-002 | Deterministic fixture generator | writes mesh/material/texture/shader/scene/animation source files to ignored output dirs |
| RADC-003 | Mesh/material/texture/shader validator | validates file bytes and dependency graph through File/VFS/Resource paths |
| RADC-004 | Scene/camera/animation validator | validates scene refs, transforms, camera refs, and clip refs or names animation blocker |
| RADC-005 | Cook/load bridge | partially implemented in `YuRuntimeAsset`; still needs production scene loader output API |
| RADC-006 | Canonical render smoke | loads cube/cylinder/cone scene and renders/captures through RenderScene/RenderCore/RHI |
| RADC-007 | Helper-oracle guard | CPU PPM/image artifact output is checked only as auxiliary evidence after runtime capture |

The slice may split implementation tasks later, but those tasks must stay
parallelizable by file family or stage and must not authorize upper-layer
editor or Game Adapter work.

## Test And Evidence Policy

Implementation evidence for a later approval must include:

- clean task #71 configure/build/test baseline before implementation starts;
- task #72 matrix mapping current RVF layers to reusable, rework, or rejected
  evidence;
- changed-path/offscope audit;
- `git diff --check` for touched files;
- configure and build for `windows-fast-gate`;
- focused tests for the required closed-loop proof names or approved
  equivalents;
- full `ctest --preset windows-fast-gate --output-on-failure`;
- label discovery for File, Resource, Asset, RenderScene, RenderCore, RHI,
  Animation, RuntimeVisualScene, Fast, PerformanceSmoke, EvidenceOracle, and
  default HardwareSmoke isolation;
- public-header and production dependency scans proving no editor/Web/UI/input,
  Game Adapter, backend-native public leak, GDI viewer, report, screenshot, or
  original-game parser dependency;
- artifact hygiene proving generated files are ignored or explicitly approved
  as tiny source fixtures;
- proof-shape scan rejecting direct fixture struct injection into RenderScene as
  data-load evidence.

Accepted proof:

- deterministic disk files produced by a checked-in generator;
- validator/cook/load records created from bytes read through approved engine
  file/resource paths;
- no-mutation tests for rejected file, dependency, output, and budget cases;
- Resource cache payload, Resource decoded payload, Resource dependency, and
  Asset dependency records written by runtime code, not private test helpers;
- runtime records/resources consumed by RenderScene/RenderCore/RHI;
- canonical scene capture from the loaded graph.

Rejected proof:

- C++ in-memory struct construction as the only source of loaded data;
- screenshots, reports, logs, sleeps, manual inspection, generated image
  artifacts, CPU PPM semantics, or GDI/software raster output as final proof;
- `YuRuntimeVisualCameraTweenViewer` as acceptance;
- direct D3D sample bypasses;
- silent skip for semantic data/loader/render gaps.

## Remaining Slice Routing

Before any remaining implementation slice is created:

1. Architecture review confirms this gate is the correct L1 data contract
   boundary and that animation is either in-scope or explicitly blocked.
2. Engine-reference review confirms the split between file data, resource
   identity, asset runtime records, scene records, render scheduling, and editor
   tooling matches mature-engine responsibility separation.
3. Evidence review confirms current RVF layers from task #72 are classified
   correctly and original-game facts do not define the first contract.
4. Performance review approves file-size bounds, dependency graph bounds,
   validator/cook/load capacities, and no hidden allocation/growth on runtime
   frame paths.
5. Implementability review confirms the tests can be enforced without
   widening lower modules or relying on viewer/image helper output.
6. PM/final gate state must issue explicit `APPROVED_FOR_FIRST_SLICE`.

## Hard Blocks

The following are blocking violations:

- starting implementation before task #71 and task #72 prerequisites are
  resolved or explicitly waived in the task thread;
- depending on editor, Web, UI, input, Game Adapter, old runtime reports, or
  original package compatibility;
- using `YuRuntimeVisualCameraTweenViewer`, GDI, software raster, CPU semantic
  PPM, image artifacts, screenshots, reports, logs, sleeps, or manual
  inspection as final acceptance;
- submitting generated assets without a format spec, validator, loader, and
  runtime render smoke;
- bypassing File/VFS/Resource by directly injecting fixture structs into
  RenderScene;
- calling hardware/display absence `BlockedByEnv` when the missing layer is
  actually data format, dependency, validator, loader, RenderScene, RenderCore,
  RHI value contract, capture route, or animation semantics.

## Exit Criteria

This gate is ready for the next implementation slice when:

1. this document and the paired plan are committed;
2. both documents record the current decision
   `FIRST_SLICE_IMPLEMENTED_WITH_COOK_LOAD_GAPS`;
3. task #71 and task #72 are listed as prerequisites;
4. data families cover mesh, material, texture descriptor/payload reference,
   shader/program descriptor, scene data, and animation clip/sampled transform
   refs;
5. common file format rules cover version/header, bounds, dependency list,
   deterministic identity/hash/size, coordinate/unit/handedness, errors, and
   validator behavior;
6. minimum proof keeps generator disk files, File/VFS/Resource read path,
   Resource/Asset registration, RenderScene/RenderCore/RHI records, and
   cube/cylinder/cone capture;
7. hard blocks prevent current helper/viewer/oracle paths from being promoted
   to final acceptance.
