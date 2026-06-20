// 模块: Tools UiWebEditorValidator
// 文件: Tools/UiWebEditorValidator/Include/YuEngine/UiWebEditorValidator/UiWebEditorValidatorIntegration.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiWebEditorPreviewProtocol/UiWebEditorPreviewProtocol.h"

namespace yuengine::ui_web_editor_validator {
constexpr std::uint32_t UI_WEB_EDITOR_VALIDATOR_INTEGRATION_VERSION = 1U;

enum class UiWebEditorValidationStatus {
    Success = 0,
    IssuesFound,
    InvalidInput,
    InvalidDocument,
    SchemaValidationFailed,
    ShellValidationFailed,
    PreviewValidationFailed,
    OutputCapacityExceeded
};

enum class UiWebEditorValidationIssueSource {
    None = 0,
    Schema,
    DuplicateId,
    ResourceRef,
    Overflow
};

struct UiWebEditorValidationRequest final {
    std::uint32_t document_id = 0U;
    yuengine::uicore::UiFileSchemaDesc schema;
    yuengine::uicore::UiNodeId selected_node_id;
    bool has_selection = false;
    yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewMessageKind preview_message_kind =
        yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewMessageKind::LoadDocument;
    std::uint32_t preview_request_id = 0U;
    std::uint32_t preview_protocol_version =
        yuengine::ui_web_editor_preview_protocol::UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
};

struct UiWebEditorValidationIssueRecord final {
    UiWebEditorValidationIssueSource source = UiWebEditorValidationIssueSource::None;
    yuengine::uicore::UiFileSchemaIssueKind schema_issue_kind =
        yuengine::uicore::UiFileSchemaIssueKind::None;
    yuengine::uicore::UiNodeId node_id;
    std::uint32_t context_key = 0U;
    std::uint32_t record_index = 0U;
    std::uint32_t duplicate_count = 0U;
    std::uint32_t status_code = 0U;
};

struct UiWebEditorValidationReport final {
    UiWebEditorValidationStatus status = UiWebEditorValidationStatus::Success;
    yuengine::uicore::UiFileSchemaStatus schema_status =
        yuengine::uicore::UiFileSchemaStatus::Success;
    yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus local_service_status =
        yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus::Success;
    yuengine::ui_web_editor_shell::UiWebEditorShellStatus shell_status =
        yuengine::ui_web_editor_shell::UiWebEditorShellStatus::Success;
    yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus preview_status =
        yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus::Success;
    std::uint32_t integration_version = UI_WEB_EDITOR_VALIDATOR_INTEGRATION_VERSION;
    std::uint32_t document_id = 0U;
    std::uint32_t layout_id = 0U;
    std::uint32_t checked_node_count = 0U;
    std::uint32_t checked_layout_count = 0U;
    std::uint32_t checked_style_ref_count = 0U;
    std::uint32_t checked_resource_ref_count = 0U;
    std::uint32_t checked_event_binding_count = 0U;
    std::uint32_t schema_issue_count = 0U;
    std::uint32_t validation_issue_count = 0U;
    std::uint32_t duplicate_id_issue_count = 0U;
    std::uint32_t resource_ref_issue_count = 0U;
    std::uint32_t overflow_issue_count = 0U;
    std::uint32_t preview_diagnostic_count = 0U;
    bool schema_checked = false;
    bool local_service_checked = false;
    bool shell_checked = false;
    bool preview_checked = false;

    /**
     * @comment 检查 Web editor validator integration 是否成功。
     * @return validator integration 完成且无 issue 时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorValidationStatus::Success;
    }
};

struct UiWebEditorValidationOutput final {
    std::span<yuengine::uicore::UiFileSchemaIssueRecord> schema_issues{};
    std::span<UiWebEditorValidationIssueRecord> validation_issues{};
    std::span<yuengine::ui_web_editor_shell::UiWebEditorHierarchyItemRecord> hierarchy{};
    yuengine::ui_web_editor_shell::UiWebEditorInspectorRecord *inspector = nullptr;
    std::span<yuengine::ui_web_editor_shell::UiWebEditorCanvasItemRecord> canvas{};
    std::span<yuengine::ui_web_editor_shell::UiWebEditorResourceItemRecord> resources{};
    std::span<yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewDiagnosticRecord>
        preview_diagnostics{};
    UiWebEditorValidationReport *report = nullptr;
};

class UiWebEditorValidatorIntegration final {
public:
    /**
     * @comment 执行 Web editor 通用 validator integration，并输出 bounded validation report。
     * @param request validation 输入。
     * @param output 调用方持有的所有输出 buffer 和 report。
     * @return 显式 validation 状态。
     */
    UiWebEditorValidationStatus Validate(
        const UiWebEditorValidationRequest &request,
        UiWebEditorValidationOutput output) const;

private:
    yuengine::uicore::UiFileSchemaStatus CountSchemaIssues(
        const yuengine::uicore::UiFileSchemaDesc &schema,
        yuengine::uicore::UiFileSchemaValidationResult *out_result) const;
    bool IsSchemaHardFailure(yuengine::uicore::UiFileSchemaStatus status) const;
    bool HasIssueCapacity(
        std::uint32_t issue_count,
        const UiWebEditorValidationOutput &output) const;
    UiWebEditorValidationIssueSource MapIssueSource(
        yuengine::uicore::UiFileSchemaIssueKind issue_kind) const;
    void CopySchemaCountResult(
        const UiWebEditorValidationRequest &request,
        yuengine::uicore::UiFileSchemaStatus status,
        const yuengine::uicore::UiFileSchemaValidationResult &schema_result,
        UiWebEditorValidationReport *out_report) const;
    void CopyLocalServiceResult(
        const yuengine::ui_web_editor_service::UiWebEditorLocalServiceResult &service_result,
        UiWebEditorValidationReport *out_report) const;
    void WriteCapacityReport(
        std::uint32_t issue_count,
        UiWebEditorValidationReport *out_report) const;
    void WriteIssueReports(
        std::span<const yuengine::uicore::UiFileSchemaIssueRecord> schema_issues,
        std::uint32_t issue_count,
        std::span<UiWebEditorValidationIssueRecord> out_validation_issues,
        UiWebEditorValidationReport *out_report) const;
    UiWebEditorValidationStatus MapShellStatus(
        yuengine::ui_web_editor_shell::UiWebEditorShellStatus status,
        UiWebEditorValidationReport *out_report) const;
    UiWebEditorValidationStatus MapPreviewStatus(
        yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus status,
        const yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewResponse &response,
        UiWebEditorValidationReport *out_report) const;
};
}
