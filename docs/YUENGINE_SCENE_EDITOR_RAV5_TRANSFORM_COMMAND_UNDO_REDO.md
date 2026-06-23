# YuEngine Scene Editor RAV5 Transform Command Undo Redo

Owner: Scene Editor implementation
Target: `YuSceneEditor`
Status: Implementation slice

## Purpose

This slice adds the first data-only transform command path for the Scene Editor.
It consumes a validated `WorldSceneAuthoringDocument`, a selected
`WorldObjectId`, and caller-owned output buffers, then emits updated transform
records plus an undo/redo ledger record.

It is not a gizmo, native editor shell, drag interaction, Preview Host viewport,
Resource Browser picker, or package/run validation path.

## Contract

`ApplySceneEditorTransformCommand` must:

- validate the authoring document before writing outputs;
- resolve the selected object and its transform record;
- stage transform records before committing caller-owned output;
- support apply, undo, and redo modes through
  `SceneEditorTransformLedgerRecord`;
- emit one ledger record describing before/after transforms, command sequence,
  and undo/redo availability;
- keep runtime data, native windows, Preview Host feedback, Resource Browser,
  RHI, and RenderScene outside this module.

## Acceptance

Required tests:

- `SceneEditorTransformCommand_AppliesSelectedTransformAndWritesUndoLedger`
- `SceneEditorTransformCommand_UndoRedoReplaysLedgerDeterministically`
- `SceneEditorTransformCommand_RejectsInvalidSelectionWithoutMutation`

Required guarantees:

- apply updates only the selected transform in caller-owned transform output;
- undo restores the previous transform from the ledger;
- redo reapplies the ledger's after transform;
- invalid selection fails without mutating transform output or ledger output.

## Boundaries

This does not complete Scene Editor editing UX. It only adds the data command
and undo/redo ledger needed by a later gizmo/native viewport interaction slice.
