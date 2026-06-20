# YuEngine UI Framework and UI Editor Execution Plan

Status: handoff plan for landing team
Owner: 八云紫, 总架构师
Requested: 2026-06-20
Current observed code checkpoint: `1e9ff3d`
Scope: UI framework, UI editor, UI runtime mechanisms, YuFramework UI migration

## 1. Executive Decision

YuEngine should build a narrow native UI stack for the known product team. It
should not copy Unity UGUI, Unreal UMG, Unity Editor, or Unreal Editor as a
public ecosystem.

The execution direction is:

```text
Stage 1: UI Core layout and nodes
Stage 2: UI component library plus required low-level mechanisms
Stage 3: YuFramework UI framework migration around UIManager + BaseUI/UICtrlBase
```

The UI editor direction is:

```text
First: layout editor, previewer, validator
Then: component templates, state preview, performance diagnostics
Never: runtime dependency on editor-only classes
```

This plan is reference-driven. It should not be implemented as an invented UI
stack with no prior art. Each landing task should state which reference it is
borrowing from, what is intentionally not copied, and what acceptance evidence
proves the borrowed mechanism fits YuEngine.

Naming rule:

- The engine remains `YuEngine`.
- New UI planning should use plain names such as `UI Framework`, `UI Core`,
  `UI Editor`, and `UI Component Library`.
- Existing `Yu*` code names do not need churn-only renames.

Key architectural correction:

- Low-level UI Framework nodes must not own interface/window lifecycle.
- Interface lifecycle belongs to `UIManager + BaseUI` semantics. The current
  source uses `UICtrlBase`; the migration can preserve or rename that at the
  project-runtime layer, not in low-level UI Core.

## Reference Inputs and Borrowing Rules

This plan uses three reference families:

1. Current YuFramework UI code as the product-behavior reference.
2. Unity UGUI / TMP as the component and workflow reference.
3. Unreal Slate / UMG as the invalidation, retainer, and UI-performance
   reference.

The goal is to borrow proven responsibilities and failure lessons, not to clone
any one system.

### YuFramework References

Local source path:

```text
C:\Steam\steamapps\common\TouhouNewWorld\yufulao\framework
```

| Reference | Local Path | Borrow | Do Not Copy |
| --- | --- | --- | --- |
| UI manager semantics | `Scripts/Core/Manager/UIManager/UIManager.cs` | active panels, loaded panels, popup/fullscreen stacks, layer roots, sorting/order, open/close/cache behavior | Unity `Transform`, `Canvas`, `GraphicRaycaster`, Addressables/TextAsset loading details |
| UI lifecycle semantics | `Scripts/Core/Manager/UIManager/UICtrlBase.cs` | `OnInit -> BindEvent -> OnOpen -> OnClose -> OnClear` as project UI runtime lifecycle | putting lifecycle callbacks into UI Core nodes |
| Layer model | `Scripts/Core/Manager/UIManager/UILayer.cs`, `Def/DefUILayer.cs` | fullscreen/popup/game/loading/debug layer concepts | Unity sorting-order implementation as-is |
| Panel metadata | `Scripts/Core/Manager/UIManager/UIPanelInfo.cs` | panel ID, panel state, layer and loaded/active bookkeeping | immediate dependency on config tables in Stage 1/2/3 |
| Window architecture | `Scripts/UI/Windows/*/*Ctrl.cs`, `*View.cs`, `*Model.cs` | Ctrl/View/Model split and real product-window behavior | wholesale migration of every window before representative samples pass |
| Grid/List behavior | `Scripts/UI/Component/FancyScrollViewExtensions/Base/GridViewBase.cs` | `FancyScrollView`-style grouped cells, pooling, scroll-to-index, selection, visible-cell reuse | a non-virtualized list that creates every item |
| Text usage | `Scripts/UI/Windows/**` using `TextMeshProUGUI` / `LocalizeTextMeshProUGUI` | TMP-like font, localization, overflow, link/action behavior | full TMP feature parity in the first slice |
| Slider usage | `Scripts/UI/Windows/**` using `Slider` | range/value/fill/handle interaction patterns | Unity event system dependency |

### Unity References

Unity is the reference for familiar authoring concepts and component behavior.

Official sources:

- uGUI Canvas:
  <https://docs.unity3d.com/Packages/com.unity.ugui%402.0/manual/class-Canvas.html>
- uGUI Basic Layout:
  <https://docs.unity3d.com/Packages/com.unity.ugui%401.0/manual/UIBasicLayout.html>
- uGUI Interaction Components:
  <https://docs.unity3d.com/Packages/com.unity.ugui%402.6/manual/UIInteractionComponents.html>
- uGUI Scroll Rect:
  <https://docs.unity3d.com/Packages/com.unity.ugui%401.0/manual/script-ScrollRect.html>
