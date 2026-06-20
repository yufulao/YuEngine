# YuEngine UIManager Runtime Framework Validation

Status: corrective replacement for Stage 3 validation
Source plan: `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md`

## 1. Scope

This document validates only the generic UIManager runtime framework.

Included:

- `BaseUI` / controller lifecycle
- generic registry records
- layer model
- loaded and active map
- popup stack
- fullscreen stack
- copied open-argument snapshots
- close, reopen, release, and cache policy

Excluded:

- game-specific windows
- old game-window batches
- native app/editor targets
- Web Editor implementation details
- config table integration
- gameplay manager adapters
- moving lifecycle into UI Core

## 2. Acceptance Mapping

| Plan item | Evidence path | Acceptance |
| --- | --- | --- |
| `UI-S3-001` | `BaseUiController`, `BaseUiLifecycleSnapshot`, `Tests/UiRuntime/BaseUiControllerTests.cpp` | Native lifecycle covers init, bind, open, close, clear, state snapshots, and close-self request semantics without moving lifecycle into UI Core. |
| `UI-S3-002` | `UiPanelRegistry`, `UiPanelManifestRecord`, `Tests/UiRuntime/UiPanelRegistryTests.cpp` | Generic IDs resolve to layout, controller, resource, and layer refs through an explicit runtime manifest while config tables stay deferred. |
| `UI-S3-003` | `UiManagerLayerModel`, `UiManagerLayerRecord`, `Tests/UiRuntime/UiManagerLayerModelTests.cpp` | Layer roots are represented by deterministic value records and layer bindings. |
| `UI-S3-004` | `UiManagerPanelMap`, `UiManagerPanelMapRecord`, `Tests/UiRuntime/UiManagerPanelMapTests.cpp` | Active and loaded maps preserve open, close, reopen, lookup, and clear semantics with bounded records. |
| `UI-S3-005` | `UiManagerPopupStack`, `UiManagerPopupStackSnapshot`, `Tests/UiRuntime/UiManagerPopupStackTests.cpp` | Popup order, bring-to-top, close, release, missing ID, and stack capacity behavior are covered. |
| `UI-S3-006` | `UiManagerFullscreenStack`, `UiManagerFullscreenStackSnapshot`, `Tests/UiRuntime/UiManagerFullscreenStackTests.cpp` | Fullscreen open, close-current, restore previous, navigate-back, release, and missing ID behavior are covered. |
| `UI-S3-007` | `UiPanelOpenArgs`, `UiPanelOpenArgsSnapshot`, `Tests/UiRuntime/UiPanelParameterTests.cpp` | Open args reach controller-side state through copied value snapshots instead of global mutable state. |
| `UI-S3-008` | map and stack tests | Close, reopen, release, loaded-cache clearing, active-state clearing, and missing-release behavior are documented by generic map and stack tests. |

No further Stage 3 item is accepted by this document.

## 3. Accepted Evidence Commits

| Commit | Plan item | Evidence summary |
| --- | --- | --- |
| `4954b24` | `UI-S3-001` | Added native lifecycle first slice and tests. |
| `bbfcc1b` | `UI-S3-002` | Added explicit runtime registry and manifest test path. |
| `05634dc` | `UI-S3-003` | Added layer model value records and fixture tests. |
| `3a1dd7f` | `UI-S3-004` | Added active and loaded map semantics. |
| `6022902` | `UI-S3-005` | Added popup stack and bring-to-top behavior. |
| `2a477b6` | `UI-S3-006` | Added fullscreen stack and back navigation behavior. |
| `80ef73b` | `UI-S3-007` | Added open-args value snapshot route. |
| `c27b2ec` | `UI-S3-008` | Added cache/release policy. |

## 4. Corrected Semantics

Accepted:

- lifecycle belongs above UI Core
- registry is generic runtime metadata
- active and loaded records are separated
- close deactivates; release clears loaded/cache/controller state
- stack order is deterministic and observable through snapshots
- open args are copied value snapshots
- config table integration is deferred

Rejected:

- using old game windows as validation samples
- treating game-window work as UI Framework progress
- adding native app/editor code as engine capability
- retaining a fallback editor path outside Web

## 5. Focused Validation Command

```powershell
ctest --preset windows-fast-gate -R "^(BaseUiController_|UiPanelRegistry_|UiRuntime_ManagerLayerModel_|UiRuntime_ManagerPanelMap_|UiRuntime_ManagerPopupStack_|UiRuntime_ManagerFullscreenStack_|UiRuntime_PanelParameter_)" --output-on-failure
```

Expected groups:

```text
BaseUiController_*
UiPanelRegistry_*
UiRuntime_ManagerLayerModel_*
UiRuntime_ManagerPanelMap_*
UiRuntime_ManagerPopupStack_*
UiRuntime_ManagerFullscreenStack_*
UiRuntime_PanelParameter_*
```

## 6. Closeout Notes

This validation document does not imply UI Framework completion. It only records
that the generic UIManager runtime framework first slice has evidence. UI Core,
components, Grid/List virtualization, invalidation, atlas, batching, and Web
Editor work still have their own gates.
