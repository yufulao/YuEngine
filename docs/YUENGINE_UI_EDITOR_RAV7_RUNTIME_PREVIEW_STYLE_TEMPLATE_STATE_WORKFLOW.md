# YuEngine UI Editor RAV7 Runtime Preview Style Template State Workflow

Owner: UI Editor implementation
Target: `YuUiEditor`
Status: implementation slice
Task: #100

## Purpose

This slice extends the RAV6 UI Editor design/inspector workflow with a bounded
engine runtime preview row for selected-node style, template, and interaction
state.

It consumes:

- a valid `UiEditorRuntimeDocument`;
- a selected runtime UI node id;
- a successful `PreviewHostFrameResult`;
- the RAV6 design command path;
- caller-owned style/template/state records for runtime UI nodes;
- a bounded style/template/state command for the selected node.

It emits caller-owned:

- hierarchy rows;
- design surface rows;
- inspector rows;
- Preview Host feedback records;
- staged runtime node records;
- the RAV6 design command ledger;
- one runtime preview style/template/state row for the selected node;
- one style/template/state command ledger record.

## Runtime Path

```text
UiEditorRuntimeDocument
-> UiCore UiNodeTree rect resolution
-> PreviewHostFrameResult feedback
-> RAV6 design/inspector workflow
-> selected-node style/template/state record validation
-> selected-node engine runtime preview state row
-> style/template/state command ledger
```

The API stages every output locally and copies to caller buffers only after
document validation, UiCore resolution, Preview Host feedback, selected node
lookup, design command validation, style/template/state validation, output
capacity checks, and style/template/state command validation pass.

## Blocked Layers

`BuildUiEditorRuntimePreviewStyleTemplateStateWorkflow` reports the first
blocking layer:

| Layer | Meaning |
| --- | --- |
| `RuntimeUiDocument` | runtime UI document/header/node validation failed |
| `UiCoreNodeTree` | UiCore node tree creation or rect query failed |
| `PreviewHostFeedback` | required Preview Host frame feedback was missing or unusable |
| `DesignWorkflow` | selected-node design/inspector or design command validation failed |
| `StyleTemplateState` | selected-node style/template/state data or command validation failed |
| `Output` | caller-owned output spans were too small or invalid |

## Acceptance

The accepted behavior is:

- engine runtime preview feedback comes from `PreviewHostFrameResult`;
- selected-node style key, template key, interaction state, resolved rect, and
  Preview Host frame status are emitted together;
- style/template/state commands stage a ledger without mutating input runtime
  data;
- missing Preview Host feedback rejects without partial output;
- invalid style/template/state records reject without partial output;
- output capacity failures reject without partial output;
- boundary flags remain false for runtime data mutation, native-window opening,
  and forbidden preview routes.

## Validation

```text
ctest --preset windows-fast-gate -R "UiEditorRuntimePreviewWorkflow_" --output-on-failure
ctest --preset windows-fast-gate -R "(UiEditor|UiCore_|UiRuntime_|PreviewHostEditorViewportInteraction_)" --output-on-failure
```

## Boundaries

This slice does not complete a full UI Editor, theme system, template editor,
runtime serialization, package/run validation, integrated native editor shell,
external Unity/Unreal/DCC bridge, final product visual closure, or original
package compatibility.

It does not use rejected editor route, browser canvas, static screenshots, CPU oracles, GDI
viewers, or manual visual claims as acceptance.
