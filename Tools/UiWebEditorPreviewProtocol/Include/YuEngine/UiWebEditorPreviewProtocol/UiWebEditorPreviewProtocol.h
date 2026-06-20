// 模块: Tools UiWebEditorPreviewProtocol
// 文件: Tools/UiWebEditorPreviewProtocol/Include/YuEngine/UiWebEditorPreviewProtocol/UiWebEditorPreviewProtocol.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"
#include "YuEngine/UiWebEditorService/UiWebEditorLocalService.h"
#include "YuEngine/UiWebEditorShell/UiWebEditorShell.h"

namespace yuengine::ui_web_editor_preview_protocol {
constexpr std::uint32_t UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION = 1U;
constexpr std::uint32_t UI_WEB_EDITOR_PREVIEW_CAPABILITY_LOAD_DOCUMENT = 1U << 0U;
constexpr std::uint32_t UI_WEB_EDITOR_PREVIEW_CAPABILITY_UPDATE_DOCUMENT = 1U << 1U;
constexpr std::uint32_t UI_WEB_EDITOR_PREVIEW_CAPABILITY_SELECT_NODE = 1U << 2U;
constexpr std::uint32_t UI_WEB_EDITOR_PREVIEW_CAPABILITY_RENDER_DIAGNOSTICS = 1U << 3U;
constexpr std::uint32_t UI_WEB_EDITOR_PREVIEW_CAPABILITY_FLAGS =
    UI_WEB_EDITOR_PREVIEW_CAPABILITY_LOAD_DOCUMENT |
    UI_WEB_EDITOR_PREVIEW_CAPABILITY_UPDATE_DOCUMENT |
    UI_WEB_EDITOR_PREVIEW_CAPABILITY_SELECT_NODE |
    UI_WEB_EDITOR_PREVIEW_CAPABILITY_RENDER_DIAGNOSTICS;

enum class UiWebEditorPreviewMessageKind {
    Invalid = 0,
    LoadDocument,
    UpdateDocument,
    SelectNode,
    RenderDiagnostics
};

enum class UiWebEditorPreviewStatus {
    Success = 0,
    InvalidInput,
    UnsupportedVersion,
    InvalidMessage,
    InvalidDocument,
    OutputCapacityExceeded
};

enum class UiWebEditorPreviewErrorKind {
    None = 0,
    UnsupportedProtocolVersion,
    InvalidMessageKind,
    InvalidDocumentPayload,
    OutputCapacityExceeded
};

enum class UiWebEditorPreviewDiagnosticKind {
    None = 0,
    Protocol,
    Schema,
    Shell,
    Render
};

enum class UiWebEditorPreviewDiagnosticSeverity {
    Info = 0,
    Warning,
    Error
};

struct UiWebEditorPreviewHandshakeRequest final {
    std::uint32_t client_protocol_version = UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
    std::uint32_t minimum_protocol_version = UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
    std::uint32_t client_capability_flags = 0U;
};

struct UiWebEditorPreviewHandshakeResponse final {
    UiWebEditorPreviewStatus status = UiWebEditorPreviewStatus::Success;
    UiWebEditorPreviewErrorKind error_kind = UiWebEditorPreviewErrorKind::None;
    std::uint32_t protocol_version = UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
    std::uint32_t accepted_protocol_version = 0U;
    std::uint32_t service_version = 0U;
    std::uint32_t shell_version = 0U;
    std::uint32_t server_capability_flags = 0U;
    std::uint32_t accepted_capability_flags = 0U;

