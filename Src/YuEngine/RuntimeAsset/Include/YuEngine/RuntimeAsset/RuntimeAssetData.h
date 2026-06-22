// 模块: YuEngine RuntimeAsset
// 文件: Src/YuEngine/RuntimeAsset/Include/YuEngine/RuntimeAsset/RuntimeAssetData.h

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/File/MountId.h"
#include "YuEngine/File/VirtualPath.h"
#include "YuEngine/Resource/ResourceDecodePlanAssetClass.h"
#include "YuEngine/Resource/ResourceDecodeResultClass.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiConstants.h"
#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiPipelineDesc.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"

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
    RhiPipelineFailed
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
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    std::uint32_t version = 0U;
    std::uint32_t schema_version = 0U;
    std::uint64_t hash = 0U;
    std::uint64_t identity_hash = 0U;
    std::size_t byte_count = 0U;
    std::size_t dependency_count = 0U;
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
};

/**
 * @brief Reports the closed-loop graph load result.
 */
struct RuntimeAssetGraphLoadResult final {
    RuntimeAssetDataStatus status = RuntimeAssetDataStatus::InvalidArgument;
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

}
