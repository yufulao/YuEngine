# YuEngine RuntimeAsset v0 Payload Bridge To RHI Route Plan

Status: planning / API-plan
Owner: Architecture
Task: #52
Production contract: `docs/YUENGINE_RUNTIME_ASSET_DATA_CONTRACT_PLAN.md`
Production-gap closure: `docs/YUENGINE_RUNTIME_ASSET_V0_PRODUCTION_GAP_CLOSURE_PLAN.md`
Format policy and validator vocabulary: `docs/YUENGINE_RUNTIME_ASSET_V0_FORMAT_POLICY_AND_VALIDATOR_VOCABULARY.md`
Related gate: `docs/gates/L1_GATE_RUNTIME_ASSET_DATA_CLOSED_LOOP.md`
Coordinates with: task #50 production contract and task #51 loader transaction

## Purpose

This plan defines the RuntimeAsset v0 cooked texture/material/shader payload
route into RHI-backed texture, material slot, shader module, and pipeline
records.

It is Phase A docs/API-plan work only. It does not implement the bridge, add a
shader compiler, add editor/import UI, define package compatibility with the
original game, or promote CPU PPM/image helper output as acceptance.

The production proof remains:

```text
File/Mount/VFS bytes
-> validate/cook/load
-> Resource/Asset records
-> RenderScene/RenderCore/RHI records
-> RHI-backed capture evidence
```

Type truth comes from internal RuntimeAsset metadata, caller-provided expected
descriptors, and dependency records. Production success must not depend on
`.yu*` suffixes, fixture names, or direct C++ struct injection.

## RAV0 Smoke Audit

RAV0 #44 and #45 provide useful route floors, but they are still smoke
implementations compared with a production cooked payload bridge.

| Area | RAV0 floor | Still smoke-only |
| --- | --- | --- |
| Texture decoded payload | #44 routes loaded texture metadata through `ResourceDecodedTextureBridge`, then `AssetManager::MarkTextureReady`, then `RenderSceneRuntimeMaterialTextureSlot`. | Payload bytes still come from generated fixture text and fixed 2D `Rgba8Unorm` assumptions. There is no cooked texture payload table carrying row pitch, slice pitch, mip/layer layout, payload hash, color space, or per-texture descriptor ownership. |
| Material slots | #44 proves three material texture slots can be filled from decoded payload uploads and consumed by the capture route. | Slot count and slot meanings are fixture-shaped. The bridge helper lives in `RuntimeAssetDataClosedLoopTests.cpp`; production material records do not yet own a cooked texture slot table, sampler table, expected format/color-space checks, or slot overflow contract before RenderScene mutation. |
| Shader/program bytecode | #45 adds `RuntimeAssetLoadedShaderProgramData` and `BuildRuntimeAssetShaderProgramPipeline`, and RHI shader modules/pipeline are created from loaded/decoded program data. | The bytecode source is `bytecode:` text inside the RAV0 source fixture. It is not production compiler output, a cooked binary payload, a Resource decoded payload, a pipeline cache, or material/shader authoring system output. |
| Shader reflection/input layout | #45 carries an input layout and texture slot count from decoded program data. | The reflection surface is minimal and test-local: required semantics, shader stage payload ownership, slot counts, constant ranges, and bytecode hashes are not yet tied to a cooked record table. |
| Failure cleanup | #44/#45 already prove several no-mutation cases and cleanup of temporary shader handles. | Production still needs one staged bridge ledger that covers texture upload, sampler creation, shader module creation, pipeline creation, RenderScene material commit, and cleanup on any partial RHI failure. |

## Production Payload Ownership

### Texture Payload Records

The texture family owns both descriptor metadata and decoded byte layout. A
production cooked texture record must carry at least:

- `texture_id`, internal `kind`, `schema`, resource type, asset type, and stable
  RuntimeAsset id;
- `format`, `width`, `height`, `depth_or_layers`, `mip_count`, `array_count`,
  and usage flags;
- `row_pitch_bytes`, `slice_pitch_bytes`, `payload_offset`,
  `payload_byte_count`, `payload_alignment`, and `payload_hash`;
- color-space policy, sampler reference, and material slot compatibility flags;
- the Resource decoded payload identity used by the bridge:
  `payload_id`, `decode_plan_id`, `decode_result_id`, and `decoded_payload_id`.

For the current RHI surface this may still narrow to 2D `Rgba8Unorm`, but the
record must say that explicitly. Unsupported formats, mips, array layers, color
spaces, or usages must fail validation rather than being inferred away.

