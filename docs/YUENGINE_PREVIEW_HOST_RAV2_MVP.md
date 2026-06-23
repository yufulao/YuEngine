# YuEngine Preview Host RAV2 MVP

Status: implemented first slice for #YuPart task #65
Owner: Preview Host implementation
Target: `YuPreviewHost`

## Scope

`YuPreviewHost` is the first engine preview-host value/session layer. It is not
a native editor app, local editor service, Scene Editor, Animation Editor, UI
Editor, Resource Browser panel, source format contract, import-settings UX, or
cook command owner.

The MVP consumes RuntimeAsset-loaded state:

```text
RuntimeAssetGraphLoadResult
-> RuntimeAssetLoadedFile records
-> RuntimeAssetSceneResourceRef records
-> RuntimeAssetSceneLoaderOutput and scene entities
-> RenderScene runtime frame/capture route
-> RenderCore/RHI capture output or explicit blocker
```

The host does not infer type from path suffixes and does not accept editor mock
scene data as proof. Renderable geometry, material, and camera values supplied
to the MVP must already be linked to loaded RuntimeAsset Resource/Asset handles
and are validated against the loaded graph before RenderScene submission.

## Public Values

The first slice adds bounded value records for:

- session identity and start/stop lifecycle;
- frame descriptor for headless or RGBA8 capture output;
- camera state identity fields for later viewport/orbit control;
- bounded diagnostics;
- hit, selection, and transform feedback records;
- frame/capture result flags proving RuntimeAsset, Resource/Asset,
  RenderScene, RenderCore, and RHI consumption.

`BuildFrame` rejects stale sessions before graph validation and writes no
selection/hit/transform feedback on failure. Diagnostics are caller-owned and
bounded; if the caller provides no diagnostic capacity, the host returns
`OutputCapacityExceeded`.

## Diagnostic Contract

| Case | Status | Diagnostic |
| --- | --- | --- |
| missing graph/result | `InvalidArgument` | `MissingRuntimeAssetGraph` |
| stale session | `StaleSession` | `StaleSession` |
| failed RuntimeAsset load/result/output | `RuntimeAssetStatusFailed` | `RuntimeAssetStatusFailed` |
| stale graph counts or stale resource refs | `RuntimeAssetGraphStale` | `RuntimeAssetGraphStale` or `StaleResourceRef` |
| missing Resource/Asset handle/ref | `MissingResourceRef` | `MissingResourceRef` |
| expected vs loaded kind/asset mismatch | `TypeMismatch` | `TypeMismatch` |
| unsupported document kind or non-canonical MVP route | `UnsupportedDocumentKind` / `UnsupportedPreviewRoute` | matching diagnostic |
| required mesh/material/texture payload not decoded/cooked | `NotCooked` | `NotCooked` |
| camera/frame/RenderScene failure | `MissingCamera` / `RenderSceneFailed` | exact frame status |
| RHI/capture failure | `RenderCoreRhiFailed` | capture missing layer |

For this MVP the canonical capture route requires the existing cube/cylinder/cone
three-entity scene. Other document kinds and non-canonical scene shapes return
explicit unsupported statuses instead of falling back to mock preview data.

## Evidence

Focused tests:

```powershell
ctest --preset windows-fast-gate -R "PreviewHost" --output-on-failure
```

The focused suite covers:

- `PreviewHost_ConsumesRuntimeAssetGraphAndCapturesThroughRhi`
- `PreviewHost_ReportsBoundedResourceDiagnostics`
- `PreviewHost_RejectsStaleSessionWithoutMutation`
- `PreviewHost_ReportsNotCookedRuntimeAssetRef`

The capture proof uses the existing RuntimeAsset disk fixture generator and
loader path, then validates loaded graph/status/ref records before calling the
RenderScene three-primitive capture route. The successful path captures through
RenderCore/RHI in the fast gate. No GDI viewer, CPU PPM substitute, screenshot,
report, Web output, or editor mock scene is part of the acceptance route.

## Current Blockers

No fast-gate blocker exists for the current NullRHI-backed capture route. If a
target hardware/D3D11 capture layer is unavailable in a later hardware smoke, the
Preview Host status to surface is `RenderCoreRhiFailed` with
`RhiCaptureTarget` as the missing layer; that is distinct from data/loader
failures such as `NotCooked`, `MissingResourceRef`, or `TypeMismatch`.

Remaining later work:

- consume RAV2 Resource Browser query records when task #64 lands;
- consume RAV2-A command/cook outputs when task #63 lands;
- broaden beyond the canonical three-primitive scene only after RuntimeAsset
  exposes production mesh/material/scene records for the wider route;
- add editor service/panel integration in later tasks, not in #65.
