# YuEngine Runtime Asset Data Contract Plan

Status: RuntimeAsset module, validator, and cook/decode smoke slices implemented; full production contract still open
Owner: Architecture
Task: #73
Related gate: `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
Resource Browser scope: `docs/YUENGINE_RESOURCE_BROWSER_IMPORT_COOK_DIAGNOSTICS_SCOPE.md`
Prerequisites:

- task #71 restores clean configure, build, and test for the main line.
- task #72 records which current RVF layers can be reused and which must be
  reworked before they count as runtime asset/data evidence.
- user scope correction approved continuing with the first RuntimeAssetData
  implementation slice after the review-only plan and matrix landed.

## Purpose

This plan defines the missing runtime data contract between generated fixtures
and the YuEngine runtime render path. The current RVF work proves important
camera, primitive, material, animation, RenderScene, RenderCore, and RHI value
floors, but it still allows too much evidence to live as C++ in-memory
construction, image artifact helpers, or viewer output.

L1 closure needs a disk-backed data path:

```text
deterministic fixture generator
-> checked file format records on disk
-> File/VFS/Resource read path
-> validator/cook/load records
-> RenderScene / RenderCore / RHI runtime records and resources
-> cube/cylinder/cone render and capture proof
```

The contract is not an editor, Web, UI, input, or Game Adapter contract. It is
also not a package compatibility contract for the original game. Original-game
resource facts may later become validation inputs only after a separate
evidence-approved package/parser gate.

## Current Gap

The runtime visual foundation plan already requires camera, geometry, material,
animation, RenderScene, RenderCore, RHI, and capture floors. The repository also
has current `RenderSceneRuntimeVisualSceneProofRoute` and RVF tests for the
cube/cylinder/cone route.

The first smoke, validator, and minimal cook/decode gaps are now partly closed by
`YuRuntimeAsset` plus `YuRuntimeAssetDataClosedLoopTests`: a
deterministic generator writes mesh/material/texture/shader/scene/animation
records to disk, `YuRuntimeAsset` reads them through `MountTable`, validates
headers and scene dependencies, registers Resource/Asset records, stores cache
payloads, creates deterministic decoded payload records for mesh/material/texture,
adds Resource and Asset dependency edges, builds RenderScene runtime records, and
captures through RenderCore/RHI. The validator smoke rejects unsupported
versions, invalid mesh bounds without output mutation, and missing/duplicate
scene dependencies. The remaining gap is the production-quality contract: full
typed dependency validation across every file family, decoded texture payload
upload into RHI material slots, shader bytecode/program ownership, animation clip
sampling from disk, and production scene loader output APIs.

The following evidence is useful but insufficient on its own:

- C++ tests that construct mesh, material, texture, shader, scene, camera, or
  animation records directly in memory;
- CPU semantic PPM or image artifact files used as helper oracle output;
- `YuRuntimeVisualCameraTweenViewer` or any other GDI/software raster viewer;
- standalone D3D/RHI samples that do not pass through File/VFS/Resource,
  validator/cook/load, RenderScene, and RenderCore ownership.

## Contract Families

The first runtime asset/data contract must define these families together
because the final visual scene references them as one graph.

| Family | Required data | Minimum runtime consumer |
| --- | --- | --- |
| Runtime mesh data | mesh id, vertex layout, vertices, indices, topology, draw ranges, bounds | RenderScene geometry records and RenderCore/RHI vertex/index input |
| Runtime material data | material id, shader/program reference, texture/sampler slots, constants, render state | RenderScene material records and RenderCore material binding |
| Texture descriptor / payload reference | texture id, format, extent, mip count, color space, payload byte range/hash, sampler reference | Resource decoded payload, RHI texture/update values, material slots |
| Shader / program descriptor | program id, stage references, entry semantics, input layout, constant ranges, texture slots | RenderCore/RHI shader, pipeline, and input-layout values |
| Scene data | scene id, entity ids, transforms, mesh refs, material refs, camera refs, dependency list | World/Transform values, RenderScene runtime frame records |
| Animation data | clip ids, track ids, keyframe ranges, sample rate, target entity/transform refs | Animation runtime sampler and transform-apply records |

Animation may be carried as a same-gate dependency or named blocker for the
first review. The final cube/cylinder/cone route must not silently replace a
missing animation contract with per-frame sample math.

## Common Format Rules

Every file family must specify the same validation vocabulary:

- magic/version/header and byte order;
- record count bounds, byte-size bounds, string/path bounds, and alignment
  requirements;
- deterministic identity, source hash, payload hash, and total file size;
- dependency list with typed references and no implicit lookup by display name;
- coordinate system, units, handedness, transform order, winding, and UV origin;
- explicit error/status values for unsupported version, invalid header,
  invalid count, invalid size, invalid dependency, duplicate id, missing
  dependency, hash mismatch, unsupported field value, output capacity exceeded,
  and budget exceeded;
- validator behavior for no-mutation failures;
- cook/load behavior for deterministic output identity;
- generated-file policy: fixture outputs are produced by checked-in tools but
  are not committed unless a reviewer approves a tiny source fixture file.

The first format can be binary, text, or mixed only after reviewers accept the
tradeoff. The important contract is not the syntax; it is that the same bytes
are read from disk through approved engine paths and produce deterministic
runtime records.

## Deterministic Fixture Generator

The generator is part of the proof path. It must write files to disk and report
their names, sizes, hashes, and dependency graph. It must not simply return C++
objects to the test.

Minimum generated fixture set:

1. cube mesh data;
2. cylinder mesh data;
3. cone mesh data;
4. one shared material with at least three texture inputs;
5. at least one texture payload or deterministic texture payload reference;
6. one shader/program descriptor with the input layout required by the meshes;
7. one scene file referencing the mesh/material/texture/shader/camera graph;
8. one animation clip file or an explicit same-gate blocker row.

The generator must be deterministic under repeated runs from the same source
inputs. A required test must compare output file sizes and hashes across two
fresh output directories.

## First Smoke And RuntimeAsset Module Slices

The first implementation slices are intentionally narrow and module-backed. They
establish that the lower runtime route can no longer be claimed from pure
C++ in-memory construction alone.

| Proof | Status | Evidence |
| --- | --- | --- |
| Deterministic disk generation | PASS | `RuntimeAssetData_GeneratorWritesDeterministicFilesAndHashes` |
| Unsupported version validation | PASS | `RuntimeAssetData_FormatHeaderRejectsUnsupportedVersion` |
| Invalid bounds no-output validation | PASS | `RuntimeAssetData_ValidatorRejectsInvalidBoundsWithoutOutputs` |
| Missing/duplicate dependency validation | PASS | `RuntimeAssetData_DependencyGraphRejectsMissingAndDuplicateRefs` |
| File/VFS read path | PASS | fixture bytes are read through `MountTable` loose mount by `YuRuntimeAsset` |
| Resource/Asset registration | PASS | scene and all generated asset families register synthetic Resource descriptors and runtime Asset handles |
| Resource/Asset dependency edges | PASS | `RuntimeAssetData_LoadRegistersResourceAndAssetDependencyEdges` records scene dependencies in both registries |
| Mesh/material/texture cook payloads | PASS | `RuntimeAssetData_CookStoresDecodedPayloadsForMeshMaterialTexture` stores decoded payload records for seven decodable runtime records |
| RenderScene records | PASS | loaded handles feed cube/cylinder/cone geometry, shared material, camera, and frame records |
| RenderCore/RHI capture | PASS | `RuntimeAssetData_RenderClosedLoop_CapturesCubeCylinderConeThroughRhi` |
| CPU oracle guard | PASS | `RuntimeAssetData_CpuPpmOracleDoesNotBypassRhiRenderCore` |
| Upper-layer dependency guard | PASS | `RuntimeAssetData_DoesNotDependOnEditorWebUiInputOrGdiViewer` |

These slices are not a complete asset system. They leave the full typed format
validators, decoded texture-to-RHI bridge use in RenderScene, shader bytecode
loader, animation-clip loader/sampler, and production scene loader APIs open.

## Validator, Cook, Load, Render

The accepted closed loop is staged:

1. **Generate**: write fixture source files to disk with deterministic metadata.
2. **Validate**: read files through File/VFS and validate headers, bounds,
   dependency graph, hashes, coordinate rules, and output capacities.
3. **Cook**: produce runtime-ready records or package/resource staging records
   with deterministic ids and hashes.
4. **Load**: use Resource and related runtime load contracts to create runtime
   mesh, material, texture, shader, scene, camera, and animation records.
5. **Render**: build RenderScene frame records from the loaded scene, execute
   through RenderCore/RHI, and capture the cube/cylinder/cone sequence.
6. **Diagnose**: if a layer is missing, report the exact missing layer and the
   file/dependency that caused it.

The loader must not stuff fixture structs directly into RenderScene. Direct
construction remains allowed inside narrow unit tests for individual value
contracts, but it cannot satisfy this closed-loop gate.

## Minimal Closed Loop Scene

The first closed-loop visual scene is the same canonical scene used by the RVF
and preview-host planning docs:

- one cube, one cylinder, and one cone;
- deterministic transforms from a scene file;
- one shared material with three texture inputs;
- one camera or camera sequence from runtime scene/camera data;
- per-object rotation from animation data, or an explicit `L1-ANIM-*` blocker;
- RenderScene emits multi-entity frame records;
- RenderCore/RHI draws and captures the frame or reports `BlockedByEnv` only
  for real target D3D11/display constraints;
- CPU semantic PPM/image artifact output may verify pixels, but only after the
  RenderCore/RHI route produced the capture source.

## Relationship To Current Plans

This plan strengthens, but does not replace:

- `docs/YUENGINE_RUNTIME_VISUAL_FOUNDATION_PLAN.md`;
- `docs/YUENGINE_L1_RUNTIME_CORE_MATRIX.md`;
- `docs/YUENGINE_L1_VERTICAL_SAMPLE_ACCEPTANCE.md`;
- `docs/gates/EDITOR_GATE_001_RUNTIME_PREVIEW_HOST.md`;
- lower-engine Resource, RenderCore, RHI, Asset, RenderScene, World, and
  Animation gates.

The runtime asset/data contract is the missing data spine for the visual route.
Editor preview host work can consume this route later, but cannot define it.
Resource Browser/import/cook/diagnostics work is also downstream of this
contract: it may display source/import/cook state and route commands, but it
must not redefine RuntimeAssetData file families, validator semantics, Resource
or Asset ownership, or runtime output records.

## First Review Questions

Reviewers should answer these before implementation is authorized:

1. Which current RVF layers from task #72 can be reused unchanged?
2. Which RVF layers must be reworked because they are test-side struct
   construction, CPU helper image generation, GDI/software viewer output, or
   non-File/VFS/Resource bypasses?
3. Should animation be inside the first gate slice or recorded as an explicit
   same-gate blocker until a separate clip-file contract is approved?
4. Which tiny source fixture files, if any, are allowed in the repo, and which
   generated outputs must stay under ignored artifact directories?
5. Which File/VFS/Resource/Package path owns source bytes, cooked bytes, and
   runtime records in the first slice?
6. What status names and no-mutation tests are mandatory before the gate can
   request `APPROVED_FOR_FIRST_SLICE`?

## Hard Blocks

The following are blocking violations:

- using editor, Web, UI, input, or Game Adapter behavior as part of the closed
  runtime data proof;
- using `YuRuntimeVisualCameraTweenViewer`, GDI, software rasterization, CPU
  semantic PPM, image artifact output, screenshots, reports, logs, or manual
  inspection as the final acceptance path;
- submitting generated output without format spec, validator, loader, and
  runtime render smoke;
- bypassing File/VFS/Resource by injecting fixture structs directly into
  RenderScene and claiming asset loading is done;
- hiding missing data/loader/render layers as `BlockedByEnv`;
- letting original-game package/resource facts define the first contract.

## Exit Criteria

This plan is ready for the next implementation slice when:

1. the paired gate records that the first smoke slice has been implemented;
2. task #71 and task #72 are recorded as prerequisites and remain passing;
3. every runtime data family lists version/header, bounds, dependencies,
   deterministic identity/hash/size, coordinate rules, statuses, and validator
   behavior;
4. the minimal generator -> File/VFS/Resource -> Resource/Asset ->
   RenderScene/RenderCore/RHI -> capture loop is proven by focused tests;
5. hard blocks prevent current RVF helper output or viewer output from being
   counted as the final proof.
