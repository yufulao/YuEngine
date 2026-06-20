// 模块: Tests UiWebEditorStatePreview
// 文件: Tests/UiWebEditorStatePreview/UiWebEditorStatePreviewDataTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <span>
#include <string_view>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"
#include "YuEngine/UiWebEditorPreviewProtocol/UiWebEditorPreviewProtocol.h"
#include "YuEngine/UiWebEditorStatePreview/UiWebEditorStatePreviewData.h"

using yuengine::uicore::UiFileEventBinding;
using yuengine::uicore::UiFileLayoutRecord;
using yuengine::uicore::UiFileNodeRecord;
using yuengine::uicore::UiFileSchemaDesc;
using yuengine::uicore::UiFileSchemaIssueRecord;
using yuengine::uicore::UiFileStyleRef;
using yuengine::uicore::UiNodeId;
using yuengine::uicore::UI_FILE_SCHEMA_ID;
using yuengine::uicore::UI_FILE_SCHEMA_VERSION;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewDiagnosticRecord;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewMessageKind;
using yuengine::ui_web_editor_shell::UiWebEditorCanvasItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorHierarchyItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorInspectorRecord;
using yuengine::ui_web_editor_shell::UiWebEditorResourceItemRecord;
using yuengine::ui_web_editor_state_preview::UiWebEditorStatePreviewCatalogResult;
using yuengine::ui_web_editor_state_preview::UiWebEditorStatePreviewData;
using yuengine::ui_web_editor_state_preview::UiWebEditorStatePreviewInputRecord;
using yuengine::ui_web_editor_state_preview::UiWebEditorStatePreviewOutput;
using yuengine::ui_web_editor_state_preview::UiWebEditorStatePreviewOutputRecord;
using yuengine::ui_web_editor_state_preview::UiWebEditorStatePreviewRequest;
using yuengine::ui_web_editor_state_preview::UiWebEditorStatePreviewResult;
using yuengine::ui_web_editor_state_preview::UiWebEditorStatePreviewStatus;
using yuengine::ui_web_editor_state_preview::UiWebEditorStatePreviewValueKind;
using yuengine::ui_web_editor_state_preview::UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT;
using yuengine::ui_web_editor_validator::UiWebEditorValidationIssueRecord;

