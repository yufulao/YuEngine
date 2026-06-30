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
Package/Resource pressure gate: `docs/gates/RTSPINE_008A_PACKAGE_RESOURCE_PRESSURE_CONTRACT.md`
Resource Browser scope: `docs/YUENGINE_RESOURCE_BROWSER_IMPORT_COOK_DIAGNOSTICS_SCOPE.md`
Canonical entry point: `docs/README.md`
Parent plan: `docs/YUENGINE_LONG_PLAN_TEAM_EXECUTION.md`
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

The contract is not an editor, rejected editor route, UI, input, or Game Adapter contract. It is
also not a package compatibility contract for the original game. Original-game
resource facts may later become validation inputs only after a separate
evidence-approved package/parser gate.

## Production Target Alignment

RuntimeAsset is part of the production spine for a native commercial game with
multi-GB packed content. The local TouhouNewWorld package is the current
practical reference bar: a small native runtime, explicit resource archives, an
asset index/database surface, shader/config files, and resource data in the
shipped-content-scale class. That shipped-content size is a pressure example
used to choose explicit budget assumptions; it is not a hard byte target.

This plan therefore optimizes for:

- source and cooked asset records that can be packed, hashed, indexed, and
  validated;
- deterministic failure and no-mutation behavior before Resource, Asset,
  RenderScene, RenderCore, RHI, Audio, or World outputs are published;
- stable asset-internal identity that survives cook, pack, load, and runtime
  instancing;
- runtime records that editor/importer tools may produce later, but cannot
  redefine;
- no old-game package compatibility requirement unless a separate gate accepts
  it.

The first production dependency chain is:

```text
Package/Resource bytes and index
-> RuntimeAsset family identity and dependency table
-> asset-internal targets such as scene nodes, model nodes, and skeleton joints
-> animation/material/shader/scene records that reference those targets
-> runtime instance mapping
-> WorldObject or editor object surfaces only after mapping exists
```

Animation, model, scene, and shader work are not independent islands. They are
families in one RuntimeAsset graph and must share identity, dependency, budget,
hash, and no-mutation rules.

## RTSPINE-008A Package/Resource Pressure Contract

`docs/gates/RTSPINE_008A_PACKAGE_RESOURCE_PRESSURE_CONTRACT.md` defines the
pressure vocabulary and sequencing for Package/Resource work. Later
RuntimeAsset packaged-validation work must consume that gate instead of
inventing a separate package-size target.

RTSPINE-008A makes these decisions:

- pressure examples are not hard byte targets;
- future archive byte ranges use unsigned 64-bit offset and size values, with
  overflow and window-size rejection before state mutation;
- required hash coverage is entry payload hash, entry metadata hash,
  dependency table hash, package table hash, and optional archive hash when a
  container is introduced;
- validation failure must happen before Package, File/VFS, Resource,
  RuntimeAsset, Asset, RenderScene, RenderCore, RHI, Audio, or World state
  mutation;
- Package/Resource implementation, File/VFS ranged IO, Resource payload window
  records, RuntimeAsset packaged validation bridge, and transaction
  rollback/proof are tracked as separately gated follow-up contracts.
  RTSPINE-008G records the packaged validation bridge below; RTSPINE-008H
  records transaction rollback/proof below. Broader Resource/File/VFS
  follow-through remains a future gate.

