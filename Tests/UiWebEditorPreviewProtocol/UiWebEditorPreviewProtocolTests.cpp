// 模块: Tests UiWebEditorPreviewProtocol
// 文件: Tests/UiWebEditorPreviewProtocol/UiWebEditorPreviewProtocolTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"
#include "YuEngine/UiCore/UiLayoutContainerType.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiCore/UiStackDirection.h"
#include "YuEngine/UiWebEditorPreviewProtocol/UiWebEditorPreviewProtocol.h"
#include "YuEngine/UiWebEditorService/UiWebEditorLocalService.h"

using yuengine::uicore::UiFileEventBinding;
using yuengine::uicore::UiFileLayoutRecord;
using yuengine::uicore::UiFileNodeRecord;
using yuengine::uicore::UiFileResourceKind;
using yuengine::uicore::UiFileResourceRef;
using yuengine::uicore::UiFileSchemaDesc;
using yuengine::uicore::UiFileSchemaHeader;
using yuengine::uicore::UiFileSchemaIssueRecord;
using yuengine::uicore::UiFileStyleRef;
using yuengine::uicore::UiLayoutContainerType;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiRectTransform;
using yuengine::uicore::UiStackDirection;
using yuengine::uicore::UI_FILE_SCHEMA_ID;
using yuengine::uicore::UI_FILE_SCHEMA_VERSION;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewDiagnosticKind;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewDiagnosticRecord;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewDiagnosticSeverity;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewErrorKind;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewHandshakeRequest;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewHandshakeResponse;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewMessageKind;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewProtocol;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewRequest;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewResponse;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewStatus;
using yuengine::ui_web_editor_preview_protocol::UI_WEB_EDITOR_PREVIEW_CAPABILITY_LOAD_DOCUMENT;
using yuengine::ui_web_editor_preview_protocol::UI_WEB_EDITOR_PREVIEW_CAPABILITY_RENDER_DIAGNOSTICS;
using yuengine::ui_web_editor_preview_protocol::UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
using yuengine::ui_web_editor_service::UiWebEditorLoadRequest;
using yuengine::ui_web_editor_service::UiWebEditorLocalDocumentRecord;
using yuengine::ui_web_editor_service::UiWebEditorLocalService;
using yuengine::ui_web_editor_service::UiWebEditorLocalServiceResult;
using yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus;
using yuengine::ui_web_editor_service::UI_WEB_EDITOR_LOCAL_SERVICE_VERSION;

