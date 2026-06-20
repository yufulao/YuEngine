# YuEngine UI Stage 0 Decisions

Status: corrective replacement
Baseline: `origin/main@ba7ff24`
Source plan: `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
Scope: boundary freeze for generic UI Framework and Web Editor direction.

## 1. Purpose

Stage 0 freezes the corrected UI boundary.

The engine-stage UI scope is:

```text
UI Core
UI Component Library
UIManager runtime framework
Web Editor tooling direction
```

The engine-stage UI scope is not old game-window work and is not a native
editor application.

## 2. Naming

Planning names:

- UI Framework
- UI Core
- UI Component Library
- UIManager Runtime Framework
- Web Editor

Existing source targets may keep existing `Yu*` names where renaming would only
create churn.

## 3. Ownership Boundaries

### UI Core

Owns:

- node tree
- rect, anchor, pivot, margin, padding, size policy
- layout containers
- dirty classification
- hit-test and input-route records
- focus foundation
- draw-element records
- render bridge contract

Does not own:

- open/close lifecycle
- loaded/active cache
- popup or fullscreen stack policy
- release policy
- config table lookup
- game state
- editor-only state

### UI Component Library

Owns:

- Text
- Image
- Button
- Slider
- Toggle
- Progress
- Scroll
- virtualized List/GridView
- atlas records
- batching records
- invalidation and rebuild diagnostics

List/GridView must follow virtualized visible-range and pooled-cell behavior.
Full-list materialization is rejected.

### UIManager Runtime Framework

Owns:

- `BaseUI` lifecycle
- generic registry records
- layer records
- loaded and active maps
- popup stack
- fullscreen stack
- open-argument value snapshots
- close, reopen, release, and cache policy

This layer remains generic. Existing code may use the term `panel`, but it means
a runtime record, not a folder of game panels.

### Web Editor

The editor direction is Web.

The Web Editor owns authoring UI and tooling workflow. A local editor service
owns file IO, schema validation, version migration, resource validation, cook
commands, and preview-process control. The game runtime must not depend on the
Web Editor.

The Web frontend is a TypeScript/React-style hot-reload workspace, not a C++
tool target. Hierarchy, inspector, canvas, resource picker, component
templates, style/theme editing, state preview, drag/drop, shortcuts, and visual
workflow must iterate through frontend files and data files without rebuilding
YuEngine C++.

C++ is allowed only for stable backend contracts: runtime schema validation,
local file/service bridge, cook/asset validation bridge, and preview protocol.
C++ must not own Web editor shell composition, panel models, template catalogs,
style/theme authoring state, or state-preview workflow.

## 4. Reference Inputs

Allowed references:

| Reference | Borrowed behavior | Not copied |
| --- | --- | --- |
| Existing `UIManager` | generic layer, loaded/active, stack, open/close/release semantics | game windows, game managers, data flow |
| Existing `UICtrlBase` / `BaseUI` | lifecycle shape above UI Core | old project class dependency |
| Existing GridView base | virtualization, pooling, grouped visible range | game data |
| Unity UGUI | common rect/component vocabulary | Unity object model |
| TextMeshPro | font asset and glyph fallback concepts | full TMP implementation |
| Unreal Slate/UMG | invalidation and UI performance vocabulary | Unreal editor/runtime architecture |
| Web platform | editor shell, DOM controls, drag/drop, fast iteration | game runtime UI |

Every landing task must state:

```text
Reference used:
Borrowed behavior:
Not copied:
YuEngine-specific acceptance:
```

## 5. Stage 0 Decisions

- UI Core excludes lifecycle.
- UIManager runtime owns lifecycle and stacks.
- Config tables are deferred.
- Web Editor is the only editor direction.
- No native app editor path is retained.
- No C++ Web editor shell/panel/template/theme/state-preview path is retained.
- No old game windows are used as validation samples.
- No old game-window matrix is part of UI Framework progress.
- List/GridView is virtualized.
- Runtime data files are the source of truth.

## 6. Layout Asset Direction

Initial UI layout data remains a versioned text schema until a later gate
chooses final extension and binary/cooked representation.

Required data:

- schema ID
- version
- layout ID
- root node ID
- deterministic node array
- rect/layout fields
- style refs
- resource refs
- stable event/binding keys as data

Excluded:

- Unity object references
- editor-only object references
- backend renderer handles
- lifecycle callbacks
- config table dependency
- game manager calls

## 7. Handoff Gates

Stage 1 may implement UI Core only.

Stage 2 may implement component library and mechanisms only.

Stage 3 may implement UIManager runtime framework only.

Web Editor work must start from Web schema/service/frontend/preview protocol
tasks. It must not add native app/editor targets, and it must not model the Web
frontend as C++ libraries.
