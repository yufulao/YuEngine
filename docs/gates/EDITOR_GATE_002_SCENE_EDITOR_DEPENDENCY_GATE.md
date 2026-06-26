# EDITOR-GATE-002: Scene Editor Dependency Gate

Status: Proposed dependency gate for #YuPart task #73
Requested decision: `SCENE_EDITOR_DEPENDENCY_GATE_REVIEW`
Current decision: `SCENE_EDITOR_SURFACE_NOT_IMPLEMENTATION_APPROVED`
Owner: Architecture
Task: #YuPart task #73
Baseline: `origin/main@e1cbc11`

Related plans and inputs:

- `docs/YUENGINE_SCENE_EDITOR_PLAN.md`
- `docs/YUENGINE_EDITOR_DEPENDENCY_CHAIN_NO_BUILD_LIST.md`
- `docs/YUENGINE_EDITOR_RUNTIME_PREVIEW_HOST_PLAN.md`
- `docs/gates/EDITOR_GATE_001_RUNTIME_PREVIEW_HOST.md`
- `docs/YUENGINE_PREVIEW_HOST_RAV2_MVP.md`
- `docs/YUENGINE_PREVIEW_HOST_RAV3_COMMAND_OUTPUT_WIRING.md`
- `docs/YUENGINE_RUNTIME_ASSET_V0_RAV2_EVIDENCE_MATRIX.md`
- `docs/YUENGINE_RUNTIME_ASSET_V0_IMPORT_COOK_COMMAND_CONTRACT.md`
- `docs/YUENGINE_RUNTIME_ASSET_V0_SCENE_ANIMATION_LOADER_PLAN.md`
- `docs/YUENGINE_RESOURCE_BROWSER_DIAGNOSTICS_CONTRACT.md`
- `docs/YUENGINE_RESOURCE_BROWSER_IMPORT_COOK_DIAGNOSTICS_SCOPE.md`
- `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`

## Purpose

Define the dependency gate that must be satisfied before Scene Editor
implementation is opened.

The gate consumes the accepted RAV2/RAV3 RuntimeAsset and Preview Host facts and
turns them into a task-ready matrix. It does not create a native Scene Editor,
local editor service, Resource Browser UI, viewport, hierarchy panel, inspector,
transform gizmo, command stack, package parser, Unity/Unreal importer, or final
product visual closure.

The first separately taskable slice named by this gate is:

```text
SE-DATA-001: Scene document records plus editor-only sidecar split
```

That slice is a lower scene-data contract/validator task. It is not a native
Scene Editor surface. A human/PM gate still must create and approve a separate
task before implementation begins.

## Accepted Inputs

The following inputs are accepted for this gate:

| Input | Status | What this gate may use |
| --- | --- | --- |
| RAV2-A import/cook command | PASS | `ExecuteRuntimeAssetImportCookCommand`, deterministic source and cooked disk outputs, missing-layer diagnostics |
| RAV2-B Resource Browser backend diagnostics | PASS | read-only RuntimeAsset/File/Resource/Asset projection records and bounded diagnostics |
| RAV2-C Preview Host MVP | PASS | `YuPreviewHost` session/frame values, RuntimeAsset graph consumption, diagnostics, hit/selection/transform feedback |
| RAV2-D cooked visual route | PASS | cooked RuntimeAsset structural route through RuntimeAssetData, RenderScene, RenderCore, and RHI capture |
| RAV3-B Preview Host command-output wiring | PASS | `YuPreviewHost` can require and validate #63 cooked command outputs before frame/capture feedback |

Accepted input does not mean editor completion. The rows above are lower runtime
and preview dependencies only.

## Source Of Truth Split

Scene Editor must separate three data classes:

| Data class | Owner | Runtime export allowed |
| --- | --- | --- |
| authoring scene document | Scene data contract | Yes, after validation and cook |
| cooked RuntimeAsset records | RuntimeAsset import/cook/load route | Yes, this is runtime input |
| editor-only sidecar | Scene Editor tooling | No, except as editor settings in a separate editor-owned file |

Authoring scene documents may contain schema version, scene id, object ids,
transform records, component attachment records, component-resource binding
records, camera references, and dependency declarations. They must validate into
RuntimeAsset source/cooked records before preview or runtime use.

Editor-only sidecar state includes selection, foldouts, viewport camera
bookmarks, grid/snap settings, gizmo mode, panel layout, undo selection cursor,
and display filters. Sidecar state may drive editor workflow, but it must not be
loaded as shipped runtime scene data and must not be required by the runtime
loader.

## Conversion And Cook Boundary

The required boundary is:

```text
authoring scene document
+ editor-only sidecar
-> validate authoring document and sidecar separation
-> validate resource refs through Resource Browser/backend diagnostics
-> import/cook command writes deterministic source/cooked RuntimeAsset outputs
-> LoadRuntimeAssetDataGraph reads cooked scene/files
-> PreviewHost validates command output and loaded graph
-> RenderScene / RenderCore / RHI frame or bounded blocker
```

Only the authoring scene document participates in runtime validation and cook.
The sidecar may be saved for editor convenience, but it must not change cooked
runtime identity, Resource/Asset dependency truth, RenderScene records, or
Preview Host status.

## Dependency Matrix

| Dependency | Required before native Scene Editor surface? | Status | Evidence or blocker |
| --- | --- | --- | --- |
| RuntimeAsset graph/load outputs | Yes | PASS | RAV2/RAV3 load/cook evidence plus #70 command-output Preview Host wiring |
| Import/cook command outputs | Yes | PASS | #63 command result and #70 `PreviewHostCommandOutputRef` validation |
| Preview Host session/frame behavior | Yes | PASS | #65 and #70 provide bounded session/frame/diagnostic/capture behavior |
| Preview Host selection/hit/transform feedback | Yes | PASS | #65 feedback records remain intact through #70 command-output validation |
| Resource Browser backend diagnostics | Yes | PASS | #64 read-only diagnostics contract and bounded display-stable codes |
| Resource Browser UI and import settings panel | For usable surface, yes | BLOCKED | Scope exists, but no UI/import-settings implementation is approved |
| Runtime scene entity/transform/camera loader outputs | Yes | PASS | RAV1/RAV2 RuntimeAsset scene output floors are accepted for the current canonical route |
| Bounded scene/animation record contract beyond fixed proof | For broader editing, yes | BLOCKED | Plan exists; first editor slice must not assume a general scene graph or arbitrary authoring parser |
| Transform/animation sampling feedback | Yes | PASS | Existing RuntimeAsset/Animation floors support sampled transform proof for the accepted canonical route |
| Viewport camera/orbit capture | Yes | PASS | Preview Host camera/capture route exists for the accepted RuntimeAsset preview path |
| Native/engine UI surface for editor panels | Yes for surface | BLOCKED | UI runtime evidence exists, but no Scene Editor native surface target is approved |
| Scene hierarchy, inspector, gizmo, drag/drop, undo/redo | Yes for surface | BLOCKED | Editor workflow implementation is not approved by this gate |
| Cook/package/run smoke for authored scenes | Before shippable editor claim | BLOCKED | Not required for the first scene-data slice, required before shippable editor data |
| Rejected editor route, rejected canvas, rejected form layout, screenshots | No | NOT-REQUIRED-NOW | Explicitly rejected as authoritative preview or editor progress |
| Original package parser | No | NOT-REQUIRED-NOW | Later evidence/package gate only; not a first native scene prerequisite |
| Unity/Unreal importer | No | NOT-REQUIRED-NOW | Reference separation only; no copied importer/API shape |
| Final device-backed product visual closure | No | NOT-REQUIRED-NOW | Useful later, not a prerequisite for the first native scene gate |

## First Task-Ready Slice

This gate names one narrow first slice that can be separately tasked after PM
approval:

```text
SE-DATA-001: Scene document records plus editor-only sidecar split
```

Required acceptance for that slice:

- define scene document header/version/id/hash records;
- define ordered object identity records;
- define transform records mapped to existing transform/value vocabulary;
- define component attachment and component-resource binding records only where
  lower gates already provide ownership;
- define editor-only sidecar records and prove they do not export as runtime
  scene data;
- validate duplicate ids, missing refs, unsupported fields, capacity overflow,
  and malformed sidecar/runtime mixing without mutation;
- provide a cook-ready dependency list for RuntimeAsset import/cook, or return
  an explicit lower-layer blocker;
- add focused tests or docs, depending on the implementation approval, that
  prove runtime data remains separate from editor-only sidecar state.

`SE-DATA-001` must not include:

- native Scene Editor app or editor shell;
- hierarchy, inspector, viewport, transform gizmo, drag/drop, or undo/redo UI;
- Resource Browser UI;
- Preview Host implementation changes unless a separate gate names a bridge;
- original package parser, Unity/Unreal importer, or game-specific scene flow;
- rejected editor route/canvas/static screenshot fallback.

## `SE-DATA-001` Implementation Anchor

The first implementation slice lands as the lower `YuWorld`
`WorldSceneAuthoringDocument` contract and validator. It defines:

- scene authoring header records with schema version, scene document id,
  deterministic document hash, runtime-family counts, dependency count, sidecar
  count, and explicit unsupported-runtime-field rejection;
