// 模块: Tools UiWebEditorStatePreview
// 文件: Tools/UiWebEditorStatePreview/Include/YuEngine/UiWebEditorStatePreview/UiWebEditorStatePreviewData.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/UiWebEditorValidator/UiWebEditorValidatorIntegration.h"

namespace yuengine::ui_web_editor_state_preview {
constexpr std::uint32_t UI_WEB_EDITOR_STATE_PREVIEW_DATA_VERSION = 1U;
constexpr std::uint32_t UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT = 3U;
constexpr std::uint32_t UI_WEB_EDITOR_STATE_PREVIEW_NAME_CAPACITY = 32U;

enum class UiWebEditorStatePreviewValueKind {
    Invalid = 0,
    Bool,
    Number1000,
    ResourceKey
};

enum class UiWebEditorStatePreviewStatus {
    Success = 0,
    IssuesFound,
    InvalidInput,
    InvalidStateInput,
    InvalidOutput,
    ValidatorRejected,
    OutputCapacityExceeded
};

struct UiWebEditorStatePreviewInputRecord final {
    std::uint32_t input_key = 0U;
    yuengine::uicore::UiNodeId node_id;
    UiWebEditorStatePreviewValueKind value_kind = UiWebEditorStatePreviewValueKind::Invalid;
    char name[UI_WEB_EDITOR_STATE_PREVIEW_NAME_CAPACITY]{};
    std::uint32_t value0 = 0U;
    std::uint32_t value1 = 0U;
    std::uint32_t value2 = 0U;
    std::uint32_t value3 = 0U;
    bool affects_visibility = false;
    bool affects_enabled = false;
    bool affects_resource = false;
};

struct UiWebEditorStatePreviewOutputRecord final {
    std::uint32_t input_key = 0U;
    yuengine::uicore::UiNodeId node_id;
    UiWebEditorStatePreviewValueKind value_kind = UiWebEditorStatePreviewValueKind::Invalid;
    std::uint32_t value0 = 0U;
    std::uint32_t value1 = 0U;
    std::uint32_t value2 = 0U;
    std::uint32_t value3 = 0U;
    bool accepted = false;
    bool schema_checked = false;
    bool validation_checked = false;
};

struct UiWebEditorStatePreviewCatalogResult final {
    UiWebEditorStatePreviewStatus status = UiWebEditorStatePreviewStatus::Success;
    std::uint32_t data_version = UI_WEB_EDITOR_STATE_PREVIEW_DATA_VERSION;
    std::uint32_t input_count = 0U;
    std::uint32_t output_capacity = 0U;

    /**
     * @comment 检查 state preview input catalog 是否写入成功。
     * @return catalog 写入成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorStatePreviewStatus::Success;
    }
};

struct UiWebEditorStatePreviewRequest final {
    std::uint32_t document_id = 0U;
    yuengine::uicore::UiFileSchemaDesc schema;
    std::span<const UiWebEditorStatePreviewInputRecord> state_inputs{};
    yuengine::uicore::UiNodeId selected_node_id;
    bool has_selection = false;
    std::uint32_t preview_request_id = 0U;
    yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewMessageKind preview_message_kind =
        yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewMessageKind::UpdateDocument;
};

struct UiWebEditorStatePreviewResult final {
    UiWebEditorStatePreviewStatus status = UiWebEditorStatePreviewStatus::Success;
    yuengine::ui_web_editor_validator::UiWebEditorValidationStatus validation_status =
        yuengine::ui_web_editor_validator::UiWebEditorValidationStatus::Success;
    yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus preview_status =
        yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus::Success;
    std::uint32_t data_version = UI_WEB_EDITOR_STATE_PREVIEW_DATA_VERSION;
    std::uint32_t document_id = 0U;
    std::uint32_t layout_id = 0U;
    std::uint32_t state_input_count = 0U;
    std::uint32_t state_output_count = 0U;
    std::uint32_t checked_node_count = 0U;
    std::uint32_t schema_issue_count = 0U;
    std::uint32_t validation_issue_count = 0U;
    std::uint32_t preview_diagnostic_count = 0U;
    bool schema_ready = false;
    bool validation_checked = false;

    /**
     * @comment 检查 state preview 是否构建成功。
     * @return state preview 构建成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorStatePreviewStatus::Success;
    }
};

struct UiWebEditorStatePreviewOutput final {
    std::span<UiWebEditorStatePreviewOutputRecord> state_outputs{};
    std::span<yuengine::uicore::UiFileSchemaIssueRecord> schema_issues{};
    std::span<yuengine::ui_web_editor_validator::UiWebEditorValidationIssueRecord> validation_issues{};
    std::span<yuengine::ui_web_editor_shell::UiWebEditorHierarchyItemRecord> hierarchy{};
    yuengine::ui_web_editor_shell::UiWebEditorInspectorRecord *inspector = nullptr;
    std::span<yuengine::ui_web_editor_shell::UiWebEditorCanvasItemRecord> canvas{};
    std::span<yuengine::ui_web_editor_shell::UiWebEditorResourceItemRecord> resources{};
    std::span<yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewDiagnosticRecord>
        preview_diagnostics{};
    UiWebEditorStatePreviewResult *result = nullptr;
};

class UiWebEditorStatePreviewData final {
public:
    /**
     * @comment 写入 Web editor 通用 state preview input 数据。
     * @param out_inputs 调用方持有的 state input 输出。
     * @param out_result 输出 catalog result。
     * @return 显式 catalog 写入状态。
     */
    UiWebEditorStatePreviewStatus WriteDefaultInputs(
        std::span<UiWebEditorStatePreviewInputRecord> out_inputs,
        UiWebEditorStatePreviewCatalogResult *out_result) const;

    /**
     * @comment 基于 state input 和 schema 构建 validator-facing preview output。
     * @param request state preview 输入。
     * @param output 调用方持有的 bounded 输出。
     * @return 显式 state preview 状态。
     */
    UiWebEditorStatePreviewStatus BuildPreview(
        const UiWebEditorStatePreviewRequest &request,
        UiWebEditorStatePreviewOutput output) const;

private:
    bool IsKnownValueKind(UiWebEditorStatePreviewValueKind value_kind) const;
    bool IsStorageValid(const void *data, std::size_t count) const;
    bool HasStorage(const void *data, std::size_t capacity, std::uint32_t required_count) const;
    bool ContainsNodeId(
        std::span<const yuengine::uicore::UiFileNodeRecord> nodes,
        yuengine::uicore::UiNodeId node_id) const;
    bool IsStateInputValid(
        const yuengine::uicore::UiFileSchemaDesc &schema,
        const UiWebEditorStatePreviewInputRecord &record) const;
    bool HasOutputCapacity(
        const UiWebEditorStatePreviewRequest &request,
        const UiWebEditorStatePreviewOutput &output) const;
    UiWebEditorStatePreviewStatus MapValidationStatus(
        yuengine::ui_web_editor_validator::UiWebEditorValidationStatus status) const;
    void CopyValidationReport(
        const yuengine::ui_web_editor_validator::UiWebEditorValidationReport &report,
        UiWebEditorStatePreviewResult *out_result) const;
    void WriteStateOutputs(
        const UiWebEditorStatePreviewRequest &request,
        UiWebEditorStatePreviewOutput output) const;
};
}
