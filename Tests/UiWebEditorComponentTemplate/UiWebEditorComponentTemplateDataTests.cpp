// 模块: Tests UiWebEditorComponentTemplate
// 文件: Tests/UiWebEditorComponentTemplate/UiWebEditorComponentTemplateDataTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"
#include "YuEngine/UiWebEditorComponentTemplate/UiWebEditorComponentTemplateData.h"
#include "YuEngine/UiWebEditorPreviewProtocol/UiWebEditorPreviewProtocol.h"
#include "YuEngine/UiWebEditorShell/UiWebEditorShell.h"
#include "YuEngine/UiWebEditorValidator/UiWebEditorValidatorIntegration.h"

using yuengine::uicore::UiFileEventBinding;
using yuengine::uicore::UiFileLayoutRecord;
using yuengine::uicore::UiFileNodeRecord;
using yuengine::uicore::UiFileResourceKind;
using yuengine::uicore::UiFileResourceRef;
using yuengine::uicore::UiFileSchemaDesc;
using yuengine::uicore::UiFileSchemaIssueKind;
using yuengine::uicore::UiFileSchemaIssueRecord;
using yuengine::uicore::UiFileStyleRef;
using yuengine::uicore::UiNodeId;
using yuengine::ui_web_editor_component_template::UiWebEditorComponentTemplateCatalogResult;
using yuengine::ui_web_editor_component_template::UiWebEditorComponentTemplateCategory;
using yuengine::ui_web_editor_component_template::UiWebEditorComponentTemplateData;
using yuengine::ui_web_editor_component_template::UiWebEditorComponentTemplateKind;
using yuengine::ui_web_editor_component_template::UiWebEditorComponentTemplateRecord;
using yuengine::ui_web_editor_component_template::UiWebEditorComponentTemplateSchemaOutput;
using yuengine::ui_web_editor_component_template::UiWebEditorComponentTemplateSchemaRequest;
using yuengine::ui_web_editor_component_template::UiWebEditorComponentTemplateSchemaResult;
using yuengine::ui_web_editor_component_template::UiWebEditorComponentTemplateStatus;
using yuengine::ui_web_editor_component_template::UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT;
using yuengine::ui_web_editor_component_template::UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_EVENT;
using yuengine::ui_web_editor_component_template::UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_RESOURCE;
using yuengine::ui_web_editor_component_template::UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_STYLE;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewDiagnosticRecord;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewMessageKind;
using yuengine::ui_web_editor_shell::UiWebEditorCanvasItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorHierarchyItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorInspectorRecord;
using yuengine::ui_web_editor_shell::UiWebEditorResourceItemRecord;
using yuengine::ui_web_editor_validator::UiWebEditorValidationIssueRecord;
using yuengine::ui_web_editor_validator::UiWebEditorValidationOutput;
using yuengine::ui_web_editor_validator::UiWebEditorValidationReport;
using yuengine::ui_web_editor_validator::UiWebEditorValidationRequest;
using yuengine::ui_web_editor_validator::UiWebEditorValidationStatus;
using yuengine::ui_web_editor_validator::UiWebEditorValidatorIntegration;

