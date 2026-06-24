# YuEngine Resource Browser Import Cook Diagnostics Scope

Status: scope proposal / no implementation approval
Requested: 2026-06-23
Owner: Architecture
Task: #39
Baseline: `origin/main@540f986`

Related:

- `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
- `docs/YUENGINE_RESOURCE_BROWSER_DIAGNOSTICS_CONTRACT.md`
- `docs/YUENGINE_RESOURCE_BROWSER_RAV6_VISIBLE_WORKFLOW.md`
- `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
- `docs/YUENGINE_EDITOR_RUNTIME_PREVIEW_HOST_PLAN.md`
- `docs/gates/EDITOR_GATE_001_RUNTIME_PREVIEW_HOST.md`
- `docs/YUENGINE_SCENE_EDITOR_PLAN.md`
- `docs/YUENGINE_ANIMATION_EDITOR_PLAN.md`
- `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`

## 1. Purpose

Define the Resource Browser, import settings, cook command, and diagnostics
scope before Scene, Animation, or UI editor surface work claims production
asset workflow progress.

The Resource Browser is editor tooling over approved runtime data contracts. It
is not a new runtime asset database, not a replacement for `YuRuntimeAsset`,
`YuResource`, `YuAsset`, or `YuPackage`, and not a preview renderer.

Correct responsibility chain:

```text
source asset file
-> source metadata and import settings
-> validate/import/cook command
-> RuntimeAssetData / Resource / Asset / Package runtime records
-> Preview Host request
-> bounded diagnostics and editor display
```

If a lower runtime data contract is missing, the Resource Browser must report a
blocking diagnostic instead of inventing editor-only runtime data.

## 2. Current Ground Truth

Existing lower surfaces that the Resource Browser must respect:

- `YuRuntimeAsset` currently knows `RuntimeAssetFileKind` for mesh, material,
  texture, shader, scene, and animation, plus validation/cook/load statuses for
  the first runtime asset data smoke.
- `YuResource` owns resource identity, decode payload records, dependency
  edges, residency, and explicit failure status.
- `YuAsset` owns runtime asset handles, stable asset ids, asset type ids,
  dependency traversal, ready records, and Resource-state synchronization.
- `YuPackage` owns package manifest/load-plan records, not editor source
  folders or UI browsing.
- `EDITOR_GATE_001` owns future preview session/frame/diagnostic contracts, not
  asset catalog UX.

This task does not change any of those APIs. It defines the tooling layer that
will call them after the relevant gates are approved.

## 3. Owns

Resource Browser tooling owns:

- catalog projection for source assets, imported records, cooked outputs, and
  runtime-ready records;
- source asset identity display using path, kind, size/hash, importer version,
  and declared dependency summary;
- import settings records and editor-side defaults;
- validation command routing for selected assets or folders;
- cook command routing and summary display;
- diagnostic grouping, filtering, severity display, and fix-target metadata;
- request construction for preview-host resource preview after the Preview Host
  gate is approved;
- editor-only selection, folder expansion, filters, tags, search text, sort,
  and view state;
- artifact hygiene policy for generated import/cook/preview outputs.

Resource Browser may display runtime handles or ids returned by lower systems,
but it does not mint authoritative runtime handles.

## 3.1 Field Provenance

Every Resource Browser field must name its source. The browser is a projection
layer; it must not silently upgrade editor cache values into runtime truth.

| Field | Source | Notes |
| --- | --- | --- |
| source path | editor import service | Must be under an approved source root and pass path validation. |
| source byte size | editor import service | Observed from source file read, not runtime identity. |
| source hash | editor import service | Used for change detection and cook identity input. |
| source family | editor import settings or source detector | Advisory until validated by the relevant importer/runtime contract. |
| importer id/version | editor import settings | Editor-side selection; validation decides whether it is supported. |
| target runtime file kind | `RuntimeAssetFileKind` | Mesh/material/texture/shader/scene/animation names must map to RuntimeAssetData vocabulary. |
| runtime data validation status | `RuntimeAssetDataStatus` or later approved equivalent | Browser only displays/filters it. |
| runtime data hash/byte count | `RuntimeAssetValidationResult` or cook summary | Produced by RuntimeAssetData validation/cook route. |
| resource type/id | `YuResource` descriptor/handle returned by approved load/cook route | Browser must not mint authoritative Resource handles. |
| resource residency/decode state | `YuResource` snapshot or explicit query result | Display only; lifecycle stays in Resource. |
| asset stable id/type | `AssetDescriptor` / `AssetRecord` / `YuAsset` result | Browser may show stable id and type but does not allocate handles. |
| asset handle/load state | `YuAsset` query or snapshot | Display only; lifecycle stays in Asset. |
| package entry/load-plan row | `YuPackage` manifest/load-plan result | Only after package route is approved for the source family. |
| dependency edge | RuntimeAssetData, Resource, Asset, or Package dependency outputs | Must show owner lane; no implicit display-name lookup. |
| preview availability | Preview Host status | Only after #38/Preview Host gate exposes the request/result contract. |
| diagnostic severity/code | Resource Browser diagnostic record wrapping lower status | Browser can group and present, but source status remains lower owner. |
| selection/filter/sort/expanded state | editor-side cache | Never exported as runtime data. |