- uGUI Slider:
  <https://docs.unity3d.com/Packages/com.unity.ugui%402.0/manual/script-Slider.html>
- Sprite Atlas:
  <https://docs.unity3d.com/6000.4/Documentation/Manual/sprite/atlas/atlas-introduction.html>
- TextMeshPro Font Assets:
  <https://docs.unity3d.com/Packages/com.unity.textmeshpro%404.0/manual/FontAssets.html>
- TextMeshPro Sprite Assets:
  <https://docs.unity3d.com/Packages/com.unity.textmeshpro%404.0/manual/Sprites.html>
- UI Toolkit / UI Builder as editor-authoring reference only:
  <https://docs.unity3d.com/6000.4/Documentation/Manual/UIElements.html>

Borrow from Unity:

- Rect/anchor/pivot mental model.
- Canvas-like abstract UI space, but not Canvas as the only batching boundary.
- Familiar Image/Button/Slider/ScrollRect component semantics.
- Selectable navigation ideas for keyboard/gamepad.
- SpriteAtlas workflow and one-atlas-reduces-draw-call lesson.
- TMP font asset, glyph atlas, fallback, sprite-in-text, outline/shadow ideas.
- UI Builder-like hierarchy/inspector/preview workflow for the UI Editor.

Do not copy from Unity:

- Unity `GameObject` / `MonoBehaviour` / `RectTransform` as runtime types.
- Canvas rebuild behavior that makes small changes trigger large rebuilds.
- Unity EventSystem dependency.
- Prefab/Addressables/EasySave/TextAsset implementation details.
- Full UI Toolkit ecosystem or USS/UXML feature breadth in the first UI plan.

### Unreal References

Unreal is the reference for UI invalidation and performance discipline.

Official sources:

- Slate/UMG invalidation:
  <https://dev.epicgames.com/documentation/unreal-engine/invalidation-in-slate-and-umg-for-unreal-engine>
- UMG optimization guidelines:
  <https://dev.epicgames.com/documentation/unreal-engine/optimization-guidelines-for-umg-in-unreal-engine>
- Invalidation Box:
  <https://dev.epicgames.com/documentation/unreal-engine/using-the-invalidation-box-for-umg-in-unreal-engine>
- Retainer Box API:
  <https://dev.epicgames.com/documentation/unreal-engine/API/Runtime/UMG/URetainerBox>

Borrow from Unreal:

- Invalidation categories and subtree invalidation.
- Separating layout/prepass, paint, and hit-test update work.
- Caching stable widget geometry/paint output.
- Invalidation Box idea for cached widget subtrees.
- Retainer/Retainer Box idea for optional render-to-texture caching when draw
  call reduction justifies memory cost.
- Performance diagnostics around redraw/rebuild behavior.

Do not copy from Unreal:

- Full Slate API surface.
- Full UMG designer/editor ecosystem.
- Blueprint/UI animation ecosystem.
- Mandatory render-target retainer path for every UI.
- Unreal naming and module structure.

### Borrowing Matrix

| YuEngine Area | Primary Reference | Secondary Reference | Decision Rule |
| --- | --- | --- | --- |
| Panel lifecycle | YuFramework `UIManager` + `UICtrlBase` | Unity window/controller habits | Preserve project semantics above UI Core |
| Layout math | Unity Rect/anchor/pivot model | UI Toolkit box model | Implement only the subset needed for product layouts |
| Component behavior | Unity UGUI + current windows | YuFramework wrappers/usages | Keep familiar states/events, remove Unity dependencies |
| Text | TMP + current localized text usage | Unreal invalidation for text dirty | Start with font/glyph/fallback/localization, not full TMP clone |
| Grid/List | YuFramework `FancyScrollView` grid | Unity ScrollRect/ListView | Virtualization is mandatory from first GridView slice |
| Rebuild | Unreal Slate/UMG invalidation | Unity Canvas rebuild pain points | Prefer subtree invalidation and cached paint/layout |
| Atlas | Unity SpriteAtlas + TMP atlas | RenderCore batching needs | Static atlas first, dynamic atlas only with safe-point updates |
| Batching | RenderCore requirements | Unity SpriteAtlas / Unreal element batching ideas | Batch by state keys; do not expose backend details to components |
| Editor workflow | Unity UI Builder-like workflow | Dear ImGui tooling shell | Editor generates/validates runtime data; preview uses real runtime path |

### Reference Use in Landing Tasks

Each implementation task should include:

```text
Reference used:
Borrowed behavior:
Not copied:
YuEngine-specific acceptance:
```

This prevents the team from treating the plan as final class design. The plan
sets scope and borrowing direction; implementation gates decide exact APIs,
data structures, and performance thresholds.

## 2. Product Scope

### 2.1 What YuEngine Needs

YuEngine needs a UI stack that can ship a small commercial game:

