// 模块: YuEngine RuntimeAsset
// 文件: Src/YuEngine/RuntimeAsset/Include/YuEngine/RuntimeAsset/RuntimeAssetData.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetStatus.h"
#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/Animation/AnimationRuntimeSampler.h"
#include "YuEngine/File/MountId.h"
#include "YuEngine/File/VirtualPath.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialRecord.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialStatus.h"
#include "YuEngine/RenderScene/RenderSceneRuntimeMaterialTextureSlot.h"
#include "YuEngine/Resource/ResourceDecodedPayloadStatus.h"
#include "YuEngine/Kernel/RuntimeFrameContext.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiFormat.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSamplerDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"
#include "YuEngine/Rhi/RhiShaderStage.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiTextureDesc.h"
#include "YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h"
#include "YuEngine/World/WorldObjectId.h"
#include "YuEngine/World/WorldTransformState.h"

namespace yuengine::asset {
class AssetManager;
}

namespace yuengine::file {
class MountTable;
}

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::runtimeasset {

/**
 * @brief Runtime asset source file family.
 */
enum class RuntimeAssetFileKind {
    Unknown,
    Mesh,
    Material,
    Texture,
    Shader,
    Scene,
    Animation
};

/**
 * @brief RuntimeAsset v0 artifact class parsed from internal metadata.
 */
enum class RuntimeAssetArtifactClass {
    Unknown,
    Source,
    Cooked
};

/**
 * @brief Runtime asset data validation, cook, and load status.
 */
enum class RuntimeAssetDataStatus {
    Success,
    InvalidArgument,
    InvalidHeader,
    UnsupportedVersion,
    InvalidKind,
    InvalidSchema,
    InvalidCount,
    InvalidSize,
    InvalidAlignment,
    InvalidBounds,
    InvalidDependency,
    MissingDependency,
    DuplicateDependency,
    TypeMismatch,
    HashMismatch,
    UnsupportedFieldValue,
    CapacityExceeded,
    BudgetExceeded,
    FileReadFailed,
    ResourceRegistrationFailed,
    ResourceLoadCommitFailed,
    ResourceResidencyFailed,
    CachePayloadStoreFailed,
    DecodePlanFailed,
    DecodeResultFailed,
    DecodedPayloadStoreFailed,
    AssetRegistrationFailed,
    ResourceDependencyFailed,
    AssetDependencyFailed,
    InvalidInputLayout,
    RhiShaderModuleFailed,
    RhiPipelineFailed,
    RhiTextureFailed,
    RhiSamplerFailed,
    RenderSceneMaterialFailed
};

/**
 * @brief Coarse phase reported by the RuntimeAsset loader transaction.
 */
enum class RuntimeAssetLoadTransactionPhase {
    Preflight,
    ReadBytes,
    ParseHeader,
    ValidateRecord,
    ValidateDependencies,
    PreflightCommit,
    StageSceneOutput,
    CommitResources,
    CommitAssets,
    CommitPayloads,
    CommitDependencies,
    CommitSceneOutput
};

/**
 * @brief Describes one runtime asset file expected by a graph load request.
 */
struct RuntimeAssetFileDesc final {
    const char *path = nullptr;
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    yuengine::resource::ResourceTypeId resource_type;
    yuengine::asset::AssetTypeId asset_type;
    std::uint64_t stable_id = 0U;
    yuengine::resource::ResourceDecodePlanAssetClass decode_asset_class =
        yuengine::resource::ResourceDecodePlanAssetClass::Unknown;
    yuengine::resource::ResourceDecodeResultClass decode_result_class =
        yuengine::resource::ResourceDecodeResultClass::Unknown;
    std::uint32_t decoded_byte_count = 0U;
};

/**
 * @brief Reports validation status and deterministic identity for one source file.
 */
struct RuntimeAssetValidationResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetArtifactClass artifact_class = RuntimeAssetArtifactClass::Unknown;
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    std::uint32_t version = 0U;
    std::uint32_t schema_version = 0U;
    std::uint64_t hash = 0U;
    std::uint64_t identity_hash = 0U;
    std::uint64_t source_hash = 0U;
    std::uint64_t payload_hash = 0U;
    std::size_t byte_count = 0U;
    std::size_t dependency_count = 0U;
    std::uint32_t dependency_table_count = 0U;
    std::uint32_t record_table_count = 0U;
    std::uint32_t record_table_byte_count = 0U;
    std::uint32_t payload_byte_count = 0U;
    std::uint32_t payload_alignment = 0U;
    std::uint32_t vertex_count = 0U;
    std::uint32_t index_count = 0U;
    std::uint32_t texture_width = 0U;
    std::uint32_t texture_height = 0U;
    std::uint32_t texture_slot_count = 0U;
    std::uint32_t shader_stage_count = 0U;
    std::uint32_t shader_bytecode_byte_count = 0U;
};

