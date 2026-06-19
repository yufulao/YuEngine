# YuEngine UI Stage 0 Decisions

Status: ENG-181A Stage 0 planning document
Baseline: `origin/main@1a2f7ed2e17f8cbd34d1685463d4be09298f83bd`
Source plan: `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md` sections 1, 2, 5, 10,
11, 12, and 13
Reference root: `C:/Steam/steamapps/common/TouhouNewWorld/yufulao/framework`
Scope: UI-S0-001 through UI-S0-003 only. This document defines UI planning,
migration inventory, and acceptance gates. It does not modify runtime code,
tests, build files, or YuFramework source files.

## 1. Purpose

Stage 0 freezes the first UI boundary before implementation starts. The goal is
to keep UI Core small, keep project window lifecycle above UI Core, and make
the YuFramework migration path explicit enough for Stage 1 scouting.

Stage 0 produces:

- UI terminology and ownership boundaries.
- Confirmation that window lifecycle is excluded from UI Core.
- YuFramework UI migration inventory categories A through F.
- The first three representative windows for later migration proof.
- The initial layout asset data-format decision.

## 2. UI-S0-001 Naming and Stage Boundary

New UI planning uses plain names: `UI Framework`, `UI Core`, `UI Component
Library`, `UI Editor`, and `Project UI Runtime`. Existing `Yu*` code names do
not require churn-only renames.

Ownership boundaries:

- `UI Framework`: umbrella term for UI Core, component library, editor support,
  project runtime integration, renderer submission, and asset format work.
- `UI Core`: low-level runtime node tree, rect/anchor/pivot values, layout
  containers, hit-test records, dirty classification, clipping records, and
  draw-element IR. UI Core must not own window lifecycle, panel stacks, config
  table lookup, renderer backend handles, editor-only state, or project-specific
  gameplay rules.
- `UI Component Library`: reusable controls such as text, image, button,
  slider, toggle, dropdown, and virtualized GridView. Components consume UI
  Core primitives but do not own project window stacks.
- `Project UI Runtime`: native equivalent of `UIManager + BaseUI/UICtrlBase`
  semantics. This layer owns `OnInit`, event binding, open, close, clear, panel
  cache, panel state, layer roots, fullscreen stack, popup stack, and project
  transition rules.
- `UI Editor`: authoring and inspection shell for hierarchy, layout, resources,
  preview, and validation. Editor-only classes must not be required by game
  runtime.
- `UI Rendering Substrate`: draw-list batching, atlas/font records, and
  RenderCore submission. UI Core emits records and does not include D3D11,
  Dear ImGui, or platform window details.

Stage boundary decisions:

- Window lifecycle is excluded from UI Core.
- Config-table integration is deferred beyond Stage 1/2/3 first slices.
- Dear ImGui is allowed only for editor tooling, not shipped game UI runtime.
- Direct D3D11, XAudio2, platform window, and gameplay dependencies are outside
  UI Core.
- Stage 1 may create UI node/layout/hit-test/draw IR prototypes after this
  document, but it must not implement project panel lifecycle inside UI Core.

## 3. Reference Evidence

