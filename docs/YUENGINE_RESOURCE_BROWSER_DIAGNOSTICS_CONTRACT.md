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