/**
 * @brief Reports Resource and Asset records created from one loaded file.
 */
struct RuntimeAssetLoadedFile final {
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    yuengine::resource::ResourceTypeId resource_type;
    yuengine::asset::AssetTypeId asset_type;
    std::uint64_t stable_id = 0U;
    std::uint64_t hash = 0U;
    std::uint32_t byte_count = 0U;
    std::uint64_t cache_payload_id = 0U;
    std::uint64_t decode_plan_payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    std::uint64_t decoded_payload_id = 0U;
    yuengine::resource::ResourceDecodePlanAssetClass decode_asset_class =
        yuengine::resource::ResourceDecodePlanAssetClass::Unknown;
    yuengine::resource::ResourceDecodeResultClass decode_result_class =
        yuengine::resource::ResourceDecodeResultClass::Unknown;
    std::uint32_t decoded_byte_count = 0U;
    bool cache_payload_stored = false;
    bool decode_plan_created = false;
    bool decode_result_committed = false;
    bool decoded_payload_stored = false;
};

/**
 * @brief Identifies a loaded runtime asset referenced by scene records.
 */
struct RuntimeAssetSceneResourceRef final {
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    std::uint64_t stable_id = 0U;
    std::uint32_t loaded_file_index = 0U;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
};

/**
 * @brief Disk scene camera record emitted by the RuntimeAsset production loader.
 */
struct RuntimeAssetSceneCameraRecord final {
    std::uint32_t camera_id = 0U;
    bool is_active = false;
};

/**
 * @brief Disk scene entity record emitted after animation sampling.
 */
struct RuntimeAssetSceneEntityRecord final {
    std::uint32_t entity_id = 0U;
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
    std::uint32_t mesh_ref_index = 0U;
    std::uint32_t material_ref_index = 0U;
    std::uint32_t texture_ref_index = 0U;
    std::uint32_t shader_ref_index = 0U;
    std::uint32_t camera_index = 0U;
    std::uint32_t animation_ref_index = 0U;
    bool is_visible = true;
    bool is_active = true;
};

/**
 * @brief Scene transform output record consumed by runtime scene builders.
 */
struct RuntimeAssetSceneTransformOutputRecord final {
    yuengine::world::WorldObjectId world_object_id{};
    yuengine::world::WorldTransformState transform{};
};

/**
 * @brief Deterministic RuntimeAsset scene loader output and diagnostics.
 */
struct RuntimeAssetSceneLoaderOutput final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    std::uint64_t scene_id = 0U;
    std::uint64_t scene_hash = 0U;
    std::uint32_t entity_count = 0U;
    std::uint32_t transform_count = 0U;
    std::uint32_t resource_ref_count = 0U;
    std::uint32_t camera_count = 0U;
    std::uint32_t animation_sampled_value_count = 0U;
    std::uint32_t entity_capacity = 0U;
    std::uint32_t transform_capacity = 0U;
    std::uint32_t resource_ref_capacity = 0U;
    std::uint32_t camera_capacity = 0U;
    std::uint32_t file_read_count = 0U;
    std::uint32_t dependency_count = 0U;
    std::uint32_t cache_payload_count = 0U;
    std::uint32_t decoded_payload_count = 0U;
    yuengine::animation::AnimationRuntimeStatus animation_sample_status =
        yuengine::animation::AnimationRuntimeStatus::MissingClip;
    yuengine::animation::AnimationRuntimeStatus animation_apply_status =
        yuengine::animation::AnimationRuntimeStatus::MissingSample;
};