| Reference used | Borrowed behavior | Not copied | YuEngine-specific acceptance |
| --- | --- | --- | --- |
| `Scripts/Core/Manager/UIManager/UIManager.cs` | Active panel table, loaded panel cache, popup stack, fullscreen stack, layer roots, sorting/order policy, close-all-popup behavior, reopen-active behavior | Unity `GameObject`, `Transform`, `Canvas`, `CanvasScaler`, `GraphicRaycaster`, `Addressables`, async prefab loading, immediate `CfgUI` dependency | Project UI Runtime owns panel registry, cache, stacks, and layer rules; tests later prove popup/fullscreen ordering without UI Core lifecycle APIs |
| `Scripts/Core/Manager/UIManager/UICtrlBase.cs` | `OnInit -> BindEvent -> OnOpen -> OnClose -> OnClear` lifecycle as project window/controller semantics | `MonoBehaviour`, Unity component lookup, direct Unity event binding, lifecycle callbacks inside UI Core nodes | `BaseUI` or equivalent lifecycle exists only above UI Core; Stage 1 UI Core headers do not expose `OnOpen`, `OnClose`, or panel stack APIs |
| `Scripts/Core/Manager/UIManager/UILayer.cs`, `Def/DefUILayer.cs`, `UIPanelInfo.cs` | Game, Fullscreen, Popup, Loading, Debug layer vocabulary; panel ID, layer, load state, parameter, root, controller bookkeeping | Unity `RectTransform` layer roots, Unity canvas sorting implementation, direct Canvas references | Native layer and panel-state records are value data; project runtime maps them onto UI Core roots and rendering records |
| `Scripts/UI/Component/FancyScrollViewExtensions/Base/GridViewBase.cs`, `GridView.cs`, `BtnGridView.cs`, `BtnGridCell.cs` | Grouped cells, visible-cell reuse, cell pool, row/column grouping, scroll-to-index, selected-index refresh, cell initialization callbacks | FancyScrollView package dependency, Unity `RectTransform`, editor preview code, full-list element instantiation | GridView first slice must be virtualized: large lists create visible plus bounded buffer cells, selection refresh touches visible cells, and scroll-to-index is deterministic |
| `Scripts/UI/Component/*`, TMP/Image/Button/Slider usages in windows | Text, localization-aware text, image wrappers, buttons, sliders, toggles, dropdowns, links, pointer areas, and fill-origin behavior | TextMeshPro package internals, Unity event system, Unity `Graphic` subclasses, plugin/editor attributes | UI Component Library ports behavior as explicit native component state and event records after UI Core node/layout exists |
| `Scripts/UI/Windows/*/*Ctrl.cs`, `*View.cs`, `*Model.cs` | Ctrl/View/Model split, model-backed refresh, project windows, animation mask concepts, focused representative flows | Wholesale migration of every window, Unity prefab hierarchy, Unity animator state machine, game-specific manager singletons in UI Core | Representative windows migrate first; later windows wait until popup, fullscreen, and GridView samples pass |

## 4. UI-S0-002 Migration Inventory

| Category | Meaning | YuFramework examples | YuEngine destination | First acceptance |
| --- | --- | --- | --- | --- |
| A | Runtime lifecycle semantics to preserve | `UIManager`, `UICtrlBase`, `UILayer`, `UIPanelInfo`, popup/fullscreen stacks | Project UI Runtime above UI Core | `UI-S3-001..008`: lifecycle, panel registry, layer roots, popup/fullscreen stacks, close/clear behavior |
| B | Component behavior to port | `TextMeshProUGUI`, localized TMP wrappers, `Button`, `Slider`, `Toggle`, `Dropdown`, `Image`, `AlphaRaycastFilterImage`, `CustomFillOriginImage`, `SwitchImageToggleComponent`, `TMP_LinkAction` | UI Component Library | `UI-S2-007..010`: text/image/button/slider state, hit-test, events, resource references |
| C | List/Grid behavior to preserve | `GridViewBase`, `GridView`, `BtnGridView`, `BtnGridCell`, Bag/Store/StallShelf grid cells | UI Component Library virtualized list/grid | `UI-S2-011..012`: grouped virtualized cells, pooling, selection, scroll-to-index |
| D | Layout/window content to migrate later | `DoubleConfirm`, `Setting`, `Bag`, `Store`, `Dialogue`, `QuestHud`, `TopLogoHud` | Stage 3 representative window migration and later product-window backlog | `UI-S3-009..011`: popup/toast, fullscreen, inventory/store GridView representatives |
| E | Unity-only implementation details | `Canvas`, `RectTransform`, `CanvasScaler`, `GraphicRaycaster`, `GameObject`, `MonoBehaviour`, prefab loading, Addressables, `TextAsset`, immediate config-table lookup | Not copied; replaced by UI layout assets, UI Core node values, Resource/Asset references, and Project UI Runtime lookup | Stage 1/2 validation rejects Unity-only runtime dependencies |
| F | Editor-only or plugin-only details | `Scripts/UI/Component/Editor/*`, Odin/Sirenix attributes, Unity custom inspectors, editor preview hooks, plugin package internals | UI Editor tooling only, with no shipped runtime dependency | UI-E1/E2/E3 editor gates prove authoring support without runtime coupling |

Migration rules:

- A owns lifecycle and panel orchestration. A must not be implemented inside UI
  Core.
- B and C wait for UI Core node/layout/hit-test primitives before component
  migration.
- C must keep virtualization. A non-virtualized GridView is rejected.
- D starts with three representatives and does not migrate all windows at once.
- E and F are references for behavior only. They are not copied into the
  runtime architecture.

## 5. Representative Windows

