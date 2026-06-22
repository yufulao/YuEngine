# L1-GATE: Runtime Asset Data Closed Loop

Status: RuntimeAsset module and RAV0 validator/cook/load floors implemented; RAV1 production contract gate in review
Requested decision: `RAV1_PHASE_A_CONTRACT_REVIEW`
Current decision: `RAV1_DOCS_GATE_NOT_IMPLEMENTATION_APPROVED`
Owner: Architecture
Task: #73 baseline; #50 RAV1 production contract amendment
Related plan: `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
Production-gap closure: `docs/YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md`
Format policy and validator vocabulary: `docs/YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md`
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
implementation slices plus the RAV0 production floors for typed validators,
path-independent family detection, shader/program bytecode to RHI pipeline,
decoded texture payload to material slots, disk animation sampling, deterministic
scene loader output, and scene loader no-mutation failures.

Task #50 adds the RAV1 Phase A contract/gate layer only. It does not approve
new runtime implementation by itself.

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

## RAV1 Production Contract Gate

The RAV1 gate treats RuntimeAsset v0 as two explicit artifact classes:

- **source artifact**: readable text or manifest-shaped input used for
  authoring/review/cook; it must carry internal magic/header, version, kind,
  schema, id, source hash, dependency table, family bounds, and coordinate
  rules;
- **cooked artifact**: runtime-optimized output, preferably binary, with
  internal magic/version/kind/schema, table directory, payload offsets, byte
  order, alignment, deterministic ids, payload hashes, dependency hashes, and
  total byte size/hash.

The loader may use a path to find bytes through File/Mount/VFS, but it must not
use the path suffix as family identity. The authoritative family identity is the
internal `kind`/`version`/`schema` plus the typed dependency table.

RAV1 production proof must cover these family rows at contract level:

| Family | Contract fields | Runtime ownership boundary |
| --- | --- | --- |
| Mesh | id, vertex layout, topology, vertices, indices, draw ranges, bounds, coordinate spec, payload hash | RuntimeAsset validates/cooks payload records; RenderScene/RenderCore/RHI consume geometry records without direct fixture structs |
| Material | id, shader/program ref, texture/sampler slots, constants, render state, dependency hashes | Material slots resolve through decoded texture payload records and loaded shader/program refs |
| Texture | id, format, extent, mip count, color space, sampler ref, payload range/hash, decoded payload budget | Resource decoded payload owns bytes; RHI texture/update route consumes those bytes |
| Shader/program | id, stage refs, bytecode size/hash, entry semantics, input layout, constants, texture slots | RuntimeAsset owns bytecode decode; RHI shader modules and pipelines are created from loaded data |
| Scene/camera | scene id, entity ids, transforms, mesh/material/texture/shader refs, camera refs, dependency order | Scene loader emits deterministic staged records; RenderScene consumes them after preflight succeeds |
| Animation | clip id, track id, target entity/transform refs, sample rate, interpolation, keyframe ranges | Animation sampler produces sampled transforms that feed staged scene output before RenderScene mutation |

Common validation failures must use the #41 `RuntimeAssetDataStatus` vocabulary:
`InvalidArgument`, `InvalidHeader`, `UnsupportedVersion`, `InvalidKind`,
`InvalidSchema`, `InvalidCount`, `InvalidSize`, `InvalidAlignment`,
`InvalidBounds`, `InvalidDependency`, `MissingDependency`,
`DuplicateDependency`, `TypeMismatch`, `HashMismatch`,
`UnsupportedFieldValue`, `CapacityExceeded`, and `BudgetExceeded`.

Integration statuses such as File read, Resource registration, cache/decoded
payload, Asset registration, dependency edge, input-layout, RHI shader module,
and RHI pipeline failures may appear only after validator/cook preflight has
classified format, dependency, capacity, and budget failures.

## RAV1 Cook / Load / Render Route

The next implementation wave must preserve this staged route:

```text
deterministic disk artifact
-> File/Mount/VFS byte read
-> source/cooked header + table validation
-> dependency/hash/budget preflight
-> cook to runtime-ready records or payload staging
-> Resource and Asset registration plus dependency edges
-> decoded payload / shader program / animation sample ownership
-> staged scene loader output
-> RenderScene frame records
-> RenderCore/RHI submit, present, and capture evidence
```

No stage may partially mutate later-stage output on failure. Scene loader
failures must leave Resource/Asset registries, decoded payload stores, staged
scene output, RenderScene records, RenderCore state, and RHI objects unchanged
unless the gate explicitly names a prior committed stage and its rollback or
cleanup semantics.

The accepted capture evidence is RenderCore/RHI output from loaded RuntimeAsset
data. CPU/PPM semantic output, screenshots, reports, logs, the GDI viewer, or
manual inspection can only be auxiliary evidence after RHI/RenderCore capture
exists.

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

RAV0 added additional proof names that RAV1 must keep passing or supersede with
approved equivalents:

- `RuntimeAssetData_MeshMaterialTextureTypedValidatorsAcceptStructuredMetadata`
- `RuntimeAssetData_MaterialValidatorRejectsMissingDuplicateAndTypeMismatchRefs`
- `RuntimeAssetData_TextureValidatorRejectsInvalidFormatExtentPayload`
- `RuntimeAssetData_ShaderSceneAnimationRequireSourceSchema`
- `RuntimeAssetData_SceneFamilyDetectionIsPathIndependent`
- `RuntimeAssetData_ShaderProgramDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs`
- `RuntimeAssetData_SceneCameraAnimationDependencyValidatorRejectsTypeMismatchWithoutMutation`
- `RuntimeAssetData_AnimationDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs`
- `RuntimeAssetData_ShaderProgramBridgeCreatesRhiPipelineFromLoadedBytecode`
- `RuntimeAssetData_ShaderProgramBridgeRejectsInvalidProgramDataWithoutRhiMutation`
- `RuntimeAssetData_ProductionSceneLoaderOutputsDeterministicRecords`
- `RuntimeAssetData_DiskAnimationSamplingFeedsSceneTransforms`
- `RuntimeAssetData_SceneLoaderRejectsInvalidEntityWithoutOutputMutation`
- `RuntimeAssetData_SceneLoaderRejectsInvalidKeyframesWithoutOutputMutation`
- `RuntimeAssetData_DecodedTexturePayloadsDriveRhiMaterialSlots`
- `RuntimeAssetData_TextureMaterialSlotBridgeFailuresDoNotMutateRenderSceneOutputs`

## Candidate First Slice

This gate recommends this RAV1 implementation routing after Phase A review:

| ID | Work item | Acceptance direction |
| --- | --- | --- |
| RAV1-A | Production contract and gate | docs-only contract/gate amendment; no runtime implementation approval |
| RAV1-B | Suffix-free loader transaction design | no path suffix family inference; staged no-mutation transaction semantics |
| RAV1-C | Cooked texture/material/shader payload bridge plan | decoded texture, material slots, shader bytecode, RHI program route stays loaded-data-owned |
| RAV1-D | Bounded scene/animation record loader plan | beyond fixed three-entity fixture; explicit capacities and sample/output semantics |
| RAV1-E | Evidence matrix and acceptance commands | maps current tests, changed paths, off-scope scans, and required commands |
| RAV1 review gate | Architecture/code/perf/evidence review | required before any implementation authorization |

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
6. PM/final gate state must issue explicit `APPROVED_FOR_FIRST_SLICE` or
   `APPROVED_FOR_NEXT_SLICE` after the RAV1 review gate closes.

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
   `RAV1_DOCS_GATE_NOT_IMPLEMENTATION_APPROVED`;
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
