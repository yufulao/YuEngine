// 模块: YuEngine ResourceBrowser
// 文件: Src/YuEngine/ResourceBrowser/Src/ResourceBrowserSurface.cpp

#include "YuEngine/ResourceBrowser/ResourceBrowserSurface.h"

#include <cstdint>

namespace yuengine::resourcebrowser {
namespace {
using RuntimeAssetDataStatus = yuengine::runtimeasset::RuntimeAssetDataStatus;
using RuntimeAssetFileKind = yuengine::runtimeasset::RuntimeAssetFileKind;

std::uint32_t SeverityRank(ResourceBrowserDiagnosticSeverity severity) {
    switch (severity) {
        case ResourceBrowserDiagnosticSeverity::Info:
            return 0U;
        case ResourceBrowserDiagnosticSeverity::Warning:
            return 1U;
        case ResourceBrowserDiagnosticSeverity::Error:
            return 2U;
        case ResourceBrowserDiagnosticSeverity::Blocker:
            return 3U;
    }

    return 0U;
}

bool IsBlockingSeverity(ResourceBrowserDiagnosticSeverity severity) {
    return SeverityRank(severity) >= SeverityRank(ResourceBrowserDiagnosticSeverity::Error);
}

bool IsStringPresent(const char *value) {
    return value != nullptr && value[0] != '\0';
}

bool StartsWith(const char *value, const char *prefix) {
    if (value == nullptr || prefix == nullptr) {
        return false;
    }

    std::uint32_t index = 0U;
    while (prefix[index] != '\0') {
        if (value[index] != prefix[index]) {
            return false;
        }

        ++index;
    }

    return true;
}

bool IsPreviewSupportedKind(RuntimeAssetFileKind kind) {
    switch (kind) {
        case RuntimeAssetFileKind::Mesh:
        case RuntimeAssetFileKind::Material:
        case RuntimeAssetFileKind::Texture:
        case RuntimeAssetFileKind::Shader:
        case RuntimeAssetFileKind::Scene:
        case RuntimeAssetFileKind::Animation:
            return true;
        case RuntimeAssetFileKind::Unknown:
            break;
    }

    return false;
}

ResourceBrowserSurfaceDocumentKind DocumentKindFor(RuntimeAssetFileKind kind) {
    switch (kind) {
        case RuntimeAssetFileKind::Scene:
            return ResourceBrowserSurfaceDocumentKind::Scene;
        case RuntimeAssetFileKind::Animation:
            return ResourceBrowserSurfaceDocumentKind::Animation;
        case RuntimeAssetFileKind::Mesh:
        case RuntimeAssetFileKind::Material:
        case RuntimeAssetFileKind::Texture:
        case RuntimeAssetFileKind::Shader:
            return ResourceBrowserSurfaceDocumentKind::Resource;
        case RuntimeAssetFileKind::Unknown:
            break;
    }

    return ResourceBrowserSurfaceDocumentKind::None;
}

ResourceBrowserSurfaceSettingValidationCode ValidateImportSettings(
    const ResourceBrowserImportSettings &settings,
    const ResourceBrowserResourceEntry &entry,
    const ResourceBrowserSurfaceRow &row) {
    if (!IsStringPresent(settings.source_path)) {
        return ResourceBrowserSurfaceSettingValidationCode::MissingSourcePath;
    }

    if (settings.target_kind == RuntimeAssetFileKind::Unknown) {
        return ResourceBrowserSurfaceSettingValidationCode::MissingTargetKind;
    }

    if (!settings.resource_type.IsValid()) {
        return ResourceBrowserSurfaceSettingValidationCode::MissingResourceType;
    }

    if (!settings.asset_type.IsValid()) {
        return ResourceBrowserSurfaceSettingValidationCode::MissingAssetType;
    }

    if (settings.stable_id == 0U) {
        return ResourceBrowserSurfaceSettingValidationCode::MissingStableId;
    }

    if (settings.importer_version == 0U) {
        return ResourceBrowserSurfaceSettingValidationCode::UnsupportedImporterVersion;
    }

    if (settings.expected_schema_version == 0U) {
        return ResourceBrowserSurfaceSettingValidationCode::UnsupportedSchemaVersion;
    }

    if (settings.expected_source_hash != 0U &&
        entry.validation.source_hash != 0U &&
        settings.expected_source_hash != entry.validation.source_hash) {
        return ResourceBrowserSurfaceSettingValidationCode::SourceHashMismatch;
    }

    if (entry.validation.status == RuntimeAssetDataStatus::Success &&
        row.header_kind != RuntimeAssetFileKind::Unknown &&
        settings.target_kind != row.header_kind) {
        return ResourceBrowserSurfaceSettingValidationCode::TargetKindMismatch;
    }

    return ResourceBrowserSurfaceSettingValidationCode::None;
}

ResourceBrowserSourceBoundary SourceBoundaryFor(const char *source_path) {
    if (StartsWith(source_path, "original-package:") ||
        StartsWith(source_path, "OriginalPackage/")) {
        return ResourceBrowserSourceBoundary::OriginalPackageBoundary;
    }

    if (StartsWith(source_path, "external-import:") ||
        StartsWith(source_path, "ExternalImport/") ||
        StartsWith(source_path, "External/")) {
        return ResourceBrowserSourceBoundary::ExternalImportBoundary;
    }

    return ResourceBrowserSourceBoundary::RuntimeAssetSource;
}

ResourceBrowserImporterReadiness ImporterReadinessForValidation(
    ResourceBrowserSurfaceSettingValidationCode validation) {
    switch (validation) {
        case ResourceBrowserSurfaceSettingValidationCode::None:
            return ResourceBrowserImporterReadiness::Ready;
        case ResourceBrowserSurfaceSettingValidationCode::MissingSourcePath:
            return ResourceBrowserImporterReadiness::MissingSourcePath;
        case ResourceBrowserSurfaceSettingValidationCode::MissingTargetKind:
            return ResourceBrowserImporterReadiness::MissingTargetKind;
        case ResourceBrowserSurfaceSettingValidationCode::MissingResourceType:
            return ResourceBrowserImporterReadiness::MissingResourceType;
        case ResourceBrowserSurfaceSettingValidationCode::MissingAssetType:
            return ResourceBrowserImporterReadiness::MissingAssetType;
        case ResourceBrowserSurfaceSettingValidationCode::MissingStableId:
            return ResourceBrowserImporterReadiness::MissingStableId;
        case ResourceBrowserSurfaceSettingValidationCode::UnsupportedImporterVersion:
            return ResourceBrowserImporterReadiness::UnsupportedImporterVersion;
        case ResourceBrowserSurfaceSettingValidationCode::UnsupportedSchemaVersion:
            return ResourceBrowserImporterReadiness::UnsupportedSchemaVersion;
        case ResourceBrowserSurfaceSettingValidationCode::SourceHashMismatch:
            return ResourceBrowserImporterReadiness::SourceHashMismatch;
        case ResourceBrowserSurfaceSettingValidationCode::TargetKindMismatch:
            return ResourceBrowserImporterReadiness::TargetKindMismatch;
    }

    return ResourceBrowserImporterReadiness::MissingSourcePath;
}

ResourceBrowserImporterReadiness ImporterReadinessFor(
    ResourceBrowserSourceBoundary boundary,
    ResourceBrowserSurfaceSettingValidationCode validation) {
    if (boundary == ResourceBrowserSourceBoundary::OriginalPackageBoundary) {
        return ResourceBrowserImporterReadiness::OriginalPackageBoundary;
    }

    if (boundary == ResourceBrowserSourceBoundary::ExternalImportBoundary) {
        return ResourceBrowserImporterReadiness::ExternalImportBoundary;
    }

    return ImporterReadinessForValidation(validation);
}

bool IsImporterReady(ResourceBrowserImporterReadiness readiness) {
    return readiness == ResourceBrowserImporterReadiness::Ready;
}

ResourceBrowserSurfacePreviewState PreviewStateFor(const ResourceBrowserResourceEntry &entry) {
    if (entry.validation.status != RuntimeAssetDataStatus::Success) {
        return ResourceBrowserSurfacePreviewState::BlockedByValidation;
    }

    if (entry.dependency_state != ResourceBrowserDependencyState::Ready) {
        return ResourceBrowserSurfacePreviewState::BlockedByDependency;
    }

    if (!entry.from_runtime_asset_load) {
        return ResourceBrowserSurfacePreviewState::BlockedByLoadRecord;
    }

    if (!entry.from_resource_registry ||
        !entry.from_asset_record ||
        !entry.resource.IsValid() ||
        !entry.asset.IsValid()) {
        return ResourceBrowserSurfacePreviewState::BlockedByResourceAssetRecord;
    }

    if (!IsPreviewSupportedKind(entry.validation.kind)) {
        return ResourceBrowserSurfacePreviewState::BlockedByUnsupportedKind;
    }

    return ResourceBrowserSurfacePreviewState::Eligible;
}

ResourceBrowserSurfacePreviewState PreviewBlockedByDiagnosticState(
    const ResourceBrowserSurfaceRow &row) {
    if (row.preview_state == ResourceBrowserSurfacePreviewState::Eligible &&
        row.has_blocking_diagnostic) {
        return ResourceBrowserSurfacePreviewState::BlockedByDiagnostic;
    }

    return row.preview_state;
}

void ApplyDiagnostics(
    const ResourceBrowserSurfaceRequest &request,
    std::uint32_t row_index,
    ResourceBrowserSurfaceRow *row) {
    if (row == nullptr) {
        return;
    }

    for (const ResourceBrowserDiagnosticRecord &diagnostic : request.diagnostics) {
        if (diagnostic.file_index != row_index) {
            continue;
        }

        ++row->diagnostic_count;
        if (row->diagnostic_count == 1U ||
            SeverityRank(diagnostic.severity) > SeverityRank(row->highest_severity)) {
            row->first_diagnostic_code = diagnostic.code;
            row->highest_severity = diagnostic.severity;
            row->blocking_phase = diagnostic.phase;
        }

        if (IsBlockingSeverity(diagnostic.severity)) {
            row->has_blocking_diagnostic = true;
        }
    }
}

ResourceBrowserSurfaceRow BuildRow(
    const ResourceBrowserSurfaceRequest &request,
    const ResourceBrowserResourceEntry &entry,
    std::uint32_t row_index) {
    ResourceBrowserSurfaceRow row{};
    row.locator_path = entry.import_settings.source_path;
    row.declared_kind = entry.import_settings.target_kind;
    row.header_kind = entry.validation.kind;
    row.artifact_class = entry.validation.artifact_class;
    row.validation_status = entry.validation.status;
    row.dependency_state = entry.dependency_state;
    row.resource = entry.resource;
    row.asset = entry.asset;
    row.stable_id = entry.import_settings.stable_id;
    row.identity_hash = entry.validation.identity_hash;
    row.source_hash = entry.validation.source_hash;
    row.payload_hash = entry.validation.payload_hash;
    row.schema_version = entry.validation.schema_version;
    row.decoded_byte_count = entry.decoded_byte_count;
    row.has_runtime_loaded_record = entry.from_runtime_asset_load;
    row.has_resource_asset_record =
        entry.from_resource_registry && entry.from_asset_record &&
        entry.resource.IsValid() && entry.asset.IsValid();
    row.preview_state = PreviewStateFor(entry);
    row.preview_document_kind = DocumentKindFor(entry.validation.kind);
    row.locator_path_is_type_truth = false;
    ApplyDiagnostics(request, row_index, &row);
    row.preview_state = PreviewBlockedByDiagnosticState(row);
    return row;
}

ResourceBrowserAssetManagerGap AssetManagerGapFor(
    const ResourceBrowserResourceEntry &entry,
    const ResourceBrowserSurfaceRow &row) {
    if (entry.dependency_state != ResourceBrowserDependencyState::Ready) {
        return ResourceBrowserAssetManagerGap::DependencyBlocked;
    }

    if (!entry.from_runtime_asset_load) {
        return ResourceBrowserAssetManagerGap::MissingRuntimeLoadRecord;
    }

    if (!entry.from_resource_registry || !entry.resource.IsValid()) {
        return ResourceBrowserAssetManagerGap::MissingResourceRegistryRecord;
    }

    if (!entry.from_asset_record || !entry.asset.IsValid()) {
        return ResourceBrowserAssetManagerGap::MissingAssetRecord;
    }

    if (row.preview_state == ResourceBrowserSurfacePreviewState::BlockedByUnsupportedKind) {
        return ResourceBrowserAssetManagerGap::UnsupportedPreviewKind;
    }

    return ResourceBrowserAssetManagerGap::None;
}

bool IsAssetManagerReady(ResourceBrowserAssetManagerGap gap) {
    return gap == ResourceBrowserAssetManagerGap::None;
}

bool IsValidRequest(const ResourceBrowserSurfaceRequest &request) {
    if (!request.entries.empty() && request.entries.data() == nullptr) {
        return false;
    }

    if (!request.diagnostics.empty() && request.diagnostics.data() == nullptr) {
        return false;
    }

    if (!request.rows.empty() && request.rows.data() == nullptr) {
        return false;
    }

    return true;
}

bool IsValidSelectionRequest(const ResourceBrowserSurfaceSelectionRequest &request) {
    if (!request.entries.empty() && request.entries.data() == nullptr) {
        return false;
    }

    if (!request.rows.empty() && request.rows.data() == nullptr) {
        return false;
    }

    if (!request.diagnostics.empty() && request.diagnostics.data() == nullptr) {
        return false;
    }

    if (request.entries.size() != request.rows.size()) {
        return false;
    }

    return true;
}

void ApplySelectionDiagnosticsForIndex(
    std::span<const ResourceBrowserDiagnosticRecord> diagnostics,
    std::uint32_t selected_index,
    ResourceBrowserSurfaceSelectionState *state) {
    if (state == nullptr) {
        return;
    }

    for (const ResourceBrowserDiagnosticRecord &diagnostic : diagnostics) {
        if (diagnostic.file_index != selected_index) {
            continue;
        }

        ++state->matched_diagnostic_count;
        if (state->matched_diagnostic_count == 1U ||
            SeverityRank(diagnostic.severity) > SeverityRank(state->blocking_diagnostic_severity)) {
            state->blocking_diagnostic_code = diagnostic.code;
            state->blocking_diagnostic_severity = diagnostic.severity;
            state->blocking_diagnostic_phase = diagnostic.phase;
        }

        if (IsBlockingSeverity(diagnostic.severity)) {
            state->diagnostic_blocks_preview = true;
        }
    }
}

ResourceBrowserSurfaceSelectionState BuildSelectionStateFromRow(
    const ResourceBrowserResourceEntry &entry,
    const ResourceBrowserSurfaceRow &row,
    std::span<const ResourceBrowserDiagnosticRecord> diagnostics,
    std::uint32_t selected_index,
    const ResourceBrowserImportSettings &import_settings,
    bool validate_import_settings) {
    ResourceBrowserSurfaceSelectionState state{};
    state.selected_index = selected_index;
    state.import_settings = entry.import_settings;
    if (validate_import_settings) {
        state.import_settings = import_settings;
    }

    state.preview_state = row.preview_state;
    state.preview_document_kind = row.preview_document_kind;
    state.blocking_diagnostic_code = row.first_diagnostic_code;
    state.blocking_diagnostic_severity = row.highest_severity;
    state.blocking_diagnostic_phase = row.blocking_phase;
    state.validation_status = row.validation_status;
    state.dependency_state = row.dependency_state;
    state.resource = row.resource;
    state.asset = row.asset;
    state.stable_id = row.stable_id;
    state.source_hash = row.source_hash;
    state.selected = true;
    state.resource_asset_mapping_preserved =
        state.resource.slot == entry.resource.slot &&
        state.resource.generation == entry.resource.generation &&
        state.asset.slot == entry.asset.slot &&
        state.asset.generation == entry.asset.generation;
    state.used_locator_path_as_type_truth = row.locator_path_is_type_truth;
    ApplySelectionDiagnosticsForIndex(diagnostics, selected_index, &state);
    return state;
}

ResourceBrowserSurfaceSelectionState BuildSelectionState(
    const ResourceBrowserSurfaceSelectionRequest &request) {
    return BuildSelectionStateFromRow(
        request.entries[request.selected_index],
        request.rows[request.selected_index],
        request.diagnostics,
        request.selected_index,
        request.import_settings,
        request.validate_import_settings);
}

bool IsValidVisibleWorkflowRequest(const ResourceBrowserVisibleWorkflowRequest &request) {
    if (!request.entries.empty() && request.entries.data() == nullptr) {
        return false;
    }

    if (!request.diagnostics.empty() && request.diagnostics.data() == nullptr) {
        return false;
    }

    if (!request.import_setting_rows.empty() && request.import_setting_rows.data() == nullptr) {
        return false;
    }

    if (!request.diagnostic_rows.empty() && request.diagnostic_rows.data() == nullptr) {
        return false;
    }

    if (!request.preview_rows.empty() && request.preview_rows.data() == nullptr) {
        return false;
    }

    if (!request.selection_ledger.empty() && request.selection_ledger.data() == nullptr) {
        return false;
    }

    return true;
}

bool HasVisibleWorkflowOutputCapacity(const ResourceBrowserVisibleWorkflowRequest &request) {
    if (request.import_setting_rows.size() < request.entries.size()) {
        return false;
    }

    if (request.preview_rows.size() < request.entries.size()) {
        return false;
    }

    if (request.diagnostic_rows.size() < request.diagnostics.size()) {
        return false;
    }

    if (!request.entries.empty() && request.selection_ledger.empty()) {
        return false;
    }

    return true;
}

bool IsValidDepthWorkflowRequest(const ResourceBrowserDepthWorkflowRequest &request) {
    if (!request.entries.empty() && request.entries.data() == nullptr) {
        return false;
    }

    if (!request.diagnostics.empty() && request.diagnostics.data() == nullptr) {
        return false;
    }

    if (!request.catalog_rows.empty() && request.catalog_rows.data() == nullptr) {
        return false;
    }

    if (!request.importer_rows.empty() && request.importer_rows.data() == nullptr) {
        return false;
    }

    if (!request.asset_gap_rows.empty() && request.asset_gap_rows.data() == nullptr) {
        return false;
    }

    if (!request.selection_ledger.empty() && request.selection_ledger.data() == nullptr) {
        return false;
    }

    return true;
}

bool HasDepthWorkflowOutputCapacity(const ResourceBrowserDepthWorkflowRequest &request) {
    if (request.catalog_rows.size() < request.entries.size()) {
        return false;
    }

    if (request.importer_rows.size() < request.entries.size()) {
        return false;
    }

    if (request.asset_gap_rows.size() < request.entries.size()) {
        return false;
    }

    if (!request.entries.empty() && request.selection_ledger.empty()) {
        return false;
    }

    return true;
}

ResourceBrowserImportSettings ImportSettingsForDepthRow(
    const ResourceBrowserDepthWorkflowRequest &request,
    std::uint32_t row_index) {
    if (request.validate_import_settings && row_index == request.selected_index) {
        return request.import_settings;
    }

    return request.entries[row_index].import_settings;
}

ResourceBrowserSurfaceSettingValidationCode ValidateDepthImportSettings(
    const ResourceBrowserDepthWorkflowRequest &request,
    const ResourceBrowserSurfaceRow &row,
    std::uint32_t row_index) {
    return ValidateImportSettings(
        ImportSettingsForDepthRow(request, row_index),
        request.entries[row_index],
        row);
}

ResourceBrowserSurfaceSelectionResult ResolveVisibleWorkflowSelection(
    const ResourceBrowserVisibleWorkflowRequest &request) {
    ResourceBrowserSurfaceSelectionResult result{};
    ResourceBrowserSurfaceRequest surface_request{};
    surface_request.entries = request.entries;
    surface_request.diagnostics = request.diagnostics;
    const ResourceBrowserSurfaceRow selected_row =
        BuildRow(surface_request, request.entries[request.selected_index], request.selected_index);
    result.state = BuildSelectionStateFromRow(
        request.entries[request.selected_index],
        selected_row,
        request.diagnostics,
        request.selected_index,
        request.import_settings,
        request.validate_import_settings);
    result.state.setting_validation =
        ValidateImportSettings(
            result.state.import_settings,
            request.entries[request.selected_index],
            selected_row);
    result.state.import_settings_valid =
        result.state.setting_validation == ResourceBrowserSurfaceSettingValidationCode::None;
    if (!result.state.import_settings_valid) {
        result.state.preview_eligible = false;
        result.state.preview_state = ResourceBrowserSurfacePreviewState::BlockedByValidation;
        result.status = ResourceBrowserSurfaceSelectionStatus::InvalidImportSettings;
        return result;
    }

    result.state.preview_state = PreviewBlockedByDiagnosticState(selected_row);
    if (result.state.diagnostic_blocks_preview) {
        if (result.state.preview_state == ResourceBrowserSurfacePreviewState::Eligible) {
            result.state.preview_state = ResourceBrowserSurfacePreviewState::BlockedByDiagnostic;
        }
        result.state.preview_eligible = false;
        result.status = ResourceBrowserSurfaceSelectionStatus::PreviewBlocked;
        return result;
    }

    result.state.preview_eligible =
        result.state.preview_state == ResourceBrowserSurfacePreviewState::Eligible;
    result.status = result.state.preview_eligible
        ? ResourceBrowserSurfaceSelectionStatus::Success
        : ResourceBrowserSurfaceSelectionStatus::PreviewBlocked;
    return result;
}

ResourceBrowserVisibleImportSettingRow BuildVisibleImportSettingRow(
    const ResourceBrowserResourceEntry &entry,
    const ResourceBrowserSurfaceRow &row,
    const ResourceBrowserSurfaceSelectionState &selection,
    std::uint32_t row_index) {
    ResourceBrowserVisibleImportSettingRow visible{};
    const bool selected = row_index == selection.selected_index;
    visible.source_path = entry.import_settings.source_path;
    visible.target_kind = entry.import_settings.target_kind;
    visible.resource_type = entry.import_settings.resource_type;
    visible.asset_type = entry.import_settings.asset_type;
    visible.stable_id = entry.import_settings.stable_id;
    visible.importer_version = entry.import_settings.importer_version;
    visible.expected_schema_version = entry.import_settings.expected_schema_version;
    visible.expected_source_hash = entry.import_settings.expected_source_hash;
    visible.validation =
        selected ? selection.setting_validation : ResourceBrowserSurfaceSettingValidationCode::None;
    visible.preview_state = row.preview_state;
    visible.selected = selected;
    visible.source_hash_matches =
        entry.import_settings.expected_source_hash == 0U ||
        entry.import_settings.expected_source_hash == row.source_hash;
    visible.target_kind_matches_header =
        row.header_kind == RuntimeAssetFileKind::Unknown ||
        entry.import_settings.target_kind == row.header_kind;
    return visible;
}

ResourceBrowserVisibleDiagnosticRow BuildVisibleDiagnosticRow(
    const ResourceBrowserDiagnosticRecord &diagnostic,
    const ResourceBrowserVisibleWorkflowRequest &request) {
    ResourceBrowserVisibleDiagnosticRow row{};
    row.code = diagnostic.code;
    row.severity = diagnostic.severity;
    row.phase = diagnostic.phase;
    row.runtime_status = diagnostic.runtime_status;
    row.source_path = diagnostic.source_path;
    row.file_index = diagnostic.file_index;
    row.dependency_index = diagnostic.dependency_index;
    row.selected = diagnostic.file_index == request.selected_index;
    row.blocks_preview = IsBlockingSeverity(diagnostic.severity);
    if (diagnostic.file_index < request.entries.size()) {
        row.stable_id = request.entries[diagnostic.file_index].import_settings.stable_id;
    }

    return row;
}

ResourceBrowserVisibleSelectionLedgerRecord BuildVisibleSelectionLedger(
    const ResourceBrowserSurfaceSelectionResult &selection_result) {
    ResourceBrowserVisibleSelectionLedgerRecord record{};
    record.selected_index = selection_result.state.selected_index;
    record.selection_status = selection_result.status;
    record.setting_validation = selection_result.state.setting_validation;
    record.preview_state = selection_result.state.preview_state;
    record.blocking_diagnostic_code = selection_result.state.blocking_diagnostic_code;
    record.resource = selection_result.state.resource;
    record.asset = selection_result.state.asset;
    record.stable_id = selection_result.state.stable_id;
    record.committed_to_preview_host =
        selection_result.status == ResourceBrowserSurfaceSelectionStatus::Success;
    record.rejected_preview_request =
        selection_result.status == ResourceBrowserSurfaceSelectionStatus::PreviewBlocked;
    record.preview_eligible = selection_result.state.preview_eligible;
    record.diagnostic_blocks_preview = selection_result.state.diagnostic_blocks_preview;
    record.resource_asset_mapping_preserved =
        selection_result.state.resource_asset_mapping_preserved;
    record.used_locator_path_as_type_truth =
        selection_result.state.used_locator_path_as_type_truth;
    return record;
}

ResourceBrowserDepthCatalogRow BuildDepthCatalogRow(
    const ResourceBrowserResourceEntry &entry,
    const ResourceBrowserSurfaceRow &surface_row,
    const ResourceBrowserImportSettings &settings,
    ResourceBrowserSourceBoundary boundary,
    ResourceBrowserImporterReadiness readiness,
    ResourceBrowserAssetManagerGap asset_gap,
    std::uint32_t row_index,
    std::uint32_t selected_index) {
    ResourceBrowserDepthCatalogRow row{};
    row.source_path = settings.source_path;
    row.target_kind = settings.target_kind;
    row.runtime_kind = surface_row.header_kind;
    row.source_boundary = boundary;
    row.importer_readiness = readiness;
    row.asset_manager_gap = asset_gap;
    row.preview_state = surface_row.preview_state;
    row.dependency_state = surface_row.dependency_state;
    row.resource = surface_row.resource;
    row.asset = surface_row.asset;
    row.stable_id = surface_row.stable_id;
    row.source_hash = surface_row.source_hash;
    row.payload_hash = surface_row.payload_hash;
    row.selected = row_index == selected_index;
    row.importer_ready = IsImporterReady(readiness);
    row.asset_manager_ready = IsAssetManagerReady(asset_gap);
    row.preview_request_ready =
        row.importer_ready &&
        row.asset_manager_ready &&
        row.preview_state == ResourceBrowserSurfacePreviewState::Eligible &&
        entry.from_runtime_asset_validation;
    return row;
}

ResourceBrowserImporterBoundaryRow BuildImporterBoundaryRow(
    const ResourceBrowserImportSettings &settings,
    ResourceBrowserSourceBoundary boundary,
    ResourceBrowserImporterReadiness readiness,
    ResourceBrowserSurfaceSettingValidationCode validation,
    std::uint32_t row_index,
    std::uint32_t selected_index) {
    ResourceBrowserImporterBoundaryRow row{};
    row.source_path = settings.source_path;
    row.target_kind = settings.target_kind;
    row.source_boundary = boundary;
    row.readiness = readiness;
    row.setting_validation = validation;
    row.stable_id = settings.stable_id;
    row.selected = row_index == selected_index;
    row.original_package_boundary =
        boundary == ResourceBrowserSourceBoundary::OriginalPackageBoundary;
    row.external_import_boundary =
        boundary == ResourceBrowserSourceBoundary::ExternalImportBoundary;
    row.importer_ready = IsImporterReady(readiness);
    return row;
}

ResourceBrowserAssetManagerGapRow BuildAssetManagerGapRow(
    const ResourceBrowserResourceEntry &entry,
    const ResourceBrowserSurfaceRow &surface_row,
    ResourceBrowserAssetManagerGap gap,
    std::uint32_t row_index,
    std::uint32_t selected_index) {
    ResourceBrowserAssetManagerGapRow row{};
    row.source_path = surface_row.locator_path;
    row.gap = gap;
    row.dependency_state = surface_row.dependency_state;
    row.resource = surface_row.resource;
    row.asset = surface_row.asset;
    row.stable_id = surface_row.stable_id;
    row.selected = row_index == selected_index;
    row.has_runtime_loaded_record = entry.from_runtime_asset_load;
    row.has_resource_registry_record = entry.from_resource_registry;
    row.has_asset_record = entry.from_asset_record;
    row.resource_handle_valid = entry.resource.IsValid();
    row.asset_handle_valid = entry.asset.IsValid();
    row.asset_manager_ready = IsAssetManagerReady(gap);
    return row;
}

ResourceBrowserDepthSelectionLedgerRecord BuildDepthSelectionLedger(
    const ResourceBrowserDepthCatalogRow &catalog_row,
    ResourceBrowserDepthWorkflowStatus status) {
    ResourceBrowserDepthSelectionLedgerRecord record{};
    record.status = status;
    record.source_boundary = catalog_row.source_boundary;
    record.importer_readiness = catalog_row.importer_readiness;
    record.asset_manager_gap = catalog_row.asset_manager_gap;
    record.preview_state = catalog_row.preview_state;
    record.resource = catalog_row.resource;
    record.asset = catalog_row.asset;
    record.stable_id = catalog_row.stable_id;
    record.selection_committed = catalog_row.preview_request_ready;
    record.selection_rejected = !catalog_row.preview_request_ready;
    record.blocked_by_original_package =
        catalog_row.source_boundary == ResourceBrowserSourceBoundary::OriginalPackageBoundary;
    record.blocked_by_external_import =
        catalog_row.source_boundary == ResourceBrowserSourceBoundary::ExternalImportBoundary;
    record.blocked_by_importer_gap = !catalog_row.importer_ready;
    record.blocked_by_asset_manager_gap = !catalog_row.asset_manager_ready;
    record.preview_request_ready = catalog_row.preview_request_ready;
    return record;
}

}

ResourceBrowserSurfaceStatus BuildResourceBrowserNativeSurface(
    const ResourceBrowserSurfaceRequest &request,
    ResourceBrowserSurfaceResult *out_result) {
    if (out_result == nullptr) {
        return ResourceBrowserSurfaceStatus::InvalidArgument;
    }

    ResourceBrowserSurfaceResult result{};
    if (!IsValidRequest(request)) {
        *out_result = result;
        return result.status;
    }

    if (request.rows.size() < request.entries.size()) {
        result.status = ResourceBrowserSurfaceStatus::OutputCapacityExceeded;
        *out_result = result;
        return result.status;
    }

    for (std::uint32_t index = 0U; index < request.entries.size(); ++index) {
        ResourceBrowserSurfaceRow row = BuildRow(request, request.entries[index], index);
        request.rows[index] = row;
        ++result.row_count;

        if (row.diagnostic_count > 0U) {
            ++result.diagnostic_row_count;
        }

        if (row.has_blocking_diagnostic) {
            ++result.blocker_row_count;
        }

        if (row.locator_path_is_type_truth) {
            result.used_locator_path_as_type_truth = true;
        }

        if (row.preview_state == ResourceBrowserSurfacePreviewState::Eligible) {
            ++result.eligible_preview_count;
        }

        if (row.preview_state != ResourceBrowserSurfacePreviewState::Eligible) {
            ++result.blocked_preview_count;
        }
    }

    result.status = ResourceBrowserSurfaceStatus::Success;
    *out_result = result;
    return result.status;
}

ResourceBrowserSurfaceSelectionStatus ResolveResourceBrowserSurfaceSelection(
    const ResourceBrowserSurfaceSelectionRequest &request,
    ResourceBrowserSurfaceSelectionResult *out_result) {
    if (out_result == nullptr) {
        return ResourceBrowserSurfaceSelectionStatus::InvalidArgument;
    }

    ResourceBrowserSurfaceSelectionResult result{};
    if (!IsValidSelectionRequest(request)) {
        *out_result = result;
        return result.status;
    }

    if (request.selected_index >= request.entries.size()) {
        result.status = ResourceBrowserSurfaceSelectionStatus::EntryOutOfRange;
        *out_result = result;
        return result.status;
    }

    result.state = BuildSelectionState(request);
    result.state.setting_validation =
        ValidateImportSettings(
            result.state.import_settings,
            request.entries[request.selected_index],
            request.rows[request.selected_index]);
    result.state.import_settings_valid =
        result.state.setting_validation == ResourceBrowserSurfaceSettingValidationCode::None;
    if (!result.state.import_settings_valid) {
        result.state.preview_eligible = false;
        result.state.preview_state = ResourceBrowserSurfacePreviewState::BlockedByValidation;
        result.status = ResourceBrowserSurfaceSelectionStatus::InvalidImportSettings;
        *out_result = result;
        return result.status;
    }

    result.state.preview_state = PreviewBlockedByDiagnosticState(request.rows[request.selected_index]);
    if (result.state.diagnostic_blocks_preview) {
        if (result.state.preview_state == ResourceBrowserSurfacePreviewState::Eligible) {
            result.state.preview_state = ResourceBrowserSurfacePreviewState::BlockedByDiagnostic;
        }
        result.state.preview_eligible = false;
        result.status = ResourceBrowserSurfaceSelectionStatus::PreviewBlocked;
        *out_result = result;
        return result.status;
    }

    result.state.preview_eligible =
        result.state.preview_state == ResourceBrowserSurfacePreviewState::Eligible;
    result.status = result.state.preview_eligible
        ? ResourceBrowserSurfaceSelectionStatus::Success
        : ResourceBrowserSurfaceSelectionStatus::PreviewBlocked;
    *out_result = result;
    return result.status;
}

ResourceBrowserVisibleWorkflowStatus BuildResourceBrowserVisibleWorkflowSurface(
    const ResourceBrowserVisibleWorkflowRequest &request,
    ResourceBrowserVisibleWorkflowResult *out_result) {
    if (out_result == nullptr) {
        return ResourceBrowserVisibleWorkflowStatus::InvalidArgument;
    }

    ResourceBrowserVisibleWorkflowResult result{};
    if (!IsValidVisibleWorkflowRequest(request)) {
        *out_result = result;
        return result.status;
    }

    if (request.selected_index >= request.entries.size()) {
        result.status = ResourceBrowserVisibleWorkflowStatus::EntryOutOfRange;
        result.selection_status = ResourceBrowserSurfaceSelectionStatus::EntryOutOfRange;
        *out_result = result;
        return result.status;
    }

    if (!HasVisibleWorkflowOutputCapacity(request)) {
        result.status = ResourceBrowserVisibleWorkflowStatus::OutputCapacityExceeded;
        result.surface_status = ResourceBrowserSurfaceStatus::OutputCapacityExceeded;
        *out_result = result;
        return result.status;
    }

    ResourceBrowserSurfaceSelectionResult preflight_selection =
        ResolveVisibleWorkflowSelection(request);
    result.selection_status = preflight_selection.status;
    result.selection_state = preflight_selection.state;
    if (preflight_selection.status == ResourceBrowserSurfaceSelectionStatus::InvalidImportSettings) {
        result.status = ResourceBrowserVisibleWorkflowStatus::InvalidImportSettings;
        *out_result = result;
        return result.status;
    }

    ResourceBrowserSurfaceRequest surface_request{};
    surface_request.entries = request.entries;
    surface_request.diagnostics = request.diagnostics;
    surface_request.rows = request.preview_rows;
    ResourceBrowserSurfaceResult surface_result{};
    BuildResourceBrowserNativeSurface(surface_request, &surface_result);
    result.surface_status = surface_result.status;
    if (!surface_result.Succeeded()) {
        result.status = surface_result.status == ResourceBrowserSurfaceStatus::OutputCapacityExceeded
            ? ResourceBrowserVisibleWorkflowStatus::OutputCapacityExceeded
            : ResourceBrowserVisibleWorkflowStatus::InvalidArgument;
        *out_result = result;
        return result.status;
    }

    ResourceBrowserSurfaceSelectionRequest selection_request{};
    selection_request.entries = request.entries;
    selection_request.rows =
        std::span<const ResourceBrowserSurfaceRow>(request.preview_rows.data(), request.entries.size());
    selection_request.diagnostics = request.diagnostics;
    selection_request.import_settings = request.import_settings;
    selection_request.selected_index = request.selected_index;
    selection_request.validate_import_settings = request.validate_import_settings;
    ResourceBrowserSurfaceSelectionResult selection_result{};
    ResolveResourceBrowserSurfaceSelection(selection_request, &selection_result);
    result.selection_status = selection_result.status;
    result.selection_state = selection_result.state;

    for (std::uint32_t index = 0U; index < request.entries.size(); ++index) {
        request.import_setting_rows[index] =
            BuildVisibleImportSettingRow(
                request.entries[index],
                request.preview_rows[index],
                result.selection_state,
                index);
    }

    for (std::uint32_t index = 0U; index < request.diagnostics.size(); ++index) {
        request.diagnostic_rows[index] =
            BuildVisibleDiagnosticRow(request.diagnostics[index], request);
    }

    request.selection_ledger[0U] = BuildVisibleSelectionLedger(selection_result);

    result.status = ResourceBrowserVisibleWorkflowStatus::Success;
    result.import_setting_row_count = static_cast<std::uint32_t>(request.entries.size());
    result.diagnostic_row_count = static_cast<std::uint32_t>(request.diagnostics.size());
    result.preview_row_count = surface_result.row_count;
    result.eligible_preview_count = surface_result.eligible_preview_count;
    result.blocked_preview_count = surface_result.blocked_preview_count;
    result.selection_ledger_count = 1U;
    result.emitted_import_settings = result.import_setting_row_count > 0U;
    result.emitted_diagnostics = result.diagnostic_row_count > 0U;
    result.emitted_preview_rows = result.preview_row_count > 0U;
    result.emitted_selection_ledger = true;
    result.selection_committed =
        selection_result.status == ResourceBrowserSurfaceSelectionStatus::Success;
    result.selection_rejected =
        selection_result.status == ResourceBrowserSurfaceSelectionStatus::PreviewBlocked;
    result.consumed_preview_host_ready_selection =
        result.selection_committed &&
        result.selection_state.preview_eligible &&
        result.selection_state.resource_asset_mapping_preserved &&
        !result.selection_state.used_locator_path_as_type_truth;
    result.used_locator_path_as_type_truth = surface_result.used_locator_path_as_type_truth ||
        result.selection_state.used_locator_path_as_type_truth;
    *out_result = result;
    return result.status;
}

ResourceBrowserDepthWorkflowStatus BuildResourceBrowserDepthWorkflowSurface(
    const ResourceBrowserDepthWorkflowRequest &request,
    ResourceBrowserDepthWorkflowResult *out_result) {
    if (out_result == nullptr) {
        return ResourceBrowserDepthWorkflowStatus::InvalidArgument;
    }

    ResourceBrowserDepthWorkflowResult result{};
    if (!IsValidDepthWorkflowRequest(request)) {
        *out_result = result;
        return result.status;
    }

    if (request.selected_index >= request.entries.size()) {
        result.status = ResourceBrowserDepthWorkflowStatus::EntryOutOfRange;
        result.selection_status = ResourceBrowserSurfaceSelectionStatus::EntryOutOfRange;
        *out_result = result;
        return result.status;
    }

    if (!HasDepthWorkflowOutputCapacity(request)) {
        result.status = ResourceBrowserDepthWorkflowStatus::OutputCapacityExceeded;
        *out_result = result;
        return result.status;
    }

    ResourceBrowserSurfaceRequest surface_request{};
    surface_request.entries = request.entries;
    surface_request.diagnostics = request.diagnostics;

    const ResourceBrowserSurfaceRow selected_surface_row =
        BuildRow(surface_request, request.entries[request.selected_index], request.selected_index);
    const ResourceBrowserSurfaceSettingValidationCode selected_validation =
        ValidateDepthImportSettings(request, selected_surface_row, request.selected_index);
    result.selected_setting_validation = selected_validation;
    if (selected_validation != ResourceBrowserSurfaceSettingValidationCode::None) {
        result.status = ResourceBrowserDepthWorkflowStatus::InvalidImportSettings;
        result.selection_status = ResourceBrowserSurfaceSelectionStatus::InvalidImportSettings;
        *out_result = result;
        return result.status;
    }

    for (std::uint32_t index = 0U; index < request.entries.size(); ++index) {
        const ResourceBrowserSurfaceRow surface_row =
            BuildRow(surface_request, request.entries[index], index);
        const ResourceBrowserImportSettings settings =
            ImportSettingsForDepthRow(request, index);
        const ResourceBrowserSurfaceSettingValidationCode validation =
            ValidateDepthImportSettings(request, surface_row, index);
        const ResourceBrowserSourceBoundary boundary =
            SourceBoundaryFor(settings.source_path);
        const ResourceBrowserImporterReadiness readiness =
            ImporterReadinessFor(boundary, validation);
        const ResourceBrowserAssetManagerGap asset_gap =
            AssetManagerGapFor(request.entries[index], surface_row);

        request.catalog_rows[index] =
            BuildDepthCatalogRow(
                request.entries[index],
                surface_row,
                settings,
                boundary,
                readiness,
                asset_gap,
                index,
                request.selected_index);
        request.importer_rows[index] =
            BuildImporterBoundaryRow(
                settings,
                boundary,
                readiness,
                validation,
                index,
                request.selected_index);
        request.asset_gap_rows[index] =
            BuildAssetManagerGapRow(
                request.entries[index],
                surface_row,
                asset_gap,
                index,
                request.selected_index);

        if (boundary == ResourceBrowserSourceBoundary::OriginalPackageBoundary) {
            ++result.original_package_boundary_count;
        }

        if (boundary == ResourceBrowserSourceBoundary::ExternalImportBoundary) {
            ++result.external_import_boundary_count;
        }

        if (request.catalog_rows[index].importer_ready) {
            ++result.importer_ready_count;
        }

        if (request.catalog_rows[index].asset_manager_ready) {
            ++result.asset_manager_ready_count;
        }

        if (request.catalog_rows[index].preview_request_ready) {
            ++result.preview_request_ready_count;
        }
    }

    ResourceBrowserDepthSelectionLedgerRecord ledger =
        BuildDepthSelectionLedger(
            request.catalog_rows[request.selected_index],
            ResourceBrowserDepthWorkflowStatus::Success);
    ledger.selected_index = request.selected_index;
    request.selection_ledger[0U] = ledger;

    result.status = ResourceBrowserDepthWorkflowStatus::Success;
    result.selection_status = ledger.selection_committed
        ? ResourceBrowserSurfaceSelectionStatus::Success
        : ResourceBrowserSurfaceSelectionStatus::PreviewBlocked;
    result.catalog_row_count = static_cast<std::uint32_t>(request.entries.size());
    result.importer_row_count = static_cast<std::uint32_t>(request.entries.size());
    result.asset_gap_row_count = static_cast<std::uint32_t>(request.entries.size());
    result.selection_ledger_count = 1U;
    result.emitted_catalog_rows = result.catalog_row_count > 0U;
    result.emitted_importer_rows = result.importer_row_count > 0U;
    result.emitted_asset_gap_rows = result.asset_gap_row_count > 0U;
    result.emitted_selection_ledger = true;
    result.selection_committed = ledger.selection_committed;
    result.selection_rejected = ledger.selection_rejected;
    result.blocked_by_original_package = ledger.blocked_by_original_package;
    result.blocked_by_external_import = ledger.blocked_by_external_import;
    result.blocked_by_importer_gap = ledger.blocked_by_importer_gap;
    result.blocked_by_asset_manager_gap = ledger.blocked_by_asset_manager_gap;
    *out_result = result;
    return result.status;
}

}
