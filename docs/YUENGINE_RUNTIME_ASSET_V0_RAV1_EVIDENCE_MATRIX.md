# YuEngine RuntimeAsset v0 RAV1 Evidence Matrix

Status: RAV1-E Phase A docs/evidence matrix
Owner: Architecture
Task: #54
Review gate: task #55
RAV0 review anchor: `4b846aa Preflight RuntimeAsset scene loader before registration`
RAV1-A contract anchor: `3cb54aa Amend RuntimeAsset RAV1 exit wording`
RAV1-B transaction anchor: `f21750f Add RuntimeAsset loader transaction plan`
RAV1-C payload bridge anchor: `bcee5de Amend RuntimeAsset payload transaction wording`
RAV1-D scene/animation anchor: `e3a79e9 Define RuntimeAsset scene animation loader plan`
Related gate: `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`

## Purpose

This matrix gives the RAV1 review gate one canonical evidence packet before any
production RuntimeAsset v0 implementation slice is authorized.

It is evidence and acceptance planning only. It does not approve runtime code,
binary parser work, cook pipeline work, editor/import bridge work, original game
package parsing, or upper-layer Preview Host / Resource Browser / editor work.

The package review question is narrow:

```text
Do RAV1-A through RAV1-D define enough contract, transaction, payload, and
scene/animation evidence for a later implementation slice to start?
```

The answer remains separate from implementation closure:

```text
RAV1_DOCS_GATE_NOT_IMPLEMENTATION_APPROVED
```

## Phase A Artifact Matrix

| Task | Anchor | Artifact | Evidence value | Gate risk to check |
| --- | --- | --- | --- | --- |
| #50 RAV1-A production contract | `3cb54aa` | `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`, `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`, `docs/YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md` | Defines source/cooked artifact classes, suffix-free identity, family rows, consumed validator vocabulary, status vocabulary, hard blocks, and implementation split wording. | Must not imply the first implementation slice is already approved; exit criteria must keep `RAV1_DOCS_GATE_NOT_IMPLEMENTATION_APPROVED`. |
| #51 RAV1-B transaction API | `f21750f` | `docs/YUENGINE_RUNTIME_ASSET_V0_LOADER_TRANSACTION_PLAN.md` | Defines suffix-free loader phases, request/plan/commit/result shape, preflight/no-mutation rules, and `mutated_state` semantics for commit failures. | Must not promise no-mutation after runtime mutation has started unless rollback is explicitly approved and tested. |
| #52 RAV1-C payload bridge | `bcee5de` | `docs/YUENGINE_RUNTIME_ASSET_V0_PAYLOAD_BRIDGE_RHI_ROUTE_PLAN.md` | Defines cooked texture/material/shader/program payload ownership, Resource decoded payload route, RHI texture/module/pipeline creation, cleanup ledger, and later focused tests. | Must keep RAV0 #44/#45 as smoke floors, not production cooked payload closure. |
| #53 RAV1-D scene/animation loader | `e3a79e9` | `docs/YUENGINE_RUNTIME_ASSET_V0_SCENE_ANIMATION_LOADER_PLAN.md` | Defines bounded scene/entity/transform/camera/animation records, caps, failure status matrix, transaction phase alignment, deterministic output order, and later tests. | Must not expand into an authoring scene parser, editor/import bridge, gameplay object factory, or World persistence format. |
| #54 RAV1-E evidence matrix | current doc | `docs/YUENGINE_RUNTIME_ASSET_V0_RAV1_EVIDENCE_MATRIX.md` | Collects review anchors, reusable RAV0 proof, smoke-only boundaries, missing production proof, exact commands, and off-scope checks. | Must remain evidence-only and avoid opening implementation by itself. |
| #55 RAV1 review gate | pending | thread/task review | Reviews the package above for PASS/AMEND before implementation task creation. | Must not approve code work unless this package is accepted and follow-up implementation tasks are explicitly split. |

## Reusable RAV0 Proof

RAV0 at `4b846aa` is reusable as evidence for these floors:

| Area | Reusable proof | Required carry-forward |
| --- | --- | --- |
| RuntimeAsset graph path | Generated source/smoke files are read through `MountTable` / File routes, then Resource/Asset records are registered. | Later production implementation must still read bytes through approved File/VFS/Resource paths, not direct test structs. |
| Status vocabulary | `RuntimeAssetDataStatus` includes common validator, dependency, capacity, input-layout, and RHI bridge statuses. | Later validators must map common failures to this vocabulary before lower integration statuses. |
| Suffix-free family identity | AMEND-B removed `.yu*` path load assumptions and requires internal schema/kind/id for all known families. | Later production source/cooked artifacts must keep suffixes as locators only. |
| Typed family validators | Mesh/material/texture validators and shader/program/scene/animation dependency validators reject malformed or mismatched records. | Later validators must operate on bounded source/cooked records and carry no-mutation probes. |
| Texture material slot floor | Decoded texture payloads can drive RenderScene material texture slots and reject failure without material/frame mutation. | Later bridge must replace smoke decoded payload assumptions with cooked payload layout, hashes, row pitch, and RHI upload ownership. |
| Shader/program RHI floor | Loaded shader/program data can create RHI shader modules and a pipeline, with cleanup on invalid data paths. | Later bridge must use cooked bytecode payload records and minimal reflection, not `bytecode:` source fixture text. |
| Scene loader staging | Scene and animation bytes are preflighted before Resource/Asset registration, and caller outputs commit after success. | Later loader must generalize to bounded N records while preserving preflight and commit semantics. |
| Animation sampling floor | Disk animation sampling feeds scene transforms through the runtime sampler and World transform bridge. | Later plan must validate clip/track/keyframe/binding tables and target resolution before mutation. |
| Upper-layer exclusion | Tests/docs exclude editor, Web, UI, input, Game Adapter, GDI viewer, screenshots, and CPU helper images as final proof. | Later implementation reviews must keep these as blockers or auxiliary evidence only. |

## Smoke-Only Boundaries

The following RAV0 facts are useful but not production closure:

| Area | Smoke-only shape | Required RAV1 replacement |
| --- | --- | --- |
| Source text fixtures | `rav0-source` text records and `.yu*` smoke names. | Source artifact contract with internal magic/version/kind/schema/id/hash and no suffix type truth. |
| Cooked payloads | Mesh/material/texture decoded payload records are deterministic smoke records. | Cooked artifact tables with byte ranges, alignments, hashes, row/slice pitch, payload identity, and budget checks. |
| Texture bridge | Fixed 2D `Rgba8Unorm` assumptions and test-owned slot bridge floors. | Production cooked texture records and material slot tables feeding ResourceDecodedTextureBridge or a production successor. |
| Shader bytecode | `bytecode:` source fixture text is loaded as bytes. | Cooked shader stage payload records with bytecode format/profile, hashes, entry point, input layout, and reflection. |
| Scene loader | Fixed cube/cylinder/cone success fixture and narrow output sizes. | Bounded scene/entity/transform/camera/animation tables with declared caps and deterministic output ordering. |
| Animation | One clip/track/two-keyframe smoke shape. | Bounded clip/track/keyframe/binding tables and target resolution against staged scene entities. |
| Render evidence | Current closed-loop tests prove RHI path for the smoke scene. | Later implementation must reprove RHI capture from production source/cooked RuntimeAsset records. |

## Missing Production Proof

These items must stay open until a later implementation wave proves them:

| Missing proof | Required implementation evidence |
| --- | --- |
| Production source parser/normalizer | Reads source artifacts with internal metadata, validates family records, hashes, dependencies, bounds, and coordinates. |
| Cooked artifact parser | Reads binary or approved cooked artifacts with table directories, payload ranges, alignments, hashes, and budgets. |
| Loader transaction implementation | Implements #51 phases with preflight, deterministic commit order, `mutated_state`, and approved rollback/cleanup semantics. |
| Cooked texture/material bridge | Converts cooked texture payloads and material slot tables into decoded payloads, RHI textures, and RenderScene material records without partial published outputs. |
| Cooked shader/program bridge | Creates RHI shader modules and pipelines from cooked bytecode payload records and reflection, with transient-handle cleanup on failure. |
| Bounded scene/animation loader | Loads more than the fixed three-entity fixture, validates scene/animation tables, samples animation, and emits deterministic RenderScene-consumable records. |
| Hardware/display evidence | D3D11/display capture may remain `BlockedByEnv` only for real environment constraints; fast-gate proof must still use the approved RHI/RenderCore route. |
| Import/editor bridge | Explicitly out of scope until RuntimeAsset v0 production cook/load/render proof is accepted. |

## RAV1 Package Review Commands

Use these commands for task #55 package review. They are docs/package commands,
not implementation closure commands:

```powershell
git fetch origin
git checkout --detach origin/main
git log --oneline --decorate -8
git show --check --format=short HEAD
git diff --check 4b846aa..HEAD -- docs\YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md docs\gates\L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md docs\YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md docs\YUENGINE_RUNTIME_ASSET_V0_LOADER_TRANSACTION_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_PAYLOAD_BRIDGE_RHI_ROUTE_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_SCENE_ANIMATION_LOADER_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_RAV1_EVIDENCE_MATRIX.md
```