namespace {
constexpr const char *TEST_CATALOG =
    "UiWebEditorComponentTemplate_Data_WritesGenericCatalog";
constexpr const char *TEST_VALIDATOR =
    "UiWebEditorComponentTemplate_Data_BuildsSchemaConsumedByValidator";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiWebEditorComponentTemplate_Data_RejectsSmallOutputWithoutMutation";
constexpr const char *TEST_INVALID_TEMPLATE =
    "UiWebEditorComponentTemplate_Data_RejectsInvalidTemplateWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t TEST_LAYOUT_ID = 7101U;
constexpr std::uint32_t TEST_DOCUMENT_ID = 8101U;
constexpr std::uint32_t SENTINEL_NODE_ID = 9101U;
constexpr std::uint32_t SENTINEL_LAYOUT_ID = 9102U;

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

UiFileNodeRecord SentinelNode() {
    UiFileNodeRecord record{};
    record.node_id = UiNodeId{SENTINEL_NODE_ID};
    record.rect_transform.dpi_scale = 1.0F;
    return record;
}

UiFileSchemaDesc SentinelSchema() {
    UiFileSchemaDesc schema{};
    schema.header.layout_id = SENTINEL_LAYOUT_ID;
    schema.header.root_node_id = UiNodeId{SENTINEL_NODE_ID};
    return schema;
}

bool FindTemplate(
    const std::array<UiWebEditorComponentTemplateRecord, UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT> &records,
    UiWebEditorComponentTemplateKind kind,
    UiWebEditorComponentTemplateRecord *out_record) {
    if (out_record == nullptr) {
        return false;
    }

    for (const UiWebEditorComponentTemplateRecord &record : records) {
        if (record.kind == kind) {
            *out_record = record;
            return true;
        }
    }

    return false;
}

bool WriteCatalog(
    std::array<UiWebEditorComponentTemplateRecord, UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT> *out_records) {
    if (out_records == nullptr) {
        return false;
    }

    UiWebEditorComponentTemplateCatalogResult catalog_result{};
    const UiWebEditorComponentTemplateData template_data{};
    const UiWebEditorComponentTemplateStatus status =
        template_data.WriteDefaultCatalog(*out_records, &catalog_result);
    if (status != UiWebEditorComponentTemplateStatus::Success || !catalog_result.Succeeded()) {
        return false;
    }

    return catalog_result.template_count == UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT;
}

UiWebEditorComponentTemplateSchemaOutput MakeOutput(
    std::array<UiFileNodeRecord, 1U> *nodes,
    std::array<UiFileLayoutRecord, 1U> *layouts,
    std::array<UiFileStyleRef, 1U> *style_refs,
    std::array<UiFileResourceRef, 1U> *resource_refs,
    std::array<UiFileEventBinding, 1U> *event_bindings,
    UiFileSchemaDesc *schema,
    UiWebEditorComponentTemplateSchemaResult *result) {
    UiWebEditorComponentTemplateSchemaOutput output{};
    output.nodes = *nodes;
    output.layouts = *layouts;
    output.style_refs = *style_refs;
    output.resource_refs = *resource_refs;
    output.event_bindings = *event_bindings;
    output.schema = schema;
    output.result = result;
    return output;
}

int ValidateGeneratedSchema(
    const UiFileSchemaDesc &schema,
    std::uint32_t expected_resource_count) {
    std::array<UiFileSchemaIssueRecord, 1U> schema_issues{};
    std::array<UiWebEditorValidationIssueRecord, 1U> validation_issues{};
    std::array<UiWebEditorHierarchyItemRecord, 1U> hierarchy{};
    UiWebEditorInspectorRecord inspector{};
    std::array<UiWebEditorCanvasItemRecord, 1U> canvas{};
    std::array<UiWebEditorResourceItemRecord, 1U> resources{};
    std::array<UiWebEditorPreviewDiagnosticRecord, 1U> diagnostics{};
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

    UiWebEditorValidationRequest request{};
    request.document_id = TEST_DOCUMENT_ID;
    request.schema = schema;
    request.selected_node_id = schema.header.root_node_id;
    request.has_selection = true;
    request.preview_message_kind = UiWebEditorPreviewMessageKind::LoadDocument;
    request.preview_request_id = 9301U;

    const UiWebEditorValidatorIntegration validator{};
    const UiWebEditorValidationStatus status = validator.Validate(request, output);
    if (status != UiWebEditorValidationStatus::Success || !report.Succeeded()) {
        return Fail("component template schema was not accepted by validator integration");
    }

    if (report.checked_node_count != 1U ||
        report.checked_resource_ref_count != expected_resource_count ||
        report.schema_issue_count != 0U ||
        report.validation_issue_count != 0U) {
        return Fail("component template validator report counts mismatch");
    }

    if (!hierarchy[0U].is_root ||
        inspector.node_id.value != schema.header.root_node_id.value ||
        canvas[0U].node_id.value != schema.header.root_node_id.value) {
        return Fail("component template validator shell output mismatch");
    }

    return 0;
}

int UiWebEditorComponentTemplateDataWritesGenericCatalog() {
    std::array<UiWebEditorComponentTemplateRecord, UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT> records{};
    UiWebEditorComponentTemplateCatalogResult catalog_result{};
    const UiWebEditorComponentTemplateData template_data{};
    const UiWebEditorComponentTemplateStatus status =
        template_data.WriteDefaultCatalog(records, &catalog_result);
    if (status != UiWebEditorComponentTemplateStatus::Success || !catalog_result.Succeeded()) {
        return Fail("component template catalog did not succeed");
    }

    if (catalog_result.template_count != UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT) {
        return Fail("component template catalog count mismatch");
    }

    if (records[0U].kind != UiWebEditorComponentTemplateKind::Container ||
        records[0U].category != UiWebEditorComponentTemplateCategory::Layout ||
        !StringEquals(records[0U].name, "Container") ||
        !records[0U].has_layout) {
        return Fail("component template catalog container record mismatch");
    }

    if (records[1U].kind != UiWebEditorComponentTemplateKind::Text ||
        records[1U].category != UiWebEditorComponentTemplateCategory::Display ||
        !StringEquals(records[1U].category_name, "Display") ||
        records[1U].resource_ref.kind != UiFileResourceKind::Font ||
        (records[1U].property_flags & UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_RESOURCE) == 0U) {
        return Fail("component template catalog text record mismatch");
    }

    if (records[2U].kind != UiWebEditorComponentTemplateKind::Button ||
        records[2U].category != UiWebEditorComponentTemplateCategory::Input ||
        !records[2U].has_event_binding ||
        (records[2U].property_flags & UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_EVENT) == 0U) {
        return Fail("component template catalog button record mismatch");
    }

    return 0;
}

int UiWebEditorComponentTemplateDataBuildsSchemaConsumedByValidator() {
    std::array<UiWebEditorComponentTemplateRecord, UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT> records{};
    if (!WriteCatalog(&records)) {
        return Fail("component template catalog setup failed");
    }

    UiWebEditorComponentTemplateRecord text_template{};
    if (!FindTemplate(records, UiWebEditorComponentTemplateKind::Text, &text_template)) {
        return Fail("component template text record missing");
    }

    std::array<UiFileNodeRecord, 1U> nodes{};
    std::array<UiFileLayoutRecord, 1U> layouts{};
    std::array<UiFileStyleRef, 1U> style_refs{};
    std::array<UiFileResourceRef, 1U> resource_refs{};
    std::array<UiFileEventBinding, 1U> event_bindings{};
    UiFileSchemaDesc schema{};
    UiWebEditorComponentTemplateSchemaResult result{};
    UiWebEditorComponentTemplateSchemaRequest request{};
    request.component_template = text_template;
    request.layout_id = TEST_LAYOUT_ID;

    const UiWebEditorComponentTemplateData template_data{};
    const UiWebEditorComponentTemplateSchemaOutput output =
        MakeOutput(&nodes, &layouts, &style_refs, &resource_refs, &event_bindings, &schema, &result);
    const UiWebEditorComponentTemplateStatus status = template_data.BuildSchema(request, output);
    if (status != UiWebEditorComponentTemplateStatus::Success || !result.Succeeded()) {
        return Fail("component template schema build did not succeed");
    }

    if (!result.schema_ready ||
        result.node_count != 1U ||
        result.style_ref_count != 1U ||
        result.resource_ref_count != 1U ||
        (result.property_flags & UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_STYLE) == 0U) {
        return Fail("component template schema result mismatch");
    }

    if (schema.header.layout_id != TEST_LAYOUT_ID ||
        schema.nodes.size() != 1U ||
        schema.resource_refs.size() != 1U ||
        schema.resource_refs[0U].kind != UiFileResourceKind::Font) {
        return Fail("component template schema payload mismatch");
    }

    return ValidateGeneratedSchema(schema, 1U);
}

int UiWebEditorComponentTemplateDataRejectsSmallOutputWithoutMutation() {
    std::array<UiWebEditorComponentTemplateRecord, UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT> records{};
    if (!WriteCatalog(&records)) {
        return Fail("component template catalog setup failed");
    }

    UiWebEditorComponentTemplateRecord button_template{};
    if (!FindTemplate(records, UiWebEditorComponentTemplateKind::Button, &button_template)) {
        return Fail("component template button record missing");
    }

    std::array<UiFileNodeRecord, 1U> nodes{SentinelNode()};
    std::array<UiFileLayoutRecord, 1U> layouts{};
    std::array<UiFileStyleRef, 0U> style_refs{};
    std::array<UiFileResourceRef, 1U> resource_refs{};
    std::array<UiFileEventBinding, 1U> event_bindings{};
    UiFileSchemaDesc schema = SentinelSchema();
    UiWebEditorComponentTemplateSchemaResult result{};
    UiWebEditorComponentTemplateSchemaRequest request{};
    request.component_template = button_template;
    request.layout_id = TEST_LAYOUT_ID;

    UiWebEditorComponentTemplateSchemaOutput output{};
    output.nodes = nodes;
    output.layouts = layouts;
    output.style_refs = style_refs;
    output.resource_refs = resource_refs;
    output.event_bindings = event_bindings;
    output.schema = &schema;
    output.result = &result;

    const UiWebEditorComponentTemplateData template_data{};
    const UiWebEditorComponentTemplateStatus status = template_data.BuildSchema(request, output);
    if (status != UiWebEditorComponentTemplateStatus::OutputCapacityExceeded ||
        result.status != UiWebEditorComponentTemplateStatus::OutputCapacityExceeded) {
        return Fail("component template small output did not fail by capacity");
    }

    if (nodes[0U].node_id.value != SENTINEL_NODE_ID ||
        schema.header.layout_id != SENTINEL_LAYOUT_ID ||
        schema.header.root_node_id.value != SENTINEL_NODE_ID) {
        return Fail("component template small output mutated records");
    }

    return 0;
}

int UiWebEditorComponentTemplateDataRejectsInvalidTemplateWithoutMutation() {
    std::array<UiWebEditorComponentTemplateRecord, UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT> records{};
    if (!WriteCatalog(&records)) {
        return Fail("component template catalog setup failed");
    }

    UiWebEditorComponentTemplateRecord invalid_template = records[0U];
    invalid_template.template_id = 0U;

    std::array<UiFileNodeRecord, 1U> nodes{SentinelNode()};
    std::array<UiFileLayoutRecord, 1U> layouts{};
    std::array<UiFileStyleRef, 1U> style_refs{};
    std::array<UiFileResourceRef, 1U> resource_refs{};
    std::array<UiFileEventBinding, 1U> event_bindings{};
    UiFileSchemaDesc schema = SentinelSchema();
    UiWebEditorComponentTemplateSchemaResult result{};
    UiWebEditorComponentTemplateSchemaRequest request{};
    request.component_template = invalid_template;
    request.layout_id = TEST_LAYOUT_ID;

    const UiWebEditorComponentTemplateSchemaOutput output =
        MakeOutput(&nodes, &layouts, &style_refs, &resource_refs, &event_bindings, &schema, &result);
    const UiWebEditorComponentTemplateData template_data{};
    const UiWebEditorComponentTemplateStatus status = template_data.BuildSchema(request, output);
    if (status != UiWebEditorComponentTemplateStatus::InvalidTemplate ||
        result.status != UiWebEditorComponentTemplateStatus::InvalidTemplate) {
        return Fail("component template invalid record did not fail");
    }

    if (nodes[0U].node_id.value != SENTINEL_NODE_ID ||
        schema.header.layout_id != SENTINEL_LAYOUT_ID ||
        schema.header.root_node_id.value != SENTINEL_NODE_ID) {
        return Fail("component template invalid record mutated records");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_CATALOG) {
        return UiWebEditorComponentTemplateDataWritesGenericCatalog();
    }

    if (name == TEST_VALIDATOR) {
        return UiWebEditorComponentTemplateDataBuildsSchemaConsumedByValidator();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiWebEditorComponentTemplateDataRejectsSmallOutputWithoutMutation();
    }

    if (name == TEST_INVALID_TEMPLATE) {
        return UiWebEditorComponentTemplateDataRejectsInvalidTemplateWithoutMutation();
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