- runtime export records for ordered object identity, transform, component
  attachment, component-resource binding, and cook-ready dependency records;
- editor-only sidecar records for selection, foldout, viewport camera bookmark,
  grid/snap, gizmo, panel layout, display filter, and undo selection cursor;
- validation that rejects duplicate object ids/handles, duplicate transforms,
  duplicate component/resource/dependency/sidecar tuples, missing transform or
  component references, missing cook dependencies, capacity overflow,
  unsupported runtime fields, invalid transform values, invalid resource
  records, sidecar references to missing objects, and sidecar records marked for
  runtime export.

Successful validation copies only runtime record families and dependency records
to caller-owned outputs. Editor-only sidecar records are counted and validated
but are not exported into runtime outputs, RuntimeAsset input, Preview Host
state, or RenderScene data. Failed validation does not mutate caller-owned
runtime output records or output counts.

This implementation still does not create a native Scene Editor surface,
hierarchy, inspector, viewport, transform gizmo, Resource Browser UI, Preview
Host bridge, original package parser, Unity/Unreal importer, rejected editor route surface, or
final product visual closure.

## Blocked Later Slices

The following slices remain blocked until their prerequisites close:

| Slice | Status | Blocker |
| --- | --- | --- |
| `SE-PREVIEW-001` Scene document to Preview Host bridge | BLOCKED | Needs `SE-DATA-001` output and Resource Browser preview request bridge |
| `SE-UI-001` Native Scene Editor surface | BLOCKED | Needs scene document contract, preview bridge, Resource Browser UI/import settings, and native editor panel approval |
| `SE-GIZMO-001` Transform gizmo command stack | BLOCKED | Needs native surface, scene document mutation command model, and preview feedback bridge |
| `SE-PACKAGE-001` Authored scene package/run smoke | BLOCKED | Needs authored scene cook/load path and package/run validation task |

## Task Creation Rules

Any future Scene Editor task must state:

- which gate row authorizes it;
- source of truth: authoring scene document, cooked RuntimeAsset record, or
  editor-only sidecar;
- dependency direction and lower systems used;
- exact records owned and exact records read only;
- bounded capacities and failure statuses;
- no-mutation behavior for invalid data and small outputs;
- validation/cook/load/preview evidence route;
- focused tests and off-scope scans;
- why the task does not revive rejected editor route, screenshots, original package parsing,
  Unity/Unreal importer shape, or product-layer scene flow.

If the task cannot state these items, it remains architecture or review work.

## Hard Blocks

The following are blocking violations:

- treating #70 Preview Host command-output wiring as Scene Editor approval;
- using editor-only sidecar state as runtime scene data;
- bypassing RuntimeAsset import/cook/load with editor in-memory scene objects;
- claiming Resource Browser backend diagnostics are Resource Browser UI;
- claiming a native Scene Editor is usable before resource selection,
  diagnostics, camera/viewport output, and transform feedback are integrated;
- accepting rejected editor route, 2D canvas, rejected form layout, CPU/GDI output, manual screenshots,
  report/oracle output, or standalone samples as authoritative scene preview;
- making RuntimeAsset, Resource, RenderScene, RenderCore, RHI, or World depend
  on editor-only code;
- copying Unity Scene/GameObject/Component or Unreal World/Actor/Component API
  shape as YuEngine runtime truth;
- using original TouhouNewWorld package parsing or product scene flow as first
  Scene Editor acceptance.

## Verification

This task is documentation-only. Required validation:

```powershell
git diff --check -- docs/gates/EDITOR_GATE_002_SCENE_EDITOR_DEPENDENCY_GATE.md docs/YUENGINE_SCENE_EDITOR_PLAN.md docs/YUENGINE_EDITOR_DEPENDENCY_CHAIN_NO_BUILD_LIST.md docs/YUENGINE_EDITOR_RUNTIME_PREVIEW_HOST_PLAN.md
git diff --cached --check
git show --check --format=short HEAD
```

No configure, build, CTest, preview-host execution, or runtime sample command is
required unless an implementation change is added later.

## Exit Criteria

This gate is ready for review when:

- it consumes RAV2/RAV3 accepted inputs without overclaiming editor completion;
- dependency rows use only `PASS`, `BLOCKED`, or `NOT-REQUIRED-NOW`;
- editable authoring scene data is separated from cooked RuntimeAsset records
  and editor-only sidecar state;
- the conversion/cook boundary is explicit;
- the only first task-ready slice is the narrow `SE-DATA-001` scene document
  and sidecar split;
- native Scene Editor surface, Resource Browser UI, original package parsing,
  Unity/Unreal importers, rejected editor route editor, CPU/GDI/manual screenshot proof, and final
  device-backed product closure remain outside this task.