namespace {
constexpr const char *TEST_HANDSHAKE =
    "UiWebEditorPreviewProtocol_Protocol_BuildsHandshake";
constexpr const char *TEST_LOAD =
    "UiWebEditorPreviewProtocol_Protocol_BuildsLoadResponse";
constexpr const char *TEST_SELECT =
    "UiWebEditorPreviewProtocol_Protocol_BuildsSelectResponse";
constexpr const char *TEST_RENDER_DIAGNOSTICS =
    "UiWebEditorPreviewProtocol_Protocol_WritesRenderDiagnostics";
constexpr const char *TEST_VERSION_MISMATCH =
    "UiWebEditorPreviewProtocol_Protocol_RejectsVersionMismatch";
constexpr const char *TEST_SMALL_DIAGNOSTICS =
    "UiWebEditorPreviewProtocol_Protocol_RejectsSmallDiagnosticOutputWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_NODE_ID = 9001U;
constexpr std::uint32_t SENTINEL_CONTEXT_KEY = 9002U;
constexpr std::uint32_t SENTINEL_STATUS_CODE = 9003U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

UiRectTransform Transform(float anchor_max_x) {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {anchor_max_x, 1.0F};
    transform.pivot = {0.5F, 0.5F};
    transform.dpi_scale = 1.0F;
    return transform;
}

UiFileSchemaHeader Header(std::uint32_t layout_id, std::uint32_t root_node_id) {
    UiFileSchemaHeader header{};
    header.schema_id = UI_FILE_SCHEMA_ID;
    header.schema_version = UI_FILE_SCHEMA_VERSION;
    header.layout_id = layout_id;
    header.root_node_id = NodeId(root_node_id);
    return header;
}

UiFileNodeRecord Node(
    std::uint32_t node_id,
    std::uint32_t parent_id,
    std::uint32_t order,
    float anchor_max_x) {
    UiFileNodeRecord record{};
    record.node_id = NodeId(node_id);
    record.parent_id = NodeId(parent_id);
    record.rect_transform = Transform(anchor_max_x);
    record.sibling_order = order;
    record.layer = static_cast<std::int32_t>(order);
    return record;
}

UiFileNodeRecord RootNode(std::uint32_t node_id) {
    UiFileNodeRecord record = Node(node_id, 0U, 0U, 1.0F);
    record.parent_id = UiNodeId{};
    return record;
}

UiFileLayoutRecord Layout(std::uint32_t container_id) {
    UiFileLayoutRecord record{};
    record.container.container_id = NodeId(container_id);
    record.container.type = UiLayoutContainerType::Stack;
    record.container.stack_direction = UiStackDirection::Vertical;
    record.container.grid_column_count = 1U;
    return record;
}

UiFileStyleRef StyleRef(std::uint32_t node_id, std::uint32_t style_key) {
    UiFileStyleRef record{};
    record.node_id = NodeId(node_id);
    record.style_key = style_key;
    return record;
}

UiFileResourceRef ResourceRef(
    std::uint32_t node_id,
    UiFileResourceKind kind,
    std::uint32_t resource_key) {
    UiFileResourceRef record{};
    record.node_id = NodeId(node_id);
    record.kind = kind;
    record.resource_key = resource_key;
    return record;
}

UiFileEventBinding EventBinding(
    std::uint32_t node_id,
    std::uint32_t binding_key,
    std::uint32_t event_key) {
    UiFileEventBinding record{};
    record.node_id = NodeId(node_id);
    record.binding_key = binding_key;
    record.event_key = event_key;
    return record;
}

UiWebEditorPreviewDiagnosticRecord SentinelDiagnostic() {
    UiWebEditorPreviewDiagnosticRecord record{};
    record.kind = UiWebEditorPreviewDiagnosticKind::Schema;
    record.severity = UiWebEditorPreviewDiagnosticSeverity::Error;
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.context_key = SENTINEL_CONTEXT_KEY;
    record.status_code = SENTINEL_STATUS_CODE;
    return record;
}

bool DiagnosticMatchesSentinel(const UiWebEditorPreviewDiagnosticRecord &record) {
    if (record.kind != UiWebEditorPreviewDiagnosticKind::Schema) {
        return false;
    }

    if (record.severity != UiWebEditorPreviewDiagnosticSeverity::Error) {
        return false;
    }

    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    if (record.context_key != SENTINEL_CONTEXT_KEY) {
        return false;
    }

    return record.status_code == SENTINEL_STATUS_CODE;
}

struct ValidSchemaFixture final {
    std::array<UiFileNodeRecord, 2U> nodes{
        RootNode(1U),
        Node(2U, 1U, 0U, 0.5F)};
    std::array<UiFileLayoutRecord, 1U> layouts{
        Layout(1U)};
    std::array<UiFileStyleRef, 2U> style_refs{
        StyleRef(1U, 11U),
        StyleRef(2U, 12U)};
    std::array<UiFileResourceRef, 1U> resource_refs{
        ResourceRef(2U, UiFileResourceKind::Sprite, 21U)};
    std::array<UiFileEventBinding, 1U> event_bindings{
        EventBinding(2U, 31U, 41U)};
};

UiFileSchemaDesc MakeValidSchema(ValidSchemaFixture *fixture, std::uint32_t layout_id) {
    UiFileSchemaDesc desc{};
    desc.header = Header(layout_id, 1U);
    desc.nodes = fixture->nodes;
    desc.layouts = fixture->layouts;
    desc.style_refs = fixture->style_refs;
    desc.resource_refs = fixture->resource_refs;
    desc.event_bindings = fixture->event_bindings;
    return desc;
}

bool LoadDocument(
    const UiFileSchemaDesc &schema,
    std::uint32_t document_id,
    UiWebEditorLocalDocumentRecord *out_document,
    UiWebEditorLocalServiceResult *out_result) {
    UiWebEditorLoadRequest request{};
    request.document_id = document_id;
    request.schema = schema;

    std::array<UiFileSchemaIssueRecord, 1U> issues{};
    const UiWebEditorLocalService service{};
    const UiWebEditorLocalServiceStatus status =
        service.LoadUiFile(request, out_document, issues, out_result);
    return status == UiWebEditorLocalServiceStatus::Success;
}

bool BuildPreviewRequest(
    ValidSchemaFixture *fixture,
    UiWebEditorPreviewMessageKind message_kind,
    std::uint32_t request_id,
    std::uint32_t selected_node_id,
    bool has_selection,
    UiWebEditorPreviewRequest *out_request) {
    if (out_request == nullptr) {
        return false;
    }

    UiFileSchemaDesc schema = MakeValidSchema(fixture, 8001U);
    UiWebEditorLocalDocumentRecord document{};
    UiWebEditorLocalServiceResult local_result{};
    if (!LoadDocument(schema, 2001U, &document, &local_result)) {
        return false;
    }

    UiWebEditorPreviewRequest request{};
    request.message_kind = message_kind;
    request.request_id = request_id;
    request.protocol_version = UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION;
    request.schema = schema;
    request.document = document;
    request.local_service_result = local_result;
    request.selected_node_id = NodeId(selected_node_id);
    request.has_selection = has_selection;
    *out_request = request;
    return true;
}

int UiWebEditorPreviewProtocolProtocolBuildsHandshake() {
    UiWebEditorPreviewHandshakeRequest request{};
    request.client_capability_flags =
        UI_WEB_EDITOR_PREVIEW_CAPABILITY_LOAD_DOCUMENT |
        UI_WEB_EDITOR_PREVIEW_CAPABILITY_RENDER_DIAGNOSTICS;

    UiWebEditorPreviewHandshakeResponse response{};
    const UiWebEditorPreviewProtocol protocol{};
    const UiWebEditorPreviewStatus status = protocol.BuildHandshakeResponse(request, &response);
    if (status != UiWebEditorPreviewStatus::Success || !response.Succeeded()) {
        return Fail("preview protocol handshake did not succeed");
    }

    if (response.accepted_protocol_version != UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION ||
        response.service_version != UI_WEB_EDITOR_LOCAL_SERVICE_VERSION) {
        return Fail("preview protocol handshake version mismatch");
    }

    if ((response.accepted_capability_flags & UI_WEB_EDITOR_PREVIEW_CAPABILITY_LOAD_DOCUMENT) == 0U ||
        (response.accepted_capability_flags & UI_WEB_EDITOR_PREVIEW_CAPABILITY_RENDER_DIAGNOSTICS) == 0U) {
        return Fail("preview protocol handshake capability mismatch");
    }

    return 0;
}

int UiWebEditorPreviewProtocolProtocolBuildsLoadResponse() {
    ValidSchemaFixture fixture{};
    UiWebEditorPreviewRequest request{};
    if (!BuildPreviewRequest(
        &fixture,
        UiWebEditorPreviewMessageKind::LoadDocument,
        301U,
        0U,
        false,
        &request)) {
        return Fail("preview protocol load fixture did not build");
    }

    UiWebEditorPreviewResponse response{};
    const UiWebEditorPreviewProtocol protocol{};
    const UiWebEditorPreviewStatus status = protocol.BuildPreviewResponse(request, {}, &response);
    if (status != UiWebEditorPreviewStatus::Success || !response.Succeeded()) {
        return Fail("preview protocol load response did not succeed");
    }

    if (response.message_kind != UiWebEditorPreviewMessageKind::LoadDocument ||
        response.request_id != 301U ||
        response.document_id != 2001U ||
        response.layout_id != 8001U) {
        return Fail("preview protocol load response identity mismatch");
    }

    if (response.node_count != 2U ||
        response.resource_ref_count != 1U ||
        response.schema_issue_count != 0U) {
        return Fail("preview protocol load response counts mismatch");
    }

    if (!response.document_ready || response.diagnostics_ready) {
        return Fail("preview protocol load response readiness mismatch");
    }

    return 0;
}

int UiWebEditorPreviewProtocolProtocolBuildsSelectResponse() {
    ValidSchemaFixture fixture{};
    UiWebEditorPreviewRequest request{};
    if (!BuildPreviewRequest(
        &fixture,
        UiWebEditorPreviewMessageKind::SelectNode,
        302U,
        2U,
        true,
        &request)) {
        return Fail("preview protocol select fixture did not build");
    }

    UiWebEditorPreviewResponse response{};
    const UiWebEditorPreviewProtocol protocol{};
    const UiWebEditorPreviewStatus status = protocol.BuildPreviewResponse(request, {}, &response);
    if (status != UiWebEditorPreviewStatus::Success) {
        return Fail("preview protocol select response did not succeed");
    }

    if (!response.has_selection || response.selected_node_id.value != 2U) {
        return Fail("preview protocol select response selection mismatch");
    }

    if (response.local_service_status != UiWebEditorLocalServiceStatus::Success) {
        return Fail("preview protocol select status mismatch");
    }

    return 0;
}

int UiWebEditorPreviewProtocolProtocolWritesRenderDiagnostics() {
    ValidSchemaFixture fixture{};
    UiWebEditorPreviewRequest request{};
    if (!BuildPreviewRequest(
        &fixture,
        UiWebEditorPreviewMessageKind::RenderDiagnostics,
        303U,
        2U,
        true,
        &request)) {
        return Fail("preview protocol diagnostic fixture did not build");
    }

    std::array<UiWebEditorPreviewDiagnosticRecord, 2U> diagnostics{};
    UiWebEditorPreviewResponse response{};
    const UiWebEditorPreviewProtocol protocol{};
    const UiWebEditorPreviewStatus status =
        protocol.BuildPreviewResponse(request, diagnostics, &response);
    if (status != UiWebEditorPreviewStatus::Success || response.diagnostic_count != 2U) {
        return Fail("preview protocol render diagnostics did not succeed");
    }

    if (diagnostics[0U].kind != UiWebEditorPreviewDiagnosticKind::Protocol ||
        diagnostics[1U].kind != UiWebEditorPreviewDiagnosticKind::Render) {
        return Fail("preview protocol render diagnostic kind mismatch");
    }

    if (diagnostics[0U].context_key != 303U ||
        diagnostics[1U].context_key != 8001U) {
        return Fail("preview protocol render diagnostic context mismatch");
    }

    return 0;
}

int UiWebEditorPreviewProtocolProtocolRejectsVersionMismatch() {
    ValidSchemaFixture fixture{};
    UiWebEditorPreviewRequest request{};
    if (!BuildPreviewRequest(
        &fixture,
        UiWebEditorPreviewMessageKind::UpdateDocument,
        304U,
        0U,
        false,
        &request)) {
        return Fail("preview protocol version fixture did not build");
    }

    request.protocol_version = UI_WEB_EDITOR_PREVIEW_PROTOCOL_VERSION + 1U;
    UiWebEditorPreviewResponse response{};
    const UiWebEditorPreviewProtocol protocol{};
    const UiWebEditorPreviewStatus status = protocol.BuildPreviewResponse(request, {}, &response);
    if (status != UiWebEditorPreviewStatus::UnsupportedVersion) {
        return Fail("preview protocol version mismatch was not rejected");
    }

    if (response.error_kind != UiWebEditorPreviewErrorKind::UnsupportedProtocolVersion ||
        response.request_id != 304U) {
        return Fail("preview protocol version mismatch response mismatch");
    }

    return 0;
}

int UiWebEditorPreviewProtocolProtocolRejectsSmallDiagnosticOutputWithoutMutation() {
    ValidSchemaFixture fixture{};
    UiWebEditorPreviewRequest request{};
    if (!BuildPreviewRequest(
        &fixture,
        UiWebEditorPreviewMessageKind::RenderDiagnostics,
        305U,
        2U,
        true,
        &request)) {
        return Fail("preview protocol small diagnostic fixture did not build");
    }

    std::array<UiWebEditorPreviewDiagnosticRecord, 1U> diagnostics{
        SentinelDiagnostic()};
    UiWebEditorPreviewResponse response{};
    const UiWebEditorPreviewProtocol protocol{};
    const UiWebEditorPreviewStatus status =
        protocol.BuildPreviewResponse(request, diagnostics, &response);
    if (status != UiWebEditorPreviewStatus::OutputCapacityExceeded) {
        return Fail("preview protocol small diagnostic output was not rejected");
    }

    if (response.diagnostic_count != 2U ||
        response.error_kind != UiWebEditorPreviewErrorKind::OutputCapacityExceeded) {
        return Fail("preview protocol small diagnostic response mismatch");
    }

    if (!DiagnosticMatchesSentinel(diagnostics[0U])) {
        return Fail("preview protocol small diagnostic output mutated diagnostics");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_HANDSHAKE) {
        return UiWebEditorPreviewProtocolProtocolBuildsHandshake();
    }

    if (name == TEST_LOAD) {
        return UiWebEditorPreviewProtocolProtocolBuildsLoadResponse();
    }

    if (name == TEST_SELECT) {
        return UiWebEditorPreviewProtocolProtocolBuildsSelectResponse();
    }

    if (name == TEST_RENDER_DIAGNOSTICS) {
        return UiWebEditorPreviewProtocolProtocolWritesRenderDiagnostics();
    }

    if (name == TEST_VERSION_MISMATCH) {
        return UiWebEditorPreviewProtocolProtocolRejectsVersionMismatch();
    }

    if (name == TEST_SMALL_DIAGNOSTICS) {
        return UiWebEditorPreviewProtocolProtocolRejectsSmallDiagnosticOutputWithoutMutation();
    }

    return Fail(ERROR_UNKNOWN_TEST_NAME);
}
}

int main(int argc, char **argv) {
    if (argc != 2) {
        return Fail(ERROR_EXPECTED_ONE_TEST_NAME);
    }

    return RunNamedTest(argv[1]);
}
