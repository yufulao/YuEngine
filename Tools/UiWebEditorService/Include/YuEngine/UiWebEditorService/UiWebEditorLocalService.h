// 模块: Tools UiWebEditorService
// 文件: Tools/UiWebEditorService/Include/YuEngine/UiWebEditorService/UiWebEditorLocalService.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"

namespace yuengine::ui_web_editor_service {
constexpr std::uint32_t UI_WEB_EDITOR_LOCAL_SERVICE_VERSION = 1U;

enum class UiWebEditorLocalServiceStatus {
    Success = 0,
    IssuesFound,
    InvalidInput,
    InvalidOutput,
    InvalidDocument,
    SchemaValidationFailed,
    OutputCapacityExceeded
};

struct UiWebEditorLocalDocumentRecord final {
    std::uint32_t document_id = 0U;
    std::uint32_t service_version = UI_WEB_EDITOR_LOCAL_SERVICE_VERSION;
    std::uint32_t schema_version = 0U;
    std::uint32_t layout_id = 0U;
    std::uint32_t node_count = 0U;
    std::uint32_t layout_count = 0U;
    std::uint32_t style_ref_count = 0U;
    std::uint32_t resource_ref_count = 0U;
    std::uint32_t event_binding_count = 0U;
    bool loaded = false;
    bool dirty = false;
};

struct UiWebEditorLoadRequest final {
    std::uint32_t document_id = 0U;
    yuengine::uicore::UiFileSchemaDesc schema;
};

struct UiWebEditorValidateRequest final {
    yuengine::uicore::UiFileSchemaDesc schema;
};

struct UiWebEditorSaveRequest final {
    UiWebEditorLocalDocumentRecord document;
    yuengine::uicore::UiFileSchemaDesc schema;
};

struct UiWebEditorLocalServiceResult final {
    UiWebEditorLocalServiceStatus status = UiWebEditorLocalServiceStatus::Success;
    yuengine::uicore::UiFileSchemaStatus schema_status = yuengine::uicore::UiFileSchemaStatus::Success;
    std::uint32_t document_id = 0U;
    std::uint32_t layout_id = 0U;
    std::uint32_t checked_node_count = 0U;
    std::uint32_t checked_layout_count = 0U;
    std::uint32_t checked_style_ref_count = 0U;
    std::uint32_t checked_resource_ref_count = 0U;
    std::uint32_t checked_event_binding_count = 0U;
    std::uint32_t issue_count = 0U;

    /**
     * @comment 检查 local editor service command 是否成功。
     * @return command 成功且 schema validation 没有 issue 时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorLocalServiceStatus::Success;
    }
};

class UiWebEditorLocalService final {
public:
    /**
     * @comment 加载通用 UI file schema payload，并生成本地 document record。
     * @param request load command 输入。
     * @param out_document 输出本地 document record。
     * @param out_issues 调用方持有的 schema issue buffer。
     * @param out_result 输出 command result。
     * @return 显式 command 状态。
     */
    UiWebEditorLocalServiceStatus LoadUiFile(
        const UiWebEditorLoadRequest &request,
        UiWebEditorLocalDocumentRecord *out_document,
        std::span<yuengine::uicore::UiFileSchemaIssueRecord> out_issues,
        UiWebEditorLocalServiceResult *out_result) const;

    /**
     * @comment 校验通用 UI file schema payload。
     * @param request validate command 输入。
     * @param out_issues 调用方持有的 schema issue buffer。
     * @param out_result 输出 command result。
     * @return 显式 command 状态。
     */
    UiWebEditorLocalServiceStatus ValidateUiFile(
        const UiWebEditorValidateRequest &request,
        std::span<yuengine::uicore::UiFileSchemaIssueRecord> out_issues,
        UiWebEditorLocalServiceResult *out_result) const;

    /**
     * @comment 保存通用 UI file schema payload，并刷新本地 document record。
     * @param request save command 输入。
     * @param out_document 输出保存后的本地 document record。
     * @param out_issues 调用方持有的 schema issue buffer。
     * @param out_result 输出 command result。
     * @return 显式 command 状态。
     */
    UiWebEditorLocalServiceStatus SaveUiFile(
        const UiWebEditorSaveRequest &request,
        UiWebEditorLocalDocumentRecord *out_document,
        std::span<yuengine::uicore::UiFileSchemaIssueRecord> out_issues,
        UiWebEditorLocalServiceResult *out_result) const;

private:
    UiWebEditorLocalServiceStatus ValidateSchema(
        const yuengine::uicore::UiFileSchemaDesc &schema,
        std::span<yuengine::uicore::UiFileSchemaIssueRecord> out_issues,
        UiWebEditorLocalServiceResult *out_result) const;
    void WriteDocument(
        std::uint32_t document_id,
        const yuengine::uicore::UiFileSchemaDesc &schema,
        bool dirty,
        UiWebEditorLocalDocumentRecord *out_document) const;
    void CopySchemaResult(
        const yuengine::uicore::UiFileSchemaValidationResult &schema_result,
        UiWebEditorLocalServiceResult *out_result) const;
};
}
