# YuEngine UI Framework and Runtime Editor Plan

Status: corrective replacement
Requested: 2026-06-20
Owner: Architecture
Scope: UI Framework, UI Core, UI Component Library, UIManager runtime framework,
runtime editor host direction

Shared preview-host gate: `docs/gates/EDITOR_GATE_001_RUNTIME_PREVIEW_HOST.md`
Shared preview-host plan: `docs/YUENGINE_EDITOR_RUNTIME_PREVIEW_HOST_PLAN.md`
RAV5 runtime UI document slice: `docs/YUENGINE_UI_EDITOR_RAV5_RUNTIME_DOCUMENT_PREVIEW_FEEDBACK.md`
RAV6 design/inspector workflow slice: `docs/YUENGINE_UI_EDITOR_RAV6_DESIGN_INSPECTOR_WORKFLOW.md`
RAV6 authored editor package/run smoke: `docs/YUENGINE_EDITOR_PACKAGE_RUN_RAV6_AUTHORED_DATA_SMOKE.md`
RAV5 review gate: `docs/YUENGINE_RAV5_EDITOR_SURFACE_REVIEW_EVIDENCE.md`
RAV6 review gate: `docs/YUENGINE_RAV6_EDITOR_MVP_WORKFLOW_REVIEW_EVIDENCE.md`

## 1. Correction

This document replaces the earlier UI plan.

The corrected direction is:

```text
YuEngine UI work = generic UI runtime framework + generic UIManager framework.
UI Editor direction = runtime UI data + native/engine preview host.
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
- making editor-only code part of shipped game runtime
- making the game runtime depend on editor-only code
- forbidden restoration of deprecated Web Editor, deprecated Web frontend workspace, React/Vite workflow, or
  HTML/CSS/browser-only preview as active editor direction
- forbidden acceptance of HTML/CSS, form UI, 2D canvas sketches, or static screenshots as
  core UI editor preview

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

## 3. UI Runtime Editor Direction

The UI Editor direction is native/engine runtime preview host over runtime UI
data. Web is no longer a YuEngine editor direction. The earlier Web Editor,
deprecated Web frontend workspace, TypeScript/React/Vite, HTML/CSS, and browser-only canvas
approach is retained only as a deprecated anti-pattern.

The editor architecture is:

```text
runtime UI data files
-> local editor service / validation bridge
-> file/schema/validator/cook commands
-> native or engine runtime preview host
-> engine UI runtime frame/status/diagnostics
```

The editor surface may be native or engine-hosted, but its authority comes from
runtime UI records and preview-host output. Editing hierarchy panels, inspector
controls, templates, themes, state-preview UI, drag/drop, shortcuts, and
workflow must write runtime data that can validate, cook, preview, and package.

HTML/CSS output, administrative-form styling, React/Vite pages, or a
browser-only canvas mock is not acceptable core UI preview.

C++ is allowed for stable runtime/editor-host contracts:

- stable runtime schema validation
- local file/service bridge
- cook/asset validation bridge
- runtime preview protocol and preview-host control

C++ must not reintroduce a deprecated Web editor shell, browser panel model, React/Vite
template catalog, HTML/CSS style/theme workflow, or browser-only state-preview
path.

### 3.1 Editor Surface

Owns:

- hierarchy panel
- inspector
- property controls
- drag and drop
- layout viewport interaction
- resource picker UI
- style/theme editing UI
- state preview UI
- validation result view
- keyboard shortcuts and editor workflow

The editor surface edits runtime data files. It must not become a game runtime
dependency, and it must not be implemented as a deprecated Web shell or browser-only
preview path.

### 3.2 Local Editor Service

Owns:

- reading and writing UI layout files
- schema validation
- version migration
- resource reference validation
- style/theme file validation
- invoking cook/pack validation
- starting and controlling preview sessions
- exposing a local API for the native/engine editor host

The service is tooling. It is not part of shipped runtime.
It may be implemented in C++ where it directly touches YuEngine runtime
contracts, but it must expose stable editor-host APIs instead of encoding
browser panels or deprecated Web catalogs in C++.

### 3.3 Engine Preview

Preview must use the real UI runtime path. It is not optional for usable-editor
acceptance.

Allowed preview modes:

- engine preview process renders runtime data and returns image/frame/status
- headless validation returns layout, hit-test, rebuild, batch, and resource
  diagnostics
- editor viewport overlays may draw selection boxes and guides

Not allowed:

- reimplementing runtime layout rules in deprecated Web code
- making deprecated Web DOM the game UI runtime
- forbidden acceptance of an HTML/CSS-styled page as the UI editor preview
- adding a deprecated Web editor compatibility fallback

### 3.4 Layout Coordinate Boundary

This is a required boundary because HTML/CSS and engine UI layout do not share
the same model.

HTML/CSS must not be used as the editor preview or layout authority. CSS
`border-box`, DOM `left/top`, flex, grid, browser padding, and browser border
behavior are not YuEngine runtime layout semantics.

YuEngine runtime layout data is the source of truth:

- parent-relative normalized anchors
- offsets
- pivot
- margin and padding
- size policy
- DPI and safe-area inputs
- engine runtime rect and content rect

Any editor viewport must render runtime data through an explicit coordinate adapter.
The adapter owns conversion between engine runtime coordinates and native/editor
viewport overlay coordinates. In particular, engine runtime rects use the engine
layout coordinate convention, while editor overlays use a viewport-local display
space owned by the native/editor host.

Required display conversion shape:

```text
editorViewportX = runtimeX * scale + panX
editorViewportY = (viewportHeight - runtimeY - runtimeHeight) * scale + panY
editorViewportWidth = runtimeWidth * scale
editorViewportHeight = runtimeHeight * scale
```

The inverse conversion is required for drag and resize operations. Dragging a
node updates runtime layout fields according to the current anchor preset; it
must not infer anchor semantics from display rect fields alone. CSS
`left/top/width/height` is a forbidden historical Web representation, not an
editor data contract.

Required rules:

- the inspector edits anchors, offsets, pivot, margin, padding, size policy,
  DPI, and safe-area data as runtime fields
- anchor preset changes are explicit user actions
- fixed-anchor drags update offsets
- stretch-edge drags update min/max offsets
- selection boxes, resize handles, guides, and borders are editor overlays
  and never part of runtime rect data
- browser fast preview and JavaScript/TypeScript rect solvers are deprecated and
  must not be used as acceptance
- runtime/headless preview is authoritative when any editor overlay disagrees
  with engine layout

The planned editor is not complete until the coordinate adapter and
runtime-layout parity tests exist.

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
| Deprecated Web editor attempt | historical anti-pattern for browser shell and DOM preview limits | active Web editor route, React/Vite workflow, HTML/CSS preview, or runtime layout semantics |
| YuEngine `UiRectTransform` and runtime rect math | authoritative anchors, offsets, pivot, margin, padding, DPI, safe-area, engine rect/content rect semantics | CSS box model, browser layout policy, DOM coordinate semantics |
| YuEngine editor preview host | engine-rendered viewport/frame/status/diagnostics for UI runtime data | browser-only visual fake, static screenshot, or HTML admin surface |

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
- document Deprecated Web Editor as rejected historical direction
- document native/engine preview host as the only editor direction
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

### Stage 4: Runtime Editor Foundation And Preview

Goal: start UI editor tooling around runtime files and connect it to the engine
UI runtime preview path without Web as an editor direction.

Work items:

| ID | Work item | Acceptance |
| --- | --- | --- |
| UI-EW-001 | Define UI file schema | schema version, node tree, layout, style refs, resource refs |
| UI-EW-002 | Local editor service skeleton | load/save/validate API, no runtime dependency |
| UI-EW-003 | Native/engine editor surface | hierarchy, inspector, resource panel, command routing; surface does not pretend to be runtime preview |
| UI-EW-004 | Runtime preview protocol | local editor-host contract to engine preview host |
| UI-EW-005 | Validator integration | schema, duplicate ID, resource ref, overflow reports |
| UI-EW-006 | Component template data | templates are data files consumed by editor tooling and runtime validator |
| UI-EW-007 | Style/theme editing | editor edits data; runtime owns interpretation |
| UI-EW-008 | State preview | editor controls state input; engine path validates output |
| UI-EW-009 | Layout coordinate spec | documents engine runtime rects, native/editor viewport overlay rects, y-axis conversion, pan/zoom, DPI, safe-area, border/overlay exclusion |
| UI-EW-010 | Coordinate adapter | converts engine rects to editor viewport rects and back; drag/resize updates runtime offsets instead of display-only fields |
| UI-EW-011 | Runtime layout parity tests | editor overlay solver matches YuEngine runtime golden fixtures; engine preview result is authoritative |
| UI-EW-012 | Engine UI viewport | UI layout/style/resource data renders through YuEngine UI runtime, not HTML/CSS |
| UI-EW-013 | UI resource preview | sprites, textures, fonts, atlases, materials, and missing-resource diagnostics are visible through engine preview |

## 6. Hard Blocks

These are blocking violations:

- adding game-specific UI names, flows, or data into UI Framework code
- using old game windows as engine validation samples
- recreating the old game-window matrix
- adding native app/editor targets as engine capabilities
- keeping a native editor shell fallback
- adding C++ targets for deprecated Web editor shell, panel model, template catalog,
  style/theme editing, or state-preview workflow
- moving window lifecycle into UI Core
- making UI runtime depend on editor-only modules
- making deprecated Web Editor logic part of shipped runtime
- requiring CMake/C++ rebuild for normal editor UI, template, theme, or state
  preview changes
- accepting deprecated Web shell, HTML/CSS, 2D canvas sketches, or static screenshots as
  core UI editor preview
- calling UI Editor usable before engine UI runtime viewport/frame/diagnostics
  are available
- using HTML/CSS border-box as runtime layout semantics
- exporting DOM `left/top/width/height` as authoritative runtime layout without
  anchor/offset/pivot conversion
- letting deprecated Web preview disagree with the runtime layout solver without a failing
  parity test
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

Deprecated browser-editor routes are not active acceptance routes. They may
appear only as residual cleanup targets until the old source/test surface is
removed.

## 8. Completion Definition

The UI Framework first round is complete only when:

- UI Core node/layout/input/draw records pass focused tests
- component library first slices pass focused tests
- Grid/List virtualization is bounded and tested
- invalidation/rebuild counters prove local subtree rebuild behavior
- static atlas path is primary and dynamic atlas path is bounded
- UIManager runtime records pass lifecycle, layer, map, stack, args, and release
  tests
- Web Editor foundation is tombstoned as a rejected direction
- UI Editor has an engine UI runtime viewport/frame/status path through the
  shared preview host
- sprites, textures, fonts, atlas refs, style refs, and missing-resource
  diagnostics are visible through engine preview
- UI Editor layout uses a documented coordinate adapter instead of treating
  HTML/CSS border-box as runtime layout
- editor overlay preview is covered by runtime-layout parity tests against the
  YuEngine authoritative layout solver
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
- implement UI editor hierarchy panel in the native/engine editor host

Not allowed:

- implement a named old game window
- validate UI Framework using a game-specific panel
- build native app editor shell
- keep a deprecated Web fallback editor path
- add a deprecated C++ Web editor shell/panel/template/theme/state-preview target
