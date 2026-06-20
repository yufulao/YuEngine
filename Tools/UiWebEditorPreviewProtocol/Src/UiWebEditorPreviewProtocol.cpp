// 模块: Tools UiWebEditorPreviewProtocol
// 文件: Tools/UiWebEditorPreviewProtocol/Src/UiWebEditorPreviewProtocol.cpp

#include "YuEngine/UiWebEditorPreviewProtocol/UiWebEditorPreviewProtocol.h"

namespace yuengine::ui_web_editor_preview_protocol {
UiWebEditorPreviewStatus UiWebEditorPreviewProtocol::BuildHandshakeResponse(
    const UiWebEditorPreviewHandshakeRequest &request,
    UiWebEditorPreviewHandshakeResponse *out_response) const {
    if (out_response == nullptr) {
        return UiWebEditorPreviewStatus::InvalidInput;
    }

    *out_response = UiWebEditorPreviewHandshakeResponse{};
    out_response->protocol_version = UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
    out_response->service_version =
        yuengine::ui_web_editor_service::UI_WEB_EDITOR_LOCAL_SERVICE_VERSION;
    out_response->server_capability_flags = UI_WEB_EDITOR_PREVIEW_CAPABILITY_FLAGS;
    out_response->accepted_capability_flags =
        request.client_capability_flags & UI_WEB_EDITOR_PREVIEW_CAPABILITY_FLAGS;
    if (!IsVersionCompatible(request.client_protocol_version, request.minimum_protocol_version)) {
        out_response->status = UiWebEditorPreviewStatus::UnsupportedVersion;
        out_response->error_kind = UiWebEditorPreviewErrorKind::UnsupportedProtocolVersion;
        return UiWebEditorPreviewStatus::UnsupportedVersion;
    }

    out_response->status = UiWebEditorPreviewStatus::Success;
    out_response->accepted_protocol_version = UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
    return UiWebEditorPreviewStatus::Success;
}

UiWebEditorPreviewStatus UiWebEditorPreviewProtocol::BuildPreviewResponse(
    const UiWebEditorPreviewRequest &request,
    std::span<UiWebEditorPreviewDiagnosticRecord> out_diagnostics,
    UiWebEditorPreviewResponse *out_response) const {
    if (out_response == nullptr) {
        return UiWebEditorPreviewStatus::InvalidInput;
    }

    *out_response = UiWebEditorPreviewResponse{};
    WriteBaseResponse(request, out_response);
    if (!IsSupportedVersion(request.protocol_version)) {
        WriteFailureResponse(
            UiWebEditorPreviewStatus::UnsupportedVersion,
            UiWebEditorPreviewErrorKind::UnsupportedProtocolVersion,
            out_response);
        return UiWebEditorPreviewStatus::UnsupportedVersion;
    }

    if (!IsPreviewMessageKind(request.message_kind)) {
        WriteFailureResponse(
            UiWebEditorPreviewStatus::InvalidMessage,
            UiWebEditorPreviewErrorKind::InvalidMessageKind,
            out_response);
        return UiWebEditorPreviewStatus::InvalidMessage;
    }

    if (!ValidateDocumentPayload(request)) {
        WriteFailureResponse(
            UiWebEditorPreviewStatus::InvalidDocument,
            UiWebEditorPreviewErrorKind::InvalidDocumentPayload,
            out_response);
        return UiWebEditorPreviewStatus::InvalidDocument;
    }

    const std::uint32_t required_diagnostic_count =
        GetRequiredDiagnosticCount(request.message_kind);
    if (out_diagnostics.size() < required_diagnostic_count) {
        WriteFailureResponse(
            UiWebEditorPreviewStatus::OutputCapacityExceeded,
            UiWebEditorPreviewErrorKind::OutputCapacityExceeded,
            out_response);
        out_response->diagnostic_count = required_diagnostic_count;
        return UiWebEditorPreviewStatus::OutputCapacityExceeded;
    }

    if (required_diagnostic_count > 0U) {
        WriteRenderDiagnostics(request, out_diagnostics);
    }

    out_response->status = UiWebEditorPreviewStatus::Success;
    out_response->error_kind = UiWebEditorPreviewErrorKind::None;
    out_response->diagnostic_count = required_diagnostic_count;
    out_response->document_ready = true;
    out_response->diagnostics_ready = request.message_kind == UiWebEditorPreviewMessageKind::RenderDiagnostics;
    return UiWebEditorPreviewStatus::Success;
}

bool UiWebEditorPreviewProtocol::IsVersionCompatible(
    std::uint32_t client_protocol_version,
    std::uint32_t minimum_protocol_version) const {
    if (minimum_protocol_version > UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION) {
        return false;
    }

    return client_protocol_version >= UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
}

bool UiWebEditorPreviewProtocol::IsSupportedVersion(std::uint32_t protocol_version) const {
    return protocol_version == UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
}

bool UiWebEditorPreviewProtocol::IsPreviewMessageKind(
    UiWebEditorPreviewMessageKind message_kind) const {
    if (message_kind == UiWebEditorPreviewMessageKind::LoadDocument) {
        return true;
    }

    if (message_kind == UiWebEditorPreviewMessageKind::UpdateDocument) {
        return true;
    }

    if (message_kind == UiWebEditorPreviewMessageKind::SelectNode) {
        return true;
    }

    return message_kind == UiWebEditorPreviewMessageKind::RenderDiagnostics;
}

bool UiWebEditorPreviewProtocol::ValidateDocumentPayload(
    const UiWebEditorPreviewRequest &request) const {
    if (request.document.document_id == 0U || !request.document.loaded) {
        return false;
    }

    if (request.document.layout_id != request.schema.header.layout_id) {
        return false;
    }

    if (request.document.schema_version != request.schema.header.schema_version) {
        return false;
    }

    if (!request.local_service_result.Succeeded()) {
        return false;
    }

    return true;
}

std::uint32_t UiWebEditorPreviewProtocol::GetRequiredDiagnosticCount(
    UiWebEditorPreviewMessageKind message_kind) const {
    if (message_kind == UiWebEditorPreviewMessageKind::RenderDiagnostics) {
        return 2U;
    }

    return 0U;
}

void UiWebEditorPreviewProtocol::WriteBaseResponse(
    const UiWebEditorPreviewRequest &request,
    UiWebEditorPreviewResponse *out_response) const {
    if (out_response == nullptr) {
        return;
    }

    out_response->message_kind = request.message_kind;
    out_response->request_id = request.request_id;
    out_response->protocol_version = request.protocol_version;
    out_response->document_id = request.document.document_id;
    out_response->layout_id = request.schema.header.layout_id;
    out_response->schema_version = request.schema.header.schema_version;
    out_response->node_count = static_cast<std::uint32_t>(request.schema.nodes.size());
    out_response->resource_ref_count = static_cast<std::uint32_t>(request.schema.resource_refs.size());
    out_response->schema_issue_count = request.local_service_result.issue_count;
    out_response->local_service_status = request.local_service_result.status;
    out_response->schema_status = request.local_service_result.schema_status;
    out_response->selected_node_id = request.selected_node_id;
    out_response->has_selection = request.has_selection;
}

void UiWebEditorPreviewProtocol::WriteFailureResponse(
    UiWebEditorPreviewStatus status,
    UiWebEditorPreviewErrorKind error_kind,
    UiWebEditorPreviewResponse *out_response) const {
    if (out_response == nullptr) {
        return;
    }

    out_response->status = status;
    out_response->error_kind = error_kind;
}

void UiWebEditorPreviewProtocol::WriteRenderDiagnostics(
    const UiWebEditorPreviewRequest &request,
    std::span<UiWebEditorPreviewDiagnosticRecord> out_diagnostics) const {
    if (out_diagnostics.size() < 2U) {
        return;
    }

    UiWebEditorPreviewDiagnosticRecord protocol_record{};
    protocol_record.kind = UiWebEditorPreviewDiagnosticKind::Protocol;
    protocol_record.severity = UiWebEditorPreviewDiagnosticSeverity::Info;
    protocol_record.node_id = request.selected_node_id;
    protocol_record.context_key = request.request_id;
    protocol_record.status_code = request.protocol_version;
    out_diagnostics[0U] = protocol_record;

    UiWebEditorPreviewDiagnosticRecord render_record{};
    render_record.kind = UiWebEditorPreviewDiagnosticKind::Render;
    render_record.severity = UiWebEditorPreviewDiagnosticSeverity::Info;
    render_record.node_id = request.selected_node_id;
    render_record.context_key = request.schema.header.layout_id;
    render_record.status_code = 0U;
    out_diagnostics[1U] = render_record;
}
}
