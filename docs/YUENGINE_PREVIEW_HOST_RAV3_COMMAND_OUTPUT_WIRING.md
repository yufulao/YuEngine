# YuEngine Preview Host RAV3 Command Output Wiring

Status: implemented first slice for #YuPart task #70
Base: `origin/main@9009eb2`
Target: `YuPreviewHost`

Next viewport session slice:
`docs/YUENGINE_PREVIEW_HOST_RAV4_VIEWPORT_SESSION_SURFACE.md`

## Scope

This slice wires the Engine Preview Host to the #63 RuntimeAsset import/cook
command output contract. The host still does not own import/cook execution,
source formats, Resource Browser UI, Scene Editor UI, original package parsing,
Unity/UE importers, or final device-backed product visual closure.

The accepted route is:

```text
ExecuteRuntimeAssetImportCookCommand
-> deterministic source+cooked disk outputs
-> LoadRuntimeAssetDataGraph from cooked scene/files
-> YuPreviewHost command-output validation
-> RenderScene / RenderCore / RHI capture route
```

`YuPreviewHost` now accepts a `PreviewHostCommandOutputRef` on a frame request.
When `require_cooked_records` is true, `BuildFrame` validates the import/cook
command result, cooked scene descriptor, cooked file descriptors, and loaded
RuntimeAsset graph before it writes frame, capture, hit, selection, or transform
feedback outputs.

## Diagnostics

| Case | Preview Host status | Diagnostic |
| --- | --- | --- |
| missing command output or missing File/VFS command output | `MissingCommandOutput` | `MissingCommandOutput` with `Command` or `FileVfs` |
| invalid cooked descriptor, cooked scene mismatch, or loaded cooked record mismatch | `InvalidCookedRecord` | `InvalidCookedRecord` with `RuntimeAssetData` |
| Resource/Asset command bridge layer requested before this host slice owns it | `UnsupportedBridgeLayer` | `UnsupportedBridgeLayer` with `Resource` or `Asset` |

The existing #65 diagnostics remain intact for RuntimeAsset graph failures,
missing/stale resource refs, type mismatches, not-cooked payloads, stale
sessions, unsupported document kinds/routes, missing camera, RenderScene
failure, and RenderCore/RHI capture failure.

## Evidence

New focused test:

```text
PreviewHost_ConsumesImportCookCommandOutputs
```

The test executes the deterministic import/cook command, loads the generated
cooked disk outputs through RuntimeAsset/File/VFS/Resource/Asset, then builds a
Preview Host frame and capture through RenderScene/RenderCore/RHI. It also
checks bounded diagnostics for missing command output, invalid cooked records,
and unsupported command bridge layers.

Recommended focused commands:

```powershell
ctest --preset windows-fast-gate -R "PreviewHost" --output-on-failure
ctest --preset windows-fast-gate -R "RuntimeAssetData_ImportCookCommand|PreviewHost_ConsumesImportCookCommandOutputs" --output-on-failure
```

## Boundaries

This slice proves Preview Host can require and verify deterministic command/cook
disk outputs before frame/capture submission. It does not make Preview Host a
cook command owner, Resource Browser diagnostics owner, editor surface, original
package parser, or product visual closure owner. #73 must treat this as one
dependency input for the later Scene Editor gate, not as Scene Editor approval.
