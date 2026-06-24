# YuEngine UI Editor RAV6 Design Inspector Workflow

Owner: UI Editor implementation
Target: `YuUiEditor`
Status: implementation slice
Task: #93

## Purpose

This slice turns the RAV5 runtime UI document surface into a minimal visible
UI design and inspector workflow. It uses runtime UI document/node records,
`UiCore::UiNodeTree` resolved rectangles, and Preview Host frame feedback as
the source of truth.

It consumes:

- a valid `UiEditorRuntimeDocument`;
- a selected runtime UI node id;
- a successful `PreviewHostFrameResult`;
- a bounded design command for the selected node.

It emits caller-owned:

- hierarchy rows;
- design surface rows with resolved rect, selection, component state, and
  Preview Host frame status;
- inspector field rows for component kind, visibility, enabled state,
  hit-test state, runtime-export state, layer, and rect transform;
- Preview Host feedback records;
- staged runtime node records;
- a command ledger for the staged component edit.

## Runtime Path

```text
UiEditorRuntimeDocument
-> UiCore UiNodeTree rect resolution
-> PreviewHostFrameResult feedback
-> design surface rows
-> selected-node inspector rows
-> staged runtime node update
-> command ledger
```

The workflow stages all outputs locally and only copies to caller-owned buffers
after document validation, UiCore resolution, Preview Host feedback, selected
node lookup, output capacity, and command validation all pass.

## Acceptance

The accepted behavior is:

- selected node state is visible in hierarchy and design surface rows;
- inspector rows expose component kind and component state for the selected
  node;
- component edit commands produce staged runtime node output and command ledger
  records;
- missing Preview Host feedback rejects without partial output;
- invalid component records reject without partial output;
- output capacity failures reject without partial output.

## Validation

```text
ctest --preset windows-fast-gate -R "UiEditorWorkflow_" --output-on-failure
ctest --preset windows-fast-gate -R "(UiEditor|UiCore_|UiRuntime_|PreviewHostEditorViewportInteraction_)" --output-on-failure
```

## Boundaries

This slice does not complete a full UI Editor, style/theme system, template
editor, Web/canvas designer, native editor shell, package/run validation,
Scene Editor, Animation Editor, or final product visual closure.
