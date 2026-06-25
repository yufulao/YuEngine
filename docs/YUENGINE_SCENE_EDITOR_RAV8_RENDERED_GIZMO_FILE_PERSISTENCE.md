# YuEngine Scene Editor RAV8 Rendered Gizmo File Persistence

Owner: Scene Editor implementation
Target: `YuSceneEditor`
Status: implementation slice
Task: #105

## Purpose

This slice extends the RAV7 gizmo/resource/save-load workflow from an in-memory
Serialize buffer proof to a File/VFS-backed scene artifact proof. It remains a
Scene Editor data/workflow surface, not a native editor app shell.

The workflow consumes:

- a validated `WorldSceneAuthoringDocument`;
- a Preview Host-ready `ResourceBrowserSurfaceSelectionState`;
- a successful `PreviewHostViewportSessionResult`;
- a successful `PreviewHostEditorViewportInteractionResult`;
- a selected-object `GizmoMode` editor-only sidecar;
- a selected-object component resource binding;
- a `MountTable`, mount id, and virtual scene persistence path;
- caller-owned staging and output spans.

It emits caller-owned:

- one rendered gizmo row tied to selected `WorldObjectId`, transform, gizmo
  mode, selected entity index, and Preview Host interaction feedback;
- one resource picker row preserving the Resource Browser Resource/Asset
  selection provenance against the current scene component binding;
- one save/load proof record showing scene records were serialized, written to
  File/VFS, read back from File/VFS, then decoded into runtime scene records;
- loaded identity, transform, attachment, and binding records decoded from the
  file-read bytes.

## Runtime Path

```text
WorldSceneAuthoringDocument
-> ResourceBrowserSurfaceSelectionState
-> PreviewHostViewportSessionResult
-> PreviewHostEditorViewportInteractionResult
-> selected GizmoMode sidecar + selected component resource binding
-> WorldSceneRecordValueStreamBridge write into staged bytes
-> MountTable::Write(scene artifact)
-> MountTable::Read(scene artifact)
-> WorldSceneRecordValueStreamBridge read from file bytes
-> caller-owned rendered gizmo, picker, proof, and loaded runtime records
```

The caller-facing spans are committed only after the full chain succeeds. A
File/VFS rejection such as a missing mount returns `FilePersistence` without
committing gizmo rows, resource picker rows, save/load proof rows, loaded records,
or staged persistence bytes back to the caller.

## Acceptance

The accepted behavior is:

- selected object must match Preview Host viewport interaction feedback;
- rendered gizmo evidence must be driven by Preview Host engine viewport
  frame/interaction flags, not by a static screenshot or CPU-only oracle;
- resource picker evidence must preserve Resource Browser selection provenance
  and match the selected scene component binding type;
- runtime scene records are written through `WorldSceneRecordValueStreamBridge`;
- serialized scene bytes are written through `MountTable::Write`;
- persisted scene bytes are read through `MountTable::Read`;
- loaded runtime records are decoded from the File/VFS-read bytes;
- file write/read byte counts and statuses are visible in the proof record;
- failure paths reject without partial caller-output mutation;
- editor-only sidecars remain out of the runtime scene stream.

## Validation

```text
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuSceneEditorSurfaceTests
ctest --preset windows-fast-gate -R "^(SceneEditorFilePersistenceWorkflow_|SceneEditorGizmoWorkflow_|SceneEditorWorkflow_)" --output-on-failure
```

## Boundaries

This slice does not complete the native editor app shell, full rendered gizmo
drawing, model/material/texture authoring, full Resource Browser importer,
Package/release delivery, external Unity/Unreal/DCC bridge parsing, or final
product visual closure. It proves Scene Editor scene persistence can cross the
approved File/VFS boundary while preserving Preview Host interaction evidence and
no-mutation failure behavior.
