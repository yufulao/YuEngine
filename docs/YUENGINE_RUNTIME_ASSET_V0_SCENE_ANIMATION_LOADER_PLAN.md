# YuEngine RuntimeAsset v0 Scene/Animation Loader Plan

Status: RAV1-D Phase A docs/API-plan
Owner: RuntimeAsset / Architecture
Task: #53
Baseline: `4b846aa Preflight RuntimeAsset scene loader before registration`
Related closure plan: `docs/YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md`
Related contract plan: `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
Related gate: `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
Format policy and validator vocabulary:
`docs/YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md`

## Purpose

This plan defines the next scene/animation loader shape after the RAV0 fixed
cube/cylinder/cone fixture. It is not an implementation approval and does not
implement a general loader.

The target is still RuntimeAsset v0 runtime data: bounded source/cooked artifact
records read through the RuntimeAsset graph loader, validated before mutation,
sampled through the approved Animation runtime sampler, and emitted as
deterministic records that RenderScene can consume. It is not an editor scene
format, import pipeline, original-game package parser, gameplay object factory,
or authoring scene parser.

## RAV0 Baseline

RAV0 proved an important floor:

- `LoadRuntimeAssetDataGraph` reads the scene and referenced files through the
  File/VFS path.
- Scene and animation bytes are validated before Resource/Asset registration.
- `BuildSceneLoaderStage` preflights entity parsing, animation sampling, sampled
  transform application, and World transform queries into private staging state.
- `CommitSceneLoaderOutput` writes caller-owned scene outputs only after the
  graph has passed preflight and Resource/Asset/dependency commit.
- Invalid entity values and invalid animation keyframes return
  `InvalidDependency` without changing caller scene outputs, ResourceRegistry,
  AssetManager, or dependency edges.

That baseline must be preserved.

## Fixture-Bound Surface To Remove

The current implementation is intentionally small and still fixture-bound:

| Area | Current RAV0 shape | RAV1-D replacement direction |
| --- | --- | --- |
| Entity count | `RUNTIME_ASSET_SCENE_ENTITY_COUNT = 3` and hardcoded `e0/e1/e2` | bounded entity table with caller/file limits |
| Mesh refs | fixed mesh ordinal 0/1/2 mapped to cube/cylinder/cone | per-entity typed mesh ref index or stable id |
| Material/texture/shader refs | one material, first texture, first shader/program for every entity | per-entity material ref, material-owned texture slots, program ref from loaded records |
| Camera | one synthetic active camera record, not parsed from a camera table | bounded camera table with one active camera selected by id |
| Animation | one animation file, one clip, one track, two keyframes, linear interpolation | bounded clip/track/keyframe tables and target bindings |
| Dependency validation | token rules such as `m0=Mesh/`, `cam=camera:`, `anim=Animation/` | typed dependency table from internal metadata and stable ids |
| Family identity | caller descriptor plus source-text tokens; `.yu*` names remain smoke examples | internal magic/version/kind/schema and dependency metadata are authoritative |
| Output buffers | caller arrays sized exactly for 9 refs, 1 camera, 3 entities, 3 transforms | caller-provided spans validated against declared limits |
| RenderScene handoff | tests still adapt records into three-primitive capture helpers | deterministic loader records feed RenderScene frame construction without fixture-only bridges |

RAV1-D may keep the cube/cylinder/cone scene as the smallest acceptance fixture,
but success must be expressed as "N bounded entities" rather than "exactly three
hardcoded entities".

## Bounded Record Model

Scene and animation records use the shared RuntimeAsset v0 vocabulary from the
format policy and the RAV1-A contract: internal `magic`, `version`, `kind`,
`schema`, `sourceHash`, `payloadHash`, `dependencyTable`, `recordTable`, and
`coordinateSpec`. File names and suffixes may appear in tests as smoke paths,
but they are not type truth. Source artifacts may stay readable for review/cook;
cooked artifacts carry the same authoritative identity in a bounded table
directory with byte ranges, alignment, dependency hashes, and loader output
capacities.

### Limits

RAV1-D should add explicit limits to the loader request or to a paired limits
record. The first production-v0 gate can use these default caps unless #50 or
#51 replaces them with a shared contract:

| Limit | Default cap | Notes |
| --- | ---: | --- |
| Scene resource refs | 64 | typed refs to loaded RuntimeAsset records |
| Scene entities | 64 | visible/active runtime entities, sorted deterministically |
| Scene transforms | 64 | one transform per entity in v0 |
| Scene cameras | 4 | exactly one active camera required for render proof |
| Animation clips | 8 | clips referenced by the scene |
| Animation tracks | 128 | tracks across all clips |
| Animation keyframes | 512 | keyframes across all tracks |
| Animation target bindings | 128 | track target -> entity/transform mapping |
| Animation sampled values | 128 | sampler output capacity before transform apply |

The file header and caller request must both expose the relevant counts. If a
file declares more records than the loader caps, return `CapacityExceeded`
before any Resource, Asset, dependency, or caller-output mutation. If counts are
malformed or internally inconsistent, return `InvalidCount`.