### Material Payload Records

The material family owns the texture/sampler binding table that RenderScene
will consume. A production material record must carry at least:

- `material_id`, material asset id, program reference, render-state id, and
  constant-buffer metadata if present;
- a bounded texture slot table with slot index, semantic/name, typed texture
  dependency, expected texture format/color space, sampler reference or sampler
  descriptor, and binding slot;
- an explicit maximum equal to the smallest active consumer limit:
  `MAX_RENDER_SCENE_RUNTIME_MATERIAL_TEXTURE_SLOTS`,
  `MAX_RHI_SAMPLED_TEXTURE_SLOTS`, and `MAX_RHI_SAMPLER_SLOTS`;
- dependency rows for every texture and shader/program reference, with missing,
  duplicate, and type-mismatch statuses from the shared vocabulary.

RenderScene records receive only validated and bridged handles. They do not own
raw cooked bytes or guess material slot layout from fixture names.

### Shader And Program Payload Records

The shader/program family owns bytecode payload identity and minimal reflection.
A production program descriptor must carry at least:

- `program_id`, internal `kind`, `schema`, pipeline class, and render-state
  compatibility fields;
- one stage record per required shader stage, including stage kind, entry point,
  bytecode payload id/range, bytecode byte count, bytecode alignment,
  bytecode hash, bytecode format/profile, and expected stage hash;
- minimal reflection: required input semantics, input layout, vertex stride,
  texture slot count, sampler slot count, constant ranges, and any required
  material slot names;
- a bounded stage table and explicit dependency rows for every shader payload.

RHI shader modules and pipelines are runtime products. They must be created from
the loaded/cooked program records and destroyed on failure; their handles are not
serialized as RuntimeAsset file truth.

## Cook/Load/RHI Route

| Stage | Contract | Mutation boundary |
| --- | --- | --- |
| File/Mount/VFS read | Read source or cooked artifact bytes through `MountTable`/File routes. The caller may provide expected descriptors, but family identity comes from internal metadata. | No Resource, Asset, RenderScene, RenderCore, or RHI mutation. |
| Validate | Parse header, schema, kind, dependency table, record table, payload table, alignment, sizes, row pitches, hashes, and bounds. | No mutation. Failure returns `RuntimeAssetDataStatus` from the shared vocabulary. |
| Cook | Convert validated source/cooked records into Resource cache payloads, decode plans, decode results, and decoded payload records. | Staged Resource payload records only; no RHI creation and no RenderScene output writes. |
| Loader preflight | Following task #51, preflight all texture, material, shader, program, dependency, and payload records before committing Resource/Asset/dependency state. | Caller outputs, `ResourceRegistry`, `AssetManager`, and dependency edges remain unchanged on failure. |
| Transaction commit | Commit Resource records, Asset records, dependency edges, and loaded RuntimeAsset metadata only after preflight succeeds. | Commit-phase failure must be preflight-proven unreachable for deterministic inputs or covered by an approved rollback/cleanup ledger; once the first runtime mutation starts, the result records `mutated_state == true` rather than claiming no-mutation. |
| Texture bridge preflight | Resolve material texture refs to loaded texture records, query decoded payload metadata, check descriptor/row pitch/byte count/hash/slot limits, and reserve a cleanup ledger. | No RenderScene material/frame/capture mutation; no output texture handles are published. |
| Texture RHI upload | Use decoded bytes and descriptor metadata to create RHI textures through `ResourceDecodedTextureBridge` or its production cooked successor. | On any texture upload/create failure, destroy any transient RHI textures from the bridge attempt and leave material output empty. |
| Shader/program bridge preflight | Resolve stage payloads, validate bytecode hashes, stage kinds, reflection, input layout, and slot counts. | No shader module or pipeline handles are published. |
| Shader module / pipeline creation | Create RHI shader modules from loaded bytecode, then create the pipeline from loaded reflection/input-layout data. | On module or pipeline failure, destroy any shader modules/pipeline created in this attempt and clear result handles. |
| RenderScene material commit | Build `RenderSceneRuntimeMaterialRecord` only after texture slots, sampler slots, shader modules, and pipeline are complete. | The material/frame/capture outputs are written once at the end. |
| Final evidence | RenderScene/RenderCore/RHI consumes the loaded material and program route and captures through RHI. | CPU PPM helpers, screenshots, reports, GDI viewers, and direct fixture structs are auxiliary at most and cannot satisfy this route. |

