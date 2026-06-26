# YuEngine UI Editor RAV5 Runtime Document Preview Feedback

Status: implemented first slice
Task: #87
Owner: Architecture / Implementation

## Scope

This slice adds a UI Editor data surface for runtime UI document records and
Preview Host frame feedback.

It is intentionally narrow:

- runtime UI node records are the source of truth
- `UiCore::UiNodeTree` validates and resolves runtime rects
- hierarchy rows are caller-owned output
- Preview Host frame feedback is caller-owned input
- missing Preview Host feedback rejects without partial output

## Runtime Path

```text
UiEditorRuntimeDocument
-> UiCore UiNodeTree validation / rect resolution
-> UiEditor hierarchy rows
-> PreviewHostFrameResult feedback
-> UiEditor preview feedback records
```

The slice does not create a native UI Editor shell, design surface, rejected editor route page,
rejected canvas, template editor, style/theme editor, or package/cook workflow.
It also does not mutate runtime data.

## Acceptance

The accepted behavior is:

- valid runtime UI documents produce deterministic hierarchy rows
- node rects come from `UiCore`, not duplicated editor layout logic
- selected node state is editor-only output
- Preview Host frame feedback is required when requested
- missing Preview Host feedback does not write partial hierarchy or preview
  records
- boundary flags stay false for runtime mutation, native window, and rejected editor route/canvas

This is not UI Editor completion. Remaining work includes full inspector
editing, resource picker/import settings, state preview, component editing,
style/theme workflows, visible runtime UI preview, cook/package validation, and
review.
