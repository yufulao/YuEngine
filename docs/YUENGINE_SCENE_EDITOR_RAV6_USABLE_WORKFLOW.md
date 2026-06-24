# YuEngine Scene Editor RAV6 Usable Workflow

Owner: Scene Editor implementation
Target: `YuSceneEditor`
Status: implementation slice
Task: #91

## Purpose

This slice connects the accepted Scene Editor data surfaces, Resource Browser
selection workflow, Preview Host viewport interaction, and transform command
ledger into one minimal usable Scene Editor workflow surface.

It consumes:

- a validated `WorldSceneAuthoringDocument`;
- a Preview Host-ready `ResourceBrowserSurfaceSelectionState`;
- a successful `PreviewHostViewportSessionResult`;
- a successful `PreviewHostEditorViewportInteractionResult`;
- a requested transform command or undo/redo history record.

It emits caller-owned:

- hierarchy rows from the authoring document;
- inspector rows for the selected world object;
- transform output records staged by `ApplySceneEditorTransformCommand`;
- transform undo/redo ledger records;
- a workflow ledger binding resource selection, viewport selection, hierarchy
  selection, inspector emission, and transform command sequence.

## Runtime Path

```text
WorldSceneAuthoringDocument
-> ResourceBrowserSurfaceSelectionState
-> PreviewHostViewportSessionResult
-> PreviewHostEditorViewportInteractionResult
-> SceneEditor native hierarchy/inspector rows
-> SceneEditor transform command output
-> SceneEditor workflow ledger
```

The workflow only commits caller outputs after the authoring document,
Resource Browser selection, Preview Host session, viewport interaction, and
transform command all pass. Blocking dependency failures return a structured
blocked layer and leave caller-owned rows and ledgers unchanged.

## Acceptance

The accepted behavior is:

- hierarchy selection drives the selected inspector row;
- viewport selected world object must match the authoring document selection;
- Resource Browser selection must be Preview Host-ready and preserve
  Resource/Asset mapping;
- Preview Host session and interaction must report successful engine-backed
  viewport feedback;
- apply, undo, and redo commands write deterministic transform ledgers;
- blocked Resource Browser, Preview Host, or transform dependencies reject
  without partial caller-output mutation.

## Validation

```text
ctest --preset windows-fast-gate -R "SceneEditorWorkflow_" --output-on-failure
ctest --preset windows-fast-gate -R "(SceneEditor|PreviewHostEditorViewportInteraction_|ResourceBrowserVisibleWorkflow_)" --output-on-failure
```

## Boundaries

This slice does not complete a native Scene Editor shell, Unity/Unreal clone,
external DCC bridge, model/material/texture authoring, gizmo rendering, package
run validation, UI Editor, Animation Editor, or final product visual closure.
