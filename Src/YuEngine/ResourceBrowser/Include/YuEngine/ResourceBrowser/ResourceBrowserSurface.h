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

enum class ResourceBrowserVisibleWorkflowStatus {
    Success,
    InvalidArgument,
    OutputCapacityExceeded,
    EntryOutOfRange,
    InvalidImportSettings
};

enum class ResourceBrowserDepthWorkflowStatus {
    Success,
    InvalidArgument,
    OutputCapacityExceeded,
    EntryOutOfRange,
    InvalidImportSettings
};

enum class ResourceBrowserImporterCommitWorkflowStatus {
    Success,
    InvalidArgument,
    OutputCapacityExceeded,
    EntryOutOfRange,
    InvalidImportCookCommand,
    InvalidImportSettings,
    Rejected,
    RuntimeLoadFailed
};

enum class ResourceBrowserImporterCommitRejectedLayer {
    None,
    ImportCookCommand,
    OriginalPackageSource,
    UnsupportedExternalImport,
    MissingPayload,
    InvalidDependency,
    UnsupportedPreviewKind,
    RuntimeAssetLoad
};

enum class ResourceBrowserSourceBoundary {
    RuntimeAssetSource,
    OriginalPackageBoundary,
    ExternalImportBoundary
};

enum class ResourceBrowserImporterReadiness {
    Ready,
    MissingSourcePath,
    MissingTargetKind,
    MissingResourceType,
    MissingAssetType,
    MissingStableId,
    UnsupportedImporterVersion,
    UnsupportedSchemaVersion,
    SourceHashMismatch,
    TargetKindMismatch,
    OriginalPackageBoundary,
    ExternalImportBoundary
};

enum class ResourceBrowserAssetManagerGap {
    None,
    MissingRuntimeLoadRecord,
    MissingResourceRegistryRecord,
    MissingAssetRecord,
    DependencyBlocked,
    UnsupportedPreviewKind
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

struct ResourceBrowserVisibleImportSettingRow final {
    const char *source_path = nullptr;
    yuengine::runtimeasset::RuntimeAssetFileKind target_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    yuengine::resource::ResourceTypeId resource_type;
    yuengine::asset::AssetTypeId asset_type;
    ResourceBrowserSurfaceSettingValidationCode validation =
        ResourceBrowserSurfaceSettingValidationCode::None;
    ResourceBrowserSurfacePreviewState preview_state =
        ResourceBrowserSurfacePreviewState::Unknown;
    std::uint64_t stable_id = 0U;
    std::uint64_t expected_source_hash = 0U;
    std::uint32_t importer_version = 0U;
    std::uint32_t expected_schema_version = 0U;
    bool selected = false;
    bool source_hash_matches = false;
    bool target_kind_matches_header = false;
};

struct ResourceBrowserVisibleDiagnosticRow final {
    ResourceBrowserDiagnosticCode code = ResourceBrowserDiagnosticCode::None;
    ResourceBrowserDiagnosticSeverity severity = ResourceBrowserDiagnosticSeverity::Info;
    ResourceBrowserDiagnosticPhase phase = ResourceBrowserDiagnosticPhase::ImportSettings;
    yuengine::runtimeasset::RuntimeAssetDataStatus runtime_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::Success;
    const char *source_path = nullptr;
    std::uint32_t file_index = 0U;
    std::uint32_t dependency_index = 0U;
    std::uint64_t stable_id = 0U;
    bool selected = false;
    bool blocks_preview = false;
};

struct ResourceBrowserVisibleSelectionLedgerRecord final {
    std::uint32_t selected_index = 0U;
    ResourceBrowserSurfaceSelectionStatus selection_status =
        ResourceBrowserSurfaceSelectionStatus::InvalidArgument;
    ResourceBrowserSurfaceSettingValidationCode setting_validation =
        ResourceBrowserSurfaceSettingValidationCode::None;
    ResourceBrowserSurfacePreviewState preview_state =
        ResourceBrowserSurfacePreviewState::Unknown;
    ResourceBrowserDiagnosticCode blocking_diagnostic_code =
        ResourceBrowserDiagnosticCode::None;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    std::uint64_t stable_id = 0U;
    bool committed_to_preview_host = false;
    bool rejected_preview_request = false;
    bool preview_eligible = false;
    bool diagnostic_blocks_preview = false;
    bool resource_asset_mapping_preserved = false;
    bool used_locator_path_as_type_truth = false;
};

struct ResourceBrowserVisibleWorkflowRequest final {
    std::span<const ResourceBrowserResourceEntry> entries{};
    std::span<const ResourceBrowserDiagnosticRecord> diagnostics{};
    ResourceBrowserImportSettings import_settings{};
    std::uint32_t selected_index = 0U;
    bool validate_import_settings = false;
    std::span<ResourceBrowserVisibleImportSettingRow> import_setting_rows{};
    std::span<ResourceBrowserVisibleDiagnosticRow> diagnostic_rows{};
    std::span<ResourceBrowserSurfaceRow> preview_rows{};
    std::span<ResourceBrowserVisibleSelectionLedgerRecord> selection_ledger{};
};

struct ResourceBrowserVisibleWorkflowResult final {
    ResourceBrowserVisibleWorkflowStatus status =
        ResourceBrowserVisibleWorkflowStatus::InvalidArgument;
    ResourceBrowserSurfaceStatus surface_status =
        ResourceBrowserSurfaceStatus::InvalidArgument;
    ResourceBrowserSurfaceSelectionStatus selection_status =
        ResourceBrowserSurfaceSelectionStatus::InvalidArgument;
    ResourceBrowserSurfaceSelectionState selection_state{};
    std::uint32_t import_setting_row_count = 0U;
    std::uint32_t diagnostic_row_count = 0U;
    std::uint32_t preview_row_count = 0U;
    std::uint32_t eligible_preview_count = 0U;
    std::uint32_t blocked_preview_count = 0U;
    std::uint32_t selection_ledger_count = 0U;
    bool emitted_import_settings = false;
    bool emitted_diagnostics = false;
    bool emitted_preview_rows = false;
    bool emitted_selection_ledger = false;
    bool selection_committed = false;
    bool selection_rejected = false;
    bool consumed_preview_host_ready_selection = false;
    bool used_locator_path_as_type_truth = false;

