// 模块: YuEngine RuntimeAsset
// 文件: Src/YuEngine/RuntimeAsset/Include/YuEngine/RuntimeAsset/RuntimeAssetData.h

#pragma once

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
    AssetDependencyFailed
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
    std::uint64_t hash = 0U;
    std::size_t byte_count = 0U;
    std::size_t dependency_count = 0U;
};

/**
 * @brief Reports Resource and Asset records created from one loaded file.
 */
struct RuntimeAssetLoadedFile final {
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    RuntimeAssetFileKind kind = RuntimeAssetFileKind::Unknown;
    std::uint64_t stable_id = 0U;
    std::uint64_t hash = 0U;
    std::uint32_t byte_count = 0U;
    std::uint64_t cache_payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    std::uint64_t decoded_payload_id = 0U;
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
 * @brief Loads a runtime asset graph from File/VFS into Resource and Asset records.
 * @param request Input graph load request.
 * @param out_result Output load result.
 * @return Explicit graph load status.
 */
RuntimeAssetDataStatus LoadRuntimeAssetDataGraph(
    const RuntimeAssetGraphLoadRequest &request,
    RuntimeAssetGraphLoadResult *out_result);

}
