// Module: YuEngine ExternalAuthoring
// File: Src/YuEngine/ExternalAuthoring/Include/YuEngine/ExternalAuthoring/ExternalAuthoringBridge.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/MountId.h"
#include "YuEngine/File/VirtualPath.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

namespace yuengine::file {
class MountTable;
}

namespace yuengine::externalauthoring {

constexpr std::size_t EXTERNAL_AUTHORING_BRIDGE_MAX_PATH_LENGTH = 160U;

enum class ExternalAuthoringToolKind {
    Unknown,
    Unity,
    Unreal,
    Dcc
};

enum class ExternalAuthoringBridgeStatus {
    Success,
    InvalidArgument,
    FileReadFailed,
    ManifestParseFailed,
    UnsupportedTool,
    UnsupportedCoordinatePolicy,
    UnsupportedMaterialPolicy,
    UnsupportedAnimationPolicy,
    MissingPayload,
    PayloadHashMismatch,
    InvalidDependencyGraph,
    UnsupportedFeature,
    OutputCapacityExceeded,
    ImportCookInputMissing
};

enum class ExternalAuthoringBridgeBlockedLayer {
    None,
    FileVfs,
    Manifest,
    Payload,
    DependencyGraph,
    MappingPolicy,
    RuntimeAssetInput,
    ImportCookCommand
};

struct ExternalAuthoringRuntimeAssetInputRow final {
    char manifest_path[EXTERNAL_AUTHORING_BRIDGE_MAX_PATH_LENGTH]{};
    char payload_path[EXTERNAL_AUTHORING_BRIDGE_MAX_PATH_LENGTH]{};
    ExternalAuthoringToolKind tool = ExternalAuthoringToolKind::Unknown;
    yuengine::runtimeasset::RuntimeAssetFileKind target_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    yuengine::resource::ResourceTypeId resource_type;
    yuengine::asset::AssetTypeId asset_type;
    std::uint64_t stable_id = 0U;
    std::uint64_t content_hash = 0U;
    std::uint32_t dependency_count = 0U;
    std::uint32_t unsupported_feature_count = 0U;
    bool manifest_readable = false;
    bool payload_available = false;
    bool dependencies_valid = false;
    bool coordinate_policy_supported = false;
    bool material_policy_supported = false;
    bool animation_policy_supported = false;
    bool runtime_asset_descriptor_ready = false;
    bool manifest_ready = false;
    bool preview_supported = false;
    bool selected = false;
    yuengine::runtimeasset::RuntimeAssetFileDesc runtime_asset_source{};
};

struct ExternalAuthoringBridgeRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    yuengine::file::VirtualPath manifest_path;
    std::span<ExternalAuthoringRuntimeAssetInputRow> runtime_asset_inputs{};
    yuengine::runtimeasset::RuntimeAssetImportCookCommandRequest *import_cook_request = nullptr;
    yuengine::runtimeasset::RuntimeAssetFileDesc *import_cook_source_files = nullptr;
    std::uint32_t import_cook_source_file_capacity = 0U;
    yuengine::runtimeasset::RuntimeAssetFileDesc *import_cook_cooked_files = nullptr;
    std::uint32_t import_cook_cooked_file_capacity = 0U;
};

struct ExternalAuthoringBridgeResult final {
    ExternalAuthoringBridgeStatus status = ExternalAuthoringBridgeStatus::InvalidArgument;
    ExternalAuthoringBridgeBlockedLayer blocked_layer = ExternalAuthoringBridgeBlockedLayer::Manifest;
    yuengine::file::FileStatus file_status = yuengine::file::FileStatus::Success;
    ExternalAuthoringToolKind tool = ExternalAuthoringToolKind::Unknown;
    std::uint32_t manifest_entry_count = 0U;
    std::uint32_t payload_read_count = 0U;
    std::uint32_t runtime_asset_input_count = 0U;
    std::uint32_t unsupported_feature_count = 0U;
    std::uint32_t dependency_count = 0U;
    bool manifest_read = false;
    bool parsed_manifest = false;
    bool validated_coordinate_policy = false;
    bool validated_material_policy = false;
    bool validated_animation_policy = false;
    bool validated_dependency_graph = false;
    bool emitted_runtime_asset_input = false;
    bool emitted_import_cook_request = false;
    bool mutated_runtime_state = false;
    bool opened_native_window = false;
    bool required_external_tool = false;
};

ExternalAuthoringBridgeStatus BuildExternalAuthoringRuntimeAssetImportBridge(
    const ExternalAuthoringBridgeRequest &request,
    ExternalAuthoringBridgeResult *out_result);

}
