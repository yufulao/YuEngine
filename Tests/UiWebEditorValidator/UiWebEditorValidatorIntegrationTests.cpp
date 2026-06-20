// 模块: Tests UiWebEditorValidator
// 文件: Tests/UiWebEditorValidator/UiWebEditorValidatorIntegrationTests.cpp

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
#include "YuEngine/UiWebEditorValidator/UiWebEditorValidatorIntegration.h"

using yuengine::uicore::UiFileEventBinding;
using yuengine::uicore::UiFileLayoutRecord;
using yuengine::uicore::UiFileNodeRecord;
using yuengine::uicore::UiFileResourceKind;
using yuengine::uicore::UiFileResourceRef;
using yuengine::uicore::UiFileSchemaDesc;
using yuengine::uicore::UiFileSchemaHeader;
using yuengine::uicore::UiFileSchemaIssueKind;
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
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewMessageKind;
using yuengine::ui_web_editor_shell::UiWebEditorCanvasItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorHierarchyItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorInspectorRecord;
using yuengine::ui_web_editor_shell::UiWebEditorResourceItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorShellStatus;
using yuengine::ui_web_editor_validator::UiWebEditorValidationIssueRecord;
using yuengine::ui_web_editor_validator::UiWebEditorValidationIssueSource;
using yuengine::ui_web_editor_validator::UiWebEditorValidationOutput;
using yuengine::ui_web_editor_validator::UiWebEditorValidationReport;
using yuengine::ui_web_editor_validator::UiWebEditorValidationRequest;
using yuengine::ui_web_editor_validator::UiWebEditorValidationStatus;
using yuengine::ui_web_editor_validator::UiWebEditorValidatorIntegration;

