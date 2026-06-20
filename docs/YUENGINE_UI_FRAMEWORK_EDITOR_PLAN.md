# YuEngine UI Framework and Web Editor Plan

Status: corrective replacement
Requested: 2026-06-20
Owner: Architecture
Scope: UI Framework, UI Core, UI Component Library, UIManager runtime framework, Web Editor

Frontend boundary supplement: `docs/YUENGINE_UI_WEB_EDITOR_FRONTEND_BOUNDARY.md`

## 1. Correction

This document replaces the earlier UI plan.

The corrected direction is:

```text
YuEngine UI work = generic UI runtime framework + generic UIManager framework.
UI Editor direction = Web.
```

There is no engine-stage scope for game-specific windows, game-specific
workflow migration, or game-specific validation samples.

The following are explicitly out of scope:

- migrating old game windows into YuEngine
- using game-specific windows as validation samples
- creating a window-family migration matrix
- adding a native app editor as an engine capability
- adding a native immediate-mode editor shell as fallback or compatibility
- making config tables part of the current UI scope
- making the Web Editor depend on game runtime code
- making the game runtime depend on Web Editor code

The only allowed use of the old framework is as a reference for generic
patterns:

- `UIManager` open, close, stack, loaded, active, and release semantics
- `UICtrlBase` / `BaseUI` lifecycle shape
- `FancyScrollView`-derived GridView/List virtualization behavior
- common component expectations such as text, image, button, slider, scroll,
  list, grid, and popup/fullscreen layering

Those references do not authorize porting game windows or game logic.

## 2. Layer Model

### 2.1 UI Core

UI Core is the low-level retained UI runtime.

Owns:

- node identity and parent/child order
- rect, anchor, pivot, margin, padding, size policy, and DPI/safe-area inputs
- layout containers and layout dirty propagation
- hit-test records and input routing foundation
- focus records
- draw-element intermediate representation
- dirty categories for layout, transform, paint, text, and hit-test
- a narrow render bridge contract

Does not own:

- window open/close lifecycle
- popup or fullscreen stack policy
- loaded/active view cache
- release policy
- game state
- config table service
- editor-only state

### 2.2 UI Component Library

The component library sits on UI Core.

Required first-round components and mechanisms:

- Text: font asset refs, glyph atlas refs, fallback status, overflow status
- Image: sprite/atlas refs, nine-slice metadata, tint, visibility
- Button: states, pointer/focus interaction, command emission
- Slider: value, range, step, command emission
- Toggle and progress: value-state components
- Scroll: viewport, content offset, clamp status
- List/GridView: virtualized visible range and pooled cell reuse, following the
  existing `FancyScrollView`-derived GridView behavior as the reference
- atlas metadata: static atlas first, dynamic atlas only as bounded auxiliary
- batching metadata: material, texture, font atlas, blend, clip, and layer keys
- rebuild/invalidation: subtree dirty propagation, cached layout and paint
  command reuse, explicit rebuild counters

The component library must stay generic. It must not contain game-specific item,
quest, story, shop, save, time, or scene flow concepts.

### 2.3 UIManager Runtime Framework

UIManager runtime framework sits above UI Core and components.

Owns:

- `BaseUI` / controller lifecycle
- view IDs and generic registry records
- layer records
- loaded and active maps
- popup stack
- fullscreen stack
- open argument value snapshots
- close, reopen, release, and cache policy
- missing view, duplicate open, capacity, and release statuses

The term "panel" in existing code means a generic UIManager runtime record. It
does not mean a folder of game panels and does not authorize any game-specific
view implementation.

Does not own:

- game-specific windows
- game manager adapters
- config tables
- old game data flow
- UI Editor code

## 3. Web Editor Direction

The UI Editor direction is Web. This is not "Web first"; it is the direction.

The editor architecture is:

```text
Web frontend workspace (TypeScript/React, hot reload)
-> local editor service / backend bridge
-> file/schema/validator/cook commands
-> optional engine preview process through IPC/WebSocket
```

The Web frontend is the editor product surface. It should iterate like the
OpenAgents workspace-style web app: change TS/React/CSS/data files, refresh or
hot reload, and validate with frontend commands. Editing hierarchy panels,
inspector controls, templates, themes, state-preview UI, drag/drop, shortcuts,
and visual workflow must not require recompiling YuEngine C++ targets.

C++ is allowed only behind the Web frontend boundary:

- stable runtime schema validation
- local file/service bridge
- cook/asset validation bridge
- runtime preview protocol and preview-process control

C++ is not allowed to own editor UI composition, panel layout, component
template catalogs, style/theme authoring state, or state-preview UI workflow.
Those belong to the Web frontend or editable data files.

