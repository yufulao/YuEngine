# YuEngine Resource Browser RAV7 Depth Gap Surface

Owner: Resource Browser implementation
Target: `YuResourceBrowser`
Status: implementation slice
Task: #97

## Purpose

This slice extends the RAV6 visible workflow with a bounded depth surface for
the next Resource Browser panel pass. It keeps the work as caller-owned data
records so native UI, importer, and asset-manager gaps are visible without
pretending those systems are complete.

The workflow consumes:

- `ResourceBrowserResourceEntry` rows produced from File/VFS,
  RuntimeAssetData, Resource, and Asset observations;
- `ResourceBrowserDiagnosticRecord` rows from the diagnostics query;
- a structured `ResourceBrowserImportSettings` record for the selected row.

It emits:

- catalog rows with source boundary, importer readiness, AssetManager gap, and
  preview request readiness;
- importer boundary rows for runtime-asset sources, original-package sources,
  and external-import sources;
- asset-manager gap rows that distinguish missing runtime load records,
  missing Resource registry records, missing Asset records, dependency blocks,
  and unsupported preview kinds;
- a selection ledger that records commit or rejection without mutating runtime
  state.

## Runtime Path

```text
ResourceBrowserRuntimeAssetDiagnostics
-> ResourceBrowserDepthWorkflowRequest
-> depth catalog rows + importer boundary rows + asset-manager gap rows
-> selected depth ledger
-> Preview Host-ready commit or explicit rejection
```

The surface does not read files, parse original game archives, run external DCC
importers, mint Resource/Asset handles, or write to AssetManager. It preserves
the lower provenance from diagnostics and validates the selected import setting
before writing any caller-owned outputs.

## Acceptance

The accepted behavior is:

- runtime-asset sources can produce importer-ready, AssetManager-ready, preview
  request-ready rows when diagnostics and handles are valid;
- original-package paths stay visible as explicit boundary rows and reject the
  selected preview request;
- external-import paths stay visible as explicit boundary rows and reject the
  selected preview request;
- missing runtime load, Resource registry, Asset record, dependency, and preview
  kind gaps are represented as AssetManager gap rows;
- invalid selected import settings reject before mutating caller-owned output
  buffers.

## Validation

```text
ctest --preset windows-fast-gate -R "ResourceBrowserDepthWorkflow_" --output-on-failure
ctest --preset windows-fast-gate -R "(ResourceBrowserDiagnostics_|ResourceBrowserSurface_|ResourceBrowserVisibleWorkflow_|ResourceBrowserDepthWorkflow_)" --output-on-failure
```

## Boundaries

This slice does not complete a full Resource Browser UI, original package
parser, external DCC importer, drag/drop import, AssetManager mutation, native
window launch, Web/canvas/browser preview, static screenshot oracle, CPU/GDI
fixture oracle, Preview Host rendering, Scene Editor, Animation Editor, UI
Editor, package/run validation, or final product visual closure.