Package review must inspect these exact anchors:

```text
RAV0: 4b846aa Preflight RuntimeAsset scene loader before registration
RAV1-A: 3cb54aa Amend RuntimeAsset RAV1 exit wording
RAV1-B: f21750f Add RuntimeAsset loader transaction plan
RAV1-C: bcee5de Amend RuntimeAsset payload transaction wording
RAV1-D: e3a79e9 Define RuntimeAsset scene animation loader plan
RAV1-E: docs/YUENGINE_RUNTIME_ASSET_V0_RAV1_EVIDENCE_MATRIX.md at review HEAD
```

## Off-Scope Review Scans

The package must not authorize upper-layer or bypass proof. Run these scans over
the RuntimeAsset RAV1 docs touched by Phase A:

```powershell
rg -n "editor|Editor|Web|UI|input|Game Adapter|original package|TouhouNewWorld package|GDI|screenshot|manual inspection" docs\YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md docs\gates\L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md docs\YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md docs\YUENGINE_RUNTIME_ASSET_V0_LOADER_TRANSACTION_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_PAYLOAD_BRIDGE_RHI_ROUTE_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_SCENE_ANIMATION_LOADER_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_RAV1_EVIDENCE_MATRIX.md
```

Matches are allowed only when they define hard exclusions, blockers, or
auxiliary evidence that cannot satisfy the gate.

Run this suffix/type-truth scan:

```powershell
rg -n "\.yu(mesh|mat|tex|program|scene|anim)|suffix|fixture name|type truth|internal metadata" docs\YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md docs\gates\L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md docs\YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md docs\YUENGINE_RUNTIME_ASSET_V0_LOADER_TRANSACTION_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_PAYLOAD_BRIDGE_RHI_ROUTE_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_SCENE_ANIMATION_LOADER_PLAN.md docs\YUENGINE_RUNTIME_ASSET_V0_RAV1_EVIDENCE_MATRIX.md
```

Allowed meaning: `.yu*` names are smoke locators only, while internal
magic/version/kind/schema/id/hash/dependency metadata is authoritative.

## Later Implementation Review Commands

When #55 approves a later implementation split, each code slice must run an
implementation command set that includes at least:

```powershell
git diff --check
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
```

Focused RuntimeAssetData tests must cover the relevant slice. Candidate proof
names from #51 through #53 include:

```text
RuntimeAssetData_LoaderRejectsMissingSchemaBeforeMutation
RuntimeAssetData_LoaderRejectsWrongKindBeforeMutation
RuntimeAssetData_LoaderRejectsHashMismatchBeforeMutation
RuntimeAssetData_LoaderIgnoresMisleadingSuffix
RuntimeAssetData_CookedTexturePayloadTableValidatesLayoutHashAndRowPitch
RuntimeAssetData_CookedMaterialTextureSlotTableResolvesLoadedPayloads
RuntimeAssetData_CookedShaderStagePayloadsCreateRhiModules
RuntimeAssetData_CookedProgramPipelineUsesLoadedReflectionAndInputLayout
RuntimeAssetData_CookedRhiPartialCreationFailureDestroysTransientHandles
RuntimeAssetData_SceneAnimationLoaderLoadsBoundedNEntityScene
RuntimeAssetData_SceneAnimationLoaderRejectsEntityCapacityOverflowWithoutMutation
RuntimeAssetData_SceneAnimationLoaderRejectsInvalidKeyframesWithoutMutation
RuntimeAssetData_RenderSceneConsumesBoundedSceneAnimationRecords
```

The exact names may change in implementation, but the proof shape may not.

## Review Gate Decision Rules

Task #55 may return PASS only if:

- #50 through #54 artifacts are present and linked;
- `git show --check` and `git diff --check` pass for the package;
- RAV0 reusable proof, smoke-only boundaries, and missing production proof are
  separated explicitly;
- #51 transaction semantics are preserved by #52 and #53;
- `.yu*` suffixes remain smoke locators only;
- CPU PPM, GDI/software viewers, screenshots, reports, editor/UI/input, Game
  Adapter, original package parsing, and direct C++ struct injection are not
  accepted as production RuntimeAsset proof;
- the decision remains `RAV1_DOCS_GATE_NOT_IMPLEMENTATION_APPROVED` unless a
  separate implementation task is created after review.

Task #55 must return AMEND if any RAV1 doc implies production implementation is
already authorized, if any route relies on filename suffixes as type truth, or
if commit/no-mutation semantics conflict with the #51 transaction plan.