    bool Succeeded() const {
        return status == ResourceBrowserVisibleWorkflowStatus::Success;
    }
};

struct ResourceBrowserDepthCatalogRow final {
    const char *source_path = nullptr;
    yuengine::runtimeasset::RuntimeAssetFileKind target_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    yuengine::runtimeasset::RuntimeAssetFileKind runtime_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    ResourceBrowserSourceBoundary source_boundary =
        ResourceBrowserSourceBoundary::RuntimeAssetSource;
    ResourceBrowserImporterReadiness importer_readiness =
        ResourceBrowserImporterReadiness::MissingSourcePath;
    ResourceBrowserAssetManagerGap asset_manager_gap =
        ResourceBrowserAssetManagerGap::MissingRuntimeLoadRecord;
    ResourceBrowserSurfacePreviewState preview_state =
        ResourceBrowserSurfacePreviewState::Unknown;
    ResourceBrowserDependencyState dependency_state =
        ResourceBrowserDependencyState::Unknown;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    std::uint64_t stable_id = 0U;
    std::uint64_t source_hash = 0U;
    std::uint64_t payload_hash = 0U;
    bool selected = false;
    bool importer_ready = false;
    bool asset_manager_ready = false;
    bool preview_request_ready = false;
};

struct ResourceBrowserImporterBoundaryRow final {
    const char *source_path = nullptr;
    yuengine::runtimeasset::RuntimeAssetFileKind target_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    ResourceBrowserSourceBoundary source_boundary =
        ResourceBrowserSourceBoundary::RuntimeAssetSource;
    ResourceBrowserImporterReadiness readiness =
        ResourceBrowserImporterReadiness::MissingSourcePath;
    ResourceBrowserSurfaceSettingValidationCode setting_validation =
        ResourceBrowserSurfaceSettingValidationCode::None;
    std::uint64_t stable_id = 0U;
    bool selected = false;
    bool original_package_boundary = false;
    bool external_import_boundary = false;
    bool importer_ready = false;
};

struct ResourceBrowserAssetManagerGapRow final {
    const char *source_path = nullptr;
    ResourceBrowserAssetManagerGap gap = ResourceBrowserAssetManagerGap::None;
    ResourceBrowserDependencyState dependency_state =
        ResourceBrowserDependencyState::Unknown;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    std::uint64_t stable_id = 0U;
    bool selected = false;
    bool has_runtime_loaded_record = false;
    bool has_resource_registry_record = false;
    bool has_asset_record = false;
    bool resource_handle_valid = false;
    bool asset_handle_valid = false;
    bool asset_manager_ready = false;
};

struct ResourceBrowserDepthSelectionLedgerRecord final {
    std::uint32_t selected_index = 0U;
    ResourceBrowserDepthWorkflowStatus status =
        ResourceBrowserDepthWorkflowStatus::InvalidArgument;
    ResourceBrowserSourceBoundary source_boundary =
        ResourceBrowserSourceBoundary::RuntimeAssetSource;
    ResourceBrowserImporterReadiness importer_readiness =
        ResourceBrowserImporterReadiness::MissingSourcePath;
    ResourceBrowserAssetManagerGap asset_manager_gap =
        ResourceBrowserAssetManagerGap::MissingRuntimeLoadRecord;
    ResourceBrowserSurfacePreviewState preview_state =
        ResourceBrowserSurfacePreviewState::Unknown;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    std::uint64_t stable_id = 0U;
    bool selection_committed = false;
    bool selection_rejected = false;
    bool blocked_by_original_package = false;
    bool blocked_by_external_import = false;
    bool blocked_by_importer_gap = false;
    bool blocked_by_asset_manager_gap = false;
    bool preview_request_ready = false;
    bool mutated_runtime_state = false;
};

struct ResourceBrowserDepthWorkflowRequest final {
    std::span<const ResourceBrowserResourceEntry> entries{};
    std::span<const ResourceBrowserDiagnosticRecord> diagnostics{};
    ResourceBrowserImportSettings import_settings{};
    std::uint32_t selected_index = 0U;
    bool validate_import_settings = false;
    std::span<ResourceBrowserDepthCatalogRow> catalog_rows{};
    std::span<ResourceBrowserImporterBoundaryRow> importer_rows{};
    std::span<ResourceBrowserAssetManagerGapRow> asset_gap_rows{};
    std::span<ResourceBrowserDepthSelectionLedgerRecord> selection_ledger{};
};

struct ResourceBrowserDepthWorkflowResult final {
    ResourceBrowserDepthWorkflowStatus status =
        ResourceBrowserDepthWorkflowStatus::InvalidArgument;
    ResourceBrowserSurfaceSelectionStatus selection_status =
        ResourceBrowserSurfaceSelectionStatus::InvalidArgument;
    ResourceBrowserSurfaceSettingValidationCode selected_setting_validation =
        ResourceBrowserSurfaceSettingValidationCode::None;
    std::uint32_t catalog_row_count = 0U;
    std::uint32_t importer_row_count = 0U;
    std::uint32_t asset_gap_row_count = 0U;
    std::uint32_t selection_ledger_count = 0U;
    std::uint32_t original_package_boundary_count = 0U;
    std::uint32_t external_import_boundary_count = 0U;
    std::uint32_t importer_ready_count = 0U;
    std::uint32_t asset_manager_ready_count = 0U;
    std::uint32_t preview_request_ready_count = 0U;
    bool emitted_catalog_rows = false;
    bool emitted_importer_rows = false;
    bool emitted_asset_gap_rows = false;
    bool emitted_selection_ledger = false;
    bool selection_committed = false;
    bool selection_rejected = false;
    bool blocked_by_original_package = false;
    bool blocked_by_external_import = false;
    bool blocked_by_importer_gap = false;
    bool blocked_by_asset_manager_gap = false;
    bool mutated_runtime_state = false;
    bool opened_native_window = false;
    bool used_deprecated_surface = false;