- deterministic runtime layout
- stable node hierarchy
- input hit-test and focus routing
- predictable partial rebuilds
- efficient batching
- static and dynamic atlas support
- text/font handling inspired by TMP and the current `UIText`
- images, buttons, sliders, and virtualized grid/list views
- an internal editor that can author, preview, validate, and diagnose UI layouts
- migration path for the existing YuFramework UI layer

### 2.2 What YuEngine Does Not Need Now

YuEngine does not need:

- a public UI plugin ABI
- a public marketplace-ready editor extension ecosystem
- a full Unity Editor clone
- a full Unreal Editor clone
- a full UMG/Slate clone
- old unpacked game package compatibility
- config-table integration inside the first UI planning window
- complete visual scripting or animation authoring in the UI editor
- a universal UI system for arbitrary products

## 3. Core Principles

### 3.1 Runtime Data Is the Source of Truth

The runtime UI layout/data format is the source of truth. The UI editor is only
an authoring, preview, validation, and diagnostics surface.

The runtime must be able to load a UI layout without editor-only code.

### 3.2 Retained-Mode Game UI

Game UI should be retained-mode:

- stable nodes
- stable widget/component instances
- cached layout results
- cached paint commands where possible
- controlled invalidation
- predictable input focus and navigation

Dear ImGui is appropriate for internal tools and editor shell UI. It should not
be the shipped game UI runtime.

### 3.3 Lifecycle Belongs Above UI Core

Low-level UI Core owns:

- node tree
- parent/child relation
- rect/layout math
- dirty flags
- hit-test data
- draw-element generation

Low-level UI Core must not own:

- `OnInit`
- `OnOpen`
- `OnClose`
- `OnClear`
- panel stack
- fullscreen stack
- popup ordering policy
- loading policy
- release policy

Those belong to the project UI runtime layer based on `UIManager +
BaseUI/UICtrlBase`.

### 3.4 Components Must Share One Substrate

Text, Image, Button, Slider, and GridView/List must use the same:

- layout system
- input routing
- style/resource reference model
- draw-element generation path
- invalidation model
- batching model
- cook/validator pipeline

The component library should not become separate special-case widgets.

### 3.5 Config Tables Are Deferred

The existing Unity `UIManager` loads panel metadata through config tables. That
idea is useful, but config-table integration is not part of the first UI
framework landing plan.

During UI migration, use a small explicit panel registry/test manifest. Later
L2/GameFlow/business work can replace it with the final config-table service.

## 4. Layer Model

### 4.1 UI Core

UI Core is the low-level runtime layer.

Responsibilities:

- UI node tree
- rect model
- anchor/pivot/margin/padding/size policy
- basic layout containers
- transform propagation
- clipping bounds
- hit-test path
- dirty classification
- draw-element intermediate representation
- minimal bridge to RenderCore

Not responsibilities:

- opening windows
- closing windows
- popup stack
- fullscreen navigation
- panel async loading
- config-driven panel registry
- business event binding
- gameplay state

### 4.2 UI Component Library

The component library owns reusable UI controls.

Initial components:

- Text
- Image
- Button
- Slider
- GridView/List

Later components:

- Toggle
- Progress
- ScrollView
- Dialog
- Toast
- HUD slot
- Tab
- Dropdown

The first GridView/List implementation should follow the project pattern based
on `FancyScrollView`:

```text
GridViewBase<TContext> : FancyGridView<object, TContext>
GridView : GridViewBase<GridContext>
BtnGridView : GridViewBase<BtnGridContext>
```

The important idea is virtualized groups/cells, not a literal dependency on the
Unity package.

### 4.3 Project UI Runtime

Project UI Runtime migrates the existing YuFramework semantics.

Reference source concepts:

- `UIManager`
- `UICtrlBase` / intended `BaseUI`
- `UILayer`
- `UIPanelInfo`
- fullscreen stack
- popup stack
- loaded-panel cache
- active-panel map
- layer sorting/order
- async open/load behavior

Responsibilities:

- panel registry
- panel loading
- panel open/close
- layer roots
- fullscreen/popup/game/loading/debug layers
- stack navigation
- panel parameter passing
- panel cache/release policy
- close-all policy
- window-level event binding

This layer owns UI lifecycle.

### 4.4 UI Rendering Substrate

The UI Rendering Substrate is the low-level mechanism below or beside the
component library.

Responsibilities:

- draw-element list
- UI batching
- static atlas access
- dynamic atlas updates
- font atlas access
- scissor/clip handling
- optional render cache/retainer targets
- UI-to-RenderCore submission

It must not expose D3D11 backend details to UI components.

### 4.5 UI Editor

UI Editor is an internal authoring surface.

Responsibilities:

- layout authoring
- hierarchy editing
- property editing
- multi-resolution preview
- safe-area preview
- runtime-path preview
- resource validation
- localization overflow preview
- component templates in later stages
- UI performance diagnostics

