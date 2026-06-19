# YuEngine UI Stage 1 Validation

Status: ENG-183B UI-S1-009 validation document
Baseline: `origin/main@bf10c4611230b12847cdc558d057a41e7d951e51`
Source plan: `docs/YUENGINE_UI_FRAMEWORK_EDITOR_PLAN.md` section 6.3

## 1. Scope

This document records the Stage 1 validation path for `UI-S1-008` and
`UI-S1-009`.

Included:

- `Samples/UiCoreSmoke` smoke sample.
- `Tests/UiCoreSmoke/UiCoreSmokeSampleTests.cpp` focused validation.
- `YuUiRenderCoreBridge` as the accepted RenderCore submission adapter.

Excluded:

- Stage2 Text/Image/Button/Slider/GridView component implementation.
- Project UI Runtime lifecycle/config/window stack.
- UI Editor runtime preview.
- Dear ImGui runtime import.
- RHI, D3D11, or backend handles in `Src/YuEngine/UiCore`.

## 2. Acceptance Mapping

| Plan item | Evidence path | Acceptance |
| --- | --- | --- |
| `UI-S1-008` | `Samples/UiCoreSmoke` | Sample loads `SimpleWindow.YuUILayout.json`, builds a UI node tree, resolves layout, creates draw elements, submits them through `YuUiRenderCoreBridge`, and reports process `PASS` or `FAIL`. |
| `UI-S1-009` | this document | Commands, evidence paths, and exclusion boundaries are recorded for Stage 1 verification. |

## 3. Commands

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuUiCoreSmokeSample YuUiCoreSmokeSampleTests -- /v:minimal
ctest --preset windows-fast-gate -R "^UiCoreSmokeSample_" --output-on-failure
.\build\windows-fast-gate-vs\Debug\YuUiCoreSmokeSample.exe --layout Samples\UiCoreSmoke\Layouts\SimpleWindow.YuUILayout.json
git diff --check
```

## 4. Expected Evidence

Expected focused CTest results:

```text
UiCoreSmokeSample_LoadsLayoutRendersWindowReportsPass
UiCoreSmokeSample_RejectsInvalidLayoutWithoutRenderSubmission
UiCoreSmokeSample_ValidationRouteDocumentsCommands
```

Expected sample output:

```text
YuUiCoreSmokeSample PASS nodes=3 containers=2 draws=2 submitted=2 renders=2
```

## 5. Boundary Notes

`Samples/UiCoreSmoke` uses the Stage 1 path:

```text
YuUILayout text -> UiNodeTree -> UiLayoutPass -> UiDrawListBuilder -> YuUiRenderCoreBridge -> RenderCore fixture -> NullRhiDevice
```

The sample does not add new UI Core public dependencies. `YuUiRenderCoreBridge`
remains the adapter that knows about RenderCore and RHI fixture types.