RTSPINE-008B records the first Package follow-up decision in
`docs/gates/RTSPINE_008B_PACKAGE_BYTE_RANGE_LEGACY_MIRROR_DECISION.md`.
RTSPINE-008C Package artifact hash/dependency integrity is PASS at
`origin/main@d18f1679ebd389ecec506055764602591f5b9ab6`, covering Package-only
payload, metadata, dependency table, and package table hash validation without
opening Resource, File/VFS, or RuntimeAsset packaged validation.
RTSPINE-008D File/VFS ranged IO is PASS at
`origin/main@c67e9710ab39f49ea01f0c194d2e5b44cbf3b97e`, covering ranged
`FileReadRequest`, exact invalid-range statuses, failed-range no-mutation, and
async ranged output-too-small no-partial-copy behavior. Focused QA task
`aebd28c5-f688-4ccc-abaf-1a3bd61879cb` reports `YuFileTests` build PASS,
`^File_` discovery/execution `23/23` PASS, ranged subset `4/4` PASS,
diff/hygiene/boundary PASS, and no broad/full CTest. It opens only the File/VFS
ranged IO contract; Resource payload windows and RuntimeAsset packaged
validation bridge remain separate until their own gates.
RTSPINE-008E Resource payload window/reference budget is PASS at
`origin/main@8bb8eff9c98d2a0aa5050c5da6ad94049fa894be`, covering cache and
decoded payload window metadata, reference-budget rejection, window
overflow/mismatch no-mutation failures, and payload-window reference/residency
stability. Focused QA task `b4fa51c3-aefc-4714-b5d8-062f8a933ac9` reports
`YuResourceTests` build PASS, Resource window/reference discovery exactly `7`
rows, execution `7/7` PASS, commit-level diff/hygiene/boundary PASS, and no
broad/full CTest. It opens only the Resource payload window/reference budget
contract; RuntimeAsset packaged validation bridge remains separate until
RTSPINE-008G below.
RTSPINE-008F Package dependency closure and budgeted load plan is PASS at
`origin/main@8509f7e1b6ba15e79c574357a465ddfff4d80e10`, covering transitive
dependency closure, shared dependency de-duplication/order, record budget
no-mutation failure, and archive byte budget no-mutation failure. Focused QA
task `4f199c8e-99a4-43b4-a776-8960285ffdaf` reports `YuPackageTests` build
PASS, exact 008F rows `4/4` PASS, `^Package_` focused suite `39/39` PASS,
diff/hygiene/scope scan PASS, no broad/full CTest, and no QA edits/staging/
commits. It opens only the Package dependency closure and budgeted load-plan
contract; RuntimeAsset packaged validation bridge is released only by
RTSPINE-008G below. RTSPINE-008H is separate.
RTSPINE-008G RuntimeAsset packaged validation bridge is PASS at
`origin/main@175b6542cf8460b279d1de8a5499e2cbd508c80a`, covering archive
byte-range/hash and payload hash preflight before graph-load mutation,
dependency/load-plan validation, duplicate load-plan record rejection, packaged
validation status mapping, and ProductRun failure reporting without graph
mutation. Focused QA task `35fdc7a2-c09d-416a-95aa-b4aabdb05d0f` reports
focused `YuRuntimeAssetDataClosedLoopTests` build PASS, exact RTSPINE-008G rows
`5/5` PASS, adjacent packaged/product rows `8/8` PASS, committed scope exactly
`CMakeLists.txt`, `RuntimeAssetData.h/.cpp`, and
`RuntimeAssetDataClosedLoopTests.cpp`, `git diff --check` PASS, and no
broad/full CTest. It opens only the RuntimeAsset packaged validation bridge;
the separate RTSPINE-008H gate below covers transaction rollback/proof.
RTSPINE-008H RuntimeAsset transaction rollback/proof is PASS at
`origin/main@1120c3659bf0375f8eb9ef87e042f24c6e5d3ca1`, covering graph-load
rollback journals, rollback status/proof fields, and commit-failure rollback of
previously committed RuntimeAsset records plus Resource/Asset snapshots without
output mutation. Focused QA task `1ec65e79-70f2-4fe5-8f08-6fb0ba2371fd`
reports focused `YuRuntimeAssetDataClosedLoopTests` build PASS, exact
`RuntimeAssetData_LoaderCommitFailureRollsBackCommittedRecords` row `1/1` PASS,
focused rollback/commit/adjacent packaged/product set `19/19` PASS,
`git diff --check` PASS, added-line hygiene PASS, exact committed scope, no
Package/File/Resource lower-module changes, and no broad/full CTest. It opens
only the RuntimeAsset transaction rollback/proof gate; WorldObject/editor
mapping and broader Resource/File/VFS follow-through remain separate gates.
`archive_byte_offset` and `archive_byte_size` are the only authoritative
shipped-content pressure byte range. `byte_offset` and `byte_size` may remain
as legacy mirrors while Streaming, RuntimeAsset, and existing tests still
consume them, but they cannot be counted as pressure evidence.

The current Package/File/Resource fixture limits remain useful as deterministic
unit-test caps. They do not prove shipped-content pressure until a follow-up
gate explicitly maps them to runtime caps, budget assumptions, or bounded
windows.

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
hardening, not a reason to reopen editor, rejected editor route, UI, input, or external authoring
scope as acceptance.

The current mesh slice also validates and loads `position,texcoord` input
layout records, vertex/index stride, `uint16` index format, and triangle-list
topology from disk mesh files. The RenderScene/RenderCore/RHI proof consumes
the loaded layout records and the Resource/RHI fixture budgets now allow the
canonical cube/cylinder/cone payload sizes required by that typed layout.

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
| Asset target identity | scene node ids, model node ids, skeleton joint ids, target names/hashes where approved, parent/index data, property vocabulary | Animation, scene, model, and instance mapping records |
| Animation data | clip ids, track ids, channel ids, keyframe ranges, sample rate, selected clip id, target id plus property refs | Animation runtime sampler and transform-apply records |

The texture/material/shader rows are refined by
`docs/YUENGINE_RUNTIME_ASSET_V0_PAYLOAD_BRIDGE_RHI_ROUTE_PLAN.md`: cooked texture
payloads own byte layout, row pitch, size, hash, and decoded payload identity;
materials own bounded texture/sampler slot tables; shader/program records own
stage bytecode payloads, hashes, minimal reflection, input layout, and slot
counts before any RHI texture, shader module, or pipeline handle is published.

