# YuEngine Preview Host RAV6 Editor Viewport Interaction

Owner: Preview Host implementation
Target: `YuPreviewHost`
Status: implementation slice
Task: #89

## Purpose

This slice adds a minimal editor viewport interaction surface over an already
accepted Preview Host viewport session. It is a bridge from engine-backed
viewport frame state to editor command feedback, not a native editor shell.

The surface consumes:

- a successful `PreviewHostViewportSessionResult` with a submitted engine frame;
- an editor viewport interaction command;
- RuntimeAsset scene entity records for selection feedback;
- caller-owned hit, selection, transform-feedback, and interaction-ledger output
  buffers.

It returns:

- updated camera state for orbit, pan, or zoom commands;
- hit/selection/transform feedback for entity selection commands;
- a bounded interaction ledger for downstream Scene, Animation, or UI editor
  workflow slices.

## Runtime Path

```text
PreviewHostViewportSessionResult
-> PreviewHostEditorViewportInteractionRequest
-> camera command or entity selection command
-> caller-owned feedback records
-> interaction ledger
```

The surface requires the viewport session to have already submitted an engine
frame through Preview Host. It does not create a native window, use Web/HTML/
canvas, use GDI, use manual screenshots, or use a CPU oracle as a preview route.

## Acceptance

The accepted behavior is:

- orbit commands update camera state and emit a camera ledger;
- selection commands consume RuntimeAsset scene entity records and emit hit,
  selection, and transform feedback;
- invalid or stale viewport sessions reject before writing partial output;
- output capacity failures reject before writing partial output;
- boundary flags remain false for native-window and forbidden preview paths.

## Validation

```text
ctest --preset windows-fast-gate -R "PreviewHostEditorViewportInteraction_" --output-on-failure
ctest --preset windows-fast-gate -R "(PreviewHost_|PreviewHostEditorViewportInteraction_)" --output-on-failure
```

## Boundaries

This slice does not complete the editor viewport, Scene Editor, Animation
Editor, UI Editor, package/run validation, Resource Browser visible workflow,
native editor shell, or final product visual closure.
