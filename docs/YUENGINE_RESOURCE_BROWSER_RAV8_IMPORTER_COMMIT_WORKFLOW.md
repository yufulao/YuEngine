# YuEngine Resource Browser RAV8 Importer Commit Workflow

Owner: Resource Browser implementation
Target: `YuResourceBrowser`
Status: implementation slice
Task: #104

## Purpose

This slice moves Resource Browser beyond RAV7 boundary rows into a first
engine-owned importer commit path. It still exposes caller-owned rows for the
panel, but the accepted path now performs real `ResourceRegistry` and
`AssetManager` mutation through the existing RuntimeAsset graph loader after
preflight.

The workflow consumes:

- RuntimeAsset import/cook command status plus caller-owned RuntimeAsset file
  descriptors;
- one selected `ResourceBrowserImportSettings` record;
- optional RAV7 external-authoring manifest-ready rows;
- caller-owned output buffers for diagnostics, loaded RuntimeAsset records,
  catalog rows, importer rows, AssetManager gap rows, and a selection ledger.

## Runtime Path

```text
RuntimeAsset import/cook descriptors
-> Resource Browser selected import preflight
-> external manifest-ready row check, when selected source is external
-> RuntimeAsset graph load
-> ResourceRegistry + AssetManager commit
-> post-commit Resource Browser rows and selection ledger
```

Preflight runs before mutation. Original-package sources, unsupported external
imports, missing payloads, invalid dependencies, unsupported preview kinds, and
invalid selected settings reject without mutating Resource or Asset state.

## Acceptance

The accepted behavior is:

- manifest-ready external rows can map to cooked RuntimeAsset descriptors and
  commit through the RuntimeAsset graph load path;
- Resource and Asset records are committed only after selected import settings,
  dependency state, payload availability, and preview readiness are valid;
- original-package rows remain blocked and do not parse game packages;
- unsupported external rows remain blocked and do not require external SDKs;
- caller-owned rows describe catalog, importer, AssetManager gap, and selection
  ledger outcomes for both commit and rejection paths.

## Validation

```text
ctest --preset windows-fast-gate -R "ResourceBrowserImporterCommitWorkflow_" --output-on-failure
ctest --preset windows-fast-gate -R "(ResourceBrowserDiagnostics_|ResourceBrowserSurface_|ResourceBrowserVisibleWorkflow_|ResourceBrowserDepthWorkflow_|ResourceBrowserImporterCommitWorkflow_)" --output-on-failure
```

## Boundaries

This slice does not implement an original package parser, broad DCC importer,
drag/drop UI, native window launch, rejected editor route preview, external Unity/Unreal/DCC SDK
dependency, final visual proof, or product completeness claim.
