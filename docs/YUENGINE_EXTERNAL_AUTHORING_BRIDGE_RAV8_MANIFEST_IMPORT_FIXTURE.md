# YuEngine External Authoring Bridge RAV8 Manifest Import Fixture

Owner: Architecture
Target: ExternalAuthoring manifest parser and RuntimeAsset import/cook input bridge
Status: implementation slice
Task: #108

## Purpose

This RAV8 slice implements the first deterministic `YuExternalAuthoringExport`
bridge after the RAV7 contract. The bridge is intentionally narrow:

```text
YuExternalAuthoringExport manifest
-> File/VFS read
-> manifest parse
-> payload presence check
-> dependency and mapping policy validation
-> RuntimeAsset source row
-> RuntimeAsset import/cook command input
```

It does not execute Unity, Unreal, DCC, FBX, glTF, live sync, external
viewports, or package/product-run closure.

## Manifest Fixture

The first accepted fixture is a line-based deterministic manifest:

```text
YuExternalAuthoringExport v1
tool=Unity
export_id=rav8_manifest_fixture
unit_scale=1
handedness=right
up_axis=y
transform_bake=world
material_policy=yu_material_v0
animation_policy=sampled_clip_v0
unsupported_feature_count=0
entry_count=2
entry.0.kind=Mesh
entry.0.stable_id=1001
entry.0.payload=Mesh/Cube.yumesh
entry.0.content_hash=0
entry.0.dependency_count=0
entry.1.kind=Scene
entry.1.stable_id=7001
entry.1.payload=Scene/Main.yuscene
entry.1.content_hash=0
entry.1.dependency_count=1
entry.1.dependency.0=1001
```

`content_hash=0` means the manifest accepts the computed payload hash and emits
it into the RuntimeAsset input row. A nonzero value must match the File/VFS
payload bytes.

## Validated Policies

The accepted mapping policy is:

- `tool` is `Unity`, `Unreal`, or `Dcc`;
- `unit_scale=1`;
- `handedness=right`;
- `up_axis=y`;
- `transform_bake=world`;
- `material_policy=yu_material_v0`;
- `animation_policy=sampled_clip_v0`;
- `unsupported_feature_count=0`;
- every declared dependency stable id resolves to another manifest entry.

Rejected manifests return an explicit blocked layer and do not write
caller-owned RuntimeAsset rows or the import/cook command request.

## RuntimeAsset Bridge

Accepted entries emit `ExternalAuthoringRuntimeAssetInputRow` records with:

- row-owned manifest and payload paths;
- parsed tool kind;
- target `RuntimeAssetFileKind`;
- stable Resource/Asset type ids for the target family;
- computed payload hash;
- dependency count and readiness flags;
- a row-owned `RuntimeAssetFileDesc` source descriptor.

The bridge also emits a `RuntimeAssetImportCookCommandRequest` for the current
`GenerateDeterministicDiskFixture` RuntimeAsset import/cook command, using the
caller-provided descriptor buffers. This proves the external-authoring preflight
can hand off to the engine-owned RuntimeAsset import/cook API without external
tooling.

## Tests

Focused tests:

```text
ExternalAuthoringBridge_AcceptsManifestAndEmitsRuntimeAssetImportCookInputs
ExternalAuthoringBridge_RejectsMissingPayloadWithoutMutation
ExternalAuthoringBridge_RejectsInvalidDependencyWithoutMutation
ExternalAuthoringBridge_RejectsUnsupportedFeatureWithoutMutation
ExternalAuthoringBridge_RejectsUnsupportedMappingPolicyWithoutMutation
ExternalAuthoringBridge_RejectsOutputCapacityWithoutMutation
```

Gate commands:

```powershell
cmake --preset windows-fast-gate
cmake --build --preset windows-fast-gate --target YuExternalAuthoringBridgeTests -- /v:minimal
ctest --preset windows-fast-gate -R "^ExternalAuthoringBridge_" --output-on-failure
```

## Boundary

This slice is not a full importer. It has no Unity Editor plugin, Unreal Editor
plugin, DCC SDK parser, live sync, embedded viewport, original package parser,
native editor mutation, Resource/Asset graph mutation, Preview Host mutation, or
final product proof.