## Proposed API Split

Task #52 does not add these APIs, but the implementation slice should split the
bridge around the same no-mutation boundary as task #51:

- `RuntimeAssetCookedTexturePayloadDesc`: texture descriptor plus byte-layout
  ownership, payload hashes, and Resource decoded payload identity;
- `RuntimeAssetCookedMaterialSlotDesc`: one material texture/sampler slot row
  with typed dependency refs and binding slots;
- `RuntimeAssetCookedShaderStagePayloadDesc`: one shader stage bytecode payload
  row with stage kind, payload range, bytecode hash, and entry point;
- `RuntimeAssetCookedProgramDesc`: program id, stage table, input layout, slot
  counts, and minimal reflection;
- `RuntimeAssetPayloadBridgePreflightRequest/Result`: validates every material,
  texture, shader, program, and payload dependency without runtime output
  mutation;
- `RuntimeAssetPayloadBridgeCommitRequest/Result`: consumes only preflighted
  records, creates RHI textures/modules/pipelines, builds RenderScene material
  records, and owns cleanup on failure.

The preflight result should be the only input accepted by the commit step. This
keeps invalid payloads from mutating lower registries or publishing partially
created RenderScene/RHI records.

## Failure And No-Mutation Matrix

| Case | Required status surface | Required no-mutation behavior |
| --- | --- | --- |
| Unsupported texture format | `RuntimeAssetDataStatus::UnsupportedFieldValue` before RHI; lower bridge may expose `ResourceDecodedTextureBridgeStatus::InvalidArgument` only after production mapping is established. | No Resource/Asset commit during validation; no RenderScene material output; no RHI texture handle published. |
| Invalid texture extent | `RuntimeAssetDataStatus::InvalidBounds`. | Same as above. |
| Payload byte count or row/slice pitch mismatch | `RuntimeAssetDataStatus::InvalidSize`. | Same as above; decoded payload bytes are not uploaded. |
| Payload or table alignment violation | `RuntimeAssetDataStatus::InvalidAlignment`. | Same as above. |
| Texture or shader payload hash mismatch | `RuntimeAssetDataStatus::HashMismatch`. | Same as above; shader module creation is not attempted. |
| Missing dependency | `RuntimeAssetDataStatus::MissingDependency`. | Loader transaction leaves caller outputs, registries, managers, and dependency edges unchanged. |
| Duplicate dependency or duplicate material slot | `RuntimeAssetDataStatus::DuplicateDependency` for file/dependency tables; `RenderSceneRuntimeMaterialStatus::DuplicateTextureSlot` may be preserved as lower detail. | No material record is published. |
| Dependency type mismatch | `RuntimeAssetDataStatus::TypeMismatch`. | No bridge attempt and no dependency edge commit. |
| Shader bytecode stage mismatch or non-bytecode stage ref | `RuntimeAssetDataStatus::TypeMismatch`. | No shader module creation. |
| Missing shader bytecode | `RuntimeAssetDataStatus::InvalidSize`. | No shader module creation. |
| Shader bytecode hash mismatch | `RuntimeAssetDataStatus::HashMismatch`. | No shader module creation. |
| Input-layout or reflection mismatch | `RuntimeAssetDataStatus::InvalidInputLayout` or `UnsupportedFieldValue`. | No pipeline creation and no material output. |
| Texture/sampler/material slot overflow | `RuntimeAssetDataStatus::CapacityExceeded`; lower details may include `ResourceDecodedTextureBridgeStatus::SampledTextureSlotOutOfRange` or `RenderSceneRuntimeMaterialStatus::TextureSlotCapacityExceeded`. | No RHI upload and no material output. |
| RHI texture capacity/upload failure | Production bridge maps to `RuntimeAssetDataStatus::CapacityExceeded` when the lower detail is `RhiStatus::CapacityExceeded`; otherwise a bridge-specific runtime failure status must be explicit. | Destroy textures created during the attempt, clear output handles, and leave material/frame/capture outputs untouched. |
| RHI shader module creation failure | `RuntimeAssetDataStatus::RhiShaderModuleFailed`. | Destroy any shader module already created in the attempt; clear result handles. |
| RHI pipeline creation failure | `RuntimeAssetDataStatus::RhiPipelineFailed`. | Destroy shader modules and any pipeline created in the attempt; clear result handles. |

The bridge must maintain a per-attempt cleanup ledger for every RHI primitive it
creates. A failure may increment lower-layer diagnostics, but it must not expose
valid handles, resolved RenderScene materials, frame draws, capture bytes, or
committed dependency edges from the failed attempt.