Animation may be carried as a same-gate dependency or named blocker for the
first review. The final cube/cylinder/cone route must not silently replace a
missing animation contract with per-frame sample math. Deeper animation work
must not proceed past selected clip and bounded table proof until
asset-internal target identity exists. Animation files must bind tracks to
asset targets and properties, not to WorldObject handles, editor object ids,
raw pointers, display names, or file paths.

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
| Asset target identity | scene/model/skeleton target ids, optional stable names/hashes, parent/index records, approved property vocabulary | target tables consumed by scene, model, animation, and instance mapping records | duplicate target ids, missing parent, invalid property, capacity overflow, hash mismatch, world/editor id leakage |
| Animation | clip ids, selected clip id, track id, channel id, asset target id, property id, sample rate, interpolation, keyframe ranges | sampled transform inputs or approved clip payload records consumed by Animation sampler | selected clip validity, target validity, time/keyframe bounds, unsupported interpolation, target/property mismatch, sample budget, no partial scene transform output |

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
8. one animation clip file with explicit selected-clip coverage or an explicit same-gate blocker row.

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
| File/VFS read path | PASS | fixture bytes are read through `MountTable` loose mount by `YuRuntimeAsset`; RTSPINE-008D adds ranged `FileReadRequest` and no-mutation File/VFS evidence at `c67e9710ab39f49ea01f0c194d2e5b44cbf3b97e` |
| Resource payload window/reference budget | PASS | RTSPINE-008E adds cache and decoded payload window metadata, reference-budget rejection, window overflow/mismatch no-mutation failures, and payload-window reference/residency stability evidence at `8bb8eff9c98d2a0aa5050c5da6ad94049fa894be` |
| Resource/Asset registration | PASS | scene and all generated asset families register synthetic Resource descriptors and runtime Asset handles |
| Resource/Asset dependency edges | PASS | `RuntimeAssetData_LoadRegistersResourceAndAssetDependencyEdges` records scene dependencies in both registries |
| Camera/tween descriptor file | PASS | `RuntimeAssetData_CameraTweenDescriptorLoadsFromDiskSceneReference` validates and loads `Camera/Main.yucamera` and cooked camera records through the runtime graph; ResourceBrowser surface/depth workflows consume the same 10-file graph |
| Mesh vertex/index payload policy | PASS | `RuntimeAssetData_MeshPayloadPolicyRejectsSizeHashAndSplitMismatch` validates bounded mesh payload bytes, payload alignment, payload hash, and vertex/index split sums for generated source and cooked mesh files |
| Mesh layout/topology records | PASS | `RuntimeAssetData_MeshLayoutTopologyDecodesIntoLoadedRecords` validates input layout, vertex/index stride, `uint16` index format, triangle-list topology, and loaded record propagation for cube/cylinder/cone disk mesh files |
| Mesh payload to geometry buffers | PASS | `RuntimeAssetData_ImportedMeshPayloadBytesFeedRenderGeometryBuffers` reads Resource decoded mesh payload records, validates payload hash and vertex/index byte split, feeds decoded bytes into RHI vertex/index buffers, and reports exact Model-layer failures for missing payload or hash mismatch |
| Material parameter semantics | PASS | `RuntimeAssetData_MaterialParameterSemanticsLoadIntoRuntimeRecords` validates and loads base color RGBA, emissive strength, metallic, roughness, opacity, alpha mode, and parameter count from disk material records |
| RenderCore material constants | PASS | `RenderCore_DrawableFramePipeline_PropagatesMaterialConstants`, `RenderCore_DrawableFramePipeline_RejectsOversizedMaterialConstantsWithoutMutation`, `RenderCore_FixturePass_BindsConstantBuffer`, and `Material_BindingFixture_PropagatesConstantBufferBindings` prove drawable frame material constant propagation, oversize rejection, and constant-buffer binding propagation |
| RenderScene material constants | PASS | `RenderScene_RuntimeMaterialCopiesMaterialConstants` and `RenderScene_RuntimeMaterialRejectsOversizedMaterialConstants` prove runtime material constant copy and no-mutation oversize rejection at the RenderScene material record boundary |
| RuntimeAsset material constant bytes | PASS | `RuntimeAssetData_MaterialConstantsPackLoadedParameters` packs loaded material parameters into deterministic material constant bytes |
| Cooked material constants bridge | PASS | `RuntimeAssetData_CookedMaterialConstantsBridgeToRenderSceneRecord` bridges cooked loaded material constants into RenderScene material records |
| Cooked material constants no-mutation | PASS | `RuntimeAssetData_CookedMaterialConstantsRejectInvalidLoadedMaterialWithoutMutation` rejects invalid loaded material constants without mutating RenderScene/RHI output |
| Material alpha blend-state bridge | PASS | `RenderCore_FixturePass_BindsAlphaBlendState`, `RenderCore_FixturePass_RejectsInvalidBlendStateBeforeCommandRecording`, `RenderCore_MaterialCopiesBlendState`, `RenderCore_DrawableFramePipeline_PropagatesAlphaBlendState`, `RenderScene_RuntimeMaterialCopiesBlendState`, `RenderScene_RuntimeMaterialRejectsInvalidBlendStateWithoutMutation`, `RuntimeAssetData_GenericRenderSceneSubmissionPreservesPerEntityMaterialBlendState`, `RuntimeAssetData_CookedMaterialAlphaBlendStateBridgeToRenderSceneRecord`, and `RuntimeAssetData_CookedMaterialAlphaBlendRejectsInvalidLoadedMaterialWithoutMutation` prove material alpha/opacity reaches `RhiBlendStateDesc` through RuntimeAsset, RenderScene, RenderCore, and RHI-facing command paths with no-mutation invalid failures |
| RHI constant-buffer binding | PASS | `RHI_ConstantBufferBinding_DefaultsAreBoundedValues`, `RHI_CommandList_RecordsConstantBufferBindingWithinCapacity`, `RHI_ConstantBufferSlotOverflow_DoesNotMutate`, `RHI_SubmitConstantBufferBinding_UpdatesNullSnapshot`, and `RHI_SubmitConstantBufferBindingRejectsStaleHandle` prove bounded command/list binding and stale-handle rejection |
| Shader import policy | PASS | `RuntimeAssetData_ShaderImportPolicyValidatesSourceCookedAndLoadedRecords` validates source/cooked shader import language, target, entries, profiles, compile flags, and loaded record policy identity |
| Shader compiler backend boundary | PASS | `RuntimeAssetData_ShaderCompilerBackendProducesProgramReflection` compiles deterministic disk program bytes into loaded program data, reports import policy, stage, bytecode hash, input layout, and texture slot reflection identity, feeds the RHI pipeline bridge, and rejects unknown or `NativeHlsl` backends before publishing compiled program, reflection, or hash counts |
| Cooked shader stage modules | PASS | `RuntimeAssetData_CookedShaderStagePayloadsCreateRhiModules` creates RHI shader modules from owned cooked stage bytecode and validates stage payload identity |
| Cooked program reflection/input layout | PASS | `RuntimeAssetData_CookedProgramPipelineUsesLoadedReflectionAndInputLayout` and `RuntimeAssetData_ShaderProgramBridgeAcceptsVariableTextureSlotReflection` build the cooked program pipeline from loaded reflection and input-layout data, including non-canonical `position,texcoord` input and `textures=0` success |
| Cooked shader mismatch no-mutation | PASS | `RuntimeAssetData_CookedShaderPayloadRejectsStageBytecodeHashAndReflectionMismatchWithoutMutation` rejects stage bytecode hash and reflection mismatch before mutating Resource/Asset/RHI output; `RuntimeAssetData_ShaderProgramBridgeRejectsInvalidProgramDataWithoutRhiMutation` rejects `input=layout:none` as `InvalidInputLayout` before RHI mutation |
| Cooked shader partial RHI cleanup | PASS | `RuntimeAssetData_CookedShaderProgramRhiPartialCreationFailureDestroysTransientHandles` destroys transient RHI handles on partial program creation failure |
| Scene camera family failures | PASS | `RuntimeAssetData_SceneAnimationLoaderRejectsCameraFamilyFailuresWithoutMutation` validates duplicate active camera, no active camera, invalid camera row, and invalid entity camera ref failures without Resource/Asset/RenderScene output mutation |
| Runtime animation output tables | PASS | `RuntimeAssetData_SceneAnimationLoaderEmitsReusableRuntimeAnimationTables` and `RuntimeAssetData_RuntimeAnimationTablesResampleAndFeedRenderSceneSubmission` prove bounded clip/track/keyframe tables can be emitted and consumed by RenderScene submission |
| Scene animation explicit clip selection | PASS | `RuntimeAssetData_SceneAnimationLoaderSamplesExplicitSelectedClip` proves `selected_animation_clip_id` selects the requested clip from a multi-clip file; `RuntimeAssetData_SceneAnimationLoaderRejectsMissingSelectedClipWithoutMutation` rejects a missing selected clip as `InvalidDependency` without mutating output |
| Scene animation focused QA | PASS | `origin/main@f211f7f95299388987ccef00b4d1e8ee6f7bf0c1` has focused `YuRuntimeAssetDataClosedLoopTests` build PASS, exact scene-animation/runtime animation CTest `11/11` PASS, `git diff --check` PASS, added-line hygiene PASS, and dependency boundary PASS |
| Asset target identity table | PASS | `origin/main@5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4` defines `RuntimeAssetTargetIdentityRecord` output tables for scene node, model node, and skeleton joint identities; `RuntimeAssetData_TargetIdentityTableLoadsSceneModelAndSkeletonJointIds`, `RuntimeAssetData_TargetIdentityTableRejectsDuplicateIdWithoutMutation`, `RuntimeAssetData_TargetIdentityTableRejectsMissingParentWithoutMutation`, and `RuntimeAssetData_TargetIdentityTableRejectsCapacityOverflowWithoutMutation` are covered by focused QA |
| Asset target identity focused QA | PASS | focused `YuRuntimeAssetDataClosedLoopTests` build PASS; exact target identity plus scene/runtime animation regression discovery found `10` rows and execution reported `10/10` PASS; `git diff --check`, added-line hygiene, and production boundary scans passed; broad full CTest was not run for this docs lane |
| Animation track target binding | PASS | `origin/main@ebe9ea35f531aa40133262b701e5e751f8ed9ccf` defines caller-owned `RuntimeAssetAnimationTrackTargetBindingRecord` output for SceneNode `target_id` plus property binding; `RuntimeAssetData_AnimationTrackTargetBindingResolvesTargetIdAndProperty`, `RuntimeAssetData_AnimationTrackTargetBindingRejectsMissingTargetWithoutMutation`, `RuntimeAssetData_AnimationTrackTargetBindingRejectsUnsupportedPropertyWithoutMutation`, and `RuntimeAssetData_AnimationTrackTargetBindingRejectsCapacityOverflowWithoutMutation` prove binding success and no-mutation failures |
| Animation track target binding focused QA | PASS | workspace task `2e2d5a4e-0bb0-4cf4-bd1b-ab3a87987b7f` reports focused `YuRuntimeAssetDataClosedLoopTests` build PASS, focused discovery `17` rows, execution `17/17` PASS, `git diff --check` PASS, added-line hygiene PASS, dependency boundary PASS, and non-goal scans PASS; broad full CTest was not run, and ModelNode/SkeletonJoint animation binding was later closed by the `3fa4ef7` target-family gate |
| ModelNode/SkeletonJoint target-family binding | PASS | `origin/main@3fa4ef7bd42da8f60bd5ebb3a7f863bd76292c84` adds RuntimeAssetData ModelNode/SkeletonJoint target-family binding support; `RuntimeAssetData_AnimationTrackTargetBindingResolvesModelAndSkeletonTargetFamilies`, `RuntimeAssetData_RuntimeInstanceMappingBuildsTargetFamilyRows`, and `RuntimeAssetData_RuntimeInstanceMappingResolvesModelAndSkeletonFamilies` are included in the focused evidence set |
| ModelNode/SkeletonJoint target-family binding VQ | PASS | implementation task `06724fe5-b2e4-410e-97e7-2b41c195c3a0` reports COMPLETE-PASS / committed, VQ task `04e2a7a6-eac5-41d2-9624-6e5e952859c4` reports COMPLETE-PASS / VQ-READY, committed scope was exactly `CMakeLists.txt`, `RuntimeAssetData.cpp`, and `RuntimeAssetDataClosedLoopTests.cpp`, focused `YuRuntimeAssetDataClosedLoopTests` build PASS, focused CTest discovery found `17` rows, focused execution `17/17` PASS, and old unsupported target-family labels are absent |
| Animation minimal interpolation | PASS | `origin/main@2bfe7e37d36ca711dd706728f21b1e4caecfd3db` adds Step/Linear fixed-time sampling plus no-mutation unsupported-interpolation and sample-output-capacity failures through `RuntimeAssetData_AnimationInterpolationSamplesStepAndLinearAtFixedTime`, `RuntimeAssetData_AnimationInterpolationRejectsUnsupportedModeWithoutMutation`, and `RuntimeAssetData_AnimationInterpolationRejectsSampleOutputCapacityWithoutMutation` |
| Animation minimal interpolation focused QA | PASS | workspace task `951a3da8-6b13-4268-960e-407f65c40db7` reports focused `YuRuntimeAssetDataClosedLoopTests` build PASS, exact RTSPINE-005 interpolation discovery `3`, execution `3/3` PASS, non-Package RuntimeAsset animation whitelist `23/23` PASS, `git diff --check` PASS, added-line hygiene PASS, production boundary/non-goal scans PASS, and no broad/full CTest run |
| Animation invalid-target failure model | PASS | `origin/main@96e0c024435f670c39ced019ff825b819a6830a3` adds `RuntimeAssetData_AnimationTrackTargetBindingRejectsTargetFamilyMismatchWithoutMutation` and `RuntimeAssetData_AnimationFailureModelReportsSampleFailuresWithoutMutation` for target-family mismatch and sample failure diagnostics without output mutation |
| Animation invalid-target failure model focused QA | PASS | workspace task `6d02c260-936a-456b-917b-5c2802bbb666` reports isolated clean worktree at `96e0c024`, focused `YuRuntimeAssetDataClosedLoopTests` build PASS, focused RuntimeAsset regex discovery/execution `8/8` PASS, exact new RTSPINE-006 rows `2/2` PASS, `git diff --check` PASS, added-line hygiene PASS, non-goal boundary scan PASS, and no broad/full CTest run |
| Mesh/material/texture cook payloads | PASS | `RuntimeAssetData_CookStoresDecodedPayloadsForMeshMaterialTexture` stores decoded payload records for seven decodable runtime records |
| RenderScene records | PASS | loaded handles feed cube/cylinder/cone geometry, shared material, camera, and frame records |
| Generic RenderScene CPU submission | PASS | `RuntimeAssetData_GenericRenderSceneSubmissionBuildsFrameFromLoadedSceneRecords`, `RuntimeAssetData_GenericRenderSceneSubmissionBindsActiveCameraIntoFrame`, `RuntimeAssetData_GenericRenderSceneSubmissionRejectsMissingCameraWithoutMutation`, `RuntimeAssetData_GenericRenderSceneSubmissionUsesMeshRefsNotEntityOrder`, `RuntimeAssetData_GenericRenderSceneSubmissionRejectsMissingTransformWithoutMutation`, `RuntimeAssetData_GenericRenderSceneSubmissionRejectsMissingMeshRefWithoutMutation`, `RuntimeAssetData_GenericRenderSceneSubmissionRejectsMissingMaterialRefWithoutMutation`, `RuntimeAssetData_GenericRenderSceneSubmissionReportsMaterialVariantsUntilFrameApiSupportsThem`, `RuntimeAssetData_GenericRenderSceneSubmissionBuildsPerEntityMaterialTableFromRefs`, `RuntimeAssetData_GenericRenderSceneSubmissionPreservesPerEntityMaterialBlendState`, `RuntimeAssetData_GenericRenderSceneSubmissionUsesMaterialRefOrderNotEntityOrder`, `RuntimeAssetData_GenericRenderSceneSubmissionRejectsSmallMaterialTableWithoutMutation`, and `RuntimeAssetData_GenericRenderSceneSubmissionRejectsDuplicateMaterialIdWithoutMutation` prove loaded scene records can build RenderScene frame records, active camera bindings, and per-entity material tables with bounded no-mutation failures |
| Package/product generic submission ledger | PASS | `RuntimeAssetData_PackageRunEmitsGenericRenderSceneSubmissionLedger`, `RuntimeAssetData_ProductRunCommandReportsGenericRenderSceneSubmissionLedger`, and `RuntimeAssetData_PackageRunRejectsGenericSubmissionCapacityWithoutMutation` prove packaged and product-run entrypoints expose generic RenderScene submission ledgers and reject undersized output capacity without mutating frame/material outputs |
| RuntimeAsset packaged validation bridge | PASS | `origin/main@175b6542cf8460b279d1de8a5499e2cbd508c80a` adds archive byte-range/hash and payload hash validation before graph-load mutation; `RuntimeAssetData_PackagedValidationBridgeConsumesArchiveByteRangesAndHashes`, archive byte-count mismatch, payload hash mismatch, duplicate load-plan record, and ProductRun packaged validation failure rows are covered by focused QA task `35fdc7a2-c09d-416a-95aa-b4aabdb05d0f`, which reports focused build PASS, exact RTSPINE-008G rows `5/5` PASS, adjacent packaged/product rows `8/8` PASS, `git diff --check` PASS, exact committed scope, and no broad/full CTest |
| RuntimeAsset transaction rollback/proof | PASS | `origin/main@1120c3659bf0375f8eb9ef87e042f24c6e5d3ca1` adds rollback journals, rollback status/proof fields, and commit-failure rollback of previously committed RuntimeAsset records plus Resource/Asset snapshots; `RuntimeAssetData_LoaderCommitFailureRollsBackCommittedRecords` is covered by focused QA task `1ec65e79-70f2-4fe5-8f08-6fb0ba2371fd`, which reports focused build PASS, exact row `1/1` PASS, rollback/commit/adjacent packaged/product set `19/19` PASS, `git diff --check` PASS, exact committed scope, and no broad/full CTest |
| RuntimeAsset post-008H payload-window chain | HEAD evidence | The `50ff335fe1ddfea77a72ce20f770baa3028df4a2` ledger records `7c41265` RTSPINE-008I package archive range to RuntimeAsset Resource payload-window handoff, `bc6d0ee` Resource U64 `payload_window` follow-through, `2c93ddf` RuntimeAsset payload logical count propagation, `6ac7ff9` Streaming Resource cache payload bridge, `08b1ccd` Package payload metadata producer, `35a84c3` legacy artifact compatibility fix, `50a654e` Streaming pipeline cache payload consumer, `baae22d` rejection/no-mutation coverage fix, `e5cd6ee` Package-to-Streaming artifact fixture, and `10f7b30` RuntimeAssetData package payload-window consumer; this row is a canonical evidence ledger and VQ status must come from each lane's workspace result |
| RuntimeAssetWorldAdapter / destination range chain | HEAD evidence | The `50ff335fe1ddfea77a72ce20f770baa3028df4a2` ledger records `e2e8c3c` RuntimeAssetWorldAdapter bridge using exact marker `RuntimeAssetWorldObjectAdapter`, `1658639` RuntimeAssetWorldAdapter restore handoff using exact marker `RuntimeAssetWorldObjectRestoreHandoff`, `0d2021c` Streaming U64 staging window, `c3cf022` RHI update destination range contract, and `50ff335` ResourceUpload destination range consumer; direct WorldObject/editor binding, broad Resource/File/VFS expansion, and stage-close claims still require their own scoped gate/VQ |
| RuntimeAssetWorldAdapter target-family alias handoff | PASS | `origin/main@296100b3bda25e962c3a3a503f9f78f0160083ce` closes `RTSPINE-RUNTIMEASSETWORLDADAPTER-TARGET-FAMILY-ALIAS-HANDOFF-U64-001`; implementation task `77376606-d3d8-45de-8079-79121593b8e7` reports COMPLETE-PASS / committed and VQ task `5fb82855-a437-4eb7-b078-373069988b2d` reports COMPLETE-PASS / VQ-READY; exact scope is `CMakeLists.txt`, `RuntimeAssetWorldObjectAdapterBridge.cpp`, and `RuntimeAssetWorldObjectAdapterBridgeTest.cpp`; focused RuntimeAssetWorldObjectAdapter matrix reports `13/13` PASS including Model/Skeleton alias handoff; direct WorldObject/editor/GameAdapter and broader Resource/File/VFS closure remain separate gates |
| RuntimeAssetWorldAdapter handoff target-family proof | PASS | `origin/main@54e02e049bb6f67fd15ca32d1675f1c61380ae70` closes `RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-TARGET-FAMILY-PROOF-U64-001`; implementation task `53b6d5dc-fd17-442c-b18b-9257c4f3650c` reports COMPLETE-PASS / committed and VQ task `8fbe251e-2c14-4786-a48c-5b8b0b6f8e14` reports COMPLETE-PASS / VQ-READY; exact scope is `CMakeLists.txt` and `RuntimeAssetWorldObjectRestoreHandoffBridgeTest.cpp`; focused RuntimeAssetWorldObjectRestoreHandoff discovery/execution reports `5/5` PASS including `RuntimeAssetWorldObjectRestoreHandoff_AppliesModelAndSkeletonTargetFamilyAliases`; the Unknown adapter-preflight negative row preserves no-mutation semantics; direct WorldObject/editor/GameAdapter and broader Resource/File/VFS closure remain separate gates |
| RenderCore/RHI capture | PASS | `RuntimeAssetData_RenderClosedLoop_CapturesCubeCylinderConeThroughRhi` |
| CPU oracle guard | PASS | `RuntimeAssetData_CpuPpmOracleDoesNotBypassRhiRenderCore` |
| Upper-layer dependency guard | PASS | `RuntimeAssetData_DoesNotDependOnEditorUiInputOrGdiViewer` |

