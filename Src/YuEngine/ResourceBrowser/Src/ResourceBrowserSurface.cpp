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

}