## Later Implementation Tests

The implementation slice should add focused tests with these proof shapes:

| Test | Proof |
| --- | --- |
| `RuntimeAssetData_CookedTexturePayloadTableValidatesLayoutHashAndRowPitch` | Valid cooked texture records carry format, extent, row pitch, byte count, alignment, and hash, and invalid rows fail before mutation. |
| `RuntimeAssetData_CookedMaterialTextureSlotTableResolvesLoadedPayloads` | Material texture slot rows resolve typed texture deps to loaded decoded payloads and RHI texture bindings. |
| `RuntimeAssetData_CookedShaderStagePayloadsCreateRhiModules` | Shader stage payload records, not `bytecode:` fixture text, create RHI shader modules. |
| `RuntimeAssetData_CookedProgramPipelineUsesLoadedReflectionAndInputLayout` | Pipeline creation consumes loaded program reflection/input-layout/slot counts. |
| `RuntimeAssetData_CookedPayloadBridgeRejectsTextureFormatExtentSizeAlignmentHashWithoutMutation` | Invalid texture payload cases do not mutate Resource/Asset/RenderScene/RHI outputs. |
| `RuntimeAssetData_CookedPayloadBridgeRejectsMissingDuplicateTypeMismatchDepsWithoutMutation` | Missing, duplicate, and type-mismatched texture/material/shader dependencies leave loader transaction state unchanged. |
| `RuntimeAssetData_CookedShaderPayloadRejectsStageBytecodeHashAndReflectionMismatchWithoutMutation` | Stage mismatch, bytecode mismatch, hash mismatch, and input-layout mismatch do not create RHI modules or pipelines. |
| `RuntimeAssetData_CookedMaterialSlotOverflowDoesNotMutateRenderSceneOutputs` | Texture/sampler/material slot overflows return `CapacityExceeded` before material/frame/capture writes. |
| `RuntimeAssetData_CookedRhiPartialCreationFailureDestroysTransientHandles` | Texture, shader module, and pipeline partial failures retire every transient RHI handle from the attempt. |
| `RuntimeAssetData_CookedPayloadRouteUsesFileResourceAssetPathNotCpuPpmOrDirectStructs` | Final proof comes from File/Mount -> Resource/Asset -> RenderScene/RenderCore/RHI, not CPU helpers or direct fixture structs. |

Existing tests reusable as floors:

- `RuntimeAssetData_DecodedTexturePayloadsDriveRhiMaterialSlots`
- `RuntimeAssetData_TextureMaterialSlotBridgeFailuresDoNotMutateRenderSceneOutputs`
- `RuntimeAssetData_ShaderProgramBridgeCreatesRhiPipelineFromLoadedBytecode`
- `RuntimeAssetData_ShaderProgramBridgeRejectsInvalidProgramDataWithoutRhiMutation`
- `RuntimeAssetData_ShaderProgramDependencyValidatorRejectsMissingDuplicateAndTypeMismatchRefs`
- `RuntimeAssetData_MaterialValidatorRejectsMissingDuplicateAndTypeMismatchRefs`
- `RuntimeAssetData_TextureValidatorRejectsInvalidFormatExtentPayload`
- `Streaming_ResourceDecodedTextureBridge_UploadsDecodedPayloadAsTextureBinding`
- `Streaming_ResourceDecodedTextureBridge_RejectsTextureByteMismatchWithoutRhiMutation`
- `Streaming_ResourceDecodedTextureBridge_RejectsSmallScratchWithoutRhiMutation`
- `Streaming_ResourceDecodedTextureBridge_RejectsSampledTextureSlotOutOfRangeWithoutRhiMutation`
- `Streaming_ResourceDecodedTextureBridge_ReportsRhiTextureCapacityWithoutWritingOutput`

## RAV1-C Acceptance

This plan is ready for review when:

1. the canonical RuntimeAsset plan/gate documents link this RAV1-C route plan;
2. RAV0 #44/#45 smoke-only boundaries are explicit;
3. texture/material/shader cooked payload ownership is defined with status and
   no-mutation requirements;
4. the File/Mount -> validate/cook/load -> Resource/Asset ->
   RenderScene/RenderCore/RHI route is specified without CPU/PPM or direct
   struct proof shortcuts;
5. later implementation tests and reusable existing tests are listed;
6. docs-only validation passes `git diff --check` and
   `git show --check --format=short HEAD`.