namespace {
constexpr const char *TEST_CATALOG =
    "UiWebEditorStatePreview_Data_WritesGenericStateInputs";
constexpr const char *TEST_VALIDATOR =
    "UiWebEditorStatePreview_Data_BuildsStateOutputThroughValidator";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiWebEditorStatePreview_Data_RejectsSmallOutputWithoutMutation";
constexpr const char *TEST_INVALID_INPUT =
    "UiWebEditorStatePreview_Data_RejectsInvalidStateInputWithoutMutation";
constexpr const char *TEST_VALIDATOR_ISSUES =
    "UiWebEditorStatePreview_Data_ReportsValidatorIssuesWithoutStateOutput";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t TEST_DOCUMENT_ID = 7101U;
constexpr std::uint32_t TEST_LAYOUT_ID = 7102U;
constexpr std::uint32_t TEST_NODE_ID = 1U;
constexpr std::uint32_t SENTINEL_INPUT_KEY = 8101U;
constexpr std::uint32_t SENTINEL_VALUE = 8102U;

int Fail(std::string_view message) {
    std::fwrite(message.data(), sizeof(char), message.size(), stderr);
    std::fputc('\n', stderr);
    return 1;
}

bool StringEquals(const char *text, std::string_view expected) {
    if (text == nullptr) {
        return false;
    }

    return std::string_view(text) == expected;
}

UiNodeId NodeId(std::uint32_t value) {
    return UiNodeId{value};
}

UiFileNodeRecord MakeNode(std::uint32_t node_id, std::uint32_t parent_id) {
    UiFileNodeRecord record{};
    record.node_id = NodeId(node_id);
    record.parent_id = NodeId(parent_id);
    record.rect_transform.anchor_min = {0.0F, 0.0F};
    record.rect_transform.anchor_max = {1.0F, 1.0F};
    record.rect_transform.pivot = {0.5F, 0.5F};
    record.rect_transform.dpi_scale = 1.0F;
    record.is_visible = true;
    record.is_enabled = true;
    record.is_hit_testable = true;
    return record;
}

UiFileSchemaDesc MakeSchema(std::span<const UiFileNodeRecord> nodes) {
    UiFileSchemaDesc schema{};
    schema.header.schema_id = UI_FILE_SCHEMA_ID;
    schema.header.schema_version = UI_FILE_SCHEMA_VERSION;
    schema.header.layout_id = TEST_LAYOUT_ID;
    schema.header.root_node_id = NodeId(TEST_NODE_ID);
    schema.nodes = nodes;
    return schema;
}

UiWebEditorStatePreviewOutputRecord SentinelOutput() {
    UiWebEditorStatePreviewOutputRecord record{};
    record.input_key = SENTINEL_INPUT_KEY;
    record.value0 = SENTINEL_VALUE;
    record.accepted = true;
    return record;
}

bool WriteDefaultInputs(
    std::array<UiWebEditorStatePreviewInputRecord, UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT> *out_inputs) {
    if (out_inputs == nullptr) {
        return false;
    }

    UiWebEditorStatePreviewCatalogResult result{};
    const UiWebEditorStatePreviewData data{};
    const UiWebEditorStatePreviewStatus status = data.WriteDefaultInputs(*out_inputs, &result);
    if (status != UiWebEditorStatePreviewStatus::Success || !result.Succeeded()) {
        return false;
    }

    return result.input_count == UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT;
}

UiWebEditorStatePreviewOutput MakeOutput(
    std::span<UiWebEditorStatePreviewOutputRecord> state_outputs,
    std::span<UiFileSchemaIssueRecord> schema_issues,
    std::span<UiWebEditorValidationIssueRecord> validation_issues,
    std::span<UiWebEditorHierarchyItemRecord> hierarchy,
    UiWebEditorInspectorRecord *inspector,
    std::span<UiWebEditorCanvasItemRecord> canvas,
    std::span<UiWebEditorResourceItemRecord> resources,
    std::span<UiWebEditorPreviewDiagnosticRecord> diagnostics,
    UiWebEditorStatePreviewResult *result) {
    UiWebEditorStatePreviewOutput output{};
    output.state_outputs = state_outputs;
    output.schema_issues = schema_issues;
    output.validation_issues = validation_issues;
    output.hierarchy = hierarchy;
    output.inspector = inspector;
    output.canvas = canvas;
    output.resources = resources;
    output.preview_diagnostics = diagnostics;
    output.result = result;
    return output;
}

UiWebEditorStatePreviewRequest MakeRequest(
    const UiFileSchemaDesc &schema,
    std::span<const UiWebEditorStatePreviewInputRecord> inputs) {
    UiWebEditorStatePreviewRequest request{};
    request.document_id = TEST_DOCUMENT_ID;
    request.schema = schema;
    request.state_inputs = inputs;
    request.selected_node_id = schema.header.root_node_id;
    request.has_selection = true;
    request.preview_request_id = 7201U;
    request.preview_message_kind = UiWebEditorPreviewMessageKind::UpdateDocument;
    return request;
}

int UiWebEditorStatePreviewDataWritesGenericStateInputs() {
    std::array<UiWebEditorStatePreviewInputRecord, UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT> inputs{};
    UiWebEditorStatePreviewCatalogResult result{};
    const UiWebEditorStatePreviewData data{};
    const UiWebEditorStatePreviewStatus status = data.WriteDefaultInputs(inputs, &result);
    if (status != UiWebEditorStatePreviewStatus::Success || !result.Succeeded()) {
        return Fail("state input catalog did not succeed");
    }

    if (inputs[0U].value_kind != UiWebEditorStatePreviewValueKind::Bool ||
        !inputs[0U].affects_visibility ||
        !StringEquals(inputs[0U].name, "Visible")) {
        return Fail("visible state input mismatch");
    }

    if (inputs[1U].value_kind != UiWebEditorStatePreviewValueKind::Bool ||
        !inputs[1U].affects_enabled ||
        !StringEquals(inputs[1U].name, "Enabled")) {
        return Fail("enabled state input mismatch");
    }

    if (inputs[2U].value_kind != UiWebEditorStatePreviewValueKind::ResourceKey ||
        !inputs[2U].affects_resource ||
        !StringEquals(inputs[2U].name, "ResourceKey")) {
        return Fail("resource state input mismatch");
    }

    return 0;
}

int UiWebEditorStatePreviewDataBuildsStateOutputThroughValidator() {
    std::array<UiWebEditorStatePreviewInputRecord, UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT> inputs{};
    if (!WriteDefaultInputs(&inputs)) {
        return Fail("state input setup failed");
    }

    std::array<UiFileNodeRecord, 1U> nodes{MakeNode(TEST_NODE_ID, 0U)};
    nodes[0U].parent_id = UiNodeId{};
    std::array<UiWebEditorStatePreviewOutputRecord, UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT> outputs{};
    std::array<UiFileSchemaIssueRecord, 1U> schema_issues{};
    std::array<UiWebEditorValidationIssueRecord, 1U> validation_issues{};
    std::array<UiWebEditorHierarchyItemRecord, 1U> hierarchy{};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 1U> canvas{};
    std::array<UiWebEditorResourceItemRecord, 1U> resources{};
    std::array<UiWebEditorPreviewDiagnosticRecord, 1U> diagnostics{};
    UiWebEditorStatePreviewResult result{};
    const UiFileSchemaDesc schema = MakeSchema(nodes);
    const UiWebEditorStatePreviewRequest request = MakeRequest(schema, inputs);
    const UiWebEditorStatePreviewOutput output =
        MakeOutput(outputs, schema_issues, validation_issues, hierarchy, &inspector, canvas, resources, diagnostics, &result);

    const UiWebEditorStatePreviewData data{};
    const UiWebEditorStatePreviewStatus status = data.BuildPreview(request, output);
    if (status != UiWebEditorStatePreviewStatus::Success || !result.Succeeded()) {
        return Fail("state preview build did not succeed");
    }

    if (!result.schema_ready ||
        !result.validation_checked ||
        result.state_output_count != UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT ||
        result.checked_node_count != 1U ||
        result.schema_issue_count != 0U) {
        return Fail("state preview result mismatch");
    }

    if (!outputs[0U].accepted ||
        outputs[0U].input_key != inputs[0U].input_key ||
        outputs[2U].value_kind != UiWebEditorStatePreviewValueKind::ResourceKey) {
        return Fail("state preview output mismatch");
    }

    if (!hierarchy[0U].is_root ||
        inspector.node_id.value != TEST_NODE_ID ||
        canvas[0U].node_id.value != TEST_NODE_ID) {
        return Fail("state preview validator output mismatch");
    }

    return 0;
}

int UiWebEditorStatePreviewDataRejectsSmallOutputWithoutMutation() {
    std::array<UiWebEditorStatePreviewInputRecord, UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT> inputs{};
    if (!WriteDefaultInputs(&inputs)) {
        return Fail("state input setup failed");
    }

    std::array<UiFileNodeRecord, 1U> nodes{MakeNode(TEST_NODE_ID, 0U)};
    nodes[0U].parent_id = UiNodeId{};
    std::array<UiWebEditorStatePreviewOutputRecord, 2U> outputs{};
    outputs[0U] = SentinelOutput();
    std::array<UiFileSchemaIssueRecord, 1U> schema_issues{};
    std::array<UiWebEditorValidationIssueRecord, 1U> validation_issues{};
    std::array<UiWebEditorHierarchyItemRecord, 1U> hierarchy{};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 1U> canvas{};
    std::array<UiWebEditorResourceItemRecord, 1U> resources{};
    std::array<UiWebEditorPreviewDiagnosticRecord, 1U> diagnostics{};
    UiWebEditorStatePreviewResult result{};
    const UiFileSchemaDesc schema = MakeSchema(nodes);
    const UiWebEditorStatePreviewRequest request = MakeRequest(schema, inputs);
    const UiWebEditorStatePreviewOutput output =
        MakeOutput(outputs, schema_issues, validation_issues, hierarchy, &inspector, canvas, resources, diagnostics, &result);

    const UiWebEditorStatePreviewData data{};
    const UiWebEditorStatePreviewStatus status = data.BuildPreview(request, output);
    if (status != UiWebEditorStatePreviewStatus::OutputCapacityExceeded ||
        result.status != UiWebEditorStatePreviewStatus::OutputCapacityExceeded) {
        return Fail("small state output did not fail by capacity");
    }

    if (outputs[0U].input_key != SENTINEL_INPUT_KEY ||
        outputs[0U].value0 != SENTINEL_VALUE) {
        return Fail("small state output mutated records");
    }

    return 0;
}

int UiWebEditorStatePreviewDataRejectsInvalidStateInputWithoutMutation() {
    std::array<UiWebEditorStatePreviewInputRecord, UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT> inputs{};
    if (!WriteDefaultInputs(&inputs)) {
        return Fail("state input setup failed");
    }

    inputs[0U].node_id = NodeId(9999U);
    std::array<UiFileNodeRecord, 1U> nodes{MakeNode(TEST_NODE_ID, 0U)};
    nodes[0U].parent_id = UiNodeId{};
    std::array<UiWebEditorStatePreviewOutputRecord, UI_WEB_EDITOR_STATE_PREVIEW_DEFAULT_INPUT_COUNT> outputs{};
    outputs[0U] = SentinelOutput();
    std::array<UiFileSchemaIssueRecord, 1U> schema_issues{};
    std::array<UiWebEditorValidationIssueRecord, 1U> validation_issues{};
    std::array<UiWebEditorHierarchyItemRecord, 1U> hierarchy{};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 1U> canvas{};
    std::array<UiWebEditorResourceItemRecord, 1U> resources{};
    std::array<UiWebEditorPreviewDiagnosticRecord, 1U> diagnostics{};
    UiWebEditorStatePreviewResult result{};
    const UiFileSchemaDesc schema = MakeSchema(nodes);
    const UiWebEditorStatePreviewRequest request = MakeRequest(schema, inputs);
    const UiWebEditorStatePreviewOutput output =
        MakeOutput(outputs, schema_issues, validation_issues, hierarchy, &inspector, canvas, resources, diagnostics, &result);

    const UiWebEditorStatePreviewData data{};
    const UiWebEditorStatePreviewStatus status = data.BuildPreview(request, output);
    if (status != UiWebEditorStatePreviewStatus::InvalidStateInput ||
        result.status != UiWebEditorStatePreviewStatus::InvalidStateInput) {
        return Fail("invalid state input did not fail");
    }

    if (outputs[0U].input_key != SENTINEL_INPUT_KEY ||
        outputs[0U].value0 != SENTINEL_VALUE) {
        return Fail("invalid state input mutated records");
    }

    return 0;
}

int UiWebEditorStatePreviewDataReportsValidatorIssuesWithoutStateOutput() {
    std::array<UiWebEditorStatePreviewInputRecord, 1U> inputs{};
    inputs[0U].input_key = 9101U;
    inputs[0U].node_id = NodeId(TEST_NODE_ID);
    inputs[0U].value_kind = UiWebEditorStatePreviewValueKind::Bool;
    inputs[0U].value0 = 1U;
    inputs[0U].name[0U] = 'A';
    inputs[0U].name[1U] = '\0';

    std::array<UiFileNodeRecord, 2U> nodes{};
    nodes[0U] = MakeNode(TEST_NODE_ID, 0U);
    nodes[0U].parent_id = UiNodeId{};
    nodes[1U] = MakeNode(TEST_NODE_ID, 0U);
    nodes[1U].parent_id = UiNodeId{};
    std::array<UiWebEditorStatePreviewOutputRecord, 1U> outputs{};
    outputs[0U] = SentinelOutput();
    std::array<UiFileSchemaIssueRecord, 2U> schema_issues{};
    std::array<UiWebEditorValidationIssueRecord, 2U> validation_issues{};
    std::array<UiWebEditorHierarchyItemRecord, 2U> hierarchy{};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 2U> canvas{};
    std::array<UiWebEditorResourceItemRecord, 1U> resources{};
    std::array<UiWebEditorPreviewDiagnosticRecord, 1U> diagnostics{};
    UiWebEditorStatePreviewResult result{};
    const UiFileSchemaDesc schema = MakeSchema(nodes);
    const UiWebEditorStatePreviewRequest request = MakeRequest(schema, inputs);
    const UiWebEditorStatePreviewOutput output =
        MakeOutput(outputs, schema_issues, validation_issues, hierarchy, &inspector, canvas, resources, diagnostics, &result);

    const UiWebEditorStatePreviewData data{};
    const UiWebEditorStatePreviewStatus status = data.BuildPreview(request, output);
    if (status != UiWebEditorStatePreviewStatus::IssuesFound ||
        result.status != UiWebEditorStatePreviewStatus::IssuesFound) {
        return Fail("validator issues did not surface through state preview");
    }

    if (result.schema_issue_count == 0U ||
        result.validation_issue_count == 0U) {
        return Fail("validator issue counts were not copied");
    }

    if (outputs[0U].input_key != SENTINEL_INPUT_KEY ||
        outputs[0U].value0 != SENTINEL_VALUE) {
        return Fail("validator issue path mutated state output");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_CATALOG) {
        return UiWebEditorStatePreviewDataWritesGenericStateInputs();
    }

    if (name == TEST_VALIDATOR) {
        return UiWebEditorStatePreviewDataBuildsStateOutputThroughValidator();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiWebEditorStatePreviewDataRejectsSmallOutputWithoutMutation();
    }

    if (name == TEST_INVALID_INPUT) {
        return UiWebEditorStatePreviewDataRejectsInvalidStateInputWithoutMutation();
    }

    if (name == TEST_VALIDATOR_ISSUES) {
        return UiWebEditorStatePreviewDataReportsValidatorIssuesWithoutStateOutput();
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