Editor-side cache fields are limited to view state, pending edits, unsaved import
settings drafts, recent validation summaries, and UI display preferences. They
must be invalidated or marked stale when source hash, importer version, runtime
contract version, or cook profile changes.

## 4. Does Not Own

Resource Browser tooling does not own:

- RuntimeAssetData file format, validator, cook/load semantics, or source-byte
  parsing;
- Resource or Asset lifecycle, residency, cache payloads, decoded payloads, or
  dependency graph mutation;
- Package manifest/load-plan ownership;
- shader compiler policy, material graph authoring, model importer internals,
  animation graph authoring, or audio decoder internals;
- Preview Host session lifecycle, frame buffers, viewport camera, hit/selection
  records, or engine-rendered preview output;
- Scene, Animation, or UI editor object/timeline/widget authoring workflows;
- shipped game runtime behavior;
- file watching, hot reload, background cook daemons, persistent asset database
  services, cloud/remote cook, or source-control integration;
- source-control suites, lock/check-out workflows, diff/merge tooling, review
  bots, marketplace, plugin ecosystem, or public package distribution;
- original TouhouNewWorld package compatibility, old editor/runtime flow,
  reports, oracle output, screenshots, or Game Adapter behavior;
- deprecated Web/editor compatibility paths.

## 5. Records

The first approved implementation must use bounded value records equivalent to
the following. Names may change, but the responsibilities must remain separate.

| Record | Purpose | Authority |
| --- | --- | --- |
| `ResourceBrowserItem` | editor display row for a source, imported, cooked, or runtime record | projection only |
| `SourceAssetRecord` | canonical source path, family, source hash/size, importer id/version, and declared dependency summary | editor/import tooling |
| `ImportSettingsRecord` | per-family import options and target runtime family | editor/import tooling |
| `ImportValidationRequest` | source record plus settings and output diagnostic capacity | local editor service |
| `ImportValidationResult` | status, changed state, dependencies, and bounded diagnostics | local editor service |
| `CookRequest` | validated source/settings plus target profile and output budget | local editor service |
| `CookSummaryRecord` | produced runtime file families, hashes, output paths, and skipped/failed rows | lower cook route |
| `ResourceDiagnosticRecord` | code, severity, source path, dependency path/id, owner, and fix target | diagnostics |
| `ResourcePreviewRequest` | runtime-ready resource/asset ids and preview document kind | Preview Host client |

Editor view state such as expanded folders, selected rows, search filters, and
sort order is sidecar state. It must never be exported as runtime data.

## 6. Import Settings Scope

Import settings are metadata that drives validation and cook. They are not
decoded payloads and do not bypass runtime validators.

Minimum common fields:

- source path relative to an approved source root;
- source hash and byte size observed by the local service;
- source asset family;
- importer id and importer version;
- target runtime file kind;
- stable runtime id policy;
- declared dependencies by typed reference;
- output profile name;
- diagnostic budget and output capacity.

Family-specific settings may include:

| Family | First settings |
| --- | --- |
| Mesh | scale, unit, axis/handedness, winding, vertex layout, index width, bounds policy |
| Texture | color space, format, mip policy, alpha policy, sampler preset |
| Material | shader/program reference, texture slot mapping, constants, render state preset |
| Shader/program | stage list, entry semantics, input layout, constant ranges, texture slot contract |
| Scene | scene id, object id policy, transform convention, dependency list, camera refs |
| Animation | clip range, sample rate, target binding policy, root motion policy or blocker |
| UI | layout/style/font/sprite refs and DPI/safe-area policy after UI runtime data gate |
| Audio | channel count, sample rate, loop policy, compression/import profile after audio import gate |

If a family-specific lower gate is missing, the browser may show the source
asset but must report `BlockedByMissingRuntimeContract` or an equivalent status.

## 7. Diagnostics Scope

Diagnostics must be explicit, bounded, and safe to display.

Required fields:

- stable diagnostic code;
- severity: info, warning, error, blocker;
- source path or runtime id;
- family and pipeline stage;
- dependency reference when applicable;
- owner lane: import, validate, cook, load, package, preview, or runtime;
- short message;
- optional fix target;
- status that caused the diagnostic;
- whether runtime output was mutated.

Required diagnostic categories:

- unsupported source family;
- missing importer;
- unsupported importer version;
- invalid source path or source root escape;
- source hash mismatch;
- invalid import setting;
- missing dependency;
- duplicate dependency;
- type mismatch;
- unsupported runtime data version;
- output capacity exceeded;
- cook output hash mismatch;
- package/load-plan unavailable;
- preview-host unavailable;
- missing runtime contract.

Diagnostics must not include raw file contents, credentials, environment
variables, provider tokens, or unbounded source snippets.

## 8. Cook And Artifact Policy

Cook outputs are generated artifacts. By default they belong under ignored
artifact/output directories, not tracked source.

Rules:

- source fixtures may be tracked only when tiny, deterministic, and approved by
  the relevant gate;
- generated outputs must record source hash, import settings hash, output hash,
  and runtime family;
- repeated cook of unchanged source/settings must produce identical summary
  hashes;
- failed validation or cook must not partially mutate lower runtime registries;
- successful cook may create lower records only through approved
  RuntimeAssetData/Resource/Asset/Package routes;
- the browser must display generated outputs as derived state, not as source of
  truth.

## 9. Dependencies And Ordering

This scope is downstream of the runtime asset/data contract and upstream of
usable editor surfaces.

Required ordering:

```text
#36 runtime asset contract v0 production-gap closure
-> #39 Resource Browser/import/cook/diagnostics scope
-> #38 Preview Host MVP gate consumes resource preview requests
-> #40 Scene/Animation/UI dependency chain can name exact browser inputs
-> later implementation gates for concrete editor surfaces
```

Task #37 UE editor audit may refine reference vocabulary, but it must not cause
YuEngine to copy Unreal AssetRegistry, Content Browser, Slate, or asset database
APIs directly.

## 10. No-Build-Yet List

Do not build any of the following from this task:

- Resource Browser UI;
- native editor app or immediate-mode editor shell;
- file watcher or hot reload loop;
- persistent asset database;
- background cook daemon;
- cloud or remote cook service;
- source-control integration suite;
- marketplace, plugin ecosystem, public package index, or external developer
  distribution workflow;
- shader compiler or model/material/animation importer;
- Scene, Animation, or UI editor surface;
- Preview Host implementation;
- RuntimeAssetData, Resource, Asset, Package, RenderScene, or RHI code changes;
- original-game package parser or compatibility layer;
- deprecated Web/frontend/browser/canvas route.

If a future task needs one of these, it needs a separate gate and explicit
approval.

## 11. Candidate Follow-Up Slices

Future work should split into parallelizable slices:

| ID | Slice | Output |
| --- | --- | --- |
| RB-001 | Source catalog and item projection | bounded records for source/cooked/runtime rows |
| RB-002 | Import settings schema | common fields plus mesh/texture/material first-family settings |
| RB-003 | Validation diagnostics model | bounded diagnostic records and no-mutation result shape |
| RB-004 | Cook summary contract | deterministic output summary, hashes, and artifact policy |
| RB-005 | RuntimeAssetData bridge | call approved runtime data validate/cook/load routes without changing them |
| RB-006 | Preview request bridge | build `ResourcePreviewRequest` only after Preview Host gate approval |
| RB-007 | UI surface implementation | display catalog/settings/diagnostics after RB-001..RB-006 are accepted |

RB-007 must not start before data records, diagnostics, and command routes are
accepted. A pretty browser without import/cook/diagnostics truth is not
production progress.

## 12. Review Questions

Reviewers should answer before implementation is opened:

1. Which RuntimeAssetData production gaps from #36 are blocking Resource Browser
   command routing?
2. Which source families are in the first Resource Browser slice: mesh,
   texture, material, shader/program, scene, animation, UI, or audio?
3. Which import settings are required for deterministic cook identity?
4. Which diagnostics are blockers versus warnings?
5. Which generated files may exist, and where are they ignored?
6. Which preview request fields must wait for #38?
7. Which Scene/Animation/UI editor dependencies should task #40 consume from
   this scope instead of redefining?

## 13. Exit Criteria

This scope is ready for review when:

- it is linked from RuntimeAssetData and Preview Host plans;
- ownership boundaries are explicit;
- records separate source metadata, import settings, validation, cook summary,
  diagnostics, and preview requests;
- Resource Browser is downstream of RuntimeAssetData and upstream of editor
  surfaces;
- no-build-yet list blocks implementation creep;
- follow-up slices can be opened independently after PM/final gate approval.