| Slot | Representative | Reference files | Borrowed behavior | Not copied | Acceptance target |
| --- | --- | --- | --- | --- | --- |
| Simple popup/toast | `DoubleConfirm` | `Scripts/UI/Windows/DoubleConfirm/DoubleConfirmCtrl.cs`, `DoubleConfirmView.cs` | Text prompt, confirm/cancel actions, mask, open/close animation, Ctrl/View split | Unity `Animator`, `GameObject.SetActive`, `Button.onClick`, direct callback casting, Unity mask implementation | Later `UI-S3-009` sample opens with text, handles confirm/cancel as command records, closes, and clears state |
| Fullscreen/settings | `Setting` | `Scripts/UI/Windows/Setting/SettingCtrl.cs`, `SettingView.cs` | Full-window controls, sliders, toggles, dropdowns, refresh-from-model, save-on-close behavior | Unity `Screen`, Unity `Slider/Toggle/Dropdown`, `SaveGameManager`, audio manager calls in UI Core | Later `UI-S3-010` sample proves fullscreen stack ownership and component state changes without gameplay/global manager coupling in UI Core |
| Inventory/store GridView | `Bag` | `Scripts/UI/Windows/Bag/BagCtrl.cs`, `BagView.cs`, GridView references above | Tab grid, item grid, virtualized cell initialization, selected tab refresh, item right-click context route | Unity mouse position, direct inventory manager mutation, Unity context menu positioning, full product inventory rules | Later `UI-S3-011` sample proves virtualized inventory/store-style grid, bounded visible cells, tab switch refresh, and command-record output |

`Store`, `Dialogue`, `QuestHud`, `TopLogoHud`, and `StallShelf` remain useful
follow-up windows, but they are not part of the first three acceptance samples.

## 6. UI-S0-003 Layout Asset Format Decision

Initial format: a UTF-8 JSON document named by the `YuUILayout` schema. The
first implementation gate may choose the final extension and asset directory,
but the data contract is fixed by this document.

Required top-level fields:

- `schema`: literal schema identifier, initially `YuEngine.UI.Layout`.
- `version`: positive integer schema version.
- `layoutId`: stable project-independent layout identifier.
- `rootNodeId`: stable ID of the root node.
- `nodes`: deterministic array of node records sorted by parent order.
- `resources`: optional table of named resource references used by nodes.

Required node fields:

- `nodeId`: stable ID that survives editor reorder and display-name changes.
- `name`: human-readable editor name.
- `type`: UI node or component type name.
- `parentNodeId`: empty only for the root node.
- `order`: sibling order.
- `rect`: anchor minimum, anchor maximum, pivot, offset minimum, offset maximum,
  margin, and padding values.
- `layout`: container type and per-container parameters.
- `style`: optional style reference.
- `resourceRefs`: named sprite, font, atlas, material, audio, or localization
  references.
- `bindings`: optional project runtime binding keys. Bindings are data only and
  do not encode lifecycle methods.

First-slice exclusions:

- No Unity prefab, `RectTransform`, `Canvas`, `MonoBehaviour`, or editor-only
  object references.
- No direct D3D11, RenderCore backend, or platform window handles.
- No mandatory config table dependency.
- No inline binary font, atlas, sprite, or audio payload.
- No window lifecycle callbacks such as `OnOpen` or `OnClose`.
- No gameplay manager calls.

Validator acceptance:

- Rejects duplicate node IDs and missing parents.
- Rejects multiple roots.
- Rejects invalid rect records and unknown container names.
- Preserves deterministic node order after load/save.
- Reports missing resource references as explicit validation findings.
- Builds a UI Core node tree and draw-element input records without requiring
  Project UI Runtime lifecycle.

## 7. Handoff Gates

ENG-181B may scout Stage 1 UI Core work using this boundary:

- UI Core owns node identity, tree mutation, rect/layout values, dirty classes,
  hit-test records, clipping records, and draw-element IR.
- UI Core must not include panel lifecycle, panel stacks, config table lookup,
  editor-only types, or backend-specific renderer handles.

ENG-181C may scout the first UI Editor shell using this boundary:

- The editor may author and inspect `YuUILayout` data.
- The editor may use Dear ImGui or another internal tooling shell.
- Runtime code must not depend on the editor shell.

ENG-181VQ should verify:

- This document covers `UI-S0-001`, `UI-S0-002`, and `UI-S0-003`.
- Every reference row states `Reference used`, `Borrowed behavior`,
  `Not copied`, and `YuEngine-specific acceptance`.
- The first three representative windows are named.
- The layout asset format supports hierarchy, rect/layout fields, resource
  references, and stable IDs.
- The work is planning-only and does not change runtime source, tests, or build
  files.
