# YuEngine UI Stage 2 Validation

Status: ENG-192A UI-S2-015 validation document
Baseline: `origin/main@7961c761ba15ed80e7860779d3a4a8a98283c498`
Source plan: `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md` section 7

## 1. Scope

This document records the Stage 2 validation path for `UI-S2-001` through
`UI-S2-015`.

Included:

- backend-independent `YuUiCore` invalidation, dirty tracking, draw batching,
  atlas, font, component, and GridView/List virtualization mechanisms.
- `Samples/UiComponentSmoke` component library smoke sample.
- `Tests/UiCore`, `Tests/UiComponentSmoke`, and the focused CTest routes listed
  below.
- Component performance diagnostics emitted by the Stage 2 smoke sample.

Excluded:

- `UI-S3-001+` UIManager runtime framework.
- Web Editor backlog work.
- native editor runtime/backend import.
- RHI, D3D11, RenderCore, or backend handles in `Src/YuEngine/UiCore`.
- game-specific window work and UIManager lifecycle policy.

## 2. Acceptance Mapping

| Plan item | Evidence path | Acceptance |
| --- | --- | --- |
| `UI-S2-001` | `docs/YUENGINE_UI_STAGE2_INVALIDATION_MODEL.md`, `UiDirtyTracker`, `UiInvalidationModel` | Dirty change types and `Self`/`Subtree` invalidation rules are documented and tested. |
| `UI-S2-002` | `UiCacheCounters`, `UiInvalidationResult`, `Tests/UiCore/UiCoreTests.cpp` | Layout and paint rebuild counts are exposed by focused tests. |
| `UI-S2-003` | `UiDrawBatcher`, `Tests/UiCore/UiDrawBatcherTests.cpp` | Draw elements batch deterministically by compatible state keys and split on text/texture state. |
| `UI-S2-004` | `UiStaticAtlasMetadata`, `Tests/UiCore/UiCoreTests.cpp` | Sprite refs resolve to page, UV, and nine-slice metadata or explicit missing/invalid status. |
| `UI-S2-005` | `UiDynamicAtlasPacker`, `Tests/UiCore/UiDynamicAtlasTests.cpp` | Dynamic atlas packing occurs outside paint path and reports overflow/capacity failures without mutation. |
| `UI-S2-006` | `UiFontGlyphAtlas`, `Tests/UiCore/UiFontGlyphAtlasTests.cpp` | Font assets, glyphs, fallback, and missing glyph status are covered. |
| `UI-S2-007` | `UiTextComponent`, `Tests/UiCore/UiTextComponentTests.cpp` | Text alignment, wrapping, localization placeholder, overflow, outline/shadow, and dirty classification are tested. |
| `UI-S2-008` | `UiImageComponent`, `Tests/UiCore/UiImageComponentTests.cpp` | Sprite, tint, nine-slice, missing sprite, small output, and paint invalidation behavior are tested. |
| `UI-S2-009` | `UiButtonComponent`, `Tests/UiCore/UiButtonComponentTests.cpp` | Normal, hover, pressed, disabled, selected, pointer activation, keyboard/gamepad placeholder activation, hooks, and no-layout-rebuild state dirty are tested. |
| `UI-S2-010` | `UiSliderComponent`, `Tests/UiCore/UiSliderComponentTests.cpp` | Value/fill/handle mapping, pointer capture, keyboard/gamepad adjustment, value hooks, and no-layout-rebuild value dirty are tested. |
| `UI-S2-011` | `UiGridViewSemantics`, `Tests/UiCore/UiGridViewSemanticsTests.cpp` | FancyScrollView-style grouping, visible buffer range, selection, and scroll-to-index semantics are mapped. |
| `UI-S2-012` | `UiGridViewVirtualizer`, `Tests/UiCore/UiGridViewVirtualizerTests.cpp` | Large lists use visible plus buffer pool records and reject full-pool non-virtualized behavior. |
| `UI-S2-013` | `Samples/UiComponentSmoke`, `Tests/UiComponentSmoke/UiComponentSmokeSampleTests.cpp` | One sample window uses Text, Image, Button, Slider, and GridView/List virtualization together. |
| `UI-S2-014` | `UiComponentPerformanceDiagnostics`, `UiComponentSmokeSample_ReportsPerformanceDiagnostics` | Sample reports draw calls, batches, atlas pages, rebuild counts, and list cell count. |
| `UI-S2-015` | this document | Mechanism evidence, component evidence, commands, sample output, and scope exclusions are recorded. |

## 3. Accepted Evidence Commits