It can use Dear ImGui for the editor shell, docking, menus, tree views, and
inspector. The preview viewport must use the real UI runtime path.

## 5. Stage 0: Preparation

Stage 0 exists to prevent immediate implementation drift.

### 5.1 Outputs

- UI terminology note in docs or first implementation gate.
- Decision that lifecycle is excluded from UI Core.
- Migration inventory for current YuFramework UI code.
- Initial data-format decision for UI layout assets.
- Initial acceptance sample list.

### 5.2 Migration Inventory Categories

Classify current YuFramework UI code into these categories:

| Category | Meaning | Example |
| --- | --- | --- |
| A | Runtime lifecycle semantics to preserve | `UIManager`, `UICtrlBase`, layer stacks |
| B | Component behavior to port | `UIText`, `UIButton`, slider, image wrappers |
| C | List/Grid behavior to preserve | `FancyScrollView`-derived GridView |
| D | Layout/window content to migrate later | Bag, Store, Dialogue, HUD windows |
| E | Unity-only implementation detail | Canvas, RectTransform, GraphicRaycaster, prefab loading |
| F | Editor-only or plugin-only detail | Odin editors, Unity custom inspectors |

### 5.3 Stage 0 Done Definition

Stage 0 is done when:

- the team agrees on the stage split
- UI Core has no window lifecycle ownership
- config-table integration is explicitly deferred
- the first three representative windows are named
- the initial layout data format is chosen
- the implementation team has a backlog derived from this plan

Recommended representative windows:

- one simple popup/toast window
- one fullscreen window
- one inventory/store-style GridView window

## 6. Stage 1: UI Core Layout and Nodes

Stage 1 builds the runtime foundation without the component library.

### 6.1 Stage 1 Goals

Stage 1 proves:

```text
layout data -> UI node tree -> layout pass -> hit-test path -> draw elements -> RenderCore submission
```

Stage 1 must not try to migrate all existing UI.

### 6.2 Stage 1 Required Capabilities

#### UI Node Tree

Required:

- create/destroy node
- parent/child attach and detach
- stable node ID
- local rect data
- world rect cache
- visibility flag
- enabled/hit-testable flag
- layer/order value

Excluded:

- panel lifecycle
- asset async loading policy
- gameplay event binding

#### Rect and Layout Model

Required:

- anchor min/max
- pivot
- offset/min/max or position/size representation
- margin
- padding
- min/preferred/flexible size policy
- DPI scale hook
- safe-area hook

#### Basic Layout Containers

Required first:

- Absolute
- Stack
- Grid
- Overlay
- Scroll viewport structure

Deferred:

- complex auto layout
- content-size fitter clone
- constraint solver
- rich responsive layout rules

#### Dirty Classification

Required dirty domains:

- layout dirty
- paint dirty
- transform dirty
- hit-test dirty
- text dirty placeholder for Stage 2

Rules:

- text change must not force full window rebuild unless layout-affecting
- hover/pressed change must not force layout rebuild
- scroll offset must not rebuild the full list
- atlas page change triggers paint dirty, not layout dirty

#### Hit-Test and Input Path

Required:

- point hit-test
- z/layer order
- disabled/non-hit-testable nodes
- clipped hit-test
- hovered/pressed routing primitive
- keyboard/gamepad focus placeholder

Deferred:

- full navigation graph authoring
- text input/IME
- drag-and-drop authoring

#### Draw-Element Intermediate Representation

Required:

- rect primitive
- textured quad primitive placeholder
- text primitive placeholder
- clip/scissor state
- layer/order
- material/style key
- texture/atlas key

UI Core generates draw elements. It does not directly submit D3D11 calls.

### 6.3 Stage 1 Backlog

| ID | Work Item | Depends On | Acceptance |
| --- | --- | --- | --- |
| UI-S0-001 | Confirm UI naming and stage boundary | none | Doc/gate says no forced `Yu` prefix for new UI names and no lifecycle in UI Core |
| UI-S0-002 | Inventory YuFramework UI migration categories | UI-S0-001 | `UIManager`, `UICtrlBase`, `GridViewBase`, and representative windows classified |
| UI-S0-003 | Choose initial UI layout asset format | UI-S0-001 | Format supports hierarchy, rect/layout fields, resource refs, and stable IDs |
| UI-S1-001 | Implement UI node identity/tree prototype | UI-S0-003 | Tests cover create/destroy/attach/detach/order |
| UI-S1-002 | Implement rect/anchor/pivot math | UI-S1-001 | Tests cover parent resize, pivot, margin, padding, DPI placeholder |
| UI-S1-003 | Implement basic layout containers | UI-S1-002 | Absolute/Stack/Grid/Overlay fixture layouts match expected rects |
| UI-S1-004 | Implement dirty classification | UI-S1-003 | Tests prove paint-only change does not trigger layout rebuild |
| UI-S1-005 | Implement hit-test path | UI-S1-003 | Tests cover layer order, clipping, disabled nodes |
| UI-S1-006 | Define draw-element IR | UI-S1-004 | Layout fixture emits deterministic draw-element list |
| UI-S1-007 | Add minimal RenderCore bridge | UI-S1-006 | UI fixture submits draw elements through RenderCore without backend leakage |
| UI-S1-008 | Add UI Core smoke sample | UI-S1-007 | Sample loads layout, renders simple window, and reports pass/fail |
| UI-S1-009 | Add Stage 1 validation doc | UI-S1-008 | Commands and acceptance evidence documented |

