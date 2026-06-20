# YuEngine UI Stage 3 Migration Matrix

Status: `UI-S3-013`

Scope: classify the remaining YuFramework UI windows and window families into
explicit migration batches after the Stage 3 representative path is proven.
This document is a planning matrix only. It does not implement product-window
migration, does not define the `UI-S3-014` validation document, and does not
introduce editor, renderer, backend, or config-table runtime scope.

## Source Inventory

Read-only source roots used for this matrix:

- `C:/Steam/steamapps/common/TouhouNewWorld/yufulao/framework2/Scripts/Core/Manager/UIManager`
- `C:/Steam/steamapps/common/TouhouNewWorld/yufulao/framework2/Scripts/UI/Windows`
- `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`
- `docs/YUENGINE_UI_STAGE0_DECISIONS.md`

`framework2/Scripts/UI/Windows` currently contains these top-level window or
helper families:

| Group | Directories |
| --- | --- |
| Representative baselines | `DoubleConfirm`, `Setting`, `Bag`, `Store` |
| Transient popup/toast/tip | `AutoSaveTip`, `BedCollisionTip`, `DeskCollisionTip`, `DinnerReminder`, `GameTimePeriodTip`, `SceneName`, `StoreCollisionTip`, `StallCollisionTip`, `ToastStyle` |
| Fullscreen or modal flow | `Cover`, `Create`, `GameOver`, `GetUp`, `Home`, `IntoDream`, `LoadGame`, `Loading`, `Pause`, `RoleMake` |
| Grid/data-heavy product UI | `CharacterStatus`, `InspirationCatalog`, `QuestHud`, `StallOnSale`, `StallShelf` |
| Narrative/gameplay-coupled UI | `Bargaining`, `CreateProcess`, `Dialogue`, `Dinner`, `GameTime`, `GameTimeDetail`, `InspirationItemResult`, `TopLogoHud` |
| Non-product or shared helper | `_Common`, `_UITemplate`, `GM` |

The directory grouping is intentionally coarse. Later implementation tasks
should rescan the concrete `*Ctrl.cs`, `*View.cs`, `*Model.cs`, and nested
`Comp` or `GridView` files before moving an individual window.

## Accepted Stage 3 Baseline

| Baseline | Representative coverage | Evidence consumed by this matrix |
| --- | --- | --- |
| `UI-S3-009` | Simple popup/toast representative | `UiSimplePopupToastRepresentativeController`, popup stack open/display/close/clear tests, release/cache tests |
| `UI-S3-010` | Fullscreen representative | `UiFullscreenRepresentativeController`, fullscreen stack and navigate-back behavior tests |
| `UI-S3-011` | GridView representative | `UiGridViewRepresentativeController`, virtualized visible/pool cell behavior and status tests |
| `UI-S3-012` | Project UI Runtime smoke sample | `UiProjectRuntimeSmokeSample` opens popup, fullscreen, and GridView representatives and returns deterministic pass/fail |

These baselines prove the native Project UI Runtime mechanisms. They do not
bulk-port the old YuFramework windows yet.

## Migration Batches

