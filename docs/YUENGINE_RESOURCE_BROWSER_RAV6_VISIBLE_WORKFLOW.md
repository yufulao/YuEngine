# YuEngine Resource Browser RAV6 Visible Workflow

Owner: Resource Browser implementation
Target: `YuResourceBrowser`
Status: implementation slice
Task: #90

## Purpose

This slice turns the existing Resource Browser diagnostics and selection
surface into a visible workflow projection for later native editor panels and
Preview Host handoff. It is still a bounded data surface, not an asset manager
or editor shell.

The workflow consumes:

- `ResourceBrowserResourceEntry` records produced from File/VFS,
  RuntimeAssetData, Resource, and Asset observations;
- `ResourceBrowserDiagnosticRecord` rows from the Resource Browser diagnostics
  query;
- a structured `ResourceBrowserImportSettings` record for the selected row.

It emits caller-owned:

- import setting rows for visible display;
- diagnostic rows for visible display;
- preview eligibility rows through the existing native surface row contract;
- a selection ledger saying whether the selection can be handed to Preview
  Host or was rejected.

## Runtime Path

```text
ResourceBrowserRuntimeAssetDiagnostics
-> ResourceBrowserVisibleWorkflowRequest
-> import setting rows + diagnostic rows + preview rows
-> ResourceBrowserSurfaceSelectionState
-> Preview Host-ready selection ledger or explicit rejection
```

The workflow does not read files itself and does not mint Resource or Asset
handles. It preserves the lower provenance from the diagnostics input and uses
the existing structured selection validation. Path suffixes remain locators,
not type truth.

## Acceptance

The accepted behavior is:

- visible import setting rows expose selected state, target kind, schema,
  stable id, source hash, Resource/Asset type ids, and setting validation;
- visible diagnostic rows expose code, severity, phase, source path, stable id,
  and whether they block preview;
- preview rows keep the existing conservative eligibility model;
- a valid selection emits a Preview Host-ready selection ledger;
- a blocking diagnostic rejects preview without creating fake eligibility;
- invalid import settings reject before writing any caller-owned outputs.

## Validation

```text
ctest --preset windows-fast-gate -R "ResourceBrowserVisibleWorkflow_" --output-on-failure
ctest --preset windows-fast-gate -R "(ResourceBrowserSurface_|ResourceBrowserVisibleWorkflow_|ResourceBrowserDiagnostics_)" --output-on-failure
```

## Boundaries

This slice does not complete a full Resource Browser UI, asset manager,
original package importer, drag/drop DCC import, Unity/UE bridge, rejected editor route UI,
native editor shell, Preview Host rendering, Scene Editor, Animation Editor,
UI Editor, package/run validation, or final product visual closure.