### 6.4 Stage 1 Done Definition

Stage 1 is done when:

- UI layout can be loaded without editor code
- node tree builds deterministically
- layout output is testable
- hit-test output is testable
- draw-element output is deterministic
- RenderCore bridge runs in fast gate or a UI gate
- no panel lifecycle API is in UI Core
- no config-table dependency exists

## 7. Stage 2: Component Library and Low-Level Mechanisms

Stage 2 builds the first component set and locks the mechanisms they depend on.

### 7.1 Stage 2 Goals

Stage 2 proves:

```text
component data -> layout -> component state -> invalidation -> batching -> render
```

It also proves virtualized lists are first-class.

### 7.2 Mechanism Decisions

#### Rebuild and Invalidation

Use a UE/Slate-like invalidation direction rather than a Unity UGUI-style large
Canvas rebuild direction.

Required:

- subtree invalidation
- cached layout results
- cached paint commands where safe
- explicit layout prepass
- explicit paint pass
- explicit hit-test path update
- rebuild counters for diagnostics

Reference concepts:

- Unreal Slate widget invalidation
- UMG Invalidation Panel / Retainer Box ideas
- Unity UGUI Canvas rebuild only as a cautionary reference

#### Batching

Batch draw elements by:

- material/shader key
- texture/atlas page
- font atlas page
- blend state
- clip/scissor state
- render layer/order

First-slice clipping:

- scissor/rect clip

Deferred:

- complex masks
- stencil-heavy nesting
- custom per-widget materials beyond simple style keys

#### Static Atlas

Static atlas is primary for product UI.

Required:

- cook-time atlas pack
- deterministic atlas page IDs
- resource reference validation
- sprite UV generation
- nine-slice metadata path

Initial contents:

- common icons
- window backgrounds
- button states
- HUD sprites
- nine-slice panels

#### Dynamic Atlas

Dynamic atlas is auxiliary.

Allowed:

- font glyph cache
- small temporary sprites
- mod/hot-load or debug-only assets later

Rules:

- no pack/repack inside paint hot path
- atlas page has fixed capacity
- referenced entries can be pinned
- eviction has explicit status
- atlas move/update causes paint dirty, not layout dirty

#### Text and Font

Text should borrow ideas from TMP and the existing `UIText`.

Required:

- font asset
- glyph atlas
- fallback font chain
- size/style key
- alignment
- wrapping
- outline/shadow first-slice
- localization length stress test

Deferred:

- full TMP feature parity
- complex bidirectional shaping unless product requires it
- complete rich text grammar

#### GridView/List Virtualization

GridView/List must follow the current project direction based on
`FancyScrollView`.

Required:

- pooled cells
- grouped grid cells
- visible-range update
- buffer item count
- fixed-size grid first
- data update dirtying only affected cells
- scroll changes update mapping, not full list allocation

Deferred:

- variable-size item list
- nested virtualized lists
- complex drag sort

### 7.3 Component Scope

#### Text

References:

- TMP for font/glyph atlas/fallback concepts
- current `UIText` for project behavior

Stage 2 requirements:

- plain text render
- alignment
- wrapping
- overflow behavior
- localize-key placeholder support
- outline/shadow first-slice
- font fallback
- text dirty vs layout dirty distinction

#### Image

References:

- Unity Image
- current project image wrappers

Stage 2 requirements:

- sprite draw
- tint
- alpha
- nine-slice
- fill placeholder or first slice
- atlas validation

#### Button

References:

- Unity Button
- current `UIButton`

Stage 2 requirements:

- normal/hover/pressed/disabled/selected state
- pointer and keyboard/gamepad activation path
- visual state update
- sound/event hook without owning audio policy
- no layout rebuild on hover/press

#### Slider

References:

- Unity Slider
- current project slider wrappers

Stage 2 requirements:

- value/range
- fill
- handle
- pointer capture
- keyboard/gamepad adjustment placeholder
- value-change event

#### GridView/List

References:

- `GridViewBase<TContext> : FancyGridView<object, TContext>`
- `GridView`
- `BtnGridView`
- project windows that derive from `GridView`