These slices are not a complete asset system. Current RuntimeAssetData coverage
accepts decoded mesh payload geometry buffers, decoded texture material slots,
loaded material constant bytes and alpha blend state into RenderScene records,
RHI constant-buffer command/list binding, loaded shader bytecode, shader
compiler backend identity, unsupported native backend prevalidation, cooked
shader module creation, cooked reflection/input-layout bridge, shader
reflection hardening for variable input/texture-slot counts, cooked shader
no-mutation failures, disk animation sampling, staged scene loader output,
runtime animation output tables, explicit selected animation clip sampling,
asset-internal scene node/model node/skeleton joint target identity tables,
SceneNode animation track `target_id` plus property binding records,
invalid-target failure model diagnostics, generic RenderScene CPU submission
records, runtime instance mapping rows, VQ-closed ModelNode/SkeletonJoint
target-family binding rows, per-entity material tables, package/cook/run ledgers,
and product-run smoke as the current mainline closed loop. The
remaining work is production hardening for broader material and scene variants,
native/non-fixture shader compiler integration, skeletal skinning and deeper
animation variants beyond the VQ-closed target-family binding rows,
WorldObject-facing instance application, blending and clip-family variants, and
scene/animation/camera production variants beyond the canonical
cube/cylinder/cone graph.

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

