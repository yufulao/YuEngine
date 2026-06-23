# YuEngine RuntimeAsset Package / Cook / Run Smoke Gate

Status: RAV3-F first smoke gate
Task: #74
Base: RAV3 RuntimeAsset visual proof and Resource Browser/PreviewHost gates
Decision: `RUNTIME_ASSET_PACKAGE_RUN_ENTRYPOINT_BLOCKED`

## Purpose

This gate defines the first executable package/cook/run smoke for the
deterministic RuntimeAsset sample without pretending that YuEngine has a final
product package runner.

The current accepted route is:

```text
RuntimeAsset import/cook command
-> deterministic source+cooked disk fixture
-> File/VFS validation and RuntimeAssetData graph load
-> Resource/Asset registration and dependency edges
-> synthetic Package manifest and Package load plan for cooked records
-> RenderScene / RenderCore / RHI cooked visual proof
-> Kernel RuntimeApp fixed-frame loop
-> BlockedByLayer=PackagedRuntimeEntryPoint
```

The final blocker is exact: YuEngine can prove the cooked RuntimeAsset graph,
Package registry/load-plan data, RHI visual route, and generic RuntimeApp loop,
but it does not yet have a packaged runtime entrypoint that consumes the Package
load plan and launches the RuntimeAsset scene as a product run.

## Current Smoke Result

`RuntimeAssetData_PackageCookRunSmokeReportsPackagedRuntimeEntryPointBlocker`
is the focused executable gate. It must report these floors before the blocker:

| Stage | Required result |
| --- | --- |
| Import/cook command | source and cooked deterministic fixtures written and validated |
| RuntimeAsset load | cooked graph loads through File/VFS into RuntimeAssetData |
| Resource/Asset | scene and dependencies are registered in Resource and Asset graphs |
| Package | cooked scene and cooked records register into a synthetic manifest and resolve as a load plan |
| Render route | cooked records drive RenderScene/RenderCore/RHI visual proof |
| Runtime loop | `RuntimeApp` runs a fixed-frame loop successfully |
| Product run | `BlockedByLayer=PackagedRuntimeEntryPoint` |

The Package stage is intentionally synthetic at this slice. It proves manifest,
entry, dependency, and load-plan wiring for the cooked RuntimeAsset records. It
is not a package-file format, installer, launcher, product boot command, remote
cook service, or original-game package adapter.

## Missing Layer

`BlockedByLayer=PackagedRuntimeEntryPoint` means:

- there is no engine-owned run command that takes a Package load plan as input;
- `RuntimeApp` does not yet own a RuntimeAsset scene bootstrap module;
- the Package registry is not yet backed by a final package artifact file;
- no product package/install/run executable has been accepted.

This is not `BlockedByEnv`. It is a missing engine/product layer.

## Required Commands

Focused gate:

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_PackageCookRunSmokeReportsPackagedRuntimeEntryPointBlocker" --output-on-failure
```

Related package/cook/render confidence:

```powershell
ctest --preset windows-fast-gate -R "RuntimeAssetData_(ImportCookCommand|CookedRecordsDriveRuntimeVisualProof|PackageCookRunSmoke)" --output-on-failure
ctest --preset windows-fast-gate -R "Package_" --output-on-failure
ctest --preset windows-fast-gate -R "Kernel_RuntimeApp" --output-on-failure
```

Full gate:

```powershell
git diff --check
git show --check --format=short HEAD
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
```

## Boundaries

This gate does not accept:

- final product packaging or installer output;
- package-file serialization/parsing beyond current Package registry/load-plan
  data;
- original TouhouNewWorld package compatibility;
- Unity/UE/DCC import bridge;
- Resource Browser UI or import settings UI;
- Scene Editor, Animation Editor, UI Editor, Web, or editor input;
- screenshots, manual inspection, CPU PPM, GDI, or software raster as final
  proof;
- device-backed visual closure beyond the existing RAV3-A route unless the
  hardware-smoke command is run and passes.

## Next Required Slice

The next product slice should implement a real packaged runtime entrypoint:

```text
Package artifact / manifest
-> Package load plan
-> RuntimeAsset graph bootstrap
-> RuntimeApp module/service ownership
-> RenderScene / RenderCore / RHI run loop
-> deterministic process exit code and capture/output ledger
```

Only that later slice may remove `BlockedByLayer=PackagedRuntimeEntryPoint`.