/**
 * @brief Requests a disk-backed runtime asset graph load through File, Resource, and Asset.
 */
struct RuntimeAssetGraphLoadRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    yuengine::file::VirtualPath scene_path;
    yuengine::resource::ResourceTypeId scene_resource_type;
    yuengine::asset::AssetTypeId scene_asset_type;
    std::uint64_t scene_stable_id = 0U;
    const RuntimeAssetFileDesc *files = nullptr;
    std::uint32_t file_count = 0U;
    yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    yuengine::asset::AssetManager *asset_manager = nullptr;
    RuntimeAssetLoadedFile *loaded_files = nullptr;
    std::uint32_t loaded_file_capacity = 0U;
    RuntimeAssetSceneResourceRef *scene_resource_refs = nullptr;
    std::uint32_t scene_resource_ref_capacity = 0U;
    RuntimeAssetSceneCameraRecord *scene_cameras = nullptr;
    std::uint32_t scene_camera_capacity = 0U;
    RuntimeAssetSceneEntityRecord *scene_entities = nullptr;
    std::uint32_t scene_entity_capacity = 0U;
    RuntimeAssetSceneTransformOutputRecord *scene_transforms = nullptr;
    std::uint32_t scene_transform_capacity = 0U;
    RuntimeAssetSceneLoaderOutput *scene_output = nullptr;
    yuengine::kernel::RuntimeFrameContext animation_frame_context{};
    std::uint64_t animation_clip_start_time_nanoseconds = 0U;
};

/**
 * @brief Bounded commit plan summary produced before runtime mutation begins.
 */
struct RuntimeAssetLoadTransactionPlan final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetLoadTransactionPhase phase = RuntimeAssetLoadTransactionPhase::Preflight;
    std::uint32_t record_count = 0U;
    std::uint32_t dependency_count = 0U;
    std::uint32_t resource_commit_count = 0U;
    std::uint32_t asset_commit_count = 0U;
    std::uint32_t cache_payload_commit_count = 0U;
    std::uint32_t decoded_payload_commit_count = 0U;
    std::uint32_t dependency_edge_commit_count = 0U;
};

/**
 * @brief Diagnostics for graph-load transaction preflight and commit.
 */
struct RuntimeAssetLoadTransactionResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetLoadTransactionPhase phase = RuntimeAssetLoadTransactionPhase::Preflight;
    std::uint32_t first_failed_record_index = 0U;
    std::uint32_t first_failed_dependency_index = 0U;
    std::uint32_t committed_resource_count = 0U;
    std::uint32_t committed_asset_count = 0U;
    std::uint32_t committed_cache_payload_count = 0U;
    std::uint32_t committed_decoded_payload_count = 0U;
    std::uint32_t committed_dependency_edge_count = 0U;
    bool mutated_state = false;
};

/**
 * @brief Reports the closed-loop graph load result.
 */
struct RuntimeAssetGraphLoadResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    RuntimeAssetLoadTransactionPlan transaction_plan;
    RuntimeAssetLoadTransactionResult transaction_result;
    RuntimeAssetLoadedFile scene;
    std::uint32_t loaded_file_count = 0U;
    std::uint32_t file_read_count = 0U;
    std::uint32_t cache_payload_count = 0U;
    std::uint32_t decoded_payload_count = 0U;
    std::uint32_t resource_dependency_count = 0U;
    std::uint32_t asset_dependency_count = 0U;
    std::size_t scene_dependency_count = 0U;
    bool scene_registered = false;
    bool scene_references_runtime_asset_families = false;
};

/**
 * @brief Owns decoded RuntimeAsset shader program data before RHI upload.
 */