Stage 2 requirements:

- fixed-size grid
- pooled cells
- cell init/update callbacks
- scroll-to-index
- selection
- auto-scroll first-slice
- dirty affected cell only

### 7.4 Stage 2 Backlog

| ID | Work Item | Depends On | Acceptance |
| --- | --- | --- | --- |
| UI-S2-001 | Define invalidation model | UI-S1-009 | Dirty/rebuild rules documented and tested |
| UI-S2-002 | Implement layout/paint cache counters | UI-S2-001 | Tests expose layout rebuild count and paint rebuild count |
| UI-S2-003 | Implement draw-element batching prototype | UI-S1-007 | Deterministic batch count for fixture UI |
| UI-S2-004 | Implement static atlas metadata path | UI-S1-006 | Sprite refs resolve to atlas page/UV or explicit missing status |
| UI-S2-005 | Implement dynamic atlas first slice | UI-S2-004 | Pack occurs outside paint path; capacity/overflow tested |
| UI-S2-006 | Implement font asset/glyph atlas path | UI-S2-005 | Text fixture resolves glyphs with fallback and explicit missing glyph status |
| UI-S2-007 | Implement Text component | UI-S2-006 | Alignment/wrapping/outline-shadow/localization stress fixture passes |
| UI-S2-008 | Implement Image component | UI-S2-004 | Sprite/tint/nine-slice fixture passes |
| UI-S2-009 | Implement Button component | UI-S2-007, UI-S2-008 | Hover/press/disabled/selected states do not trigger layout rebuild |
| UI-S2-010 | Implement Slider component | UI-S2-009 | Value/fill/handle/input capture fixture passes |
| UI-S2-011 | Define FancyScrollView-based GridView semantics | UI-S0-002 | Cell/group/pool/selection behavior mapped from project code |
| UI-S2-012 | Implement GridView/List virtualization | UI-S2-011 | Large list creates only visible + buffer cells |
| UI-S2-013 | Add component library smoke sample | UI-S2-012 | Sample uses Text/Image/Button/Slider/GridView in one window |
| UI-S2-014 | Add component performance diagnostics | UI-S2-013 | Sample reports draw calls, batches, atlas pages, rebuild counts, list cell count |
| UI-S2-015 | Add Stage 2 validation doc | UI-S2-014 | Mechanism evidence and component evidence documented |

### 7.5 Stage 2 Done Definition

Stage 2 is done when:

- Text/Image/Button/Slider/GridView run through the same UI Core path
- static atlas and dynamic atlas rules are tested
- batching produces deterministic batch counts
- rebuild counters prove local state changes stay local
- GridView virtualizes large data sets
- component library sample runs in fast/UI gate
- no Unity UI implementation dependency remains in the new runtime

## 8. Stage 3: YuFramework UI Migration

Stage 3 migrates the existing project UI framework semantics.

### 8.1 Stage 3 Goals

Stage 3 proves:

```text
UIManager + BaseUI/UICtrlBase semantics -> native UI Core/components -> representative windows
```

The goal is not to migrate every window immediately. The goal is to prove the
framework migration path and then scale window migration safely.

### 8.2 Project Runtime Semantics to Preserve

From current `UIManager`:

- active panel map
- loaded panel cache
- panel data cache
- loading task handling
- fullscreen stack
- popup stack
- layer roots
- sorting/order update
- close all
- get controller by panel ID/type
- fullscreen navigate back
- popup bring-to-top

From current `UICtrlBase` / intended `BaseUI`:

- initialize once
- bind events
- open per activation
- close per deactivation
- clear on destroy
- close self through manager

The exact C++ API can improve the naming, but the semantics should remain
recognizable.

### 8.3 Migration Strategy

Do not migrate all windows first.

Migrate in this order:

1. Project UI runtime skeleton.
2. Layer model.
3. Panel registry/test manifest.
4. `BaseUI`/controller lifecycle.
5. One simple popup/toast.
6. One fullscreen panel.
7. One GridView-heavy inventory/store panel.
8. Window stack/back navigation.
9. Async load/release policy.
10. Broader window migration.

### 8.4 Config Deferral

Config is deferred.

For Stage 3 use:

- small explicit panel registry
- test manifest
- stable integer or string panel ID
- resource refs in layout assets

Later, L2 config work can replace the test manifest with final table-driven
panel metadata.

### 8.5 Stage 3 Backlog

