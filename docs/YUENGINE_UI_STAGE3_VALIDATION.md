# YuEngine UI Stage 3 Validation

Status: `UI-S3-014`
Baseline: `origin/main@0a51675b08a26b543decaa180c43fa241b8431ef`
Source plan: `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md` section 8
Migration matrix: `docs/YUENGINE_UI_STAGE3_MIGRATION_MATRIX.md`

## 1. Scope

This document records the Stage 3 validation path for `UI-S3-001` through
`UI-S3-014`.

Included:

- Project UI Runtime lifecycle, registry, layer, map, stack, args, and
  release/cache semantics.
- Simple popup/toast, fullscreen, and GridView representative windows.
- `UiProjectRuntimeSmokeSample` as the integrated representative route.
- `UI-S3-013` migration matrix and its remaining-window batch semantics.
- Remaining gaps that must be consumed by later migration batches and editor
  validation work.

Excluded:

- Product-window completion claims for YuFramework windows.
- Bulk YuFramework C# window migration.
- UI Editor E1, E2, or E3 implementation.
- Dear ImGui, RHI, D3D11, RenderCore, backend, or editor backend scope.
- UiCore ownership or lifecycle movement.
- Real config table service integration.
- Gameplay manager adapters beyond documenting the required boundary.

## 2. Acceptance Mapping

| Plan item | Evidence path | Acceptance recorded here |
| --- | --- | --- |
| `UI-S3-001` | `BaseUiController`, `BaseUiLifecycleSnapshot`, `Tests/UiRuntime/BaseUiControllerTests.cpp` | Native Project UI Runtime lifecycle covers init, bind, open, close, clear, state snapshots, and close-self request semantics without moving lifecycle into UI Core. |
| `UI-S3-002` | `UiPanelRegistry`, `UiPanelManifestRecord`, `Tests/UiRuntime/UiPanelRegistryTests.cpp` | Panel IDs resolve to layout, controller, and resource refs through an explicit test manifest while config tables stay deferred. |
| `UI-S3-003` | `UiManagerLayerModel`, `UiManagerLayerRecord`, `Tests/UiRuntime/UiManagerLayerModelTests.cpp` | Fullscreen, popup, game, loading, and debug layer roots are represented by deterministic value records and layer bindings. |
| `UI-S3-004` | `UiManagerPanelMap`, `UiManagerPanelMapRecord`, `Tests/UiRuntime/UiManagerPanelMapTests.cpp` | Active and loaded panel maps preserve open, close, reopen, lookup, and clear semantics with bounded records. |
| `UI-S3-005` | `UiManagerPopupStack`, `UiManagerPopupStackSnapshot`, `Tests/UiRuntime/UiManagerPopupStackTests.cpp` | Popup open order, bring-to-top, close, release, missing panel, and stack capacity behavior are covered. |
| `UI-S3-006` | `UiManagerFullscreenStack`, `UiManagerFullscreenStackSnapshot`, `Tests/UiRuntime/UiManagerFullscreenStackTests.cpp` | Fullscreen open, close-current, restore previous, navigate-back, release, and missing panel behavior are covered. |
| `UI-S3-007` | `UiPanelOpenArgs`, `UiPanelOpenArgsSnapshot`, `Tests/UiRuntime/UiPanelParameterTests.cpp` | Panel open args reach controller-side state through copied value snapshots instead of global state. |
| `UI-S3-008` | `UiManagerPanelMap`, `UiManagerPopupStack`, `UiManagerFullscreenStack`, focused `Tests/UiRuntime` routes | Close, reopen, release, loaded-cache clearing, active-state clearing, and missing-release behavior are documented by map and stack tests. |
| `UI-S3-009` | `UiSimplePopupToastRepresentativeController`, `Tests/UiRuntime/UiSimplePopupToastRepresentativeTests.cpp` | Simple popup/toast representative opens with args, displays state, closes, clears, and interacts with popup stack/release semantics. |
| `UI-S3-010` | `UiFullscreenRepresentativeController`, `Tests/UiRuntime/UiFullscreenRepresentativeTests.cpp` | Fullscreen representative proves fullscreen stack and navigate-back behavior with copied open args. |
| `UI-S3-011` | `UiGridViewRepresentativeController`, `Tests/UiRuntime/UiGridViewRepresentativeTests.cpp` | Inventory/store-style GridView representative proves bounded visible plus buffer cells, selection, scroll, refresh, and clear behavior. |
| `UI-S3-012` | `UiProjectRuntimeSmokeSample`, `Tests/UiRuntime/UiProjectRuntimeSmokeSampleTests.cpp` | Integrated smoke route opens popup, fullscreen, and GridView representatives and reports deterministic pass/fail values. |
| `UI-S3-013` | `docs/YUENGINE_UI_STAGE3_MIGRATION_MATRIX.md` | Remaining YuFramework window families are classified into migration batches M0 through M5, including the `CreateProcess` remediation. |
| `UI-S3-014` | this document | Migration semantics, accepted evidence, remaining gaps, validation standards, and closeout risks are recorded. |