struct RuntimeAssetLoadedShaderProgramData final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    std::uint32_t program_id = 0U;
    RuntimeAssetValidationResult validation{};
    std::array<std::uint8_t, yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES> vertex_bytecode{};
    std::array<std::uint8_t, yuengine::rhi::MAX_RHI_SHADER_BYTECODE_BYTES> pixel_bytecode{};
    std::size_t vertex_bytecode_size = 0U;
    std::size_t pixel_bytecode_size = 0U;
    std::uint64_t vertex_bytecode_hash = 0U;
    std::uint64_t pixel_bytecode_hash = 0U;
    yuengine::rhi::RhiInputLayoutDesc input_layout{};
    std::uint32_t texture_slot_count = 0U;
};

/**
 * @brief Requests a RuntimeAsset-owned shader program bridge into RHI primitives.
 */
struct RuntimeAssetShaderProgramPipelineRequest final {
    yuengine::rhi::IRhiDevice *device = nullptr;
    const RuntimeAssetLoadedShaderProgramData *program = nullptr;
};

/**
 * @brief Reports shader module and pipeline ownership created from RuntimeAsset bytecode.
 */
struct RuntimeAssetShaderProgramPipelineResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    std::uint32_t program_id = 0U;
    std::uint64_t vertex_bytecode_hash = 0U;
    std::uint64_t pixel_bytecode_hash = 0U;
    std::uint32_t texture_slot_count = 0U;
    yuengine::rhi::RhiShaderModuleHandle vertex_shader{};
    yuengine::rhi::RhiShaderModuleHandle pixel_shader{};
    yuengine::rhi::RhiPipelineHandle pipeline{};
    yuengine::rhi::RhiPipelineDesc pipeline_desc{};
};

/**
 * @brief Color-space tag carried by cooked RuntimeAsset texture payload descriptors.
 */
enum class RuntimeAssetCookedTextureColorSpace {
    Unknown,
    Linear,
    Srgb
};

/**
 * @brief Describes one cooked texture payload that can be verified before RHI upload.
 */
struct RuntimeAssetCookedTexturePayloadDesc final {
    const RuntimeAssetLoadedFile *loaded_texture = nullptr;
    yuengine::rhi::RhiTextureDesc texture_desc{};
    RuntimeAssetCookedTextureColorSpace color_space = RuntimeAssetCookedTextureColorSpace::Unknown;
    std::uint32_t row_pitch_bytes = 0U;
    std::uint32_t slice_pitch_bytes = 0U;
    std::uint32_t payload_offset_bytes = 0U;
    std::uint32_t payload_byte_count = 0U;
    std::uint32_t payload_alignment_bytes = 1U;
    std::uint64_t payload_hash = 0U;
    std::uint64_t decoded_payload_id = 0U;
    std::uint64_t staging_request_id = 0U;
    std::uint64_t upload_id = 0U;
};

/**
 * @brief Describes one material texture slot that references a cooked texture payload.
 */
struct RuntimeAssetCookedMaterialSlotDesc final {
    std::uint32_t material_slot = 0U;
    std::uint32_t texture_payload_index = 0U;
    yuengine::rhi::RhiFormat expected_format = yuengine::rhi::RhiFormat::Rgba8Unorm;
    RuntimeAssetCookedTextureColorSpace expected_color_space = RuntimeAssetCookedTextureColorSpace::Unknown;
    std::uint32_t texture_binding_slot = 0U;
    std::uint32_t sampler_binding_slot = 0U;
    yuengine::rhi::RhiSamplerDesc sampler_desc{};
};

/**
 * @brief Requests a cooked texture payload bridge into RHI textures and RenderScene material slots.
 */
struct RuntimeAssetCookedTextureMaterialBridgeRequest final {
    yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    yuengine::asset::AssetManager *asset_manager = nullptr;
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    yuengine::asset::AssetHandle material_asset{};
    std::uint32_t material_id = 0U;
    yuengine::rhi::RhiPipelineHandle pipeline{};
    std::span<const RuntimeAssetCookedTexturePayloadDesc> textures{};
    std::span<const RuntimeAssetCookedMaterialSlotDesc> material_slots{};
    std::span<std::uint8_t> scratch_bytes{};
    yuengine::renderscene::RenderSceneRuntimeMaterialRecord *out_material = nullptr;
};