## Next Slice Decision

RTSPINE-003 target identity docs and VQ gates are closed, RTSPINE-004
implementation plus focused QA are PASS, RTSPINE-005 minimal interpolation
implementation plus focused QA are PASS, and RTSPINE-006 invalid-target failure
model implementation plus focused QA are PASS. RTSPINE-007 runtime instance
mapping implementation plus focused QA are PASS at
`37a112549190ac2123abcd72b5c688cdfa5b01e5`, covering caller-owned runtime
instance rows for SceneNode targets and no-mutation failures for missing scene
entities and capacity overflow; ModelNode/SkeletonJoint mapping is now covered
by the later `3fa4ef7` target-family binding VQ.
RTSPINE-008G RuntimeAsset packaged validation bridge implementation plus focused
QA are PASS at `175b6542cf8460b279d1de8a5499e2cbd508c80a`, covering packaged
validation preflight before graph mutation and ProductRun failure reporting.
RTSPINE-008H transaction rollback/proof implementation plus focused QA are PASS
at `1120c3659bf0375f8eb9ef87e042f24c6e5d3ca1`, covering commit-failure
rollback without output mutation. The `50ff335` ledger then records the
post-008H RuntimeAssetWorldAdapter, Resource/Streaming payload-window,
Package payload metadata, RuntimeAssetData payload-window consumer, RHI
destination-range, and ResourceUpload destination-range implementation chain.
Treat that chain as canonical implementation/focused evidence unless a workspace
VQ result explicitly marks the individual lane COMPLETE-PASS. Direct
WorldObject/editor binding and broader Resource/File/VFS expansion still need
their own gates. RTSPINE-RUNTIMEASSETWORLDADAPTER-TARGET-FAMILY-ALIAS-HANDOFF-U64-001
is now VQ-closed at `296100b3bda25e962c3a3a503f9f78f0160083ce`; it accepts
SceneNode, ModelNode, and SkeletonJoint runtime instance mappings through the
existing scene entity/scene transform plus identity-record handoff path, while
preserving invalid-kind, duplicate, capacity, null, and no-mutation semantics.
RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-TARGET-FAMILY-PROOF-U64-001 is now
VQ-closed at `54e02e049bb6f67fd15ca32d1675f1c61380ae70`; it proves
ModelNode/SkeletonJoint alias records pass through the restore handoff path,
including adapter, world active restore gate, and transform restore bridge,
while the Unknown negative row preserves adapter-preflight no-mutation failure
semantics. No production bridge source was changed.
Read-only scout `06ca8037-d242-4482-bfe2-3eee93342bf3` selected
`RTSPINE-RUNTIMEASSETWORLDADAPTER-HANDOFF-ATTACHMENT-BINDING-GATE-PROOF-U64-001`
as the next same-module sidecar proof gate. It should cover non-zero World
component attachment and component resource binding snapshot records through
the existing World active restore gate, while keeping World/Resource as
read-only evidence and keeping direct WorldObject/editor/GameAdapter plus broad
Resource/File/VFS out of scope.
RTSPINE-RUNTIMEASSET-MODEL-SKELETON-TARGET-BINDING-U64-001 is VQ-closed at
`3fa4ef7bd42da8f60bd5ebb3a7f863bd76292c84`: implementation task
`06724fe5-b2e4-410e-97e7-2b41c195c3a0` reports COMPLETE-PASS / committed and VQ
task `04e2a7a6-eac5-41d2-9624-6e5e952859c4` reports COMPLETE-PASS / VQ-READY.
It closes ModelNode/SkeletonJoint target-family binding for RuntimeAssetData
focused rows, but it does not open direct WorldObject/editor binding or broader
Resource/File/VFS follow-through.
Selected clip sampling remains the earlier closed slice because it only selects
among bounded clip records and proves no-mutation failure for a missing selected
clip.

