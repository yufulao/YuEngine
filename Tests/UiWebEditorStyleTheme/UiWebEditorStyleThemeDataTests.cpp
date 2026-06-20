// 模块: Tests UiWebEditorStyleTheme
// 文件: Tests/UiWebEditorStyleTheme/UiWebEditorStyleThemeDataTests.cpp

#include <array>
#include <cstdint>
#include <cstdio>
#include <string_view>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"
#include "YuEngine/UiWebEditorPreviewProtocol/UiWebEditorPreviewProtocol.h"
#include "YuEngine/UiWebEditorShell/UiWebEditorShell.h"
#include "YuEngine/UiWebEditorStyleTheme/UiWebEditorStyleThemeData.h"
#include "YuEngine/UiWebEditorValidator/UiWebEditorValidatorIntegration.h"

using yuengine::uicore::UiFileEventBinding;
using yuengine::uicore::UiFileLayoutRecord;
using yuengine::uicore::UiFileNodeRecord;
using yuengine::uicore::UiFileResourceRef;
using yuengine::uicore::UiFileSchemaDesc;
using yuengine::uicore::UiFileSchemaIssueRecord;
using yuengine::uicore::UiFileStyleRef;
using yuengine::uicore::UiNodeId;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewDiagnosticRecord;
using yuengine::ui_web_editor_preview_protocol::UiWebEditorPreviewMessageKind;
using yuengine::ui_web_editor_shell::UiWebEditorCanvasItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorHierarchyItemRecord;
using yuengine::ui_web_editor_shell::UiWebEditorInspectorRecord;
using yuengine::ui_web_editor_shell::UiWebEditorResourceItemRecord;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleEditRecord;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeCatalogResult;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeData;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeRecord;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeSchemaOutput;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeSchemaRequest;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeSchemaResult;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeStatus;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeTokenRecord;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeTokenRole;
using yuengine::ui_web_editor_style_theme::UiWebEditorStyleThemeValueKind;
using yuengine::ui_web_editor_style_theme::UI_WEB_EDITOR_STYLE_THEME_DEFAULT_THEME_COUNT;
using yuengine::ui_web_editor_style_theme::UI_WEB_EDITOR_STYLE_THEME_DEFAULT_TOKEN_COUNT;
using yuengine::ui_web_editor_validator::UiWebEditorValidationIssueRecord;
using yuengine::ui_web_editor_validator::UiWebEditorValidationOutput;
using yuengine::ui_web_editor_validator::UiWebEditorValidationReport;
using yuengine::ui_web_editor_validator::UiWebEditorValidationRequest;
using yuengine::ui_web_editor_validator::UiWebEditorValidationStatus;
using yuengine::ui_web_editor_validator::UiWebEditorValidatorIntegration;

