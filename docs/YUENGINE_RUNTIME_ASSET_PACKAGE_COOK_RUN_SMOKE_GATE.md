# YuEngine RuntimeAsset Package / Cook / Run Smoke Gate

Status: RAV3-G packaged RuntimeAsset entrypoint smoke
Task: #75
Base: RAV3 RuntimeAsset visual proof and Resource Browser/PreviewHost gates
Decision: `RUNTIME_ASSET_PACKAGE_RUN_ENTRYPOINT_PASS`

## Purpose

This gate defines the first executable package/cook/run smoke for the
deterministic RuntimeAsset sample without pretending that YuEngine has a final
installer or release package runner.

The current accepted route is:

```text
RuntimeAsset import/cook command
-> deterministic source+cooked disk fixture
-> File/VFS validation and RuntimeAssetData graph load
-> Resource/Asset registration and dependency edges
-> synthetic Package manifest and Package load plan for cooked records
-> RenderScene / RenderCore / RHI cooked visual proof
-> RuntimeAsset packaged run entrypoint consumes the Package load plan
-> RuntimeApp owns the entrypoint module and fixed-frame loop
-> deterministic PASS ledger
```

RAV3-G removes the previous exact blocker
`BlockedByLayer=PackagedRuntimeEntryPoint`. YuEngine now exposes an
engine-owned RuntimeAsset packaged run entrypoint that consumes a Package load
plan, bootstraps the cooked RuntimeAsset graph, and runs the render proof through
`RuntimeApp` module ownership.

## Current Smoke Result

`RuntimeAssetData_PackageCookRunSmokeRunsPackagedRuntimeEntryPoint` is the
focused executable gate. It must report these floors:

| Stage | Required result |
| --- | --- |
| Import/cook command | source and cooked deterministic fixtures written and validated |
| RuntimeAsset load | cooked graph loads through File/VFS into RuntimeAssetData |
| Resource/Asset | scene and dependencies are registered in Resource and Asset graphs |
| Package | cooked scene and cooked records register into a synthetic manifest and resolve as a load plan |
| Render route | cooked records drive RenderScene/RenderCore/RHI visual proof |
| Runtime entrypoint | `RunRuntimeAssetPackagedEntryPoint` consumes the Package load plan |
| Runtime loop | `RuntimeApp` runs the entrypoint module through a fixed-frame loop successfully |
| Product run | PASS with `RuntimeAssetPackagedRunBlockedLayer::None` |

The Package stage is intentionally synthetic at this slice. It proves manifest,
entry, dependency, and load-plan wiring for the cooked RuntimeAsset records. It
is not a package-file format, installer, launcher, product boot command, remote
cook service, or original-game package adapter.

## Closed Layer

The previous `BlockedByLayer=PackagedRuntimeEntryPoint` meant:

- there is no engine-owned run command that takes a Package load plan as input;
- `RuntimeApp` does not yet own a RuntimeAsset scene bootstrap module;
- the Package registry is not yet backed by a final package artifact file;
- no product package/install/run executable has been accepted.

RAV3-G closes the first two bullets for the deterministic cooked RuntimeAsset
route. The remaining bullets stay intentionally out of scope: Package file
artifacts, installers, final launchers, and release packaging are not approved by
this gate.

## Required Commands

Focused gate:

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_PackageCookRunSmokeRunsPackagedRuntimeEntryPoint" --output-on-failure
```

Related package/cook/render confidence:

```powershell
ctest --preset windows-fast-gate -R "RuntimeAssetData_(ImportCookCommand|CookedRecordsDriveRuntimeVisualProof|PackageCookRunSmoke)" --output-on-failure
ctest --preset windows-fast-gate -R "RuntimeAssetData_(PackageRunEmitsGenericRenderSceneSubmissionLedger|PackageRunRejectsGenericSubmissionCapacityWithoutMutation)" --output-on-failure
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
- Scene Editor, Animation Editor, UI Editor, rejected editor route, or editor input;
- screenshots, manual inspection, CPU PPM, GDI, or software raster as final
  proof;
- device-backed visual closure beyond the existing RAV3-A route unless the
  hardware-smoke command is run and passes.

## Next Required Slice

The next product slice should move beyond the deterministic synthetic manifest
into a separately approved package artifact or product-run layer:

```text
Package artifact / manifest
-> Package load plan
-> RuntimeAsset graph bootstrap
-> RuntimeApp module/service ownership
-> RenderScene / RenderCore / RHI run loop
-> deterministic process exit code and capture/output ledger
```

RAV3-G does not authorize that broader package artifact or final product visual
closure work.

RAV4-A and the product-run/generic submission ledger rows are tracked
separately in `docs/YUENGINE_RUNTIME_ASSET_PACKAGE_ARTIFACT_PRODUCT_RUN_GATE.md`.