RTSPINE-003 has implementation and focused QA evidence at
`origin/main@5ea838f6fd3428e7e67b77c1ca85c41e6e1c09e4`, plus VQ PASS evidence
from workspace task `fdd78da4-da12-4956-b6ac-63ff9e377121`. RTSPINE-004 has
implementation evidence at `origin/main@ebe9ea35f531aa40133262b701e5e751f8ed9ccf`
and focused QA PASS by workspace task `2e2d5a4e-0bb0-4cf4-bd1b-ab3a87987b7f`.
RTSPINE-005 has implementation evidence at
`origin/main@2bfe7e37d36ca711dd706728f21b1e4caecfd3db` and focused QA PASS by
workspace task `951a3da8-6b13-4268-960e-407f65c40db7`. The next accepted
RuntimeAsset spine constraints are:

1. preserve source and cooked asset target tables for scene nodes, model nodes,
   and skeleton joints as the binding root;
2. keep RTSPINE-004 binding limited to SceneNode target id plus property without
   WorldObject, editor object, raw pointer, display name, or file path
   references;
3. count ModelNode/SkeletonJoint target-family binding only through the later
   `3fa4ef7` VQ-closed gate, not through the older SceneNode-only RTSPINE-004
   PASS;
4. keep RTSPINE-005 limited to Step/Linear interpolation and the covered
   no-mutation interpolation failures; do not treat it as cubic, blend tree,
   montage, timeline, skeletal skinning, or editor authoring proof;
