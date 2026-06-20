// 模块: Tools UiWebEditorService
// 文件: Tools/UiWebEditorService/Src/UiWebEditorLocalService.cpp

#include "YuEngine/UiWebEditorService/UiWebEditorLocalService.h"

namespace yuengine::ui_web_editor_service {
namespace {
UiWebEditorLocalServiceStatus MapSchemaStatus(yuengine::uicore::UiFileSchemaStatus status) {
    if (status == yuengine::uicore::UiFileSchemaStatus::Success) {
        return UiWebEditorLocalServiceStatus::Success;
    }

    if (status == yuengine::uicore::UiFileSchemaStatus::IssuesFound) {
        return UiWebEditorLocalServiceStatus::IssuesFound;
    }

    if (status == yuengine::uicore::UiFileSchemaStatus::OutputCapacityExceeded) {
        return UiWebEditorLocalServiceStatus::OutputCapacityExceeded;
    }

    return UiWebEditorLocalServiceStatus::SchemaValidationFailed;
}
}

UiWebEditorLocalServiceStatus UiWebEditorLocalService::LoadUiFile(
    const UiWebEditorLoadRequest &request,
    UiWebEditorLocalDocumentRecord *out_document,
    std::span<yuengine::uicore::UiFileSchemaIssueRecord> out_issues,
    UiWebEditorLocalServiceResult *out_result) const {
    if (out_result == nullptr) {
        return UiWebEditorLocalServiceStatus::InvalidInput;
    }

    *out_result = UiWebEditorLocalServiceResult{};
    out_result->document_id = request.document_id;
    out_result->layout_id = request.schema.header.layout_id;
    if (request.document_id == 0U) {
        out_result->status = UiWebEditorLocalServiceStatus::InvalidDocument;
        return UiWebEditorLocalServiceStatus::InvalidDocument;
    }

    if (out_document == nullptr) {
        out_result->status = UiWebEditorLocalServiceStatus::InvalidOutput;
        return UiWebEditorLocalServiceStatus::InvalidOutput;
    }

    const UiWebEditorLocalServiceStatus schema_status =
        ValidateSchema(request.schema, out_issues, out_result);
    if (schema_status != UiWebEditorLocalServiceStatus::Success) {
        out_result->status = schema_status;
        return schema_status;
    }

    WriteDocument(request.document_id, request.schema, false, out_document);
    out_result->status = UiWebEditorLocalServiceStatus::Success;
    return UiWebEditorLocalServiceStatus::Success;
}

UiWebEditorLocalServiceStatus UiWebEditorLocalService::ValidateUiFile(
    const UiWebEditorValidateRequest &request,
    std::span<yuengine::uicore::UiFileSchemaIssueRecord> out_issues,
    UiWebEditorLocalServiceResult *out_result) const {
    if (out_result == nullptr) {
        return UiWebEditorLocalServiceStatus::InvalidInput;
    }

    *out_result = UiWebEditorLocalServiceResult{};
    out_result->layout_id = request.schema.header.layout_id;
    const UiWebEditorLocalServiceStatus schema_status =
        ValidateSchema(request.schema, out_issues, out_result);
    out_result->status = schema_status;
    return schema_status;
}

UiWebEditorLocalServiceStatus UiWebEditorLocalService::SaveUiFile(
    const UiWebEditorSaveRequest &request,
    UiWebEditorLocalDocumentRecord *out_document,
    std::span<yuengine::uicore::UiFileSchemaIssueRecord> out_issues,
    UiWebEditorLocalServiceResult *out_result) const {
    if (out_result == nullptr) {
        return UiWebEditorLocalServiceStatus::InvalidInput;
    }

    *out_result = UiWebEditorLocalServiceResult{};
    out_result->document_id = request.document.document_id;
    out_result->layout_id = request.schema.header.layout_id;
    if (request.document.document_id == 0U || !request.document.loaded) {
        out_result->status = UiWebEditorLocalServiceStatus::InvalidDocument;
        return UiWebEditorLocalServiceStatus::InvalidDocument;
    }

    if (out_document == nullptr) {
        out_result->status = UiWebEditorLocalServiceStatus::InvalidOutput;
        return UiWebEditorLocalServiceStatus::InvalidOutput;
    }

    const UiWebEditorLocalServiceStatus schema_status =
        ValidateSchema(request.schema, out_issues, out_result);
    if (schema_status != UiWebEditorLocalServiceStatus::Success) {
        out_result->status = schema_status;
        return schema_status;
    }

    WriteDocument(request.document.document_id, request.schema, false, out_document);
    out_result->status = UiWebEditorLocalServiceStatus::Success;
    return UiWebEditorLocalServiceStatus::Success;
}

UiWebEditorLocalServiceStatus UiWebEditorLocalService::ValidateSchema(
    const yuengine::uicore::UiFileSchemaDesc &schema,
    std::span<yuengine::uicore::UiFileSchemaIssueRecord> out_issues,
    UiWebEditorLocalServiceResult *out_result) const {
    if (out_result == nullptr) {
        return UiWebEditorLocalServiceStatus::InvalidInput;
    }

    yuengine::uicore::UiFileSchemaValidationResult schema_result{};
    const yuengine::uicore::UiFileSchemaValidator validator{};
    const yuengine::uicore::UiFileSchemaStatus schema_status =
        validator.Validate(schema, out_issues, &schema_result);
    out_result->schema_status = schema_status;
    CopySchemaResult(schema_result, out_result);
    return MapSchemaStatus(schema_status);
}

void UiWebEditorLocalService::WriteDocument(
    std::uint32_t document_id,
    const yuengine::uicore::UiFileSchemaDesc &schema,
    bool dirty,
    UiWebEditorLocalDocumentRecord *out_document) const {
    if (out_document == nullptr) {
        return;
    }

    UiWebEditorLocalDocumentRecord document{};
    document.document_id = document_id;
    document.service_version = UI_WEB_EDITOR_LOCAL_SERVICE_VERSION;
    document.schema_version = schema.header.schema_version;
    document.layout_id = schema.header.layout_id;
    document.node_count = static_cast<std::uint32_t>(schema.nodes.size());
    document.layout_count = static_cast<std::uint32_t>(schema.layouts.size());
    document.style_ref_count = static_cast<std::uint32_t>(schema.style_refs.size());
    document.resource_ref_count = static_cast<std::uint32_t>(schema.resource_refs.size());
    document.event_binding_count = static_cast<std::uint32_t>(schema.event_bindings.size());
    document.loaded = true;
    document.dirty = dirty;
    *out_document = document;
}

void UiWebEditorLocalService::CopySchemaResult(
    const yuengine::uicore::UiFileSchemaValidationResult &schema_result,
    UiWebEditorLocalServiceResult *out_result) const {
    if (out_result == nullptr) {
        return;
    }

    out_result->checked_node_count = schema_result.checked_node_count;
    out_result->checked_layout_count = schema_result.checked_layout_count;
    out_result->checked_style_ref_count = schema_result.checked_style_ref_count;
    out_result->checked_resource_ref_count = schema_result.checked_resource_ref_count;
    out_result->checked_event_binding_count = schema_result.checked_event_binding_count;
    out_result->issue_count = schema_result.issue_count;
}
}