    /**
     * @comment 检查 preview protocol handshake 是否成功。
     * @return handshake 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorPreviewStatus::Success;
    }
};

struct UiWebEditorPreviewRequest final {
    UiWebEditorPreviewMessageKind message_kind = UiWebEditorPreviewMessageKind::Invalid;
    std::uint32_t request_id = 0U;
    std::uint32_t protocol_version = UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
    yuengine::uicore::UiFileSchemaDesc schema;
    yuengine::ui_web_editor_service::UiWebEditorLocalDocumentRecord document;
    yuengine::ui_web_editor_service::UiWebEditorLocalServiceResult local_service_result;
    yuengine::ui_web_editor_shell::UiWebEditorShellSnapshot shell_snapshot;
    yuengine::uicore::UiNodeId selected_node_id;
    bool has_selection = false;
};

struct UiWebEditorPreviewResponse final {
    UiWebEditorPreviewStatus status = UiWebEditorPreviewStatus::Success;
    UiWebEditorPreviewErrorKind error_kind = UiWebEditorPreviewErrorKind::None;
    UiWebEditorPreviewMessageKind message_kind = UiWebEditorPreviewMessageKind::Invalid;
    yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus local_service_status =
        yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus::Success;
    yuengine::uicore::UiFileSchemaStatus schema_status = yuengine::uicore::UiFileSchemaStatus::Success;
    yuengine::ui_web_editor_shell::UiWebEditorShellStatus shell_status =
        yuengine::ui_web_editor_shell::UiWebEditorShellStatus::Success;
    std::uint32_t request_id = 0U;
    std::uint32_t protocol_version = UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
    std::uint32_t document_id = 0U;
    std::uint32_t layout_id = 0U;
    std::uint32_t schema_version = 0U;
    std::uint32_t node_count = 0U;
    std::uint32_t resource_ref_count = 0U;
    std::uint32_t schema_issue_count = 0U;
    std::uint32_t diagnostic_count = 0U;
    yuengine::uicore::UiNodeId selected_node_id;
    bool has_selection = false;
    bool document_ready = false;
    bool shell_snapshot_ready = false;
    bool diagnostics_ready = false;

    /**
     * @comment 检查 preview protocol response 是否成功。
     * @return response 成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorPreviewStatus::Success;
    }
};

struct UiWebEditorPreviewDiagnosticRecord final {
    UiWebEditorPreviewDiagnosticKind kind = UiWebEditorPreviewDiagnosticKind::None;
    UiWebEditorPreviewDiagnosticSeverity severity = UiWebEditorPreviewDiagnosticSeverity::Info;
    yuengine::uicore::UiNodeId node_id;
    std::uint32_t context_key = 0U;
    std::uint32_t status_code = 0U;
};

class UiWebEditorPreviewProtocol final {
public:
    /**
     * @comment 构建 runtime preview protocol handshake response。
     * @param request handshake 输入。
     * @param out_response 输出 handshake response。
     * @return 显式 protocol 状态。
     */
    UiWebEditorPreviewStatus BuildHandshakeResponse(
        const UiWebEditorPreviewHandshakeRequest &request,
        UiWebEditorPreviewHandshakeResponse *out_response) const;

    /**
     * @comment 基于 generic UI schema、local service result 和 Web editor shell snapshot 构建 preview response。
     * @param request preview message 输入。
     * @param out_diagnostics 调用方持有的 diagnostic 输出。
     * @param out_response 输出 preview response。
     * @return 显式 protocol 状态。
     */
    UiWebEditorPreviewStatus BuildPreviewResponse(
        const UiWebEditorPreviewRequest &request,
        std::span<UiWebEditorPreviewDiagnosticRecord> out_diagnostics,
        UiWebEditorPreviewResponse *out_response) const;

private:
    bool IsVersionCompatible(std::uint32_t client_protocol_version, std::uint32_t minimum_protocol_version) const;
    bool IsSupportedVersion(std::uint32_t protocol_version) const;
    bool IsPreviewMessageKind(UiWebEditorPreviewMessageKind message_kind) const;
    bool ValidateDocumentPayload(const UiWebEditorPreviewRequest &request) const;
    std::uint32_t GetRequiredDiagnosticCount(UiWebEditorPreviewMessageKind message_kind) const;
    void WriteBaseResponse(
        const UiWebEditorPreviewRequest &request,
        UiWebEditorPreviewResponse *out_response) const;
    void WriteFailureResponse(
        UiWebEditorPreviewStatus status,
        UiWebEditorPreviewErrorKind error_kind,
        UiWebEditorPreviewResponse *out_response) const;
    void WriteRenderDiagnostics(
        const UiWebEditorPreviewRequest &request,
        std::span<UiWebEditorPreviewDiagnosticRecord> out_diagnostics) const;
};
}