5. keep RTSPINE-006 scoped to target-family mismatch and sample failure
   diagnostics; broader animation production variants still need separate gates;
6. keep RTSPINE-007 scoped to caller-owned runtime instance mapping records for
   asset targets and scene entities; do not treat it as direct WorldObject or
   editor-object binding.

Shader/reflection hardening remains a separate RuntimeAsset family and may
continue only when the active evidence gate is closed and the work does not
pretend to solve scene/model/animation target identity.

## Next Slice Questions

Reviewers should answer these before the next implementation slice starts:

1. Which current RuntimeAssetData layers can be reused unchanged?
2. Which RVF layers must be reworked because they are test-side struct
   construction, CPU helper image generation, GDI/software viewer output, or
   non-File/VFS/Resource bypasses?
3. Which WorldObject-facing application gate can consume RTSPINE-007 records
   without moving object identity into asset files?
4. Which tiny source fixture files, if any, are allowed in the repo, and which
   generated outputs must stay under ignored artifact directories?
5. Which File/VFS/Resource/Package path owns source bytes, cooked bytes, and
   runtime records in the first slice?
6. What status names and no-mutation tests are mandatory before the next slice
   can keep `RUNTIME_ASSET_DATA_CLOSED_LOOP_CURRENT_SLICE_PASS`?
7. Which package/index/hash/budget constraints must be checked against
   shipped-content pressure examples and explicit budget assumptions?

## Hard Blocks

The following are blocking violations:

- using editor, rejected editor route, UI, input, or Game Adapter behavior as part of the closed
  runtime data proof;
- binding animation or scene data directly to WorldObject handles, editor
  object ids, raw pointers, display names, or file paths;
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
