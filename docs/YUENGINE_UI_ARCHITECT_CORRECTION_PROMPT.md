# Deprecated UI Framework / Web Editor Correction Prompt

Status: tombstone / historical anti-pattern
Task: #45 Web cleanup B

## Decision

This file previously instructed the team to continue with a deprecated Web
Editor, deprecated Web frontend, React/Vite-style frontend workspace, HTML/CSS
editing surface, and WebSocket/IPC preview protocol. That instruction is
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

The old Web Editor prompt is retained only as a record of the historical error.

## Forbidden Restorations

Do not restore:

- forbidden Web Editor as a live plan;
- forbidden Web frontend as editor direction;
- forbidden React, Vite, TypeScript hot reload, or frontend workspace as editor
  acceptance;
- forbidden HTML/CSS, DOM, browser-only canvas, or static screenshots as runtime preview;
- forbidden WebSocket protocol work as proof of editor viewport capability;
- browser shell work as YuEngine editor progress.

Future architecture prompts must route UI/Scene/Animation editor work through
runtime data, native/engine preview host, Resource/Asset/Cook validation,
RenderScene/RenderCore/RHI or UI runtime output, diagnostics, and package/run
smoke.