| Commit | Plan item | Evidence summary |
| --- | --- | --- |
| `fdfc301` | `UI-S2-001`, `UI-S2-002` | Added invalidation model, cache counters, and Stage 2 invalidation document. |
| `193b1ea` | `UI-S2-003` | Added draw-element batching prototype and tests. |
| `6c44811` | `UI-S2-004` | Added static atlas metadata path and tests. |
| `22b59a1` | `UI-S2-005` | Added dynamic atlas first slice and tests. |
| `ce928e9` | `UI-S2-006` | Added font asset/glyph atlas path and tests. |
| `c851e99` | `UI-S2-007` | Added Text component first slice and tests. |
| `57e4349` | `UI-S2-008` | Added Image component first slice and tests. |
| `791de7b` | `UI-S2-009` | Added Button component first slice and tests. |
| `4f6611f` | `UI-S2-010` | Added Slider component first slice and tests. |
| `dbdd3ff` | `UI-S2-011` | Added GridView semantics model and tests. |
| `6ebd535` | `UI-S2-012` | Added GridView virtualization path and tests. |
| `d155d9a` | `UI-S2-013` | Added component library smoke sample and tests. |
| `7961c76` | `UI-S2-014` | Added component performance diagnostics to the smoke sample. |

## 4. Validation Commands

Focused build and test route:

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuUiCoreTests YuUiDrawBatcherTests YuUiDynamicAtlasTests YuUiGridViewSemanticsTests YuUiGridViewVirtualizerTests YuUiFontGlyphAtlasTests YuUiButtonComponentTests YuUiSliderComponentTests YuUiImageComponentTests YuUiTextComponentTests YuUiComponentSmokeSample YuUiComponentSmokeSampleTests -- /v:minimal
ctest --preset windows-fast-gate -R "^(UiCore_(DirtyTracker|InvalidationModel|DrawBatcher|StaticAtlasMetadata|DynamicAtlasPacker|FontGlyphAtlas|TextComponent|ImageComponent|ButtonComponent|SliderComponent|GridViewSemantics|GridViewVirtualizer)_|UiComponentSmokeSample_)" --output-on-failure
.\build\windows-fast-gate-vs\Debug\YuUiComponentSmokeSample.exe --layout Samples\UiComponentSmoke\Layouts\ComponentWindow.YuUILayout.json
git diff --check
```

Optional full Stage 2 UI sample route:

```powershell
cmake --build --preset windows-fast-gate --target YuUiComponentSmokeSample YuUiComponentSmokeSampleTests -- /v:minimal
ctest --preset windows-fast-gate -R "^UiComponentSmokeSample_" --output-on-failure
```

## 5. Expected Evidence

Expected focused CTest groups:

```text
UiCore_DirtyTracker_*
UiCore_InvalidationModel_*
UiCore_DrawBatcher_*
UiCore_StaticAtlasMetadata_*
UiCore_DynamicAtlasPacker_*
UiCore_FontGlyphAtlas_*
UiCore_TextComponent_*
UiCore_ImageComponent_*
UiCore_ButtonComponent_*
UiCore_SliderComponent_*
UiCore_GridViewSemantics_*
UiCore_GridViewVirtualizer_*
UiComponentSmokeSample_*
```

Expected component smoke sample output:

```text
YuUiComponentSmokeSample PASS nodes=11 text=4 image=1 button=900 slider=0.25 gridVisible=10 gridPool=20 gridDirty=3 diagDraw=5 diagBatches=2 diagAtlasPages=1 diagLayoutRebuild=10 diagPaintRebuild=1 diagListCells=20
```

## 6. Mechanism Notes

Stage 2 validates this low-level path:

```text
component data -> UiNodeTree -> layout/invalidation -> component state -> draw records -> batching/atlas diagnostics
```

Mechanism constraints:

- `HoverState`, `PaintOnly`, `Text`, `ScrollOffset`, `AtlasPage`, and slider
  value dirty routes do not trigger layout rebuild counters.
- Dynamic atlas packing is a safe-point operation and is rejected from paint hot
  path packing.
- Draw batching stays deterministic and splits when element type, texture, text,
  material, or style keys differ.
- GridView/List uses visible plus buffer pool cells; full item creation is a
  failure condition.
- Diagnostics are observational values emitted by the smoke sample; they do not
  own runtime behavior.

## 7. Boundary Notes

`Samples/UiComponentSmoke` is a Stage 2 component-library sample. It intentionally
does not route through RenderCore or RHI. The accepted Stage 2 evidence keeps the
component model inside `YuUiCore` value records and focused test fixtures.

Generic UIManager runtime and Web Editor work remain future stages:

- UIManager runtime lifecycle, registry, layer, map, stack, args, and
  cache/release policy begin at `UI-S3-001+`.
- Web Editor work starts from schema, local backend service, a real
  TypeScript/React-style frontend workspace, and preview protocol tasks.
- Normal hierarchy/inspector/canvas/template/theme/state-preview iteration must
  not be modeled as C++ tool targets or require CMake rebuilds.
- Native app editors, immediate-mode editor fallbacks, and game-window migration
  are out of scope.
