// 模块: YuEngine ResourceBrowser
// 文件: Src/YuEngine/ResourceBrowser/Include/YuEngine/ResourceBrowser/ResourceBrowserSurface.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/Asset/AssetHandle.h"
#include "YuEngine/Resource/ResourceHandle.h"
#include "YuEngine/ResourceBrowser/ResourceBrowserDiagnostics.h"
#include "YuEngine/RuntimeAsset/RuntimeAssetData.h"

namespace yuengine::resourcebrowser {

enum class ResourceBrowserSurfaceStatus {
    Success,
    InvalidArgument,
    OutputCapacityExceeded
};

enum class ResourceBrowserSurfacePreviewState {
    Unknown,
    Eligible,
    BlockedByValidation,
    BlockedByDependency,
    BlockedByDiagnostic,
    BlockedByLoadRecord,
    BlockedByResourceAssetRecord,
    BlockedByUnsupportedKind
};

enum class ResourceBrowserSurfaceDocumentKind {
    None,
    Resource,
    Scene,
    Animation
};

enum class ResourceBrowserSurfaceSelectionStatus {
    Success,
    InvalidArgument,
    EntryOutOfRange,
    InvalidImportSettings,
    PreviewBlocked
};

enum class ResourceBrowserSurfaceSettingValidationCode {
    None,
    MissingSourcePath,
    MissingTargetKind,
    MissingResourceType,
    MissingAssetType,
    MissingStableId,
    UnsupportedImporterVersion,
    UnsupportedSchemaVersion,
    SourceHashMismatch,
    TargetKindMismatch
};

struct ResourceBrowserSurfaceRow final {
    const char *locator_path = nullptr;
    yuengine::runtimeasset::RuntimeAssetFileKind declared_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    yuengine::runtimeasset::RuntimeAssetFileKind header_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    yuengine::runtimeasset::RuntimeAssetArtifactClass artifact_class =
        yuengine::runtimeasset::RuntimeAssetArtifactClass::Unknown;
    yuengine::runtimeasset::RuntimeAssetDataStatus validation_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::InvalidArgument;
    ResourceBrowserDependencyState dependency_state = ResourceBrowserDependencyState::Unknown;
    ResourceBrowserDiagnosticCode first_diagnostic_code = ResourceBrowserDiagnosticCode::None;
    ResourceBrowserDiagnosticSeverity highest_severity = ResourceBrowserDiagnosticSeverity::Info;
    ResourceBrowserDiagnosticPhase blocking_phase = ResourceBrowserDiagnosticPhase::ImportSettings;
    ResourceBrowserSurfacePreviewState preview_state = ResourceBrowserSurfacePreviewState::Unknown;
    ResourceBrowserSurfaceDocumentKind preview_document_kind = ResourceBrowserSurfaceDocumentKind::None;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    std::uint64_t stable_id = 0U;
    std::uint64_t identity_hash = 0U;
    std::uint64_t source_hash = 0U;
    std::uint64_t payload_hash = 0U;
    std::uint32_t schema_version = 0U;
    std::uint32_t decoded_byte_count = 0U;
    std::uint32_t diagnostic_count = 0U;
    bool has_runtime_loaded_record = false;
    bool has_resource_asset_record = false;
    bool has_blocking_diagnostic = false;
    bool locator_path_is_type_truth = false;
};

struct ResourceBrowserSurfaceRequest final {
    std::span<const ResourceBrowserResourceEntry> entries{};
    std::span<const ResourceBrowserDiagnosticRecord> diagnostics{};
    std::span<ResourceBrowserSurfaceRow> rows{};
};

struct ResourceBrowserSurfaceResult final {
    ResourceBrowserSurfaceStatus status = ResourceBrowserSurfaceStatus::InvalidArgument;
    std::uint32_t row_count = 0U;
    std::uint32_t diagnostic_row_count = 0U;
    std::uint32_t blocker_row_count = 0U;
    std::uint32_t eligible_preview_count = 0U;
    std::uint32_t blocked_preview_count = 0U;
    bool used_locator_path_as_type_truth = false;

    bool Succeeded() const {
        return status == ResourceBrowserSurfaceStatus::Success;
    }
};

struct ResourceBrowserSurfaceSelectionRequest final {
    std::span<const ResourceBrowserResourceEntry> entries{};
    std::span<const ResourceBrowserSurfaceRow> rows{};
    std::span<const ResourceBrowserDiagnosticRecord> diagnostics{};
    ResourceBrowserImportSettings import_settings{};
    std::uint32_t selected_index = 0U;
    bool validate_import_settings = false;
};

struct ResourceBrowserSurfaceSelectionState final {
    std::uint32_t selected_index = 0U;
    ResourceBrowserImportSettings import_settings{};
    ResourceBrowserSurfaceSettingValidationCode setting_validation =
        ResourceBrowserSurfaceSettingValidationCode::None;
    ResourceBrowserSurfacePreviewState preview_state = ResourceBrowserSurfacePreviewState::Unknown;
    ResourceBrowserSurfaceDocumentKind preview_document_kind = ResourceBrowserSurfaceDocumentKind::None;
    ResourceBrowserDiagnosticCode blocking_diagnostic_code = ResourceBrowserDiagnosticCode::None;
    ResourceBrowserDiagnosticSeverity blocking_diagnostic_severity = ResourceBrowserDiagnosticSeverity::Info;
    ResourceBrowserDiagnosticPhase blocking_diagnostic_phase = ResourceBrowserDiagnosticPhase::ImportSettings;
    yuengine::runtimeasset::RuntimeAssetDataStatus validation_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::InvalidArgument;
    ResourceBrowserDependencyState dependency_state = ResourceBrowserDependencyState::Unknown;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    std::uint64_t stable_id = 0U;
    std::uint64_t source_hash = 0U;
    std::uint32_t matched_diagnostic_count = 0U;
    bool selected = false;
    bool import_settings_valid = false;
    bool preview_eligible = false;
    bool diagnostic_blocks_preview = false;
    bool resource_asset_mapping_preserved = false;
    bool used_locator_path_as_type_truth = false;
    bool mutated_runtime_state = false;
};

struct ResourceBrowserSurfaceSelectionResult final {
    ResourceBrowserSurfaceSelectionStatus status = ResourceBrowserSurfaceSelectionStatus::InvalidArgument;
    ResourceBrowserSurfaceSelectionState state{};

    bool Succeeded() const {
        return status == ResourceBrowserSurfaceSelectionStatus::Success;
    }
};

ResourceBrowserSurfaceStatus BuildResourceBrowserNativeSurface(
    const ResourceBrowserSurfaceRequest &request,
    ResourceBrowserSurfaceResult *out_result);

ResourceBrowserSurfaceSelectionStatus ResolveResourceBrowserSurfaceSelection(
    const ResourceBrowserSurfaceSelectionRequest &request,
    ResourceBrowserSurfaceSelectionResult *out_result);

}
