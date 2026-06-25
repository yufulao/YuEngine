# YuEngine Scene Editor RAV7 Gizmo Resource Save Load Workflow

Owner: Scene Editor implementation
Target: `YuSceneEditor`
Status: implementation slice
Task: #98

## Purpose

This slice extends the RAV6 usable Scene Editor workflow with a bounded
rendered-gizmo, resource-picker, and save/load proof surface. It still remains a
SceneEditor value/workflow surface, not a native editor shell.

The workflow consumes:

- a validated `WorldSceneAuthoringDocument`;
- a Preview Host-ready `ResourceBrowserSurfaceSelectionState`;
- a successful `PreviewHostViewportSessionResult`;
- a successful `PreviewHostEditorViewportInteractionResult`;
- a selected-object `GizmoMode` editor-only sidecar;
- a selected-object component resource binding;
- a caller-owned persistence byte buffer and caller-owned output spans.

It emits caller-owned:

- one rendered gizmo row bound to selected `WorldObjectId`, transform, gizmo
  mode, selected entity index, and Preview Host frame feedback;
- one resource picker row binding the selected scene component resource to the
  current Resource Browser selection while preserving Resource/Asset mapping;
- one save/load proof record showing runtime scene records were written and read
  through `WorldSceneRecordValueStreamBridge`;
- loaded identity, transform, attachment, and resource binding records from the
  deterministic value stream.

## Runtime Path

```text
WorldSceneAuthoringDocument
-> ResourceBrowserSurfaceSelectionState
-> PreviewHostViewportSessionResult
-> PreviewHostEditorViewportInteractionResult
-> selected GizmoMode sidecar + selected component resource binding
-> rendered gizmo row + resource picker row
-> WorldSceneRecordValueStreamBridge write/read
-> caller-owned loaded runtime scene records
```

The save/load proof uses existing World/Serialize value-stream contracts. It
does not add File/Package persistence policy and does not serialize editor-only
sidecars into the runtime scene stream.

## Acceptance

The accepted behavior is:

- selected object must match Preview Host viewport interaction feedback;
- gizmo rows require a selected-object `GizmoMode` sidecar and transform record;
- resource picker rows require a current scene resource binding and a
  Preview Host-ready Resource Browser selection with matching resource type;
- save/load proof writes and reads identity, transform, attachment, and binding
  records through `WorldSceneRecordValueStreamBridge`;
- editor-only sidecars are counted as skipped for runtime stream proof;
- missing gizmo sidecar and save/load failures reject without committing gizmo,
  picker, or save/load rows;
- no native window, Web/canvas fallback, CPU oracle, manual screenshot, or
  external Unity/Unreal/DCC bridge is introduced.

## Validation

```text
ctest --preset windows-fast-gate -R "SceneEditorGizmoWorkflow_" --output-on-failure
ctest --preset windows-fast-gate -R "(SceneEditorGizmoWorkflow_|SceneEditorWorkflow_)" --output-on-failure
```

## Boundaries

This slice does not complete the native editor shell, rendered gizmo drawing
implementation, file-backed scene persistence, complete Resource Browser UI,
model/material/texture authoring, external authoring bridge, package/release
delivery, or final product visual closure. It proves the next Scene Editor
workflow surface can join Preview Host feedback, resource picker state, and
World value-stream save/load evidence without crossing those ownership
boundaries.
