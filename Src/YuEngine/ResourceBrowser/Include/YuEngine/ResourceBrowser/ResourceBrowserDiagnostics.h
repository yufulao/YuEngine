// 模块: YuEngine ResourceBrowser
// 文件: Src/YuEngine/ResourceBrowser/Include/YuEngine/ResourceBrowser/ResourceBrowserDiagnostics.h

#pragma once

#include <cstdint>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Asset/AssetSnapshot.h"
#include "YuEngine/Asset/AssetTypeId.h"
#include "YuEngine/File/FileSnapshot.h"
#include "YuEngine/File/FileStatus.h"
#include "YuEngine/File/MountId.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/Resource/ResourceLoadCommitStatus.h"
#include "YuEngine/Resource/ResourceLoadState.h"
#include "YuEngine/Resource/ResourceSnapshot.h"
#include "YuEngine/Resource/ResourceTypeId.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

namespace yuengine::asset {
class AssetManager;
}

namespace yuengine::file {
class MountTable;
}

namespace yuengine::resource {
class ResourceRegistry;
}

namespace yuengine::resourcebrowser {

enum class ResourceBrowserDiagnosticsStatus {
    Success,
    InvalidArgument,
    OutputCapacityExceeded
};

enum class ResourceBrowserDiagnosticSeverity {
    Info,
    Warning,
    Error,
    Blocker
};

enum class ResourceBrowserDiagnosticPhase {
    ImportSettings,
    FileRead,
    Validate,
    Cook,
    Load,
    Resource,
    Asset,
    Dependency
};

enum class ResourceBrowserDiagnosticCode {
    None,
    FileReadFailed,
    ValidateFailed,
    LoadFailed,
    MissingDependency,
    DuplicateDependency,
    TypeMismatch,
    StaleHash,
    StaleSchema,
    Unsupported,
    CapacityExceeded,
    BudgetExceeded,
    ResourceQueryFailed,
    AssetQueryFailed
};

enum class ResourceBrowserDependencyState {
    Unknown,
    Ready,
    Missing,
    Duplicate,
    TypeMismatch,
    StaleHash,
    StaleSchema,
    Unsupported,
    CapacityExceeded,
    BudgetExceeded
};

struct ResourceBrowserImportSettings final {
    const char *source_path = nullptr;
    yuengine::runtimeasset::RuntimeAssetFileKind target_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    yuengine::resource::ResourceTypeId resource_type;
    yuengine::asset::AssetTypeId asset_type;
    std::uint64_t stable_id = 0U;
    std::uint32_t importer_version = 1U;
    std::uint32_t expected_schema_version = 1U;
    std::uint64_t expected_source_hash = 0U;
};

struct ResourceBrowserDiagnosticRecord final {
    ResourceBrowserDiagnosticCode code = ResourceBrowserDiagnosticCode::None;
    ResourceBrowserDiagnosticSeverity severity = ResourceBrowserDiagnosticSeverity::Info;
    ResourceBrowserDiagnosticPhase phase = ResourceBrowserDiagnosticPhase::ImportSettings;
    yuengine::runtimeasset::RuntimeAssetDataStatus runtime_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::Success;
    yuengine::file::FileStatus file_status = yuengine::file::FileStatus::Success;
    yuengine::resource::ResourceLoadCommitStatus resource_load_status =
        yuengine::resource::ResourceLoadCommitStatus::Success;
    const char *source_path = nullptr;
    yuengine::runtimeasset::RuntimeAssetFileKind expected_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    yuengine::runtimeasset::RuntimeAssetArtifactClass artifact_class =
        yuengine::runtimeasset::RuntimeAssetArtifactClass::Unknown;
    std::uint32_t file_index = 0U;
    std::uint32_t dependency_index = 0U;
    std::uint32_t expected_schema_version = 0U;
    std::uint32_t actual_schema_version = 0U;
    std::uint64_t expected_hash = 0U;
    std::uint64_t actual_hash = 0U;
    const char *message = nullptr;
};

struct ResourceBrowserResourceEntry final {
    ResourceBrowserImportSettings import_settings{};
    yuengine::runtimeasset::RuntimeAssetValidationResult validation{};
    ResourceBrowserDependencyState dependency_state = ResourceBrowserDependencyState::Unknown;
    yuengine::file::FileStatus file_status = yuengine::file::FileStatus::Success;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    yuengine::resource::ResourceLoadCommitStatus resource_load_status =
        yuengine::resource::ResourceLoadCommitStatus::Success;
    yuengine::resource::ResourceLoadState resource_load_state =
        yuengine::resource::ResourceLoadState::Unloaded;
    std::uint64_t cache_payload_id = 0U;
    std::uint64_t decode_plan_id = 0U;
    std::uint64_t decode_result_id = 0U;
    std::uint64_t decoded_payload_id = 0U;
    std::uint32_t decoded_byte_count = 0U;
    bool from_file_vfs = false;
    bool from_runtime_asset_validation = false;
    bool from_runtime_asset_load = false;
    bool from_resource_registry = false;
    bool from_asset_record = false;
};

struct ResourceBrowserDiagnosticsRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    const yuengine::runtimeasset::RuntimeAssetFileDesc *files = nullptr;
    std::uint32_t file_count = 0U;
    const yuengine::runtimeasset::RuntimeAssetLoadedFile *loaded_files = nullptr;
    std::uint32_t loaded_file_count = 0U;
    const yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    const yuengine::asset::AssetManager *asset_manager = nullptr;
    ResourceBrowserResourceEntry *entries = nullptr;
    std::uint32_t entry_capacity = 0U;
    ResourceBrowserDiagnosticRecord *diagnostics = nullptr;
    std::uint32_t diagnostic_capacity = 0U;
};

struct ResourceBrowserDiagnosticsResult final {
    ResourceBrowserDiagnosticsStatus status = ResourceBrowserDiagnosticsStatus::InvalidArgument;
    std::uint32_t entry_count = 0U;
    std::uint32_t diagnostic_count = 0U;
    std::uint32_t file_read_count = 0U;
    std::uint32_t validation_count = 0U;
    std::uint32_t validation_failure_count = 0U;
    std::uint32_t loaded_record_count = 0U;
    std::uint32_t resource_query_count = 0U;
    bool used_file_vfs = false;
    bool observed_resource_registry = false;
    bool observed_asset_manager = false;
    bool mutated_runtime_state = false;
    bool diagnostic_overflow = false;
    yuengine::file::FileSnapshot file_snapshot{};
    yuengine::resource::ResourceSnapshot resource_snapshot{};
    yuengine::asset::AssetSnapshot asset_snapshot{};
};

ResourceBrowserDiagnosticsStatus BuildResourceBrowserRuntimeAssetDiagnostics(
    const ResourceBrowserDiagnosticsRequest &request,
    ResourceBrowserDiagnosticsResult *out_result);

}
