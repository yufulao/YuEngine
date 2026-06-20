// 模块: Tests UiWebEditorService
// 文件: Tests/UiWebEditorService/UiWebEditorLocalServiceTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"
#include "YuEngine/UiCore/UiLayoutContainerType.h"
#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiRectTransform.h"
#include "YuEngine/UiCore/UiStackDirection.h"
#include "YuEngine/UiWebEditorService/UiWebEditorLocalService.h"

using yuengine::uicore::UiFileEventBinding;
using yuengine::uicore::UiFileLayoutRecord;
using yuengine::uicore::UiFileNodeRecord;
using yuengine::uicore::UiFileResourceKind;
using yuengine::uicore::UiFileResourceRef;
using yuengine::uicore::UiFileSchemaDesc;
using yuengine::uicore::UiFileSchemaHeader;
using yuengine::uicore::UiFileSchemaIssueKind;
using yuengine::uicore::UiFileSchemaIssueRecord;
using yuengine::uicore::UiFileSchemaStatus;
using yuengine::uicore::UiFileStyleRef;
using yuengine::uicore::UiLayoutContainerType;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UiRectTransform;
using yuengine::uicore::UiStackDirection;
using yuengine::uicore::UI_FILE_SCHEMA_ID;
using yuengine::uicore::UI_FILE_SCHEMA_VERSION;
using yuengine::ui_web_editor_service::UiWebEditorLoadRequest;
using yuengine::ui_web_editor_service::UiWebEditorLocalDocumentRecord;
using yuengine::ui_web_editor_service::UiWebEditorLocalService;
using yuengine::ui_web_editor_service::UiWebEditorLocalServiceResult;
using yuengine::ui_web_editor_service::UiWebEditorLocalServiceStatus;
using yuengine::ui_web_editor_service::UiWebEditorSaveRequest;
using yuengine::ui_web_editor_service::UiWebEditorValidateRequest;
using yuengine::ui_web_editor_service::UI_WEB_EDITOR_LOCAL_SERVICE_VERSION;