### 3.1 Web Frontend

Owns:

- hierarchy panel
- inspector
- property controls
- drag and drop
- layout canvas interaction
- resource picker UI
- style/theme editing UI
- state preview UI
- validation result view
- keyboard shortcuts and editor workflow

The frontend edits runtime data files. It must not become a game runtime
dependency, and it must not be modeled as C++ libraries under `Tools/`.

### 3.2 Local Editor Service

Owns:

- reading and writing UI layout files
- schema validation
- version migration
- resource reference validation
- style/theme file validation
- invoking cook/pack validation
- starting and controlling preview sessions
- exposing a local HTTP/WebSocket API for the Web frontend

The service is tooling. It is not part of shipped runtime.
It may be implemented in C++ where it directly touches YuEngine runtime
contracts, but it must expose stable APIs to the Web frontend instead of
encoding frontend panels or editor catalogs in C++.

### 3.3 Engine Preview

Preview must use the real UI runtime path where possible.

Allowed preview modes:

- engine preview process renders runtime data and returns image/frame/status
- headless validation returns layout, hit-test, rebuild, batch, and resource
  diagnostics
- Web canvas may draw editor overlays, selection boxes, and guides

Not allowed:

- reimplementing runtime layout rules only in Web
- making Web DOM the game UI runtime
- adding a native app editor fallback
- adding a native immediate-mode editor shell fallback

## 4. Reference Inputs

Reference use is constrained to generic behavior.

| Reference | Use | Not copied |
| --- | --- | --- |
| Existing `UIManager` | generic layer, loaded/active, stack, open/close/release semantics | game-specific windows, managers, data flow |
| Existing `UICtrlBase` / `BaseUI` | generic lifecycle naming and state transitions | engine dependency on old project classes |
| Existing GridView base | virtualization, cell pooling, grouped visible range | game-specific item data |
| Unity UGUI | rect/anchor/pivot, common component behavior | Canvas rebuild model as-is, Unity object model |
| TextMeshPro | font asset/glyph/fallback concepts | full TMP feature parity in first slice |
| Unreal Slate/UMG | invalidation, retainer-like caching, UI performance vocabulary | Unreal editor/runtime architecture |
| Web platform | editor shell, DOM controls, drag/drop, fast iteration | game runtime UI |

Each landing task must record:

```text
Reference used:
Borrowed behavior:
Not copied:
YuEngine-specific acceptance:
```

## 5. Execution Plan

### Stage 0: Cleanup and Boundary Freeze

Goal: remove wrong scope and freeze the corrected boundary.

Required:

- remove native app/editor engine targets and tests
- remove game-window sample controllers and tests
- remove the old game-window matrix
- rewrite validation to cover only generic UIManager runtime records
- document Web Editor as the only editor direction
- keep existing UI Core, component, and UIManager generic runtime tests

Done when:

- no native editor app target exists
- no UI Editor C++ tool directory exists
- no game-window sample runtime code remains
- no migration-matrix document remains
- fast gate passes

### Stage 1: UI Core

Goal: complete the retained low-level UI runtime.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| UI-S1-001 | Node identity and tree | create, destroy, attach, detach, order tests |
| UI-S1-002 | Rect and layout records | anchors, pivots, margins, padding, size policy tests |
| UI-S1-003 | Layout containers | horizontal, vertical, overlay, absolute tests |
| UI-S1-004 | Dirty propagation | layout/paint/transform/text/hit-test counters are deterministic |
| UI-S1-005 | Hit-test records | bounds, visibility, layer order, disabled state tests |
| UI-S1-006 | Focus foundation | focusable order, clear, disable, missing-node status tests |
| UI-S1-007 | Draw-element IR | stable command records and validation statuses |
| UI-S1-008 | Render bridge contract | no backend leakage into UI Core |

### Stage 2: Component Library and Mechanisms

Goal: implement generic components and performance-critical mechanisms.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| UI-S2-001 | Text component | font refs, glyph refs, overflow, fallback status |
| UI-S2-002 | Image component | sprite/atlas refs, tint, nine-slice metadata |
| UI-S2-003 | Button component | state transitions and command output |
| UI-S2-004 | Slider component | range/value/step and command output |
| UI-S2-005 | Toggle/progress components | deterministic value-state updates |
| UI-S2-006 | Scroll viewport | offset, clamp, visible rect tests |
| UI-S2-007 | Grid/List virtualization | bounded visible range and pooled cell reuse |
| UI-S2-008 | Static atlas metadata | no runtime dependency on editor-only data |
| UI-S2-009 | Dynamic atlas auxiliary path | bounded capacity and explicit failure status |
| UI-S2-010 | Draw batching metadata | stable batching keys and counters |
| UI-S2-011 | Invalidation and rebuild cache | subtree counters and no whole-tree rebuild for local change |
| UI-S2-012 | Component diagnostics | draw count, batch count, atlas page count, dirty subtree count |