namespace {
constexpr const char *TEST_CATALOG =
    "UiWebEditorStyleTheme_Data_WritesGenericCatalog";
constexpr const char *TEST_VALIDATOR =
    "UiWebEditorStyleTheme_Data_BuildsSchemaConsumedByValidator";
constexpr const char *TEST_SMALL_OUTPUT =
    "UiWebEditorStyleTheme_Data_RejectsSmallOutputWithoutMutation";
constexpr const char *TEST_INVALID_EDIT =
    "UiWebEditorStyleTheme_Data_RejectsInvalidStyleEditWithoutMutation";
constexpr const char *ERROR_EXPECTED_ONE_TEST_NAME = "expected one test name";
constexpr const char *ERROR_UNKNOWN_TEST_NAME = "unknown test name";
constexpr std::uint32_t TEST_LAYOUT_ID = 4101U;
constexpr std::uint32_t TEST_DOCUMENT_ID = 4102U;
constexpr std::uint32_t TEST_NODE_ID = 4103U;
constexpr std::uint32_t TEST_STYLE_KEY = 4104U;
constexpr std::uint32_t TEST_THEME_KEY = 4105U;
constexpr std::uint32_t TEST_TOKEN_KEY = 4106U;
constexpr std::uint32_t SENTINEL_STYLE_KEY = 5101U;
constexpr std::uint32_t SENTINEL_LAYOUT_ID = 5102U;

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

UiFileNodeRecord MakeNode(std::uint32_t node_id) {
    UiFileNodeRecord record{};
    record.node_id = UiNodeId{node_id};
    record.rect_transform.anchor_min = {0.0F, 0.0F};
    record.rect_transform.anchor_max = {1.0F, 1.0F};
    record.rect_transform.pivot = {0.5F, 0.5F};
    record.rect_transform.dpi_scale = 1.0F;
    record.is_visible = true;
    record.is_enabled = true;
    record.is_hit_testable = true;
    return record;
}

UiFileSchemaDesc MakeBaseSchema(std::span<const UiFileNodeRecord> nodes) {
    UiFileSchemaDesc schema{};
    schema.header.layout_id = TEST_LAYOUT_ID;
    schema.header.root_node_id = UiNodeId{TEST_NODE_ID};
    schema.nodes = nodes;
    return schema;
}

UiFileSchemaDesc SentinelSchema() {
    UiFileSchemaDesc schema{};
    schema.header.layout_id = SENTINEL_LAYOUT_ID;
    schema.header.root_node_id = UiNodeId{TEST_NODE_ID};
    return schema;
}

UiWebEditorStyleEditRecord MakeStyleEdit() {
    UiWebEditorStyleEditRecord record{};
    record.node_id = UiNodeId{TEST_NODE_ID};
    record.style_key = TEST_STYLE_KEY;
    record.theme_key = TEST_THEME_KEY;
    record.token_key = TEST_TOKEN_KEY;
    record.value_kind = UiWebEditorStyleThemeValueKind::ColorRgba8;
    record.value0 = 0x11223344U;
    record.overrides_theme = true;
    return record;
}

bool WriteCatalog(
    std::array<UiWebEditorStyleThemeTokenRecord, UI_WEB_EDITOR_STYLE_THEME_DEFAULT_TOKEN_COUNT> *out_tokens,
    std::array<UiWebEditorStyleThemeRecord, UI_WEB_EDITOR_STYLE_THEME_DEFAULT_THEME_COUNT> *out_themes) {
    if (out_tokens == nullptr || out_themes == nullptr) {
        return false;
    }

    UiWebEditorStyleThemeCatalogResult result{};
    const UiWebEditorStyleThemeData data{};
    const UiWebEditorStyleThemeStatus status = data.WriteDefaultCatalog(*out_tokens, *out_themes, &result);
    if (status != UiWebEditorStyleThemeStatus::Success || !result.Succeeded()) {
        return false;
    }

    if (result.token_count != UI_WEB_EDITOR_STYLE_THEME_DEFAULT_TOKEN_COUNT) {
        return false;
    }

    return result.theme_count == UI_WEB_EDITOR_STYLE_THEME_DEFAULT_THEME_COUNT;
}

int ValidateGeneratedSchema(const UiFileSchemaDesc &schema) {
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
    request.preview_request_id = 6101U;

    const UiWebEditorValidatorIntegration validator{};
    const UiWebEditorValidationStatus status = validator.Validate(request, output);
    if (status != UiWebEditorValidationStatus::Success || !report.Succeeded()) {
        return Fail("style theme schema was not accepted by validator integration");
    }

    if (report.checked_node_count != 1U ||
        report.checked_style_ref_count != 1U ||
        report.schema_issue_count != 0U ||
        report.validation_issue_count != 0U) {
        return Fail("style theme validator report counts mismatch");
    }

    if (!hierarchy[0U].is_root ||
        inspector.node_id.value != schema.header.root_node_id.value ||
        canvas[0U].node_id.value != schema.header.root_node_id.value) {
        return Fail("style theme validator shell output mismatch");
    }

    return 0;
}

int UiWebEditorStyleThemeDataWritesGenericCatalog() {
    std::array<UiWebEditorStyleThemeTokenRecord, UI_WEB_EDITOR_STYLE_THEME_DEFAULT_TOKEN_COUNT> tokens{};
    std::array<UiWebEditorStyleThemeRecord, UI_WEB_EDITOR_STYLE_THEME_DEFAULT_THEME_COUNT> themes{};
    UiWebEditorStyleThemeCatalogResult result{};
    const UiWebEditorStyleThemeData data{};
    const UiWebEditorStyleThemeStatus status = data.WriteDefaultCatalog(tokens, themes, &result);
    if (status != UiWebEditorStyleThemeStatus::Success || !result.Succeeded()) {
        return Fail("style theme catalog did not succeed");
    }

    if (tokens[0U].role != UiWebEditorStyleThemeTokenRole::Color ||
        tokens[0U].value_kind != UiWebEditorStyleThemeValueKind::ColorRgba8 ||
        !StringEquals(tokens[0U].name, "PrimaryColor")) {
        return Fail("style theme default token mismatch");
    }

    if (tokens[2U].role != UiWebEditorStyleThemeTokenRole::Number ||
        tokens[2U].value_kind != UiWebEditorStyleThemeValueKind::Number1000 ||
        !StringEquals(tokens[2U].name, "TextSize")) {
        return Fail("style theme number token mismatch");
    }

    if (!themes[0U].is_default ||
        themes[0U].first_token_index != 0U ||
        themes[0U].token_count != 3U ||
        !StringEquals(themes[0U].name, "Default")) {
        return Fail("style theme default theme mismatch");
    }

    if (themes[1U].is_default ||
        themes[1U].first_token_index != 3U ||
        !StringEquals(themes[1U].name, "HighContrast")) {
        return Fail("style theme alternate theme mismatch");
    }

    return 0;
}

int UiWebEditorStyleThemeDataBuildsSchemaConsumedByValidator() {
    std::array<UiWebEditorStyleThemeTokenRecord, UI_WEB_EDITOR_STYLE_THEME_DEFAULT_TOKEN_COUNT> tokens{};
    std::array<UiWebEditorStyleThemeRecord, UI_WEB_EDITOR_STYLE_THEME_DEFAULT_THEME_COUNT> themes{};
    if (!WriteCatalog(&tokens, &themes)) {
        return Fail("style theme catalog setup failed");
    }

    std::array<UiFileNodeRecord, 1U> nodes{MakeNode(TEST_NODE_ID)};
    std::array<UiWebEditorStyleEditRecord, 1U> edits{MakeStyleEdit()};
    edits[0U].theme_key = themes[0U].theme_key;
    edits[0U].token_key = tokens[0U].token_key;
    std::array<UiFileStyleRef, 1U> style_refs{};
    UiFileSchemaDesc schema{};
    UiWebEditorStyleThemeSchemaResult result{};
    UiWebEditorStyleThemeSchemaRequest request{};
    request.base_schema = MakeBaseSchema(nodes);
    request.style_edits = edits;

    UiWebEditorStyleThemeSchemaOutput output{};
    output.style_refs = style_refs;
    output.schema = &schema;
    output.result = &result;

    const UiWebEditorStyleThemeData data{};
    const UiWebEditorStyleThemeStatus status = data.BuildSchema(request, output);
    if (status != UiWebEditorStyleThemeStatus::Success || !result.Succeeded()) {
        return Fail("style theme schema build did not succeed");
    }

    if (!result.schema_ready ||
        result.style_edit_count != 1U ||
        result.style_ref_count != 1U ||
        schema.style_refs.size() != 1U ||
        schema.style_refs[0U].style_key != TEST_STYLE_KEY) {
        return Fail("style theme schema payload mismatch");
    }

    return ValidateGeneratedSchema(schema);
}

int UiWebEditorStyleThemeDataRejectsSmallOutputWithoutMutation() {
    std::array<UiFileNodeRecord, 1U> nodes{MakeNode(TEST_NODE_ID)};
    std::array<UiWebEditorStyleEditRecord, 1U> edits{MakeStyleEdit()};
    std::array<UiFileStyleRef, 0U> style_refs{};
    UiFileSchemaDesc schema = SentinelSchema();
    UiWebEditorStyleThemeSchemaResult result{};
    UiWebEditorStyleThemeSchemaRequest request{};
    request.base_schema = MakeBaseSchema(nodes);
    request.style_edits = edits;

    UiWebEditorStyleThemeSchemaOutput output{};
    output.style_refs = style_refs;
    output.schema = &schema;
    output.result = &result;

    const UiWebEditorStyleThemeData data{};
    const UiWebEditorStyleThemeStatus status = data.BuildSchema(request, output);
    if (status != UiWebEditorStyleThemeStatus::OutputCapacityExceeded ||
        result.status != UiWebEditorStyleThemeStatus::OutputCapacityExceeded) {
        return Fail("style theme small output did not fail by capacity");
    }

    if (schema.header.layout_id != SENTINEL_LAYOUT_ID ||
        schema.style_refs.size() != 0U) {
        return Fail("style theme small output mutated schema");
    }

    return 0;
}

int UiWebEditorStyleThemeDataRejectsInvalidStyleEditWithoutMutation() {
    std::array<UiFileNodeRecord, 1U> nodes{MakeNode(TEST_NODE_ID)};
    std::array<UiWebEditorStyleEditRecord, 1U> edits{MakeStyleEdit()};
    edits[0U].style_key = 0U;
    std::array<UiFileStyleRef, 1U> style_refs{};
    style_refs[0U].style_key = SENTINEL_STYLE_KEY;
    UiFileSchemaDesc schema = SentinelSchema();
    UiWebEditorStyleThemeSchemaResult result{};
    UiWebEditorStyleThemeSchemaRequest request{};
    request.base_schema = MakeBaseSchema(nodes);
    request.style_edits = edits;

    UiWebEditorStyleThemeSchemaOutput output{};
    output.style_refs = style_refs;
    output.schema = &schema;
    output.result = &result;

    const UiWebEditorStyleThemeData data{};
    const UiWebEditorStyleThemeStatus status = data.BuildSchema(request, output);
    if (status != UiWebEditorStyleThemeStatus::InvalidStyleEdit ||
        result.status != UiWebEditorStyleThemeStatus::InvalidStyleEdit) {
        return Fail("style theme invalid edit did not fail");
    }

    if (schema.header.layout_id != SENTINEL_LAYOUT_ID ||
        style_refs[0U].style_key != SENTINEL_STYLE_KEY) {
        return Fail("style theme invalid edit mutated records");
    }

    return 0;
}

int RunNamedTest(std::string_view name) {
    if (name == TEST_CATALOG) {
        return UiWebEditorStyleThemeDataWritesGenericCatalog();
    }

    if (name == TEST_VALIDATOR) {
        return UiWebEditorStyleThemeDataBuildsSchemaConsumedByValidator();
    }

    if (name == TEST_SMALL_OUTPUT) {
        return UiWebEditorStyleThemeDataRejectsSmallOutputWithoutMutation();
    }

    if (name == TEST_INVALID_EDIT) {
        return UiWebEditorStyleThemeDataRejectsInvalidStyleEditWithoutMutation();
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
