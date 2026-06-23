# YuEngine Preview Host RAV4 Viewport Session Surface

Owner: Preview Host implementation
Target: `YuPreviewHost`
Status: Implementation slice

## Purpose

This slice adds the first bounded viewport-session surface over the accepted
Preview Host frame path. It is still local engine tooling, not a native editor
application and not a web/canvas/static screenshot surface.

The surface consumes:

- a live `PreviewHostFrameRequest` backed by a RuntimeAsset graph, scene loader
  output, Resource/Asset records, RenderScene inputs, and Preview Host feedback
  output buffers;
- a `ResourceBrowserSurfaceSelectionState` produced by the native Resource
  Browser surface/selection workflow;
- viewport controls: frame size, camera/orbit state, and selected entity index.

It returns:

- the underlying `PreviewHostFrameResult`;
- a viewport ledger recording consumed controls, Resource Browser selection
  state, preview eligibility, selected entity availability, and hit/selection/
  transform feedback emission;
- bounded diagnostics when Resource Browser selection blocks preview before
  frame submission.

## Acceptance

The implementation must prove that a viewport session can be built from the
Resource Browser selection plus the RuntimeAsset-backed Preview Host frame path.

Required checks:

- accepted Resource Browser selection is consumed without using locator suffixes
  as type truth;
- viewport width/height/camera orbit controls are preserved into the result;
- RuntimeAsset graph and scene/resource refs are consumed by `BuildFrame`;
- hit, selection, and transform feedback records are emitted for the submitted
  entities;
- blocked Resource Browser selection returns a Preview Host diagnostic without
  submitting a RenderScene frame or mutating feedback outputs.

## Validation

```text
ctest --preset windows-fast-gate -R "PreviewHost_" --output-on-failure
ctest --preset windows-fast-gate -R "PreviewHost_(BuildsViewportSessionSurface|RejectsBlockedViewportSelection)" --output-on-failure
```

## Boundaries

This slice does not approve Scene Editor hierarchy/inspector/gizmo, Animation
Editor, UI Editor, final native editor application, package launcher, original
package compatibility, or external Unity/UE/DCC bridge.

The next scene-document bridge slice is recorded in
`docs/YUENGINE_PREVIEW_HOST_RAV5_SCENE_DOCUMENT_BRIDGE.md`.

It does not make Resource Browser locator paths authoritative type truth. It
does not bypass RuntimeAsset graph validation, Resource/Asset mapping, or
RenderScene/RenderCore/RHI paths with a UI-only preview.