/**
 * @brief Reports cooked payload bridge validation, upload, material binding, and cleanup state.
 */
struct RuntimeAssetCookedTextureMaterialBridgeResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    yuengine::resource::ResourceDecodedPayloadStatus decoded_payload_status =
        yuengine::resource::ResourceDecodedPayloadStatus::Success;
    yuengine::streaming::ResourceDecodedTextureBridgeStatus texture_bridge_status =
        yuengine::streaming::ResourceDecodedTextureBridgeStatus::Success;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::Success;
    yuengine::asset::AssetStatus asset_status = yuengine::asset::AssetStatus::Success;
    yuengine::renderscene::RenderSceneRuntimeMaterialStatus material_status =
        yuengine::renderscene::RenderSceneRuntimeMaterialStatus::Success;
    std::uint32_t runtime_texture_upload_count = 0U;
    std::uint32_t material_texture_slot_count = 0U;
    std::uint32_t cleanup_texture_count = 0U;
    std::uint32_t cleanup_sampler_count = 0U;
    bool mutated_state = false;
    bool published_material = false;
};

/**
 * @brief Cooked shader bytecode format accepted by the RuntimeAsset bridge.
 */
enum class RuntimeAssetCookedShaderBytecodeFormat {
    Unknown,
    OpaqueBytecode
};

/**
 * @brief Runtime pipeline class declared by a cooked RuntimeAsset program.
 */
enum class RuntimeAssetCookedProgramPipelineClass {
    Unknown,
    Graphics
};

/**
 * @brief One cooked shader stage payload row owned by RuntimeAsset data.
 */
struct RuntimeAssetCookedShaderStagePayloadDesc final {
    yuengine::rhi::RhiShaderStage stage = yuengine::rhi::RhiShaderStage::Unsupported;
    const char *entry_point = nullptr;
    const char *bytecode_profile = nullptr;
    RuntimeAssetCookedShaderBytecodeFormat bytecode_format =
        RuntimeAssetCookedShaderBytecodeFormat::Unknown;
    std::uint64_t payload_id = 0U;
    const std::uint8_t *payload_bytes = nullptr;
    std::uint32_t payload_byte_count = 0U;
    std::uint32_t bytecode_offset = 0U;
    std::uint32_t bytecode_byte_count = 0U;
    std::uint32_t bytecode_alignment = 0U;
    std::uint64_t bytecode_hash = 0U;
    std::uint64_t expected_stage_hash = 0U;
};

/**
 * @brief Cooked shader program reflection and stage table used for RHI creation.
 */
struct RuntimeAssetCookedProgramDesc final {
    std::uint32_t program_id = 0U;
    RuntimeAssetCookedProgramPipelineClass pipeline_class =
        RuntimeAssetCookedProgramPipelineClass::Unknown;
    const RuntimeAssetCookedShaderStagePayloadDesc *stages = nullptr;
    std::uint32_t stage_count = 0U;
    yuengine::rhi::RhiInputLayoutDesc input_layout{};
    std::uint32_t vertex_stride_bytes = 0U;
    std::uint32_t texture_slot_count = 0U;
    std::uint32_t sampler_slot_count = 0U;
    std::array<yuengine::rhi::RhiInputElementSemantic, yuengine::rhi::MAX_RHI_INPUT_ELEMENTS>
        required_input_semantics{};
    std::uint32_t required_input_semantic_count = 0U;
    std::uint32_t constant_range_count = 0U;
};

/**
 * @brief Requests cooked RuntimeAsset shader/program RHI bridge creation.
 */
struct RuntimeAssetCookedShaderProgramPipelineRequest final {
    yuengine::rhi::IRhiDevice *device = nullptr;
    const RuntimeAssetCookedProgramDesc *program = nullptr;
};

/**
 * @brief Reports cooked shader/program bridge outputs and cleanup ledger counts.
 */
