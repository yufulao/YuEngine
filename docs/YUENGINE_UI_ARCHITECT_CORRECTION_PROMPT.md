# Deprecated UI Framework / rejected editor route Editor Correction Prompt

Status: tombstone / historical anti-pattern
Task: #45 rejected editor route cleanup B

## Decision

This file previously instructed the team to continue with a deprecated rejected editor route
Editor, deprecated rejected editor route editor route, rejected editor toolchain-style editor route workspace, rejected form layout
editing surface, and deprecated IPC/IPC preview protocol. That instruction is
revoked.

Do not send the old prompt to implementers. Do not use it to create tasks.

## Replacement Direction

The active UI editor direction is:

```text
runtime UI data
-> Resource / Asset / Cook validation
-> native or engine runtime preview host
-> engine UI runtime frame/status/diagnostics
-> build / run / package smoke
```

The old rejected editor route Editor prompt is retained only as a record of the historical error.

## Forbidden Restorations

Do not restore:

- forbidden rejected editor route Editor as a live plan;
- forbidden rejected editor route editor route as editor direction;
- forbidden rejected editor framework, rejected editor build tool, editor script hot reload, or editor route workspace as editor
  acceptance;
- forbidden rejected form layout, rejected document tree, editor-route-only canvas, or static screenshots as runtime preview;
- forbidden deprecated IPC protocol work as proof of editor viewport capability;
- browser shell work as YuEngine editor progress.

Future architecture prompts must route UI/Scene/Animation editor work through
runtime data, native/engine preview host, Resource/Asset/Cook validation,
RenderScene/RenderCore/RHI or UI runtime output, diagnostics, and package/run
smoke.