| ID | Work Item | Depends On | Acceptance |
| --- | --- | --- | --- |
| UI-S3-001 | Define C++ `BaseUI`/controller lifecycle | UI-S2-015 | Lifecycle exists only in Project UI Runtime, not UI Core |
| UI-S3-002 | Implement panel registry/test manifest | UI-S3-001 | Panel ID resolves to layout/controller/resource refs without config tables |
| UI-S3-003 | Implement UIManager layer model | UI-S3-002 | Fullscreen/popup/game/loading/debug layer roots tested |
| UI-S3-004 | Implement active/loaded panel maps | UI-S3-003 | Open/close/reopen semantics match current project expectations |
| UI-S3-005 | Implement popup stack and bring-to-top | UI-S3-004 | Popup order fixture passes |
| UI-S3-006 | Implement fullscreen stack/back navigation | UI-S3-004 | Navigate-back fixture passes |
| UI-S3-007 | Implement panel parameter passing | UI-S3-004 | Open args reach controller without global state |
| UI-S3-008 | Implement panel cache/release policy | UI-S3-004 | Close/reopen/release fixture passes |
| UI-S3-009 | Migrate simple popup/toast representative | UI-S3-008 | Window opens, displays, closes, and clears |
| UI-S3-010 | Migrate fullscreen representative | UI-S3-008 | Fullscreen stack behavior proven |
| UI-S3-011 | Migrate GridView representative | UI-S2-012, UI-S3-008 | Inventory/store-style virtualized grid proven |
| UI-S3-012 | Add project UI smoke sample | UI-S3-011 | Opens popup/fullscreen/grid windows and reports pass/fail |
| UI-S3-013 | Add Stage 3 migration matrix | UI-S3-012 | Remaining windows classified into migration batches |
| UI-S3-014 | Add Stage 3 validation doc | UI-S3-013 | Migration semantics and remaining gaps documented |

### 8.6 Stage 3 Done Definition

Stage 3 is done when:

- `UIManager + BaseUI/UICtrlBase` semantics have native equivalents
- lifecycle is not duplicated in UI Core
- layer model is proven
- popup/fullscreen stacks are proven
- at least three representative windows run
- GridView migration path is proven with virtualization
- config tables are still optional/deferred
- remaining windows have a migration matrix

`UI-S3-013` migration batches are owned by
`docs/YUENGINE_UI_STAGE3_MIGRATION_MATRIX.md`.

`UI-S3-014` validation evidence is owned by
`docs/YUENGINE_UI_STAGE3_VALIDATION.md`.

## 9. UI Editor Plan

The UI Editor should advance beside the runtime. It should not block Stage 1,
but the editor must use the real runtime preview path as soon as Stage 1 can
render a layout.

### 9.1 Editor Stage E1: Layout Editor, Previewer, Validator

E1 goals:

- edit UI hierarchy
- edit rect/anchor/pivot/margin/padding/size policy
- preview multiple resolutions
- preview DPI/safe area
- validate resource references
- validate duplicate IDs
- validate missing event names
- preview localization overflow with placeholder text
- run through real UI runtime rendering path

Dear ImGui can provide:

- docking shell
- hierarchy panel
- inspector panel
- asset/reference panel
- validation result panel
- console/log panel

The preview viewport must not be an ImGui-only fake preview.

### 9.2 Editor Stage E2: Component Templates and State Preview

E2 goals:

- create Text/Image/Button/Slider/GridView from templates
- edit style/theme values
- preview button states
- preview slider value
- preview GridView data count
- preview basic transition/animation placeholders
- simulate panel open args

### 9.3 Editor Stage E3: Performance Diagnostics

E3 goals:

- show draw-element count
- show batch count
- show atlas pages used
- show dynamic atlas allocation/update events
- show layout rebuild count
- show paint rebuild count
- show dirty subtree paths
- show GridView created/visible/reused cell counts
- flag common anti-patterns

Anti-pattern examples:

- too many atlas pages in one window
- full-window rebuild on hover
- list creates all items
- complex clipping breaks batches
- missing localization key
- text overflow at target resolution

### 9.4 UI Editor Backlog

| ID | Work Item | Depends On | Acceptance |
| --- | --- | --- | --- |
| UI-E1-001 | Create Dear ImGui editor shell | UI-S1-001 | Docking shell opens hierarchy/inspector/preview placeholders |
| UI-E1-002 | Load UI layout asset in editor | UI-S1-003 | Hierarchy and inspector reflect runtime data |
| UI-E1-003 | Add runtime preview viewport | UI-S1-007 | Preview uses real UI Core/RenderCore path |
| UI-E1-004 | Add resolution/DPI/safe-area preview | UI-S1-003 | Layout can be previewed across target sizes |
| UI-E1-005 | Add resource reference validator | UI-S2-004 | Missing sprite/font/style/localization refs reported |
| UI-E1-006 | Add ID/event validator | UI-S3-002 | Duplicate IDs and missing event names reported |
| UI-E2-001 | Add component templates | UI-S2-013 | Text/Image/Button/Slider/GridView templates create valid layout nodes |
| UI-E2-002 | Add state preview | UI-S2-009 | Button states and disabled/selected state visible |
| UI-E2-003 | Add GridView data simulation | UI-S2-012 | Editor previews visible/buffer cells for sample counts |
| UI-E3-001 | Add UI performance panel | UI-S2-014 | Draw/batch/atlas/rebuild/list counters visible |
| UI-E3-002 | Add anti-pattern warnings | UI-E3-001 | At least five warnings implemented with fixture layouts |