namespace {
constexpr const char *TEST_LOAD =
    "UiWebEditorService_LocalService_LoadValidatesAndCreatesDocument";
constexpr const char *TEST_VALIDATE_ISSUES =
    "UiWebEditorService_LocalService_ValidateReportsSchemaIssues";
constexpr const char *TEST_SAVE =
    "UiWebEditorService_LocalService_SaveValidatesGenericPayload";
constexpr const char *TEST_INVALID_DOCUMENT =
    "UiWebEditorService_LocalService_RejectsInvalidDocumentWithoutMutation";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiWebEditorService_LocalService_RejectsSmallIssueOutputWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_DOCUMENT_ID = 777U;
constexpr std::uint32_t SENTINEL_LAYOUT_ID = 888U;
constexpr std::uint32_t SENTINEL_NODE_ID = 999U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

UiRectTransform DefaultTransform() {
    UiRectTransform transform{};
    transform.anchor_min = {0.0F, 0.0F};
    transform.anchor_max = {1.0F, 1.0F};
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

UiFileNodeRecord Node(std::uint32_t node_id, std::uint32_t parent_id, std::uint32_t order) {
    UiFileNodeRecord record{};
    record.node_id = NodeId(node_id);
    record.parent_id = NodeId(parent_id);
    record.rect_transform = DefaultTransform();
    record.sibling_order = order;
    return record;
}

UiFileNodeRecord RootNode(std::uint32_t node_id) {
    UiFileNodeRecord record = Node(node_id, 0U, 0U);
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

UiFileSchemaIssueRecord SentinelIssue() {
    UiFileSchemaIssueRecord record{};
    record.issue_kind = UiFileSchemaIssueKind::MissingRootNode;
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.context_key = SENTINEL_LAYOUT_ID;
    record.record_index = 7U;
    record.duplicate_count = 3U;
    return record;
}

UiWebEditorLocalDocumentRecord SentinelDocument() {
    UiWebEditorLocalDocumentRecord document{};
    document.document_id = SENTINEL_DOCUMENT_ID;
    document.layout_id = SENTINEL_LAYOUT_ID;
    document.node_count = 9U;
    document.loaded = true;
    document.dirty = true;
    return document;
}

bool IssueMatchesSentinel(const UiFileSchemaIssueRecord &record) {
    if (record.issue_kind != UiFileSchemaIssueKind::MissingRootNode) {
        return false;
    }

    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    return record.context_key == SENTINEL_LAYOUT_ID;
}

bool DocumentMatchesSentinel(const UiWebEditorLocalDocumentRecord &document) {
    if (document.document_id != SENTINEL_DOCUMENT_ID) {
        return false;
    }

    if (document.layout_id != SENTINEL_LAYOUT_ID || document.node_count != 9U) {
        return false;
    }

    return document.loaded && document.dirty;
}

struct ValidSchemaFixture final {
    std::array<UiFileNodeRecord, 2U> nodes{
        RootNode(1U),
        Node(2U, 1U, 0U)};
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

int UiWebEditorServiceLocalServiceLoadValidatesAndCreatesDocument() {
    ValidSchemaFixture fixture{};
    UiWebEditorLoadRequest request{};
    request.document_id = 501U;
    request.schema = MakeValidSchema(&fixture, 9001U);

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiWebEditorLocalDocumentRecord document{};
    UiWebEditorLocalServiceResult result{};
    const UiWebEditorLocalService service{};
    const UiWebEditorLocalServiceStatus status =
        service.LoadUiFile(request, &document, issues, &result);
    if (status != UiWebEditorLocalServiceStatus::Success || !result.Succeeded()) {
        return Fail("local service load did not succeed");
    }

    if (document.document_id != 501U ||
        document.service_version != UI_WEB_EDITOR_LOCAL_SERVICE_VERSION ||
        document.schema_version != UI_FILE_SCHEMA_VERSION ||
        document.layout_id != 9001U) {
        return Fail("local service load document identity mismatch");
    }

    if (document.node_count != 2U ||
        document.layout_count != 1U ||
        document.style_ref_count != 2U ||
        document.resource_ref_count != 1U ||
        document.event_binding_count != 1U) {
        return Fail("local service load document counts mismatch");
    }

    if (!document.loaded || document.dirty) {
        return Fail("local service load document state mismatch");
    }

    if (result.checked_node_count != 2U || result.issue_count != 0U) {
        return Fail("local service load result counts mismatch");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("local service load mutated issue buffer");
    }

    return 0;
}

int UiWebEditorServiceLocalServiceValidateReportsSchemaIssues() {
    std::array<UiFileNodeRecord, 2U> nodes{
        RootNode(1U),
        Node(1U, 99U, 0U)};
    UiFileSchemaDesc schema{};
    schema.header = Header(9002U, 8U);
    schema.nodes = nodes;

    UiWebEditorValidateRequest request{};
    request.schema = schema;

    std::array<UiFileSchemaIssueRecord, 3U> issues{};
    UiWebEditorLocalServiceResult result{};
    const UiWebEditorLocalService service{};
    const UiWebEditorLocalServiceStatus status =
        service.ValidateUiFile(request, issues, &result);
    if (status != UiWebEditorLocalServiceStatus::IssuesFound || result.Succeeded()) {
        return Fail("local service validate did not report schema issues");
    }

    if (result.schema_status != UiFileSchemaStatus::IssuesFound || result.issue_count != 3U) {
        return Fail("local service validate issue count mismatch");
    }

    if (issues[0U].issue_kind != UiFileSchemaIssueKind::MissingRootNode ||
        issues[1U].issue_kind != UiFileSchemaIssueKind::DuplicateNodeId ||
        issues[2U].issue_kind != UiFileSchemaIssueKind::MissingParentNode) {
        return Fail("local service validate issue order mismatch");
    }

    return 0;
}

int UiWebEditorServiceLocalServiceSaveValidatesGenericPayload() {
    ValidSchemaFixture fixture{};
    UiWebEditorSaveRequest request{};
    request.document.document_id = 601U;
    request.document.loaded = true;
    request.document.dirty = true;
    request.schema = MakeValidSchema(&fixture, 9003U);

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiWebEditorLocalDocumentRecord document{};
    UiWebEditorLocalServiceResult result{};
    const UiWebEditorLocalService service{};
    const UiWebEditorLocalServiceStatus status =
        service.SaveUiFile(request, &document, issues, &result);
    if (status != UiWebEditorLocalServiceStatus::Success || !result.Succeeded()) {
        return Fail("local service save did not succeed");
    }

    if (document.document_id != 601U || document.layout_id != 9003U) {
        return Fail("local service save document identity mismatch");
    }

    if (!document.loaded || document.dirty) {
        return Fail("local service save state mismatch");
    }

    if (result.checked_node_count != 2U || result.checked_resource_ref_count != 1U) {
        return Fail("local service save result counts mismatch");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("local service save mutated issue buffer");
    }

    return 0;
}

int UiWebEditorServiceLocalServiceRejectsInvalidDocumentWithoutMutation() {
    ValidSchemaFixture fixture{};
    UiWebEditorSaveRequest request{};
    request.document.document_id = 0U;
    request.document.loaded = false;
    request.schema = MakeValidSchema(&fixture, 9004U);

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiWebEditorLocalDocumentRecord document = SentinelDocument();
    UiWebEditorLocalServiceResult result{};
    const UiWebEditorLocalService service{};
    const UiWebEditorLocalServiceStatus status =
        service.SaveUiFile(request, &document, issues, &result);
    if (status != UiWebEditorLocalServiceStatus::InvalidDocument) {
        return Fail("local service invalid document was not rejected");
    }

    if (!DocumentMatchesSentinel(document)) {
        return Fail("local service invalid document mutated output document");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("local service invalid document mutated issue buffer");
    }

    return 0;
}

int UiWebEditorServiceLocalServiceRejectsSmallIssueOutputWithoutMutation() {
    std::array<UiFileNodeRecord, 2U> nodes{
        RootNode(1U),
        Node(1U, 99U, 0U)};
    UiFileSchemaDesc schema{};
    schema.header = Header(9005U, 8U);
    schema.nodes = nodes;

    UiWebEditorValidateRequest request{};
    request.schema = schema;

    std::array<UiFileSchemaIssueRecord, 1U> issues{SentinelIssue()};
    UiWebEditorLocalServiceResult result{};
    const UiWebEditorLocalService service{};
    const UiWebEditorLocalServiceStatus status =
        service.ValidateUiFile(request, issues, &result);
    if (status != UiWebEditorLocalServiceStatus::OutputCapacityExceeded) {
        return Fail("local service small issue output was not rejected");
    }

    if (result.schema_status != UiFileSchemaStatus::OutputCapacityExceeded || result.issue_count != 3U) {
        return Fail("local service small issue output count mismatch");
    }

    if (!IssueMatchesSentinel(issues[0U])) {
        return Fail("local service small issue output mutated issue buffer");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_LOAD) {
        return UiWebEditorServiceLocalServiceLoadValidatesAndCreatesDocument();
    }

    if (name == TEST_VALIDATE_ISSUES) {
        return UiWebEditorServiceLocalServiceValidateReportsSchemaIssues();
    }

    if (name == TEST_SAVE) {
        return UiWebEditorServiceLocalServiceSaveValidatesGenericPayload();
    }

    if (name == TEST_INVALID_DOCUMENT) {
        return UiWebEditorServiceLocalServiceRejectsInvalidDocumentWithoutMutation();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiWebEditorServiceLocalServiceRejectsSmallIssueOutputWithoutMutation();
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
