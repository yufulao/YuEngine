# YuEngine Runtime Asset Data Contract Plan

Status: RuntimeAssetData current slice PASS; production hardening remains on main
Owner: Architecture
Task: #73 baseline; #50 RAV1 production contract amendment
Production-gap closure: `docs/YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md`
Format policy and validator vocabulary: `docs/YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md`
Loader transaction plan: `docs/YUENGINE_RUNTIME_ASSET_V0_LOADER_TRANSACTION_PLAN.md`
Payload bridge RHI route: `docs/YUENGINE_RUNTIME_ASSET_V0_PAYLOAD_BRIDGE_RHI_ROUTE_PLAN.md`
RAV1 evidence matrix: `docs/YUENGINE_RUNTIME_ASSET_V0_RAV1_EVIDENCE_MATRIX.md`
Related gate: `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
Resource Browser scope: `docs/YUENGINE_RESOURCE_BROWSER_IMPORT_COOK_DIAGNOSTICS_SCOPE.md`
Prerequisites:

- clean `main` configure, build, and test stay green before each next slice.
- the runtime visual evidence matrix records which current layers are reusable
  and which must be reworked before they count as runtime asset/data evidence.
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

The first smoke, validator, and minimal cook/decode gaps are now closed for the
current slice by
`YuRuntimeAsset` plus `YuRuntimeAssetDataClosedLoopTests`: a
deterministic generator writes mesh/material/texture/shader/scene/animation/camera
records to disk, `YuRuntimeAsset` reads them through `MountTable`, validates
headers and scene dependencies, registers Resource/Asset records, stores cache
payloads, creates deterministic decoded payload records for mesh/material/texture,
adds Resource and Asset dependency edges, builds RenderScene runtime records, and
captures through RenderCore/RHI. The validator smoke rejects unsupported
versions, invalid mesh bounds without output mutation, and missing/duplicate
scene dependencies.

Later RAV slices added typed family validators, path-independent family
detection, shader/program dependency validation and RHI pipeline creation from
loaded bytecode, cooked shader stage payloads, decoded texture payloads driving
material slots, disk animation sampling into scene transforms, deterministic
staged scene loader output, package/cook/run smoke, product-run command
consumption, first-class camera tween descriptor source/cooked file loading,
and no-mutation failure coverage. These floors close the previous
"smoke only" blockers for the current slice. The remaining production work is
hardening, not a reason to reopen editor, Web, UI, input, or external authoring
scope as acceptance.

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
| Camera data | camera id, projection, FOV, near/far planes, tween keyframes, look target | Runtime camera records and RenderScene frame camera binding |
| Scene data | scene id, entity ids, transforms, mesh refs, material refs, camera refs, dependency list | World/Transform values, RenderScene runtime frame records |
| Animation data | clip ids, track ids, keyframe ranges, sample rate, target entity/transform refs | Animation runtime sampler and transform-apply records |

The texture/material/shader rows are refined by
`docs/YUENGINE_RUNTIME_ASSET_V0_PAYLOAD_BRIDGE_RHI_ROUTE_PLAN.md`: cooked texture
payloads own byte layout, row pitch, size, hash, and decoded payload identity;
materials own bounded texture/sampler slot tables; shader/program records own
stage bytecode payloads, hashes, minimal reflection, input layout, and slot
counts before any RHI texture, shader module, or pipeline handle is published.

Animation may be carried as a same-gate dependency or named blocker for the
first review. The final cube/cylinder/cone route must not silently replace a
missing animation contract with per-frame sample math.

## RAV1 Production RuntimeAsset v0 Contract

RuntimeAsset v0 identity is internal and suffix-free. A path, extension, or
fixture name may help tests locate bytes, but it is never authoritative for
type, version, schema, dependency identity, payload identity, or accepted proof.

### Source Artifact Contract

Source artifacts are the deterministic authoring/review input to the cook. They
may be readable text records or manifests when that improves single-team
iteration, but every source artifact must normalize to these fields before
validation or cook:

| Field | Contract |
| --- | --- |
| `magic` / header | family-independent source marker; reject absent or malformed markers as `InvalidHeader` |
| `version` | v0 source version and minimum compatible source version; unsupported values return `UnsupportedVersion` |
| `kind` | mesh, material, texture, shader/program, scene, camera-if-split, animation, or approved extension; mismatches return `InvalidKind` or `TypeMismatch` |
| `schema` | v0 source schema identifier such as the current `rav0-source` vocabulary; missing or incompatible schemas return `InvalidSchema` |
| `id` / stable identity | deterministic logical id used for Resource/Asset registration; duplicates within a graph are invalid |
| `sourceHash` | deterministic hash over the exact source bytes or manifest expansion used for cook |
| `payloadHash` | deterministic hash over embedded or referenced payload bytes when a family owns payload data |
| `dependencyTable` | typed refs to other RuntimeAsset ids; no implicit lookup by display name or filename suffix |
| `record bounds` | counts, byte sizes, string lengths, coordinate values, extents, keyframes, and slots within the family budget |
| `coordinateSpec` | units, handedness, transform order, winding, UV origin, and animation time basis where relevant |

The current `.yumesh`, `.yumat`, `.yutex`, `.yuprogram`, `.yuscene`, and
`.yuanim` names remain smoke-fixture names only. Source artifacts must keep
working if the same bytes arrive through another approved path.

### Cooked Artifact Contract

Cooked artifacts are runtime-optimized output. They should be binary unless a
later approved gate names a readable cooked representation for a narrow reason.
Every cooked artifact must carry enough internal metadata to validate and load
without the source path:

| Field | Contract |
| --- | --- |
| internal magic/version/kind/schema | same authoritative identity as source, encoded in the cooked header or table directory |
| table directory | bounded record tables, payload table offsets, sizes, alignments, and byte order |
| deterministic ids | graph-stable asset id, Resource type id, Asset type id, scene/entity/camera/animation ids as applicable |
| payload ownership | whether bytes are embedded, external within the cooked bundle, or referenced through an approved Resource payload id |
| dependency table | typed refs with expected family, required/optional bit, dependency hash, and load order constraints |
| hash coverage | source hash, cooked payload hash, dependency hash where required, and total byte size/hash |
| budgets | explicit file size, table count, dependency count, decoded payload byte count, animation sample count, and loader output capacities |

Cooked validation must reject invalid counts, sizes, alignments, hash mismatches,
unsupported field values, capacity overflow, and budget overflow before Resource,
Asset, RenderScene, RenderCore, RHI, or scene loader output mutation.

### Family-Level Contract

| Family | Required source fields | Cooked payload ownership | Required validation focus |
| --- | --- | --- | --- |
| Mesh | mesh id, vertex layout, topology, vertices, indices, draw ranges, bounds, coordinate spec | vertex/index payload ranges or approved primitive decode records | vertex/index count, stride/alignment, topology, bounds, payload hash, material/scene ref compatibility |
| Material | material id, shader/program ref, texture/sampler slots, constants, render state | material record plus typed refs to decoded texture payloads and shader program | required slots, duplicate slots, shader/type mismatch, unsupported constants/state, payload dependency hashes |
| Texture | texture id, format, extent, mip count, color space, sampler ref, payload ref/hash | descriptor plus decoded payload bytes or Resource decoded payload id | format/extent/mip bounds, alignment, byte count, hash, decoded payload budget, material slot compatibility |
| Shader/program | program id, stage refs, bytecode refs/hashes, entry semantics, input layout, constants, texture slots | shader bytecode payloads and pipeline/input-layout descriptor | stage presence, bytecode size/hash, unsupported semantics, input-layout validity, RHI module/pipeline no-mutation failures |
| Scene/camera | scene id, entity ids, transforms, mesh/material/texture/shader refs, camera refs, dependency order | staged scene loader output records and camera records | typed refs, entity/camera bounds, transform bounds, missing decoded payload/program/camera, output capacity |
| Animation | clip id, track id, target entity/transform refs, sample rate, interpolation, keyframe ranges | sampled transform inputs or approved clip payload records consumed by Animation sampler | time/keyframe bounds, unsupported interpolation, target mismatch, sample budget, no partial scene transform output |

### Status Vocabulary

Family validators and cook/load/render preflight use the #41
`RuntimeAssetDataStatus` vocabulary exactly for common failures:
`InvalidArgument`, `InvalidHeader`, `UnsupportedVersion`, `InvalidKind`,
`InvalidSchema`, `InvalidCount`, `InvalidSize`, `InvalidAlignment`,
`InvalidBounds`, `InvalidDependency`, `MissingDependency`,
`DuplicateDependency`, `TypeMismatch`, `HashMismatch`,
`UnsupportedFieldValue`, `CapacityExceeded`, and `BudgetExceeded`.

Post-validation integration may additionally surface the existing RuntimeAsset
load statuses for File, Resource, Asset, decoded payload, dependency edge, input
layout, RHI shader module, and RHI pipeline failures. These integration statuses
do not replace the #41 validator vocabulary and must not be used to hide a
format, dependency, capacity, or budget failure.

## Common Format Rules

Every file family must specify the same validation vocabulary:

- magic/version/header and byte order;
- record count bounds, byte-size bounds, string/path bounds, and alignment
  requirements;
- deterministic identity, source hash, payload hash, and total file size;
- dependency list with typed references and no implicit lookup by display name;
- file-family identification from internal `kind` / `version` / `schema` or
  equivalent metadata, not from `.yu*` filename suffixes;
- coordinate system, units, handedness, transform order, winding, and UV origin;
- explicit error/status values for unsupported version, invalid header,
  invalid count, invalid size, invalid dependency, duplicate id, missing
  dependency, hash mismatch, unsupported field value, output capacity exceeded,
  and budget exceeded;
- validator behavior for no-mutation failures;
- cook/load behavior for deterministic output identity;
- generated-file policy: fixture outputs are produced by checked-in tools but
  are not committed unless a reviewer approves a tiny source fixture file.

The current `.yu*` file names are smoke-fixture names only, not a production
format naming policy. Source and authoring-side data should prefer AI- and
human-readable schema-shaped text or manifests when that improves single-team
iteration. Runtime/cook/export output should produce high-performance binary data
with internal magic, version, kind, hash, dependency, and table metadata. The
important contract is not the suffix; it is that the same bytes are read from
disk through approved engine paths and produce deterministic runtime records.
Do not sacrifice clean YuEngine runtime data for external ecosystem, plugin
marketplace, or commercial-engine format compatibility.

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
| Camera/tween descriptor file | PASS | `RuntimeAssetData_CameraTweenDescriptorLoadsFromDiskSceneReference` validates and loads `Camera/Main.yucamera` and cooked camera records through the runtime graph; ResourceBrowser surface/depth workflows consume the same 10-file graph |
| Mesh/material/texture cook payloads | PASS | `RuntimeAssetData_CookStoresDecodedPayloadsForMeshMaterialTexture` stores decoded payload records for seven decodable runtime records |
| RenderScene records | PASS | loaded handles feed cube/cylinder/cone geometry, shared material, camera, and frame records |
| RenderCore/RHI capture | PASS | `RuntimeAssetData_RenderClosedLoop_CapturesCubeCylinderConeThroughRhi` |
| CPU oracle guard | PASS | `RuntimeAssetData_CpuPpmOracleDoesNotBypassRhiRenderCore` |
| Upper-layer dependency guard | PASS | `RuntimeAssetData_DoesNotDependOnEditorWebUiInputOrGdiViewer` |

These slices are not a complete asset system. Current RuntimeAssetData coverage
accepts decoded texture material slots, loaded shader bytecode, cooked shader
payloads, disk animation sampling, staged scene loader output, package/cook/run,
and product-run smoke as the current mainline closed loop. The remaining work is
production hardening for richer mesh vertex/index payload policy, shader
compiler/import policy, broader material parameter semantics, and
scene/animation/camera family coverage beyond the canonical cube/cylinder/cone
graph.

## Validator, Cook, Load, Render

The accepted closed loop is staged:

1. **Generate / read source**: deterministic source artifacts are written or
   mounted on disk; tests may generate them, but runtime proof must read bytes.
2. **File / Mount / VFS**: `MountTable` and approved File/VFS paths open those
   bytes. A path may locate the bytes but not identify the family.
3. **Validate**: validate internal magic, version, kind, schema, bounds,
   dependency graph, hashes, coordinate rules, alignments, and output
   capacities.
4. **Cook**: produce cooked artifacts or runtime-ready staging records with
   deterministic ids, hashes, dependency tables, and payload ownership.
5. **Load**: register Resource and Asset records, store cache/decoded payloads,
   create dependency edges, and build staged scene loader outputs only after
   validation/cook succeeds.
6. **Render**: RenderScene consumes the staged loader output, RenderCore/RHI owns
   submission, shader/pipeline/texture handles, and capture proof.
7. **Diagnose**: if a layer is missing, report the exact missing layer, status,
   file/dependency, and no-mutation point that caused it.

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

## Next Slice Questions

Reviewers should answer these before the next implementation slice starts:

1. Which current RuntimeAssetData layers can be reused unchanged?
2. Which RVF layers must be reworked because they are test-side struct
   construction, CPU helper image generation, GDI/software viewer output, or
   non-File/VFS/Resource bypasses?
3. Should animation be inside the first gate slice or recorded as an explicit
   same-gate blocker until a separate clip-file contract is approved?
4. Which tiny source fixture files, if any, are allowed in the repo, and which
   generated outputs must stay under ignored artifact directories?
5. Which File/VFS/Resource/Package path owns source bytes, cooked bytes, and
   runtime records in the first slice?
6. What status names and no-mutation tests are mandatory before the next slice
   can keep `RUNTIME_ASSET_DATA_CLOSED_LOOP_CURRENT_SLICE_PASS`?

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

This plan is current-slice accepted when:

1. the paired gate records `RUNTIME_ASSET_DATA_CLOSED_LOOP_CURRENT_SLICE_PASS`;
2. implementation stays on `main` and does not require branch-only evidence;
3. clean configure, build, focused RuntimeAssetData tests, and full fast gate
   remain passing;
4. every runtime data family lists version/header, bounds, dependencies,
   deterministic identity/hash/size, coordinate rules, statuses, and validator
   behavior;
5. the minimal generator -> File/VFS/Resource -> Resource/Asset ->
   RenderScene/RenderCore/RHI -> capture loop is proven by focused tests;
6. hard blocks prevent current RVF helper output or viewer output from being
   counted as the final proof.
