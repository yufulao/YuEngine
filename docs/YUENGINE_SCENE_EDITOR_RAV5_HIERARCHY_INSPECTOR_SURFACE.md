# YuEngine Scene Editor RAV5 Hierarchy And Inspector Surface

Owner: Scene Editor implementation
Target: `YuSceneEditor`
Status: Implementation slice

## Purpose

This slice adds the first native Scene Editor data surface around the accepted
scene authoring document contract. It is a bounded value-record surface, not a
native editor application, transform gizmo, command stack, viewport bridge, or
resource picker.

The surface consumes:

- a validated `WorldSceneAuthoringDocument`;
- ordered object identity records;
- transform, component attachment, and component-resource binding records;
- editor-only sidecar records for selection and foldout state.

It returns caller-owned hierarchy rows and inspector rows. Hierarchy rows follow
authoring document object order and expose explicit visible/active state derived
from validated object identity records. Inspector rows follow editor-only
selection sidecar order and report separate runtime-export field counts and
editor-only sidecar field counts. The result records scene id/hash, selected
object count, foldout count, component count, resource binding count, and
boundary flags proving this slice did not open a native window, use preview
feedback, mutate runtime data, or export runtime records.

## Acceptance

Required checks:

- hierarchy rows expose world object id, object handle, deterministic row index,
  depth, component count, resource binding count, transform presence, selection
  state, visibility, active state, and foldout state;
- inspector rows expose selected object identity, transform values when present,
  component count, resource binding count, selected state, runtime-export field
  count, editor-only sidecar field count, and explicit field separation;
- invalid authoring documents reject before row output mutation;
- too-small hierarchy or inspector buffers reject before row output mutation;
- missing selection can be reported explicitly when a caller requires selected
  inspector data;
- editor-only sidecar state remains editor data only.

## Boundaries

This slice does not implement:

- Scene document to Preview Host bridge;
- viewport image, camera controls, gizmo, drag/drop, or undo-redo commands;
- native editor shell or window creation;
- Resource Browser picker or import settings panel;
- RuntimeAsset cook/load, package, original package compatibility, or product
  visual closure;
- rejected editor route, canvas, rejected markup, rejected style, GDI, manual screenshot, or report-only acceptance.

`YuSceneEditor` depends on `YuWorld` records. Lower runtime modules do not depend
on `YuSceneEditor`.

## Validation

```powershell
cmake --build --preset windows-fast-gate --target YuSceneEditorSurfaceTests -- /v:minimal
ctest --preset windows-fast-gate -R "SceneEditorSurface_" --output-on-failure
```