| Batch | Window families | Representative dependency | Required migration work | Verification path | Main risks or blockers |
| --- | --- | --- | --- | --- | --- |
| M0 accepted baseline | `DoubleConfirm`, `Setting`, `Bag` and `Store` representative behavior only | `UI-S3-009..012` | Keep representative tests green and treat them as the contract for later product windows | Existing representative focused tests plus `UiProjectRuntimeSmokeSample` | Product `DoubleConfirm`, `Setting`, `Bag`, and `Store` still need real content/data migration after this matrix |
| M1 popup/toast/tip first product batch | `AutoSaveTip`, `BedCollisionTip`, `DeskCollisionTip`, `DinnerReminder`, `GameTimePeriodTip`, `SceneName`, `StoreCollisionTip`, `StallCollisionTip`, `ToastStyle`, product `DoubleConfirm` | `UI-S3-009`, `UI-S3-005`, `UI-S3-007`, `UI-S3-008` | Map message IDs, style keys, optional callbacks, and close semantics onto popup stack plus panel args; replace Unity animation and `GameObject.SetActive` with explicit display state | Open with args, display observed, close clears active state, release clears loaded/cache/controller state, duplicate open and missing panel status tests | Callback casting, animation-mask behavior, toast prefab references, and mouse-position detail panels need command records instead of Unity objects |
| M2 fullscreen and navigation panels | `Setting`, `Home`, `Pause`, `Loading`, `LoadGame`, `GameOver`, `IntoDream`, `GetUp`, `RoleMake` | `UI-S3-010`, `UI-S3-006`, `UI-S3-007`, `UI-S3-008` | Move fullscreen open/close/back behavior to fullscreen stack; express screen state, selected slot, loading state, and resume/exit commands as value records | Fullscreen open, stack order, navigate-back, close-current, release/reopen fresh lifecycle, and args snapshot tests | `Setting` currently reaches `Screen`, `SaveGameManager`, and audio managers; loading/save flows must stay outside UI Core and become runtime command outputs |
| M3 inventory/store/grid-heavy batch | Product `Bag`, `Store`, `StallShelf`, `StallOnSale`, `InspirationCatalog`, `CharacterStatus`, `QuestHud`, `Cover`, `Create` | `UI-S3-011`, `UI-S2-011`, `UI-S2-012`, `UI-S3-008` | Rebuild item/tab/list views on the virtualized GridView contract; split model refresh from global inventory/store managers; keep item selection and scroll-to-index deterministic | Bounded visible plus buffer cells, cell reuse counters, tab switch refresh, selected-index update, release/clear cleanup, and large-list smoke tests | Direct manager mutation, mouse-position detail panels, nested GridView cells, config table row access, and item drag/drop routes need explicit adapter boundaries |
| M4 narrative and gameplay-coupled panels | `Dialogue`, `Dinner`, `Bargaining`, `CreateProcess`, `GameTime`, `GameTimeDetail`, `InspirationItemResult`, `TopLogoHud` | `UI-S3-004..012`, then M1/M2/M3 patterns | Keep UI layer responsible for display and command records only; route dialogue choices, quest changes, creation progress/completion, time state, cooking/bargaining results, and HUD refresh through gameplay adapters outside UI Core | Deterministic model snapshot input, command-output assertions, popup/fullscreen/grid route tests as appropriate, no direct gameplay singleton dependency inside UI Core | These windows call quest, dialogue, creation, action, time, and gameplay managers directly today; migration needs a separate adapter contract before product behavior can be called complete |
| M5 debug/template/shared helper cleanup | `GM`, `_UITemplate`, `_Common` helpers including detail hang panels | Depends on target product batch | Keep reusable detail panels only when consumed by M1/M3/M4; keep `GM` tooling out of shipped runtime path; remove template-only scope from product migration | Compile/test only when a product batch consumes the helper; otherwise document as deferred | Debug-only windows and templates are not product acceptance blockers; shared detail panels depend on pointer/mouse-position policy not yet finalized |

## Batch Ordering Rules

1. M1 must be first because it is the smallest product window batch and directly
   exercises popup stack, args, close, and release semantics.
2. M2 should follow M1 because fullscreen stack behavior is already proven but
   more windows touch save/load or app navigation state.
3. M3 should follow M1 and M2 because it depends on both Project UI Runtime
   ownership and GridView virtualization.
4. M4 should start only after the needed adapter contracts are explicit. These
   windows are UI-heavy but also gameplay-state-heavy.
5. M5 is opportunistic cleanup. It should not block product-window batches.

## Per-Batch Definition of Ready

| Batch | Ready when |
| --- | --- |
| M1 | Popup/toast product IDs are listed, each message/style/callback input is expressible as `UiPanelOpenArgs` or a small value record, and Unity animation behavior is either replaced by a deterministic state or explicitly deferred. |
| M2 | Fullscreen product IDs are listed, navigate-back behavior is defined, save/load/screen/audio calls have command-output adapters, and panel release policy is unchanged. |
| M3 | Each GridView window has item count, visible group size, selection behavior, detail-panel route, and data-provider boundary documented before implementation. |
| M4 | Gameplay adapter inputs and command outputs are named for dialogue, quest, creation progress, time, and result flows; direct singleton access is not moved into UI Core. |
| M5 | A consuming product batch names the helper, or the helper is marked deferred/non-product. |

## UI-S3-014 Inputs

`UI-S3-014` should consume the following remaining gaps instead of rediscovering
them:

- Product windows still require real panel IDs, layout/resource references, and
  per-window open argument schemas.
- Config tables remain deferred; migration batches must keep explicit test
  manifests until a later config integration task replaces them.
- Gameplay manager calls in `Dialogue`, `Dinner`, `Bargaining`,
  `CreateProcess`, HUD, save/load, and settings flows need adapter contracts
  before product behavior is accepted.
- Pointer-position detail panels need a native hit-test/input coordinate policy
  before they can be marked fully migrated.
- Animation and mask behavior should be reduced to explicit state records until
  scissor/rect clip and later visual animation policies are accepted.
- Visual validation still needs screenshot or captured render evidence at stage
  close; this matrix only defines migration ordering and functional checks.

## Scope Guard

This matrix does not:

- implement `UI-S3-014` validation prose
- migrate YuFramework C# windows
- add Dear ImGui, editor, RHI, D3D11, RenderCore, backend, or UI Editor scope
- move Project UI Runtime ownership into UI Core
- require real config tables
- declare any product window complete without a focused migration task and
  matching verification evidence