## 10. Verification Strategy

### 10.1 Gate Families

Future implementation should add dedicated test gates as the UI code lands:

- UI core unit tests
- UI layout fixture tests
- UI invalidation tests
- UI batching tests
- UI atlas tests
- UI component tests
- UI editor validator tests
- UI runtime smoke sample

### 10.2 Required Commands

When the implementation exists, minimum commands should include:

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate
ctest --preset windows-fast-gate
```

If a UI gate preset is added:

```powershell
ctest --preset windows-ui-gate
```

If a UI smoke app is added:

```powershell
.\build\windows-fast-gate\apps\UISmoke\Debug\YuUISmoke.exe
```

The executable name can follow the repository's later naming decision. The
important requirement is deterministic pass/fail output.

### 10.3 Evidence Required Before Stage Close

Stage close requires:

- test output
- direct smoke output
- screenshot or captured render output for UI visual validation
- batch/rebuild/atlas diagnostics output
- migration matrix update
- remaining gap list
- explicit skip reasons for environment-dependent paths

## 11. Hard Blocks

These are not allowed in the UI plan:

- moving `OnOpen`/`OnClose`/window stack policy into low-level UI Core
- using Dear ImGui as shipped game UI runtime
- tying UI Core directly to D3D11
- making UI runtime depend on editor-only classes
- making config tables mandatory in Stage 1/2/3
- migrating every YuFramework window before representative windows pass
- implementing GridView as non-virtualized full item creation
- dynamic atlas repack in paint hot path
- one global full-window rebuild for minor state changes
- complex masks before scissor/rect clip is stable
- old unpacked game package compatibility disguised as UI migration
- renaming existing `Yu*` code only for naming purity

## 12. Completion Matrix

| Area | Stage 1 | Stage 2 | Stage 3 | Editor |
| --- | --- | --- | --- | --- |
| Node tree | Complete | Harden | Used by runtime | Editable |
| Layout | Complete first slice | Used by components | Used by windows | Editable/previewable |
| Hit-test | Complete first slice | Used by components | Used by windows | Debuggable |
| Draw IR | Complete first slice | Batched | Used by windows | Visual diagnostics |
| Text | Placeholder | Component complete first slice | Used by migrated windows | Template/validation |
| Image | Placeholder | Component complete first slice | Used by migrated windows | Template/validation |
| Button | Excluded | Component complete first slice | Used by migrated windows | State preview |
| Slider | Excluded | Component complete first slice | Used by migrated windows | Template/validation |
| GridView | Scroll viewport only | FancyScrollView-style virtualization | Representative window migrated | Data simulation |
| UIManager | Excluded | Excluded | Native runtime semantics | Panel flow preview later |
| BaseUI/UICtrlBase | Excluded | Excluded | Native lifecycle semantics | Event binding validation |
| Static atlas | Placeholder refs | First slice | Used by windows | Validation |
| Dynamic atlas | Excluded | First slice | Used by text/temp refs | Diagnostics |
| Batching | Draw IR only | First slice | Runtime evidence | Diagnostics |
| Rebuild | Dirty flags | Subtree invalidation/cache | Window evidence | Diagnostics |
| Config tables | Deferred | Deferred | Deferred/test manifest only | Deferred |

## 13. First Landing Batch Recommendation

If the landing team wants task batches, start with this batch:

1. `UI-S0-001` through `UI-S0-003`
2. `UI-S1-001` through `UI-S1-006`
3. `UI-E1-001` as editor shell only

Do not start Text/Image/Button/GridView implementation until:

- UI Core layout can emit draw elements
- dirty categories exist
- atlas/batching/invalidation decisions are accepted

Do not start YuFramework UI migration until:

- Text/Image/Button/GridView first slices pass
- `UIManager + BaseUI/UICtrlBase` native semantics are specified
- config deferral strategy is accepted

## 14. Final Target

The final target of this plan is:

```text
A YuEngine-native UI stack where low-level UI Core handles layout/nodes/render
data, UI Component Library handles reusable controls, Project UI Runtime
preserves UIManager + BaseUI/UICtrlBase behavior, and UI Editor authors,
previews, validates, and diagnoses layouts without becoming a runtime
dependency.
```

This is the smallest direction that still supports commercial production:

- it preserves proven YuFramework UI semantics
- it avoids Unity/UE ecosystem overreach
- it fixes the low-level rebuild/batching/atlas risks early
- it gives the landing team a complete backlog from layout to migration
- it keeps config-table work out of the UI critical path until L2 integration
