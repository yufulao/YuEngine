# YuEngine Preview Host RAV5 Scene Document Bridge

Owner: Preview Host implementation
Target: `YuPreviewHost`
Status: Implementation slice

## Purpose

This slice connects the first accepted scene authoring document data surface to
the Preview Host viewport-session path.

The route is:

```text
WorldSceneAuthoringDocument
-> WorldSceneAuthoringRuntimeExport
-> RuntimeAsset scene entity records
-> PreviewHostViewportSessionRequest
-> RenderScene frame/feedback
```

It is a bridge between scene authoring data and the existing engine preview
surface. It is not a full Scene Editor UI, hierarchy/inspector/gizmo workflow,
undo/redo system, Animation Editor, or UI Editor.

## Contract

`PreviewHost::BuildSceneDocumentViewportSession` must:

- validate and export `WorldSceneAuthoringDocument` before touching scene output;
- stage RuntimeAsset scene entity records locally and apply exported transform
  records by `WorldObjectId`;
- reuse Resource Browser selection and Preview Host viewport controls from the
  existing viewport-session surface;
- publish updated scene entity records and hit/selection/transform feedback only
  after the viewport session succeeds;
- report a precise blocked layer when authoring document validation, runtime
  scene mapping, or viewport session construction fails.

## Acceptance

Required tests:

- `PreviewHost_BuildsViewportSessionFromSceneAuthoringDocument`
- `PreviewHost_RejectsInvalidSceneAuthoringDocumentWithoutMutation`

Required guarantees:

- the success path consumes scene authoring document/runtime export records,
  RuntimeAsset graph/scene records, Resource Browser selection, viewport
  controls, and Preview Host feedback buffers;
- the exported authoring transform records override the staged RuntimeAsset
  entity transforms before RenderScene frame submission;
- invalid authoring documents fail before RuntimeAsset scene output and Preview
  Host feedback buffers are mutated.

## Boundaries

This does not make the Scene Editor complete. It only proves the first usable
document-to-preview route needed by later hierarchy, inspector, selection,
transform command, undo/redo, Animation Editor, and UI Editor slices.

It does not bypass File/VFS/RuntimeAsset, Resource/Asset mapping, Resource
Browser selection, RenderScene frame submission, or Preview Host diagnostics
with private test-only objects.