    bool Succeeded() const {
        return status == ResourceBrowserDepthWorkflowStatus::Success;
    }
};

struct ResourceBrowserExternalAuthoringSourceRow final {
    const char *manifest_path = nullptr;
    const char *source_path = nullptr;
    const char *payload_path = nullptr;
    yuengine::runtimeasset::RuntimeAssetFileKind target_kind =
        yuengine::runtimeasset::RuntimeAssetFileKind::Unknown;
    std::uint64_t stable_id = 0U;
    std::uint64_t content_hash = 0U;
    std::uint32_t runtime_asset_index = 0U;
    std::uint32_t dependency_count = 0U;
    std::uint32_t unsupported_feature_count = 0U;
    bool manifest_readable = false;
    bool payload_available = false;
    bool dependencies_valid = false;
    bool runtime_asset_descriptor_ready = false;
    bool manifest_ready = false;
    bool preview_supported = true;
    bool selected = false;
};

struct ResourceBrowserImporterCommitSelectionLedgerRecord final {
    std::uint32_t selected_index = 0U;
    ResourceBrowserImporterCommitWorkflowStatus status =
        ResourceBrowserImporterCommitWorkflowStatus::InvalidArgument;
    ResourceBrowserImporterCommitRejectedLayer rejected_layer =
        ResourceBrowserImporterCommitRejectedLayer::None;
    ResourceBrowserSourceBoundary source_boundary =
        ResourceBrowserSourceBoundary::RuntimeAssetSource;
    ResourceBrowserImporterReadiness importer_readiness =
        ResourceBrowserImporterReadiness::MissingSourcePath;
    ResourceBrowserAssetManagerGap asset_manager_gap =
        ResourceBrowserAssetManagerGap::MissingRuntimeLoadRecord;
    ResourceBrowserSurfaceSettingValidationCode setting_validation =
        ResourceBrowserSurfaceSettingValidationCode::None;
    yuengine::runtimeasset::RuntimeAssetDataStatus runtime_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::InvalidArgument;
    yuengine::resource::ResourceHandle resource;
    yuengine::asset::AssetHandle asset;
    std::uint64_t stable_id = 0U;
    bool preflighted_before_mutation = false;
    bool mutation_allowed = false;
    bool committed_resource_registry = false;
    bool committed_asset_manager = false;
    bool selection_committed = false;
    bool selection_rejected = false;
    bool mutated_runtime_state = false;
    bool external_manifest_ready = false;
    bool original_package_rejected = false;
    bool unsupported_external_rejected = false;
    bool missing_payload_rejected = false;
    bool invalid_dependency_rejected = false;
    bool unsupported_preview_kind_rejected = false;
};

struct ResourceBrowserImporterCommitWorkflowRequest final {
    yuengine::file::MountTable *mount_table = nullptr;
    yuengine::file::MountId mount;
    const yuengine::runtimeasset::RuntimeAssetImportCookCommandResult *import_cook_result = nullptr;
    yuengine::runtimeasset::RuntimeAssetFileDesc scene{};
    std::span<const yuengine::runtimeasset::RuntimeAssetFileDesc> files{};
    std::span<const ResourceBrowserExternalAuthoringSourceRow> external_source_rows{};
    yuengine::resource::ResourceRegistry *resource_registry = nullptr;
    yuengine::asset::AssetManager *asset_manager = nullptr;
    ResourceBrowserImportSettings import_settings{};
    std::uint32_t selected_index = 0U;
    bool validate_import_settings = false;
    std::span<ResourceBrowserResourceEntry> entries{};
    std::span<ResourceBrowserDiagnosticRecord> diagnostics{};
    std::span<yuengine::runtimeasset::RuntimeAssetLoadedFile> loaded_files{};
    std::span<yuengine::runtimeasset::RuntimeAssetSceneResourceRef> scene_resource_refs{};
    std::span<yuengine::runtimeasset::RuntimeAssetSceneCameraRecord> scene_cameras{};
    std::span<yuengine::runtimeasset::RuntimeAssetSceneEntityRecord> scene_entities{};
    std::span<yuengine::runtimeasset::RuntimeAssetSceneTransformOutputRecord> scene_transforms{};
    yuengine::runtimeasset::RuntimeAssetSceneLoaderOutput *scene_output = nullptr;
    std::span<ResourceBrowserDepthCatalogRow> catalog_rows{};
    std::span<ResourceBrowserImporterBoundaryRow> importer_rows{};
    std::span<ResourceBrowserAssetManagerGapRow> asset_gap_rows{};
    std::span<ResourceBrowserImporterCommitSelectionLedgerRecord> selection_ledger{};
};

struct ResourceBrowserImporterCommitWorkflowResult final {
    ResourceBrowserImporterCommitWorkflowStatus status =
        ResourceBrowserImporterCommitWorkflowStatus::InvalidArgument;
    ResourceBrowserImporterCommitRejectedLayer rejected_layer =
        ResourceBrowserImporterCommitRejectedLayer::None;
    ResourceBrowserDiagnosticsStatus preflight_diagnostics_status =
        ResourceBrowserDiagnosticsStatus::InvalidArgument;
    ResourceBrowserDiagnosticsStatus post_commit_diagnostics_status =
        ResourceBrowserDiagnosticsStatus::InvalidArgument;
    yuengine::runtimeasset::RuntimeAssetDataStatus runtime_status =
        yuengine::runtimeasset::RuntimeAssetDataStatus::InvalidArgument;
    yuengine::runtimeasset::RuntimeAssetGraphLoadResult graph_load_result{};
    ResourceBrowserSurfaceSettingValidationCode selected_setting_validation =
        ResourceBrowserSurfaceSettingValidationCode::None;
    std::uint32_t entry_count = 0U;
    std::uint32_t diagnostic_count = 0U;
    std::uint32_t loaded_file_count = 0U;
    std::uint32_t catalog_row_count = 0U;
    std::uint32_t importer_row_count = 0U;
    std::uint32_t asset_gap_row_count = 0U;
    std::uint32_t selection_ledger_count = 0U;
    bool preflighted_before_mutation = false;
    bool mutation_allowed = false;
    bool mutated_runtime_state = false;
    bool committed_resource_registry = false;
    bool committed_asset_manager = false;
    bool selection_committed = false;
    bool selection_rejected = false;
    bool external_manifest_ready = false;
    bool emitted_catalog_rows = false;
    bool emitted_importer_rows = false;
    bool emitted_asset_gap_rows = false;
    bool emitted_selection_ledger = false;

    bool Succeeded() const {
        return status == ResourceBrowserImporterCommitWorkflowStatus::Success;
    }
};

ResourceBrowserSurfaceStatus BuildResourceBrowserNativeSurface(
    const ResourceBrowserSurfaceRequest &request,
    ResourceBrowserSurfaceResult *out_result);

ResourceBrowserSurfaceSelectionStatus ResolveResourceBrowserSurfaceSelection(
    const ResourceBrowserSurfaceSelectionRequest &request,
    ResourceBrowserSurfaceSelectionResult *out_result);

ResourceBrowserVisibleWorkflowStatus BuildResourceBrowserVisibleWorkflowSurface(
    const ResourceBrowserVisibleWorkflowRequest &request,
    ResourceBrowserVisibleWorkflowResult *out_result);

ResourceBrowserDepthWorkflowStatus BuildResourceBrowserDepthWorkflowSurface(
    const ResourceBrowserDepthWorkflowRequest &request,
    ResourceBrowserDepthWorkflowResult *out_result);

ResourceBrowserImporterCommitWorkflowStatus BuildResourceBrowserImporterCommitWorkflow(
    const ResourceBrowserImporterCommitWorkflowRequest &request,
    ResourceBrowserImporterCommitWorkflowResult *out_result);

}
