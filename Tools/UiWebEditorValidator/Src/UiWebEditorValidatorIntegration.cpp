// 模块: Tools UiWebEditorValidator
// 文件: Tools/UiWebEditorValidator/Src/UiWebEditorValidatorIntegration.cpp

#include "YuEngine/UiWebEditorValidator/UiWebEditorValidatorIntegration.h"

#include <cstddef>

namespace yuengine::ui_web_editor_validator {
UiWebEditorValidationStatus UiWebEditorValidatorIntegration::Validate(
    const UiWebEditorValidationRequest &request,
    UiWebEditorValidationOutput output) const {
    if (output.report == nullptr) {
        return UiWebEditorValidationStatus::InvalidInput;
    }

    *output.report = UiWebEditorValidationReport{};
    output.report->document_id = request.document_id;
    output.report->layout_id = request.schema.header.layout_id;
    if (output.inspector == nullptr) {
        output.report->status = UiWebEditorValidationStatus::InvalidInput;
        return UiWebEditorValidationStatus::InvalidInput;
    }

    if (request.document_id == 0U) {
        output.report->status = UiWebEditorValidationStatus::InvalidDocument;
        return UiWebEditorValidationStatus::InvalidDocument;
    }

    yuengine::uicore::UiFileSchemaValidationResult schema_count_result{};
    const yuengine::uicore::UiFileSchemaStatus schema_count_status =
        CountSchemaIssues(request.schema, &schema_count_result);
    CopySchemaCountResult(request, schema_count_status, schema_count_result, output.report);
    if (IsSchemaHardFailure(schema_count_status)) {
        output.report->status = UiWebEditorValidationStatus::SchemaValidationFailed;
        return UiWebEditorValidationStatus::SchemaValidationFailed;
    }

    if (!HasIssueCapacity(schema_count_result.issue_count, output)) {
        WriteCapacityReport(schema_count_result.issue_count, output.report);
        return UiWebEditorValidationStatus::OutputCapacityExceeded;
    }

    yuengine::ui_web_editor_service::UiWebEditorValidateRequest validate_request{};
    validate_request.schema = request.schema;

    yuengine::ui_web_editor_service::UiWebEditorLocalServiceResult validate_result{};
    const yuengine::ui_web_editor_service::UiWebEditorLocalService service{};
    const yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus validate_status =
        service.ValidateUiFile(validate_request, output.schema_issues, &validate_result);
    CopyLocalServiceResult(validate_result, output.report);
    output.report->local_service_checked = true;
    if (validate_status == yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus::OutputCapacityExceeded) {
        WriteCapacityReport(validate_result.issue_count, output.report);
        return UiWebEditorValidationStatus::OutputCapacityExceeded;
    }

    if (validate_status == yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus::IssuesFound) {
        WriteIssueReports(
            output.schema_issues,
            validate_result.issue_count,
            output.validation_issues,
            output.report);
        output.report->status = UiWebEditorValidationStatus::IssuesFound;
        return UiWebEditorValidationStatus::IssuesFound;
    }

    if (validate_status != yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus::Success) {
        output.report->status = UiWebEditorValidationStatus::SchemaValidationFailed;
        return UiWebEditorValidationStatus::SchemaValidationFailed;
    }

    yuengine::ui_web_editor_service::UiWebEditorLoadRequest load_request{};
    load_request.document_id = request.document_id;
    load_request.schema = request.schema;

    yuengine::ui_web_editor_service::UiWebEditorLocalDocumentRecord document{};
    yuengine::ui_web_editor_service::UiWebEditorLocalServiceResult load_result{};
    const yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus load_status =
        service.LoadUiFile(load_request, &document, output.schema_issues, &load_result);
    CopyLocalServiceResult(load_result, output.report);
    if (load_status != yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus::Success) {
        output.report->status = UiWebEditorValidationStatus::SchemaValidationFailed;
        return UiWebEditorValidationStatus::SchemaValidationFailed;
    }

    yuengine::ui_web_editor_shell::UiWebEditorShellRequest shell_request{};
    shell_request.document = document;
    shell_request.schema = request.schema;
    shell_request.selected_node_id = request.selected_node_id;
    shell_request.has_selection = request.has_selection;

    yuengine::ui_web_editor_shell::UiWebEditorShellSnapshot shell_snapshot{};
    const yuengine::ui_web_editor_shell::UiWebEditorShell shell{};
    const yuengine::ui_web_editor_shell::UiWebEditorShellStatus shell_status =
        shell.BuildSnapshot(
            shell_request,
            output.hierarchy,
            output.inspector,
            output.canvas,
            output.resources,
            &shell_snapshot);
    output.report->shell_checked = true;
    output.report->shell_status = shell_status;
    if (shell_status != yuengine::ui_web_editor_shell::UiWebEditorShellStatus::Success) {
        return MapShellStatus(shell_status, output.report);
    }

    yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewRequest preview_request{};
    preview_request.message_kind = request.preview_message_kind;
    preview_request.request_id = request.preview_request_id;
    preview_request.protocol_version = request.preview_protocol_version;
    preview_request.schema = request.schema;
    preview_request.document = document;
    preview_request.local_service_result = load_result;
    preview_request.shell_snapshot = shell_snapshot;
    preview_request.selected_node_id = request.selected_node_id;
    preview_request.has_selection = request.has_selection;

    yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewResponse preview_response{};
    const yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewProtocol preview_protocol{};
    const yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus preview_status =
        preview_protocol.BuildPreviewResponse(
            preview_request,
            output.preview_diagnostics,
            &preview_response);
    output.report->preview_checked = true;
    output.report->preview_status = preview_status;
    output.report->preview_diagnostic_count = preview_response.diagnostic_count;
    if (preview_status != yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus::Success) {
        return MapPreviewStatus(preview_status, preview_response, output.report);
    }

    output.report->status = UiWebEditorValidationStatus::Success;
    return UiWebEditorValidationStatus::Success;
}

yuengine::uicore::UiFileSchemaStatus UiWebEditorValidatorIntegration::CountSchemaIssues(
    const yuengine::uicore::UiFileSchemaDesc &schema,
    yuengine::uicore::UiFileSchemaValidationResult *out_result) const {
    const yuengine::uicore::UiFileSchemaValidator validator{};
    return validator.Validate(schema, {}, out_result);
}

bool UiWebEditorValidatorIntegration::IsSchemaHardFailure(
    yuengine::uicore::UiFileSchemaStatus status) const {
    if (status == yuengine::uicore::UiFileSchemaStatus::Success) {
        return false;
    }

    return status != yuengine::uicore::UiFileSchemaStatus::OutputCapacityExceeded;
}

bool UiWebEditorValidatorIntegration::HasIssueCapacity(
    std::uint32_t issue_count,
    const UiWebEditorValidationOutput &output) const {
    const std::size_t required_count = static_cast<std::size_t>(issue_count);
    if (output.schema_issues.size() < required_count) {
        return false;
    }

    return output.validation_issues.size() >= required_count;
}

UiWebEditorValidationIssueSource UiWebEditorValidatorIntegration::MapIssueSource(
    yuengine::uicore::UiFileSchemaIssueKind issue_kind) const {
    if (issue_kind == yuengine::uicore::UiFileSchemaIssueKind::DuplicateNodeId) {
        return UiWebEditorValidationIssueSource::DuplicateId;
    }

    if (issue_kind == yuengine::uicore::UiFileSchemaIssueKind::MissingResourceRefNode) {
        return UiWebEditorValidationIssueSource::ResourceRef;
    }

    return UiWebEditorValidationIssueSource::Schema;
}

void UiWebEditorValidatorIntegration::CopySchemaCountResult(
    const UiWebEditorValidationRequest &request,
    yuengine::uicore::UiFileSchemaStatus status,
    const yuengine::uicore::UiFileSchemaValidationResult &schema_result,
    UiWebEditorValidationReport *out_report) const {
    if (out_report == nullptr) {
        return;
    }

    out_report->schema_checked = true;
    out_report->schema_status = status;
    out_report->document_id = request.document_id;
    out_report->layout_id = request.schema.header.layout_id;
    out_report->checked_node_count = schema_result.checked_node_count;
    out_report->checked_layout_count = schema_result.checked_layout_count;
    out_report->checked_style_ref_count = schema_result.checked_style_ref_count;
    out_report->checked_resource_ref_count = schema_result.checked_resource_ref_count;
    out_report->checked_event_binding_count = schema_result.checked_event_binding_count;
    out_report->schema_issue_count = schema_result.issue_count;
}

void UiWebEditorValidatorIntegration::CopyLocalServiceResult(
    const yuengine::ui_web_editor_service::UiWebEditorLocalServiceResult &service_result,
    UiWebEditorValidationReport *out_report) const {
    if (out_report == nullptr) {
        return;
    }

    out_report->local_service_status = service_result.status;
    out_report->schema_status = service_result.schema_status;
    out_report->checked_node_count = service_result.checked_node_count;
    out_report->checked_layout_count = service_result.checked_layout_count;
    out_report->checked_style_ref_count = service_result.checked_style_ref_count;
    out_report->checked_resource_ref_count = service_result.checked_resource_ref_count;
    out_report->checked_event_binding_count = service_result.checked_event_binding_count;
    out_report->schema_issue_count = service_result.issue_count;
}

void UiWebEditorValidatorIntegration::WriteCapacityReport(
    std::uint32_t issue_count,
    UiWebEditorValidationReport *out_report) const {
    if (out_report == nullptr) {
        return;
    }

    out_report->status = UiWebEditorValidationStatus::OutputCapacityExceeded;
    out_report->schema_issue_count = issue_count;
    out_report->overflow_issue_count = 1U;
}

void UiWebEditorValidatorIntegration::WriteIssueReports(
    std::span<const yuengine::uicore::UiFileSchemaIssueRecord> schema_issues,
    std::uint32_t issue_count,
    std::span<UiWebEditorValidationIssueRecord> out_validation_issues,
    UiWebEditorValidationReport *out_report) const {
    if (out_report == nullptr) {
        return;
    }

    out_report->validation_issue_count = issue_count;
    for (std::size_t index = 0U; index < static_cast<std::size_t>(issue_count); ++index) {
        const yuengine::uicore::UiFileSchemaIssueRecord &schema_issue = schema_issues[index];
        UiWebEditorValidationIssueRecord report_issue{};
        report_issue.source = MapIssueSource(schema_issue.issue_kind);
        report_issue.schema_issue_kind = schema_issue.issue_kind;
        report_issue.node_id = schema_issue.node_id;
        report_issue.context_key = schema_issue.context_key;
        report_issue.record_index = schema_issue.record_index;
        report_issue.duplicate_count = schema_issue.duplicate_count;
        report_issue.status_code = static_cast<std::uint32_t>(schema_issue.issue_kind);
        out_validation_issues[index] = report_issue;

        if (report_issue.source == UiWebEditorValidationIssueSource::DuplicateId) {
            ++out_report->duplicate_id_issue_count;
        }

        if (report_issue.source == UiWebEditorValidationIssueSource::ResourceRef) {
            ++out_report->resource_ref_issue_count;
        }
    }
}

UiWebEditorValidationStatus UiWebEditorValidatorIntegration::MapShellStatus(
    yuengine::ui_web_editor_shell::UiWebEditorShellStatus status,
    UiWebEditorValidationReport *out_report) const {
    if (status == yuengine::ui_web_editor_shell::UiWebEditorShellStatus::OutputCapacityExceeded) {
        WriteCapacityReport(0U, out_report);
        return UiWebEditorValidationStatus::OutputCapacityExceeded;
    }

    if (out_report != nullptr) {
        out_report->status = UiWebEditorValidationStatus::ShellValidationFailed;
    }

    return UiWebEditorValidationStatus::ShellValidationFailed;
}

UiWebEditorValidationStatus UiWebEditorValidatorIntegration::MapPreviewStatus(
    yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus status,
    const yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewResponse &response,
    UiWebEditorValidationReport *out_report) const {
    if (status == yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus::OutputCapacityExceeded) {
        if (out_report != nullptr) {
            out_report->preview_diagnostic_count = response.diagnostic_count;
        }

        WriteCapacityReport(0U, out_report);
        return UiWebEditorValidationStatus::OutputCapacityExceeded;
    }

    if (out_report != nullptr) {
        out_report->status = UiWebEditorValidationStatus::PreviewValidationFailed;
    }

    return UiWebEditorValidationStatus::PreviewValidationFailed;
}
}