## 3. Accepted Evidence Commits

| Commit | Plan item | Evidence summary |
| --- | --- | --- |
| `4954b24` | `UI-S3-001` | Added native `BaseUiController` lifecycle first slice and tests. |
| `bbfcc1b` | `UI-S3-002` | Added explicit `UiPanelRegistry` and panel manifest test path. |
| `05634dc` | `UI-S3-003` | Added `UiManagerLayerModel` value records and layer fixture tests. |
| `3a1dd7f` | `UI-S3-004` | Added active and loaded panel map semantics. |
| `6022902` | `UI-S3-005` | Added popup stack and bring-to-top behavior. |
| `2a477b6` | `UI-S3-006` | Added fullscreen stack and back navigation behavior. |
| `80ef73b` | `UI-S3-007` | Added panel open-args value snapshot route. |
| `c27b2ec` | `UI-S3-008` | Added panel cache/release policy. |
| `40a6ec4` | `UI-S3-009` | Added simple popup/toast representative. |
| `78f6390` | `UI-S3-010` | Added fullscreen representative. |
| `38b860b` | `UI-S3-011` | Added GridView representative. |
| `f9a581d` | `UI-S3-012` | Added Project UI Runtime smoke sample. |
| `d718830` | `UI-S3-013` | Added Stage 3 migration matrix. |
| `0a51675` | `UI-S3-013` | Fixed `CreateProcess` batch classification in the matrix. |

## 4. Migration Semantics

Stage 3 validates this migration path:

```text
YuFramework UIManager and UICtrlBase semantics
-> native Project UI Runtime value records
-> UI Core components and representatives
-> later product-window migration batches
```

Accepted semantics:

- `BaseUI` and controller lifecycle belongs to Project UI Runtime, not UI Core.
- Panel metadata is an explicit registry/test manifest until config tables are
  integrated later.
- Panel IDs are stable values that resolve layout, controller, resource, and
  layer refs.
- Active panel state and loaded panel cache are separate records.
- Close deactivates a panel, while release clears loaded/cache/controller state.
- Popup and fullscreen stack order is deterministic and exposed through
  snapshots.
- Open parameters are copied into `UiPanelOpenArgsSnapshot` records.
- Representative windows prove popup/toast, fullscreen, and GridView migration
  mechanics without claiming the old product windows are complete.
- GridView migration uses bounded visible plus buffer cells and rejects
  full-list materialization as the accepted direction.

## 5. Remaining Gaps

These gaps remain after Stage 3 and are not product-window completion evidence:

| Gap | Required follow-up | Current Stage 3 boundary |
| --- | --- | --- |
| Product panel IDs | Name the real panel IDs and aliases for each migrated product window. | Stage 3 uses representative and test-manifest IDs only. |
| Layout/resource refs | Bind each product window to real layout assets, image/font refs, and resource keys. | Stage 3 records layout/resource refs as values but does not import product assets. |
| Open argument schemas | Define per-window args for callbacks, style keys, selected tabs, item IDs, save slots, and runtime state. | Stage 3 proves generic `UiPanelOpenArgs` copy semantics only. |
| Config tables | Replace test manifests with final table-driven panel metadata. | Config tables are explicitly deferred. |
| Gameplay adapters | Route dialogue, quest, time, creation, inventory, store, save/load, and audio effects through named adapter inputs and command outputs. | UI Core and Project UI Runtime must not call gameplay singletons directly. |
| Pointer and hit-test policy | Define native coordinate, pointer-position detail panel, drag/drop, and context-menu behavior. | Stage 3 only preserves the need for pointer-position evidence. |
| Animation and mask behavior | Reduce Unity animator, mask, and active-state behavior to explicit state records before product migration completion. | Representatives validate functional state, not visual animation parity. |
| Visual validation | Capture screenshot or render-output evidence during later stage closeout. | `UI-S3-014` documents the need; it does not add visual backend scope. |

## 6. Migration Batch Validation Standards

Later product-window batches from the migration matrix should use these
standards:

| Batch | Validation standard |
| --- | --- |
| M1 popup/toast/tip | Each popup or toast opens with explicit args, displays the requested message/style state, closes through the popup stack, clears active state, and releases loaded/cache/controller records. |
| M2 fullscreen/navigation | Each fullscreen panel opens on the fullscreen stack, preserves navigate-back behavior, emits command records for save/load/screen/audio state, and releases cleanly on close or cache clear. |
| M3 inventory/store/grid-heavy | Each GridView window proves bounded visible plus buffer cells, deterministic selection, tab refresh, scroll-to-index, detail-panel routing, and data-provider boundaries. |
| M4 narrative/gameplay-coupled | Each panel consumes a deterministic model snapshot, emits command-output records for gameplay actions, and avoids direct gameplay singleton access inside UI Core or Project UI Runtime. |
| M5 debug/template/shared helper | Each helper is validated only when a consuming product batch names it; otherwise it remains deferred or non-product. |

## 7. Validation Commands

Focused Stage 3 route:

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuUiRuntimeTests -- /v:minimal
ctest --preset windows-fast-gate -R "^(UiRuntime_|BaseUiController_|UiManager|UiPanel|UiSimplePopupToastRepresentative_|UiFullscreenRepresentative_|UiGridViewRepresentative_|UiProjectRuntimeSmokeSample_)" --output-on-failure
git diff --check
```

Optional documentation-only checks:

```powershell
git diff --check
```

## 8. Expected Evidence

Expected focused CTest groups:

```text
BaseUiController_*
UiPanelRegistry_*
UiManagerLayerModel_*
UiManagerPanelMap_*
UiManagerPopupStack_*
UiManagerFullscreenStack_*
UiPanelParameter_*
UiSimplePopupToastRepresentative_*
UiFullscreenRepresentative_*
UiGridViewRepresentative_*
UiProjectRuntimeSmokeSample_*
```

Expected smoke sample status:

```text
UiProjectRuntimeSmokeSampleStatus::Success
UiProjectRuntimeSmokeSampleResult::passed == true
```

## 9. Closeout Risks

Stage 3 closeout should treat these as risks rather than completed work:

- A representative passing does not mean every old YuFramework product window is
  migrated.
- Test manifests are not final config tables.
- Product resources and layout assets are not validated until real window
  batches consume them.
- Gameplay behavior remains outside UI Core and must be bridged through explicit
  adapters before product windows are accepted.
- Visual parity needs captured render or screenshot evidence in a later
  validation route.
- Editor validation must consume the runtime records later; this document does
  not implement editor panels or editor preview backend work.

## 10. Boundary Notes

`UI-S3-014` validates Stage 3 semantics and documents the remaining migration
gaps. It does not:

- migrate YuFramework C# windows
- declare product windows complete
- implement UI Editor E1, E2, or E3
- add Dear ImGui, ImGui, RHI, D3D11, RenderCore, backend, or editor backend code
- move `BaseUI` or controller lifecycle into UI Core
- require real config tables
- add gameplay adapters or gameplay singleton calls to UI Core
