# YuEngine UI Editor RAV8 Style Theme Template Serialization Workflow

Owner: UI Editor implementation
Target: `YuUiEditor`
Status: implementation slice
Task: #107

## Purpose

This slice extends the RAV7 selected-node runtime preview workflow into a
bounded style/theme/template authoring and runtime serialization path.

It consumes:

- a valid `UiEditorRuntimeDocument`;
- a selected runtime UI node id;
- a successful `PreviewHostFrameResult`;
- the RAV6 design command path;
- caller-owned style/theme/template/state records;
- a bounded style/theme/template/state command for the selected node;
- a YuFile `MountTable`, mount id, and virtual path for save/load.

It emits caller-owned:

- hierarchy rows;
- design surface rows;
- inspector rows;
- Preview Host feedback records;
- staged runtime node records;
- the RAV6 design command ledger;
- one runtime preview style/theme/template/state row;
- one style/theme/template/state command ledger record;
- one serialization provenance row preserving RuntimeAsset and UI runtime
  boundary values.

## Runtime Path

```text
UiEditorRuntimeDocument
-> UiCore UiNodeTree rect resolution
-> PreviewHostFrameResult feedback
-> RAV7 selected-node runtime preview row
-> style/theme/template/state command ledger
-> YuSerialize value stream
-> YuFile MountTable write
-> YuFile MountTable read
-> YuSerialize value stream read
-> RuntimeAsset/UI runtime provenance row
```

All editor outputs are staged locally. Caller-owned output spans are copied only
after the runtime preview workflow, serialization, YuFile write/read, deserialize,
and provenance checks pass.

## Blocked Layers

`BuildUiEditorStyleThemeTemplateSerializationWorkflow` reports the first blocking
layer:

| Layer | Meaning |
| --- | --- |
| `RuntimeUiDocument` | runtime UI document/header/node validation failed |
| `UiCoreNodeTree` | UiCore node tree creation or rect query failed |
| `PreviewHostFeedback` | required Preview Host frame feedback was missing or unusable |
| `DesignWorkflow` | selected-node design/inspector or design command validation failed |
| `StyleThemeTemplate` | selected-node style/theme/template/state data or command validation failed |
| `Serialize` | YuSerialize write or read failed |
| `FileVfs` | YuFile mount/path was unavailable or save/load failed |
| `RuntimeAssetBoundary` | loaded values did not preserve component/template/state provenance |
| `Output` | caller-owned output spans were too small or invalid |

## Acceptance

The accepted behavior is:

- theme keys are editable alongside style and template keys;
- style/theme/template/state commands stage a caller-owned ledger without
  mutating input runtime data;
- runtime preview feedback still comes from `PreviewHostFrameResult`;
- save/load uses YuSerialize and YuFile `MountTable` write/read;
- missing File/VFS availability returns `FileVfs` without mutating caller-owned
  output spans;
- output capacity failures reject before file write and without partial output;
- the serialization row preserves component kind, template key, and state
  revision into both RuntimeAsset and UI runtime boundary fields.

## Validation

```text
ctest --preset windows-fast-gate -R "(UiEditorStyleThemeTemplateSerializationWorkflow_|UiEditorRuntimePreviewWorkflow_)" --output-on-failure
ctest --preset windows-fast-gate -R "(UiEditor|UiCore_|UiRuntime_|PreviewHostEditorViewportInteraction_|Serialize_|File_)" --output-on-failure
```

## Remaining Gaps

This slice still does not complete the full UI Editor. Remaining work includes:

- full multi-node style/theme/template file authoring;
- resource picker integration for sprites, textures, fonts, atlases, and
  material refs;
- style/theme/template validators beyond selected-node numeric keys;
- editor-host interaction for template catalog browsing and state preview
  controls;
- runtime UI viewport rendering of complete layout/style/resource data;
- package/release validation and complete product visual closure.

## Boundaries

This slice does not add a browser rejected document tree/rejected style/static form /canvas designer, native shell
ownership, package/release closure, or final product visual closure.

It does not accept static screenshots, manual visual claims, CPU/GDI oracle
routes, or any preview source other than the engine Preview Host/runtime UI frame
feedback already carried through RAV7.
