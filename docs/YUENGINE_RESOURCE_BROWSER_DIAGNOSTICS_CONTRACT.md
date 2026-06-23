# YuEngine Resource Browser Diagnostics Contract

Status: RAV2-B implementation contract
Requested: 2026-06-23
Owner: Resource Browser backend
Task: #64
Baseline: `origin/main@84143d9`

Related:

- `docs/YUENGINE_RESOURCE_BROWSER_IMPORT_COOK_DIAGNOSTICS_SCOPE.md`
- `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
- `docs/YUENGINE_RUNTIME_ASSET_V0_RAV1_IMPLEMENTATION_EVIDENCE.md`
- `docs/YUENGINE_RUNTIME_ASSET_V0_RAV2_EVIDENCE_MATRIX.md`

## Purpose

`YuResourceBrowser` is a backend projection layer for later Resource Browser UI
and Preview Host work. It does not render UI, mint runtime handles, import
original packages, or infer type truth from file suffixes.

The first contract is
`BuildResourceBrowserRuntimeAssetDiagnostics`. It reads listed RuntimeAsset
files through `MountTable`, validates bytes through `YuRuntimeAsset`, and may
observe `YuResource` and `YuAsset` snapshots or previously loaded
`RuntimeAssetLoadedFile` records. Resource and Asset state remain read-only for
this query.

## Records

`ResourceBrowserImportSettings` records the source path, target
`RuntimeAssetFileKind`, Resource/Asset type ids, stable id, importer version,
expected schema version, and expected source hash.

`ResourceBrowserResourceEntry` is the reusable row for Resource Browser UI and
Preview Host queries. It records the import settings, RuntimeAsset validation
result, dependency state, mounted file status, loaded Resource/Asset handles,
decoded payload ids, Resource load state, and provenance flags.

`ResourceBrowserDiagnosticRecord` is the bounded diagnostic output. It records
code, severity, phase, underlying RuntimeAsset/File/Resource status, source
path, expected kind, artifact class, schema/hash evidence, and a stable short
message.

## Diagnostic Codes

The backend currently normalizes lower statuses into these display-stable
codes:

| Code | Source status |
| --- | --- |
| `FileReadFailed` | `FileStatus` read failure through `MountTable` |
| `MissingDependency` | `RuntimeAssetDataStatus::MissingDependency` |
| `DuplicateDependency` | `RuntimeAssetDataStatus::DuplicateDependency` |
| `TypeMismatch` | `InvalidKind` or `TypeMismatch` |
| `StaleHash` | `HashMismatch` |
| `StaleSchema` | `InvalidSchema` |
| `Unsupported` | `UnsupportedVersion` or `UnsupportedFieldValue` |
| `CapacityExceeded` | output/runtime capacity failure |
| `BudgetExceeded` | `BudgetExceeded` |
| `ResourceQueryFailed` | Resource load-state query failure |
| `ValidateFailed` / `LoadFailed` | fallback wrappers for lower statuses |

## Mutation Boundary

The diagnostics query is a projection. It may read files via the mounted file
path to prove diagnostics came from actual File/VFS/RuntimeAsset input. It must
not mutate `YuResource`, `YuAsset`, RenderScene, RHI, package, or editor UI
state. Tests snapshot Resource and Asset before and after diagnostics to enforce
that boundary.

## Validation

Focused tests:

- `ResourceBrowserDiagnostics_EntriesComeFromRuntimeAssetFileResourceAssetPath`
- `ResourceBrowserDiagnostics_ClassifiesMissingTypeMismatchStaleSchemaAndHash`
- `ResourceBrowserDiagnostics_ClassifiesCapacityAndBudgetWithoutResourceAssetMutation`

These cover real mounted files, RuntimeAsset validation, loaded
Resource/Asset handles, missing dependency, type mismatch, stale schema,
stale hash, unsupported field, capacity failure, budget failure, and read-only
Resource/Asset query behavior.

## Native Surface Projection

Task #72 adds `BuildResourceBrowserNativeSurface` as the first read-only native
Resource Browser surface projection. It consumes only
`ResourceBrowserResourceEntry` and `ResourceBrowserDiagnosticRecord` outputs; it
does not read files, import assets, create runtime handles, render UI widgets,
or call Preview Host.

Task #79 keeps that boundary but adds the first native/editor-facing workflow
model through `ResolveResourceBrowserSurfaceSelection`. The workflow consumes
existing surface rows, entries, diagnostics, and a structured
`ResourceBrowserImportSettings` record. It produces a stable selection state
with selected row identity, Resource/Asset handles, import-setting validation,
diagnostic blockers, and preview eligibility; it does not infer target kind from
the locator path and does not mutate Resource, Asset, RuntimeAsset, Preview
Host, Scene, Animation, or UI state.

Each `ResourceBrowserSurfaceRow` exposes the fields the first native/editor
surface needs to display:

- locator path, kept as a locator only and never used as type truth;
- declared `RuntimeAssetFileKind` from import settings;
- header/runtime kind, artifact class, schema version, validation status, and
  hash payload metadata from `YuRuntimeAsset`;
- Resource/Asset handles and loaded-record provenance from the backend query;
- first highest-severity diagnostic code/phase and diagnostic count;
- preview eligibility and requested surface document kind.

Preview eligibility is intentionally conservative. A row is eligible only when
validation succeeds, dependencies are ready, a RuntimeAsset loaded record exists,
Resource/Asset records are visible, and the validated runtime kind is one of the
approved RuntimeAsset families. Blocking diagnostics gate otherwise eligible
rows before preview. Otherwise the row records the exact blocker: validation,
dependency, missing loaded record, missing Resource/Asset record, unsupported
kind, or diagnostic gate. This is Resource Browser surface eligibility only; it
does not claim final Preview Host behavior, Scene Editor workflow, package/run
closure, external importer support, or product completeness.

Selection validation is structured-record based. It requires source path, target
kind, Resource/Asset type ids, stable id, importer version, schema version, and
optional expected source hash consistency. If validation already succeeded for
the selected row, target kind is checked against the validated runtime header
kind, not the source-path suffix. Validation failures block preview while
preserving the selected Resource/Asset handle mapping.

Additional focused tests:

- `ResourceBrowserSurface_BuildsRowsWithStatusAndPreviewEligibility`
- `ResourceBrowserSurface_DoesNotUseLocatorSuffixAsTypeTruth`
- `ResourceBrowserSurface_RejectsSmallOutputWithoutPartialRows`
- `ResourceBrowserSurface_SelectionWorkflowAcceptsValidSelection`
- `ResourceBrowserSurface_SelectionWorkflowRejectsInvalidSetting`
- `ResourceBrowserSurface_DiagnosticBlocksPreviewEligibility`
- `ResourceBrowserSurface_SelectionValidationDoesNotMutateResourceAssetMapping`
