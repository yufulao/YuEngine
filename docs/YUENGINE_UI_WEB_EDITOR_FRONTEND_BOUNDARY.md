# YuEngine Web Editor Frontend Boundary

Status: corrective boundary
Owner: Architecture
Scope: Web Editor frontend, local backend bridge, runtime preview contract

## Decision

The Web Editor product surface is a Web frontend workspace, not a C++ tool
surface.

The expected iteration model is:

```text
edit TS/React/CSS/data
-> frontend hot reload / refresh
-> call local service APIs
-> validate or preview through YuEngine backend only when runtime behavior changes
```

Changing editor panels, inspector controls, component templates, style/theme
authoring, state-preview UI, drag/drop, shortcuts, or workflow must not require
rebuilding YuEngine C++.

## Allowed C++ Scope

C++ may exist behind the frontend boundary only for stable backend contracts:

- UI runtime file schema validation
- local file load/save/validate service
- cook/resource validation bridge
- runtime preview protocol
- preview-process control or IPC/WebSocket bridge

These targets are backend support for the Web frontend. They are not the Web
Editor UI.

## Forbidden C++ Scope

Do not add or keep C++ targets for:

- Web editor shell or four-panel composition
- hierarchy/inspector/canvas/resource picker panel models
- component template catalogs
- style/theme authoring state
- state-preview workflow
- editor shortcuts, drag/drop, or workflow behavior

Those belong in the Web frontend workspace or editable data files.

## Frontend Shape

The first frontend slice should be a small web workspace with:

- hierarchy panel
- inspector/property controls
- layout canvas shell
- resource picker shell
- validation result panel
- JSON/TS component template catalog
- JSON/TS style/theme catalog
- state-preview input panel
- client for the local service API

Acceptable stacks are the same class as the OpenAgents workspace pattern:
TypeScript, React, a hot-reload dev server, and ordinary frontend validation
commands. The final choice can be Next or Vite, but the acceptance criterion is
fast UI iteration without C++ rebuild.

## Required Handoff Wording

Use this split when creating tasks:

```text
Frontend task:
  Implement/adjust Web Editor UI in the TS/React workspace.
  Acceptance: frontend command passes; no CMake rebuild required.

Backend task:
  Implement/adjust local service or runtime preview protocol.
  Acceptance: CMake/CTest passes because YuEngine C++ contracts changed.
```

Do not name a C++ library `WebEditorShell` or equivalent; that name makes the
frontend ownership ambiguous and encourages the wrong compile-bound workflow.
