# YuEngine RuntimeAsset Package Artifact Product-Run Gate

Status: RAV4-A file-backed Package artifact smoke
Tasks: #76, #77
Decision: `RUNTIME_ASSET_PACKAGE_ARTIFACT_PRODUCT_RUN_SMOKE`
RAV6 authored editor data smoke: `docs/YUENGINE_EDITOR_PACKAGE_RUN_RAV6_AUTHORED_DATA_SMOKE.md`

## Purpose

This gate moves one layer beyond the RAV3-G synthetic in-memory Package
manifest/load-plan route. The accepted first slice is a deterministic Package
artifact fixture that is written through File/VFS, read back through File/VFS,
rebuilt into a Package registry, resolved into a load plan, and then consumed by
the existing RuntimeAsset packaged run entrypoint.

RAV4-B adds the first product-run command seam on top of that artifact. The
command takes a Package artifact virtual path and package lookup key, reads the
artifact through File/VFS, rebuilds the Package registry, resolves the load
plan internally, and then calls the RuntimeAsset packaged entrypoint. The caller
does not provide a `PackageLoadPlan` pointer as the product-run boundary.

The accepted route is:

```text
RuntimeAsset import/cook command
-> deterministic source+cooked disk fixture
-> Package artifact file written through MountTable/File
-> Package artifact file read through MountTable/File
-> Package registry rebuilt from artifact bytes
-> Package load plan resolved from rebuilt registry
-> RunRuntimeAssetPackagedEntryPoint consumes that load plan
-> cooked RuntimeAsset graph / Resource / Asset / shader decode
-> RenderScene / RenderCore / RHI cooked visual proof
-> RuntimeApp fixed-frame loop
-> deterministic PASS ledger
```

## Executable Gates

Focused Package artifact tests:

```powershell
ctest --preset windows-fast-gate -R "Package_FileBackedArtifact" --output-on-failure
```

Focused RuntimeAsset product-run smoke:

```powershell
ctest --preset windows-fast-gate -R "RuntimeAssetData_PackageArtifactCookRunSmokeRunsProductRuntimeEntryPoint" --output-on-failure
```

Focused product-run command seam:

```powershell
ctest --preset windows-fast-gate -R "RuntimeAssetData_ProductRunCommand" --output-on-failure
```

Related package/cook confidence:

```powershell
ctest --preset windows-fast-gate -R "RuntimeAssetData_(ImportCookCommand|CookedRecordsDriveRuntimeVisualProof|PackageCookRunSmoke|PackageArtifactCookRunSmoke)" --output-on-failure
ctest --preset windows-fast-gate -R "Package" --output-on-failure
```

Full gate:

```powershell
git diff --check
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
```

## Boundaries

This gate accepts only deterministic fixture artifact ownership. It does not
accept:

- installer, patcher, launcher, or release package output;
- a final command-line interface or user-facing executable launcher;
- original TouhouNewWorld package compatibility;
- archive compression, encryption, signing, or patch manifest format;
- remote cook service;
- Unity/UE/DCC import bridge;
- Resource Browser UI, Preview Host UI, Scene Editor, Animation Editor, UI
  Editor, rejected editor route, or editor input;
- screenshots, manual inspection, CPU PPM, GDI, or software raster as final
  proof.

## Next Slice

The next slice must be separately scoped. Likely candidates are a product run
command/executable boundary or a richer package artifact manifest, but neither
is approved by this gate.
