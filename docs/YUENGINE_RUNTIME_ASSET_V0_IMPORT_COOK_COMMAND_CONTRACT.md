# YuEngine RuntimeAsset v0 Import/Cook Command Contract

Status: RAV2-A implementation contract
Task: #63
Base: RAV1 implementation baseline `origin/main@d1a1b86`

Related evidence:

- `docs/YUENGINE_RUNTIME_ASSET_V0_RAV2_EVIDENCE_MATRIX.md`
- `docs/YUENGINE_RUNTIME_ASSET_PACKAGE_COOK_RUN_SMOKE_GATE.md`
- `docs/YUENGINE_EXTERNAL_AUTHORING_BRIDGE_RAV7_CONTRACT.md`
- `docs/YUENGINE_EXTERNAL_AUTHORING_BRIDGE_RAV8_MANIFEST_IMPORT_FIXTURE.md`

## Purpose

RAV2-A adds the first engine-owned import/cook command surface before Resource
Browser UI, Preview Host, or final runtime visual proof work starts.

The command is intentionally narrow:

```text
RuntimeAsset import/cook command
-> deterministic source+cooked fixture generation
-> File/VFS disk writes
-> RuntimeAssetData validation
-> descriptors reusable by Resource/Asset graph load
```

It is not an editor workflow, original package parser, Unity/UE importer,
Resource Browser UI, Preview Host, or render evidence path. External authoring
exports must first pass the boundary contract in
`docs/YUENGINE_EXTERNAL_AUTHORING_BRIDGE_RAV7_CONTRACT.md` before they become
inputs to this command.

## Public API

The command entry is:

```cpp
RuntimeAssetDataStatus ExecuteRuntimeAssetImportCookCommand(
    const RuntimeAssetImportCookCommandRequest &request,
    RuntimeAssetImportCookCommandResult *out_result);
```

The first command kind is:

```cpp
RuntimeAssetImportCookCommandKind::GenerateDeterministicDiskFixture
```

It delegates to:

```cpp
RuntimeAssetDataStatus GenerateRuntimeAssetDeterministicDiskFixture(
    const RuntimeAssetDeterministicDiskFixtureRequest &request,
    RuntimeAssetDeterministicDiskFixtureResult *out_result);
```

The request must provide a `MountTable`, a `MountId`, and output buffers for
`RUNTIME_ASSET_DETERMINISTIC_FIXTURE_FILE_COUNT` source descriptors and cooked
descriptors. The command writes files through `MountTable::Write`; callers load
the returned descriptors through `LoadRuntimeAssetDataGraph`.

## Fixture Layout

The generator writes source and cooked artifacts for every RuntimeAsset family
currently accepted by RAV1:

| Family | Source locator | Cooked locator |
| --- | --- | --- |
| Mesh | `Mesh/Cube.yumesh`, `Mesh/Cylinder.yumesh`, `Mesh/Cone.yumesh` | `Mesh/Cube.racooked`, `Mesh/Cylinder.racooked`, `Mesh/Cone.racooked` |
| Material | `Material/Shared.yumat` | `Material/Shared.racooked` |
| Texture | `Texture/Albedo.yutex`, `Texture/Normal.yutex`, `Texture/Mask.yutex` | `Texture/Albedo.racooked`, `Texture/Normal.racooked`, `Texture/Mask.racooked` |
| Shader/program | `Shader/RuntimeProgram.yuprogram` | `Shader/RuntimeProgram.racooked` |
| Scene | `Scene/CanonicalScene.yuscene` | `Scene/CanonicalScene.racooked` |
| Animation | `Animation/Spin.yuanim` | `Animation/Spin.racooked` |

The `.yu*` and `.racooked` suffixes are locators only. Type truth comes from
the internal header and metadata:

```text
YUASSET/YUCOOKED magic
version
kind
schema
id
sourceHash
payloadHash
dependencyTable
recordTable
recordBytes
payloadBytes
payloadAlign
```

## Diagnostics

`RuntimeAssetImportCookMissingLayer` reports the first layer that blocks a
command:

| Layer | Meaning |
| --- | --- |
| `Command` | Unknown command kind or malformed command envelope. |
| `FileVfs` | Missing `MountTable` or File/VFS write failure. |
| `RuntimeAssetData` | Descriptor buffer capacity, validation, or RuntimeAssetData contract failure. |
| `Resource` | Reserved for follow-up commands that register or stage Resource records. |
| `Asset` | Reserved for follow-up commands that register Asset records. |

Failures return concrete `RuntimeAssetDataStatus` and, for File/VFS write
failure, the underlying `FileStatus`. The command must not report an
environment skip for a missing implementation layer.

## Verification

Focused RAV2-A tests:

```text
RuntimeAssetData_ImportCookCommandWritesSourceAndCookedDiskFixtures
RuntimeAssetData_ImportCookCommandLoadsGeneratedSourceAndCookedViaFileResourceRoute
RuntimeAssetData_ImportCookCommandReportsMissingLayerStatus
RuntimeAssetData_PackageCookRunSmokeRunsPackagedRuntimeEntryPoint
```

Gate commands:

```powershell
git diff --check
git show --check --format=short HEAD
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuRuntimeAssetDataClosedLoopTests -- /v:minimal
ctest --preset windows-fast-gate -R "RuntimeAssetData_ImportCookCommand" --output-on-failure
ctest --preset windows-fast-gate -R "RuntimeAssetData" --output-on-failure
cmake --build --preset windows-fast-gate -- /v:minimal
ctest --preset windows-fast-gate --output-on-failure
```

## Boundaries

RAV2-A does not accept:

- editor UI or Resource Browser UI;
- original package parsing;
- Unity/UE importers;
- Preview Host;
- final render closure;
- GDI, CPU PPM, screenshots, or software image generation as final evidence.