### Stage 3: UIManager Runtime Framework

Goal: finish the generic `UIManager + BaseUI` runtime layer.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| UI-S3-001 | BaseUI lifecycle | init, bind, open, close, clear, snapshot tests |
| UI-S3-002 | Registry records | IDs resolve to generic layout/controller/resource/layer refs |
| UI-S3-003 | Layer model | deterministic layer root and ordering tests |
| UI-S3-004 | Loaded/active map | open, close, reopen, lookup, clear tests |
| UI-S3-005 | Popup stack | open, bring-to-top, close, release, missing, capacity tests |
| UI-S3-006 | Fullscreen stack | open, close-current, restore, back, release tests |
| UI-S3-007 | Open args | copied value snapshots, no global mutable state |
| UI-S3-008 | Cache/release policy | close/reopen/release state is deterministic |

No additional Stage 3 items are allowed until these generic records are reviewed
and accepted.

### Stage 4: Web Editor Foundation

Goal: start the Web Editor as tooling around runtime files.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| UI-EW-001 | Define UI file schema | schema version, node tree, layout, style refs, resource refs |
| UI-EW-002 | Local editor service backend | C++ only for file/schema/cook/preview bridge; no frontend panel/model ownership |
| UI-EW-003 | Web frontend workspace | Next/Vite-style TS/React app with hierarchy, inspector, canvas, resource panel, hot reload |
| UI-EW-004 | Runtime preview protocol | WebSocket/IPC contract to engine preview process; no C++ editor shell dependency |
| UI-EW-005 | Validator integration endpoint | schema, duplicate ID, resource ref, overflow reports exposed through the service API |
| UI-EW-006 | Component template catalog | JSON/TS/data-file catalog consumed by Web frontend and runtime validator, not compiled C++ |
| UI-EW-007 | Style/theme editing | Web edits data files; runtime owns interpretation |
| UI-EW-008 | State preview UI | Web controls state input; engine path validates output |

## 6. Hard Blocks

These are blocking violations:

- adding game-specific UI names, flows, or data into UI Framework code
- using old game windows as engine validation samples
- recreating the old game-window matrix
- adding native app/editor targets as engine capabilities
- keeping a native editor shell fallback
- adding C++ targets for Web editor shell, panel model, template catalog,
  style/theme editing, or state-preview workflow
- moving window lifecycle into UI Core
- making UI runtime depend on editor-only modules
- making Web Editor logic part of shipped runtime
- requiring CMake/C++ rebuild for normal editor UI, template, theme, or state
  preview changes
- making config tables mandatory for current UI Framework work
- implementing Grid/List by materializing all items
- allowing dynamic atlas repack during paint hot path

## 7. Verification Commands

Required after each UI cleanup or runtime-framework change:

```powershell
git diff --check
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate --output-on-failure
```

Focused UI runtime route:

```powershell
ctest --preset windows-fast-gate -R "^(BaseUiController_|UiPanelRegistry_|UiRuntime_ManagerLayerModel_|UiRuntime_ManagerPanelMap_|UiRuntime_ManagerPopupStack_|UiRuntime_ManagerFullscreenStack_|UiRuntime_PanelParameter_)" --output-on-failure
```

## 8. Completion Definition

The UI Framework first round is complete only when:

- UI Core node/layout/input/draw records pass focused tests
- component library first slices pass focused tests
- Grid/List virtualization is bounded and tested
- invalidation/rebuild counters prove local subtree rebuild behavior
- static atlas path is primary and dynamic atlas path is bounded
- UIManager runtime records pass lifecycle, layer, map, stack, args, and release
  tests
- Web Editor foundation is specified as Web-only tooling
- no native app/editor engine capability remains
- no game-specific window or game-specific sample remains in engine scope

## 9. Team Handoff Rule

When creating landing tasks, use generic UI capability names only.

Allowed examples:

- implement UI Core layout container
- implement Button state records
- implement GridView virtualization counters
- implement UIManager popup stack
- implement UI schema validator
- implement Web Editor hierarchy panel in the Web frontend workspace

Not allowed:

- implement a named old game window
- validate UI Framework using a game-specific panel
- build native app editor shell
- keep a fallback editor path outside Web
- add a C++ Web editor shell/panel/template/theme/state-preview target
