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

void ApplySelectionDiagnostics(
    const ResourceBrowserSurfaceSelectionRequest &request,
    ResourceBrowserSurfaceSelectionState *state) {
    if (state == nullptr) {
        return;
    }

    for (const ResourceBrowserDiagnosticRecord &diagnostic : request.diagnostics) {
        if (diagnostic.file_index != request.selected_index) {
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

ResourceBrowserSurfaceSelectionState BuildSelectionState(
    const ResourceBrowserSurfaceSelectionRequest &request) {
    const ResourceBrowserResourceEntry &entry = request.entries[request.selected_index];
    const ResourceBrowserSurfaceRow &row = request.rows[request.selected_index];
    ResourceBrowserSurfaceSelectionState state{};
    state.selected_index = request.selected_index;
    state.import_settings = entry.import_settings;
    if (request.validate_import_settings) {
        state.import_settings = request.import_settings;
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
    ApplySelectionDiagnostics(request, &state);
    return state;
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
        } else {
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

}