### Scene Tables

The scene record set contains:

- scene header: scene id, schema version, coordinate spec, active camera id,
  deterministic sort policy, and table counts;
- dependency table: typed refs to mesh, material, texture, shader/program,
  camera, and animation records, each carrying expected kind and hash metadata;
- entity table: entity id, world object id, transform index, mesh ref, material
  ref, optional camera visibility mask, animation binding range, active/visible
  flags, and deterministic sort key;
- transform table: translation, rotation, scale, and transform-order metadata
  mapped to `WorldTransformState` during preflight;
- camera table: camera id, active flag, pose, projection, target surface/capture
  intent, clear color, and supported v0 projection kind;
- scene-output table: staged resource refs, camera records, entity records,
  transform output records, diagnostics counts, and animation sample/apply
  statuses.

Required validation:

- table ranges must be inside the declared byte and record bounds;
- every entity transform index and resource ref index must resolve;
- exactly one active camera must resolve for the closed-loop render proof;
- all ids must be deterministic and duplicate-free where the table requires
  uniqueness;
- entity order in loader output is by scene sort key, then stable entity id, not
  input path order or hash-table iteration.

### Animation Tables

The animation record set contains:

- clip table: clip id, duration, sample rate, first track index, track count,
  layer count, loop flag, and interpolation policy constraints;
- track table: track id, target binding, channel, interpolation mode, first
  keyframe index, keyframe count, and validity flag;
- keyframe table: time, scalar/vector/quaternion component value, tangent fields
  only if v0 explicitly supports them, and validity flag;
- target binding table: typed target family, entity id, transform channel, and
  optional transform component index;
- sample request metadata: frame context, clip start time, and sample range.

Required validation:

- clip ranges and track ranges must be non-empty and within table bounds;
- keyframe times must be finite, monotonic per track, and inside the clip
  duration unless a later contract explicitly allows extrapolation;
- track target bindings must resolve to scene entities that exist in the same
  staged scene;
- unsupported channels or interpolation modes return `UnsupportedFieldValue`;
- invalid numeric transform/keyframe values return `InvalidBounds`;
- sampled value capacity overflow returns `CapacityExceeded`;
- target family mismatch returns `TypeMismatch`.

## Transaction And No-Mutation API Shape

RAV1-D aligns with the #51 loader transaction plan. Scene/animation parsing and
output staging live inside the `RuntimeAssetLoadTransactionPhase` flow rather
than beside it. The scene/animation part needs these phases:

1. **Read bytes** through the approved File/VFS path.
2. **Parse metadata**: header, kind, schema, table ranges, dependency table, and
   declared counts.
3. **Validate graph refs**: typed scene dependencies, hashes, expected kind, and
   duplicate/missing refs.
4. **Validate and preflight scene/animation** into transaction-owned staging:
   - parse scene tables;
   - parse animation tables;
   - build a private World/Transform bridge from staged entities;
   - sample animation through `AnimationRuntimeSampler`;
   - apply sampled transforms to the private transform bridge;
   - query final transforms into staged output records.
5. **Preflight commit intents**: capacity-check all Resource, Asset, cache
   payload, decoded payload, dependency edge, and scene output writes before
   committing.
6. **Stage scene output** under `RuntimeAssetLoadTransactionPhase::StageSceneOutput`.
   This is transaction scratch only; caller spans still remain unchanged.
7. **Commit Resource/Asset/dependency state** only after
   parse/validate/preflight/stage succeeds.
8. **Commit caller scene outputs** only as the final deterministic commit step.

If the RAV1-A route records animation sample ownership after Resource/Asset
registration, RAV1-D still requires a scratch sample/apply preflight before any
registry mutation. The implementation can publish committed animation ownership
after Resource/Asset commit, but it cannot discover invalid scene transforms,
keyframes, targets, sample capacity, or apply failures after it has already
mutated registries or caller scene outputs.

The public implementation should reuse the #51 transaction names:

- `BuildRuntimeAssetLoadTransactionPlan`
- `PreflightRuntimeAssetLoadCommit`
- `CommitRuntimeAssetLoadTransaction`

Scene/animation-specific helpers may sit under those phases with names such as:

- `ValidateRuntimeAssetSceneAnimationRecords`
- `StageRuntimeAssetSceneAnimationOutput`
- `CommitRuntimeAssetSceneOutputs`

Exact helper names may change, but the semantic split must remain: no caller
outputs, ResourceRegistry state, AssetManager state, or dependency edges change
on parse, dependency, capacity, transform, keyframe, sample, apply, or
`StageSceneOutput` failure. If commit begins, follow the #51 rule that the result
must record `mutated_state` once the first runtime mutation succeeds; a commit
failure is not a no-mutation pass unless a later rollback ledger is approved and
tested.

## Failure Status Matrix

Use `RuntimeAssetDataStatus` from the shared vocabulary:

| Failure | Required status |
| --- | --- |
| null request/output pointers or inconsistent caller spans | `InvalidArgument` |
| missing/malformed magic/header | `InvalidHeader` |
| unsupported version | `UnsupportedVersion` |
| missing/unknown kind | `InvalidKind` |
| missing/incompatible schema | `InvalidSchema` |
| malformed or contradictory table counts | `InvalidCount` |
| byte ranges outside file bounds | `InvalidSize` |
| binary table/payload misalignment | `InvalidAlignment` |
| invalid transform, projection, keyframe time/value, duration, or sample rate | `InvalidBounds` |
| invalid ref syntax, out-of-range table index, or unresolved scene-local index | `InvalidDependency` |
| required mesh/material/program/camera/animation ref absent | `MissingDependency` |
| duplicate unique id or duplicate required dependency | `DuplicateDependency` |
| dependency kind or animation target family mismatch | `TypeMismatch` |
| source/payload/dependency hash mismatch | `HashMismatch` |
| unsupported channel, interpolation, projection, transform order, or camera mode | `UnsupportedFieldValue` |
| caller/file caps exceeded | `CapacityExceeded` |
| configured memory/file/record budget exceeded | `BudgetExceeded` |

Integration failures after successful preflight keep their existing statuses
(`ResourceRegistrationFailed`, `AssetRegistrationFailed`,
`ResourceDependencyFailed`, `AssetDependencyFailed`, and related payload statuses)
but still must not commit caller scene outputs.

## RenderScene Consumption Contract

The later implementation must prove that RenderScene consumes loader records
directly:

- scene entities provide world object id, final sampled transform, mesh/material
  refs, visibility, active state, and deterministic order;
- camera records provide the active camera frame data;
- material/shader/texture refs are the records produced by the RuntimeAsset
  load path, not locally invented test constants;
- RenderScene frame construction may adapt RuntimeAsset records into
  RenderScene request structs, but that adapter must be production code and must
  accept N bounded entities, not a three-primitive-only helper;
- CPU PPM/oracle helpers may inspect capture output only after RenderCore/RHI
  produced the capture source.

## Hard Boundaries

RAV1-D does not authorize:

- editor scene authoring, import UI, Resource Browser UX, Preview Host UX, or
  hot reload;
- original TouhouNewWorld package parsing or compatibility;
- external DCC/FBX/glTF import policy;
- gameplay object spawning, script VM/native bridge, save-game policy, or
  general World scene persistence;
- Web, UI editor, Game Adapter, report/oracle ownership, screenshot/manual
  inspection, or GDI/software viewer acceptance.

The loader may use existing World/Animation value contracts for preflight, but
it does not make World own file parsing or make Animation own Resource/Asset
registration.

## Later Implementation Tests

The RAV1-D implementation slice should add or rename tests in this shape:

| Test | Required proof |
| --- | --- |
| `RuntimeAssetData_SceneAnimationLoaderLoadsBoundedNEntityScene` | loads more than the three-entity fixture and emits deterministic entity/transform/camera counts |
| `RuntimeAssetData_SceneAnimationLoaderRejectsEntityCapacityOverflowWithoutMutation` | entity or transform cap overflow leaves caller outputs, registries, and dependency edges unchanged |
| `RuntimeAssetData_SceneAnimationLoaderRejectsMissingTypedRefsWithoutMutation` | missing mesh/material/program/camera/animation refs fail before mutation |
| `RuntimeAssetData_SceneAnimationLoaderRejectsInvalidTransformsWithoutMutation` | NaN/out-of-range transform values fail before mutation |
| `RuntimeAssetData_SceneAnimationLoaderRejectsInvalidTrackRangesWithoutMutation` | clip/track/keyframe table ranges fail before mutation |
| `RuntimeAssetData_SceneAnimationLoaderRejectsInvalidKeyframesWithoutMutation` | non-monotonic, out-of-duration, or non-finite keyframes fail before mutation |
| `RuntimeAssetData_SceneAnimationLoaderRejectsAnimationTargetMismatchWithoutMutation` | track target that does not resolve to the staged scene entity fails before mutation |
| `RuntimeAssetData_SceneAnimationLoaderIsPathIndependent` | internal kind/schema/dependency metadata, not `.yu*` suffixes, selects scene/animation families |
| `RuntimeAssetData_RenderSceneConsumesBoundedSceneAnimationRecords` | RenderScene builds a frame from the loader output records and captures through RenderCore/RHI |
| `RuntimeAssetData_SceneAnimationLoaderRejectsHashMismatchBeforeMutation` | dependency or payload hash mismatch leaves registries and caller outputs unchanged |

Focused validation for the docs/API-plan task is docs-only:

```text
git diff --check -- docs/YUENGINE_RUNTIME_ASSET_V0_SCENE_ANIMATION_LOADER_PLAN.md
git diff --cached --check
git show --check --format=short HEAD
```

Implementation slices must also run the focused RuntimeAssetData tests and the
full `windows-fast-gate` after code changes.