namespace {
constexpr const char *TEST_SUCCESS =
    "UiWebEditorValidator_Integration_BuildsSuccessReport";
constexpr const char *TEST_ISSUES =
    "UiWebEditorValidator_Integration_ReportsDuplicateAndResourceIssues";
constexpr const char *TEST_SCHEMA_CAPACITY =
    "UiWebEditorValidator_Integration_ReportsSchemaIssueOutputCapacity";
constexpr const char *TEST_NORMALIZED_CAPACITY =
    "UiWebEditorValidator_Integration_ReportsValidationIssueOutputCapacity";
constexpr const char *TEST_SHELL_CAPACITY =
    "UiWebEditorValidator_Integration_ReportsShellOutputCapacity";
constexpr const char *TEST_PREVIEW_CAPACITY =
    "UiWebEditorValidator_Integration_ReportsPreviewDiagnosticOutputCapacity";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t SENTINEL_NODE_ID = 9001U;
constexpr std::uint32_t SENTINEL_CONTEXT_KEY = 9002U;
constexpr std::uint32_t SENTINEL_RESOURCE_KEY = 9003U;

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

UiFileSchemaIssueRecord SentinelSchemaIssue() {
    UiFileSchemaIssueRecord record{};
    record.issue_kind = UiFileSchemaIssueKind::MissingRootNode;
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.context_key = SENTINEL_CONTEXT_KEY;
    return record;
}

UiWebEditorValidationIssueRecord SentinelValidationIssue() {
    UiWebEditorValidationIssueRecord record{};
    record.source = UiWebEditorValidationIssueSource::Overflow;
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.context_key = SENTINEL_CONTEXT_KEY;
    record.status_code = 77U;
    return record;
}

UiWebEditorHierarchyItemRecord SentinelHierarchy() {
    UiWebEditorHierarchyItemRecord record{};
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.parent_id = NodeId(SENTINEL_CONTEXT_KEY);
    record.sibling_order = 77U;
    return record;
}

UiWebEditorCanvasItemRecord SentinelCanvas() {
    UiWebEditorCanvasItemRecord record{};
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.sibling_order = 77U;
    return record;
}

UiWebEditorResourceItemRecord SentinelResource() {
    UiWebEditorResourceItemRecord record{};
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.resource_key = SENTINEL_RESOURCE_KEY;
    return record;
}

UiWebEditorPreviewDiagnosticRecord SentinelDiagnostic() {
    UiWebEditorPreviewDiagnosticRecord record{};
    record.kind = UiWebEditorPreviewDiagnosticKind::Schema;
    record.severity = UiWebEditorPreviewDiagnosticSeverity::Error;
    record.node_id = NodeId(SENTINEL_NODE_ID);
    record.context_key = SENTINEL_CONTEXT_KEY;
    record.status_code = 77U;
    return record;
}

bool SchemaIssueMatchesSentinel(const UiFileSchemaIssueRecord &record) {
    if (record.issue_kind != UiFileSchemaIssueKind::MissingRootNode) {
        return false;
    }

    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    return record.context_key == SENTINEL_CONTEXT_KEY;
}

bool ValidationIssueMatchesSentinel(const UiWebEditorValidationIssueRecord &record) {
    if (record.source != UiWebEditorValidationIssueSource::Overflow) {
        return false;
    }

    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    return record.status_code == 77U;
}

bool HierarchyMatchesSentinel(const UiWebEditorHierarchyItemRecord &record) {
    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    return record.sibling_order == 77U;
}

bool DiagnosticMatchesSentinel(const UiWebEditorPreviewDiagnosticRecord &record) {
    if (record.kind != UiWebEditorPreviewDiagnosticKind::Schema) {
        return false;
    }

    if (record.node_id.value != SENTINEL_NODE_ID) {
        return false;
    }

    return record.status_code == 77U;
}

struct ValidSchemaFixture final {
    std::array<UiFileNodeRecord, 3U> nodes{
        RootNode(1U),
        Node(2U, 1U, 0U, 0.5F),
        Node(3U, 1U, 1U, 0.75F)};
    std::array<UiFileLayoutRecord, 1U> layouts{
        Layout(1U)};
    std::array<UiFileStyleRef, 3U> style_refs{
        StyleRef(1U, 11U),
        StyleRef(2U, 12U),
        StyleRef(3U, 13U)};
    std::array<UiFileResourceRef, 2U> resource_refs{
        ResourceRef(2U, UiFileResourceKind::Sprite, 21U),
        ResourceRef(3U, UiFileResourceKind::Font, 22U)};
    std::array<UiFileEventBinding, 2U> event_bindings{
        EventBinding(2U, 31U, 41U),
        EventBinding(3U, 32U, 42U)};
};

struct InvalidSchemaFixture final {
    std::array<UiFileNodeRecord, 2U> nodes{
        RootNode(1U),
        Node(1U, 99U, 0U, 0.5F)};
    std::array<UiFileResourceRef, 1U> resource_refs{
        ResourceRef(77U, UiFileResourceKind::Sprite, 55U)};
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

UiFileSchemaDesc MakeInvalidSchema(InvalidSchemaFixture *fixture, std::uint32_t layout_id) {
    UiFileSchemaDesc desc{};
    desc.header = Header(layout_id, 8U);
    desc.nodes = fixture->nodes;
    desc.resource_refs = fixture->resource_refs;
    return desc;
}

UiWebEditorValidationRequest MakeRequest(
    const UiFileSchemaDesc &schema,
    std::uint32_t document_id,
    UiWebEditorPreviewMessageKind message_kind,
    std::uint32_t request_id) {
    UiWebEditorValidationRequest request{};
    request.document_id = document_id;
    request.schema = schema;
    request.selected_node_id = NodeId(2U);
    request.has_selection = true;
    request.preview_message_kind = message_kind;
    request.preview_request_id = request_id;
    return request;
}

int UiWebEditorValidatorIntegrationBuildsSuccessReport() {
    ValidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeValidSchema(&fixture, 8101U);
    const UiWebEditorValidationRequest request =
        MakeRequest(schema, 2101U, UiWebEditorPreviewMessageKind::RenderDiagnostics, 3101U);

    std::array<UiFileSchemaIssueRecord, 1U> schema_issues{SentinelSchemaIssue()};
    std::array<UiWebEditorValidationIssueRecord, 1U> validation_issues{SentinelValidationIssue()};
    std::array<UiWebEditorHierarchyItemRecord, 3U> hierarchy{};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 3U> canvas{};
    std::array<UiWebEditorResourceItemRecord, 2U> resources{};
    std::array<UiWebEditorPreviewDiagnosticRecord, 3U> diagnostics{};
    UiWebEditorValidationReport report{};
    UiWebEditorValidationOutput output{};
    output.schema_issues = schema_issues;
    output.validation_issues = validation_issues;
    output.hierarchy = hierarchy;
    output.inspector = &inspector;
    output.canvas = canvas;
    output.resources = resources;
    output.preview_diagnostics = diagnostics;
    output.report = &report;

    const UiWebEditorValidatorIntegration validator{};
    const UiWebEditorValidationStatus status = validator.Validate(request, output);
    if (status != UiWebEditorValidationStatus::Success || !report.Succeeded()) {
        return Fail("validator integration success report did not succeed");
    }

    if (!report.schema_checked ||
        !report.local_service_checked ||
        !report.shell_checked ||
        !report.preview_checked) {
        return Fail("validator integration success checks were not all visited");
    }

    if (report.checked_node_count != 3U ||
        report.checked_resource_ref_count != 2U ||
        report.schema_issue_count != 0U ||
        report.preview_diagnostic_count != 3U) {
        return Fail("validator integration success counts mismatch");
    }

    if (!hierarchy[1U].is_selected ||
        inspector.node_id.value != 2U ||
        resources[0U].resource_key != 21U ||
        diagnostics[1U].kind != UiWebEditorPreviewDiagnosticKind::Shell) {
        return Fail("validator integration success output mismatch");
    }

    if (!ValidationIssueMatchesSentinel(validation_issues[0U])) {
        return Fail("validator integration success mutated empty issue output");
    }

    return 0;
}

int UiWebEditorValidatorIntegrationReportsDuplicateAndResourceIssues() {
    InvalidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeInvalidSchema(&fixture, 8102U);
    const UiWebEditorValidationRequest request =
        MakeRequest(schema, 2102U, UiWebEditorPreviewMessageKind::LoadDocument, 3102U);

    std::array<UiFileSchemaIssueRecord, 4U> schema_issues{};
    std::array<UiWebEditorValidationIssueRecord, 4U> validation_issues{};
    UiWebEditorInspectorRecord inspector{};
    UiWebEditorValidationReport report{};
    UiWebEditorValidationOutput output{};
    output.schema_issues = schema_issues;
    output.validation_issues = validation_issues;
    output.inspector = &inspector;
    output.report = &report;

    const UiWebEditorValidatorIntegration validator{};
    const UiWebEditorValidationStatus status = validator.Validate(request, output);
    if (status != UiWebEditorValidationStatus::IssuesFound) {
        return Fail("validator integration schema issues were not reported");
    }

    if (report.schema_issue_count != 4U ||
        report.validation_issue_count != 4U ||
        report.duplicate_id_issue_count != 1U ||
        report.resource_ref_issue_count != 1U) {
        return Fail("validator integration schema issue counts mismatch");
    }

    if (schema_issues[0U].issue_kind != UiFileSchemaIssueKind::MissingRootNode ||
        schema_issues[1U].issue_kind != UiFileSchemaIssueKind::DuplicateNodeId ||
        schema_issues[2U].issue_kind != UiFileSchemaIssueKind::MissingParentNode ||
        schema_issues[3U].issue_kind != UiFileSchemaIssueKind::MissingResourceRefNode) {
        return Fail("validator integration schema issue order mismatch");
    }

    if (validation_issues[1U].source != UiWebEditorValidationIssueSource::DuplicateId ||
        validation_issues[3U].source != UiWebEditorValidationIssueSource::ResourceRef) {
        return Fail("validator integration normalized issue source mismatch");
    }

    if (report.shell_checked || report.preview_checked) {
        return Fail("validator integration schema issues should stop shell and preview checks");
    }

    return 0;
}

int UiWebEditorValidatorIntegrationReportsSchemaIssueOutputCapacity() {
    InvalidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeInvalidSchema(&fixture, 8103U);
    const UiWebEditorValidationRequest request =
        MakeRequest(schema, 2103U, UiWebEditorPreviewMessageKind::LoadDocument, 3103U);

    std::array<UiFileSchemaIssueRecord, 2U> schema_issues{
        SentinelSchemaIssue(),
        SentinelSchemaIssue()};
    std::array<UiWebEditorValidationIssueRecord, 4U> validation_issues{
        SentinelValidationIssue(),
        SentinelValidationIssue(),
        SentinelValidationIssue(),
        SentinelValidationIssue()};
    UiWebEditorInspectorRecord inspector{};
    UiWebEditorValidationReport report{};
    UiWebEditorValidationOutput output{};
    output.schema_issues = schema_issues;
    output.validation_issues = validation_issues;
    output.inspector = &inspector;
    output.report = &report;

    const UiWebEditorValidatorIntegration validator{};
    const UiWebEditorValidationStatus status = validator.Validate(request, output);
    if (status != UiWebEditorValidationStatus::OutputCapacityExceeded) {
        return Fail("validator integration schema issue capacity was not reported");
    }

    if (report.schema_issue_count != 4U || report.overflow_issue_count != 1U) {
        return Fail("validator integration schema issue capacity count mismatch");
    }

    if (!SchemaIssueMatchesSentinel(schema_issues[0U]) ||
        !ValidationIssueMatchesSentinel(validation_issues[0U])) {
        return Fail("validator integration schema issue capacity mutated outputs");
    }

    return 0;
}

int UiWebEditorValidatorIntegrationReportsValidationIssueOutputCapacity() {
    InvalidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeInvalidSchema(&fixture, 8104U);
    const UiWebEditorValidationRequest request =
        MakeRequest(schema, 2104U, UiWebEditorPreviewMessageKind::LoadDocument, 3104U);

    std::array<UiFileSchemaIssueRecord, 4U> schema_issues{
        SentinelSchemaIssue(),
        SentinelSchemaIssue(),
        SentinelSchemaIssue(),
        SentinelSchemaIssue()};
    std::array<UiWebEditorValidationIssueRecord, 2U> validation_issues{
        SentinelValidationIssue(),
        SentinelValidationIssue()};
    UiWebEditorInspectorRecord inspector{};
    UiWebEditorValidationReport report{};
    UiWebEditorValidationOutput output{};
    output.schema_issues = schema_issues;
    output.validation_issues = validation_issues;
    output.inspector = &inspector;
    output.report = &report;

    const UiWebEditorValidatorIntegration validator{};
    const UiWebEditorValidationStatus status = validator.Validate(request, output);
    if (status != UiWebEditorValidationStatus::OutputCapacityExceeded) {
        return Fail("validator integration normalized issue capacity was not reported");
    }

    if (!SchemaIssueMatchesSentinel(schema_issues[0U]) ||
        !ValidationIssueMatchesSentinel(validation_issues[0U])) {
        return Fail("validator integration normalized issue capacity mutated outputs");
    }

    return 0;
}

int UiWebEditorValidatorIntegrationReportsShellOutputCapacity() {
    ValidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeValidSchema(&fixture, 8105U);
    const UiWebEditorValidationRequest request =
        MakeRequest(schema, 2105U, UiWebEditorPreviewMessageKind::LoadDocument, 3105U);

    std::array<UiFileSchemaIssueRecord, 1U> schema_issues{};
    std::array<UiWebEditorValidationIssueRecord, 1U> validation_issues{};
    std::array<UiWebEditorHierarchyItemRecord, 2U> hierarchy{
        SentinelHierarchy(),
        SentinelHierarchy()};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 3U> canvas{
        SentinelCanvas(),
        SentinelCanvas(),
        SentinelCanvas()};
    std::array<UiWebEditorResourceItemRecord, 2U> resources{
        SentinelResource(),
        SentinelResource()};
    UiWebEditorValidationReport report{};
    UiWebEditorValidationOutput output{};
    output.schema_issues = schema_issues;
    output.validation_issues = validation_issues;
    output.hierarchy = hierarchy;
    output.inspector = &inspector;
    output.canvas = canvas;
    output.resources = resources;
    output.report = &report;

    const UiWebEditorValidatorIntegration validator{};
    const UiWebEditorValidationStatus status = validator.Validate(request, output);
    if (status != UiWebEditorValidationStatus::OutputCapacityExceeded) {
        return Fail("validator integration shell capacity was not reported");
    }

    if (!report.shell_checked ||
        report.shell_status != UiWebEditorShellStatus::OutputCapacityExceeded ||
        report.overflow_issue_count != 1U) {
        return Fail("validator integration shell capacity report mismatch");
    }

    if (!HierarchyMatchesSentinel(hierarchy[0U])) {
        return Fail("validator integration shell capacity mutated hierarchy");
    }

    return 0;
}

int UiWebEditorValidatorIntegrationReportsPreviewDiagnosticOutputCapacity() {
    ValidSchemaFixture fixture{};
    const UiFileSchemaDesc schema = MakeValidSchema(&fixture, 8106U);
    const UiWebEditorValidationRequest request =
        MakeRequest(schema, 2106U, UiWebEditorPreviewMessageKind::RenderDiagnostics, 3106U);

    std::array<UiFileSchemaIssueRecord, 1U> schema_issues{};
    std::array<UiWebEditorValidationIssueRecord, 1U> validation_issues{};
    std::array<UiWebEditorHierarchyItemRecord, 3U> hierarchy{};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 3U> canvas{};
    std::array<UiWebEditorResourceItemRecord, 2U> resources{};
    std::array<UiWebEditorPreviewDiagnosticRecord, 2U> diagnostics{
        SentinelDiagnostic(),
        SentinelDiagnostic()};
    UiWebEditorValidationReport report{};
    UiWebEditorValidationOutput output{};
    output.schema_issues = schema_issues;
    output.validation_issues = validation_issues;
    output.hierarchy = hierarchy;
    output.inspector = &inspector;
    output.canvas = canvas;
    output.resources = resources;
    output.preview_diagnostics = diagnostics;
    output.report = &report;

    const UiWebEditorValidatorIntegration validator{};
    const UiWebEditorValidationStatus status = validator.Validate(request, output);
    if (status != UiWebEditorValidationStatus::OutputCapacityExceeded) {
        return Fail("validator integration preview capacity was not reported");
    }

    if (!report.preview_checked ||
        report.preview_diagnostic_count != 3U ||
        report.overflow_issue_count != 1U) {
        return Fail("validator integration preview capacity report mismatch");
    }

    if (!DiagnosticMatchesSentinel(diagnostics[0U]) ||
        !DiagnosticMatchesSentinel(diagnostics[1U])) {
        return Fail("validator integration preview capacity mutated diagnostics");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_SUCCESS) {
        return UiWebEditorValidatorIntegrationBuildsSuccessReport();
    }

    if (name == TEST_ISSUES) {
        return UiWebEditorValidatorIntegrationReportsDuplicateAndResourceIssues();
    }

    if (name == TEST_SCHEMA_CAPACITY) {
        return UiWebEditorValidatorIntegrationReportsSchemaIssueOutputCapacity();
    }

    if (name == TEST_NORMALIZED_CAPACITY) {
        return UiWebEditorValidatorIntegrationReportsValidationIssueOutputCapacity();
    }

    if (name == TEST_SHELL_CAPACITY) {
        return UiWebEditorValidatorIntegrationReportsShellOutputCapacity();
    }

    if (name == TEST_PREVIEW_CAPACITY) {
        return UiWebEditorValidatorIntegrationReportsPreviewDiagnosticOutputCapacity();
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