struct RuntimeAssetCookedShaderProgramPipelineResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
    std::uint32_t program_id = 0U;
    std::uint64_t vertex_bytecode_hash = 0U;
    std::uint64_t pixel_bytecode_hash = 0U;
    std::uint32_t texture_slot_count = 0U;
    std::uint32_t sampler_slot_count = 0U;
    std::uint32_t preflight_stage_count = 0U;
    std::uint32_t created_shader_module_count = 0U;
    std::uint32_t destroyed_shader_module_count = 0U;
    bool published_handles = false;
    yuengine::rhi::RhiShaderModuleHandle vertex_shader{};
    yuengine::rhi::RhiShaderModuleHandle pixel_shader{};
    yuengine::rhi::RhiPipelineHandle pipeline{};
    yuengine::rhi::RhiPipelineDesc pipeline_desc{};
};

/**
 * @brief Returns the file kind token used by the runtime asset header.
 * @param kind Input file family.
 * @return Static token string.
 */
const char *RuntimeAssetFileKindName(RuntimeAssetFileKind kind);
/**
 * @brief Computes a deterministic identity hash for runtime asset bytes.
 * @param bytes Input byte span.
 * @return Stable hash value.
 */
std::uint64_t HashRuntimeAssetDataBytes(std::span<const std::uint8_t> bytes);
/**
 * @brief Validates runtime asset bytes without mutating runtime output state.
 * @param bytes Input byte span.
 * @param expected_kind Expected file family.
 * @param out_result Output validation result.
 * @return Explicit validation status.
 */
RuntimeAssetDataStatus ValidateRuntimeAssetDataBytes(
    std::span<const std::uint8_t> bytes,
    RuntimeAssetFileKind expected_kind,
    RuntimeAssetValidationResult *out_result);
/**
 * @brief Decodes RuntimeAsset shader program bytes into bridge-owned data.
 * @param bytes Source/cooked program bytes.
 * @param program_id Stable shader program id.
 * @param out_data Output decoded program payload.
 * @return Explicit decode status.
 */
RuntimeAssetDataStatus DecodeRuntimeAssetShaderProgramData(
    std::span<const std::uint8_t> bytes,
    std::uint32_t program_id,
    RuntimeAssetLoadedShaderProgramData *out_data);
/**
 * @brief Loads a runtime asset graph from File/VFS into Resource and Asset records.
 * @param request Input graph load request.
 * @param out_result Output load result.
 * @return Explicit graph load status.
 */
RuntimeAssetDataStatus LoadRuntimeAssetDataGraph(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphLoadResult *out_result);
/**
 * @brief Creates RHI shader modules and a pipeline from RuntimeAsset-owned bytecode.
 * @param request Input bytecode, layout, and RHI device.
 * @param out_result Output module and pipeline ownership records.
 * @return Explicit bridge status.
 */
RuntimeAssetDataStatus BuildRuntimeAssetShaderProgramPipeline(
    const RuntimeAssetShaderProgramPipelineRequest &request,
    RuntimeAssetShaderProgramPipelineResult *out_result);
/**
 * @brief Bridges validated cooked texture payloads to RHI handles and RenderScene material texture slots.
 * @param request Input descriptors, registries, RHI device, and output material storage.
 * @param out_result Output bridge result and cleanup ledger.
 * @return Explicit bridge status.
 */
RuntimeAssetDataStatus BuildRuntimeAssetCookedTextureMaterialBridge(
    const RuntimeAssetCookedTextureMaterialBridgeRequest &request,
    RuntimeAssetCookedTextureMaterialBridgeResult *out_result);
/**
 * @brief Creates RHI shader modules and a pipeline from cooked RuntimeAsset bytecode payload rows.
 * @param request Input cooked stage payloads, reflection, and RHI device.
 * @param out_result Output module, pipeline, and cleanup ledger records.
 * @return Explicit bridge status.
 */
RuntimeAssetDataStatus BuildRuntimeAssetCookedShaderProgramPipeline(
    const RuntimeAssetCookedShaderProgramPipelineRequest &request,
    RuntimeAssetCookedShaderProgramPipelineResult *out_result);

}
