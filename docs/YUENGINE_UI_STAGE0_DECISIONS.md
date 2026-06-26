# YuEngine UI Stage 0 Decisions

Status: corrective replacement
Baseline: `origin/main@ba7ff24`
Source plan: `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
Scope: boundary freeze for generic UI Framework and engine-runtime editor
direction after rejected editor route removal.

## 1. Purpose

Stage 0 freezes the corrected UI boundary.

The engine-stage UI scope is:

```text
UI Core
UI Component Library
UIManager runtime framework
engine runtime editor host direction
```

The engine-stage UI scope is not old game-window work and is not a rejected editor route
surface.

## 2. Naming

Planning names:

- UI Framework
- UI Core
- UI Component Library
- UIManager Runtime Framework
- UI Runtime Editor Host

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

### UI Runtime Editor Host

The editor direction is native/engine runtime preview host plus runtime data.

The UI editor owns authoring of runtime UI layout/style/resource data through a
native or engine-hosted editor surface. A local editor service may own file IO,
schema validation, version migration, resource validation, cook commands, and
preview-process control. The game runtime must not depend on editor code.

Hierarchy, inspector, viewport, resource picker, component templates,
style/theme editing, state preview, drag/drop, shortcuts, and visual workflow
must be backed by runtime data and engine preview output. They are not accepted
through rejected form layout, editor-route-only canvas, rejected editor toolchain, or other deprecated rejected editor route shell work.

C++ is allowed for stable runtime/editor-host contracts: runtime schema
validation, local file/service bridge, cook/asset validation bridge, and
engine preview host protocol. C++ must not reintroduce a rejected editor route shell,
rejected panel model, rejected editor toolchain workflow, or rejected form layout preview path.

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
| Rejected editor route attempt | historical anti-pattern showing why rejected editor shells, rejected document tree controls, and rejected form layout preview are not editor direction | any active rejected editor route editor route, backlog, gate, workflow, or acceptance |

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
- rejected editor route Editor is rejected as an editor direction.
- Native/engine runtime preview host is the only editor direction.
- No native app editor path is retained.
- No deprecated C++ rejected editor route editor shell/panel/template/theme/state-preview path is retained.
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

Editor work must start from runtime data schemas, Resource/Asset/Cook
validation, native/engine preview host output, and runtime diagnostics. It must
not add deprecated rejected editor route schema/editor route/workspace/protocol tasks, and it must not
model a rejected editor route route as C++ libraries.
