// 模块: Tools UiWebEditorStyleTheme
// 文件: Tools/UiWebEditorStyleTheme/Src/UiWebEditorStyleThemeData.cpp

#include "YuEngine/UiWebEditorStyleTheme/UiWebEditorStyleThemeData.h"

#include <cstddef>
#include <string_view>

namespace yuengine::ui_web_editor_style_theme {
namespace {
constexpr std::uint32_t THEME_KEY_DEFAULT = 2101U;
constexpr std::uint32_t THEME_KEY_HIGH_CONTRAST = 2102U;
constexpr std::uint32_t TOKEN_KEY_DEFAULT_PRIMARY = 3101U;
constexpr std::uint32_t TOKEN_KEY_DEFAULT_ACCENT = 3102U;
constexpr std::uint32_t TOKEN_KEY_DEFAULT_TEXT_SIZE = 3103U;
constexpr std::uint32_t TOKEN_KEY_HIGH_CONTRAST_PRIMARY = 3201U;
constexpr std::uint32_t TOKEN_KEY_HIGH_CONTRAST_ACCENT = 3202U;
constexpr std::uint32_t TOKEN_KEY_HIGH_CONTRAST_TEXT_SIZE = 3203U;
constexpr std::uint32_t COLOR_WHITE = 0xFFFFFFFFU;
constexpr std::uint32_t COLOR_BLACK = 0x000000FFU;
constexpr std::uint32_t COLOR_BLUE = 0x3366CCFFU;
constexpr std::uint32_t COLOR_YELLOW = 0xFFD33DFFU;
constexpr std::uint32_t TEXT_SIZE_DEFAULT = 16000U;
constexpr std::uint32_t TEXT_SIZE_LARGE = 18000U;

void CopyString(std::string_view value, char *out_text, std::size_t capacity) {
    if (out_text == nullptr) {
        return;
    }

    if (capacity == 0U) {
        return;
    }

    std::size_t copy_count = value.size();
    if (copy_count >= capacity) {
        copy_count = capacity - 1U;
    }

    for (std::size_t index = 0U; index < copy_count; ++index) {
        out_text[index] = value[index];
    }

    out_text[copy_count] = '\0';
}

bool IsStorageValid(const void *data, std::size_t count) {
    if (count == 0U) {
        return true;
    }

    return data != nullptr;
}

bool HasStorage(const void *data, std::size_t capacity, std::uint32_t required_count) {
    if (required_count == 0U) {
        return true;
    }

    if (capacity < static_cast<std::size_t>(required_count)) {
        return false;
    }

    return data != nullptr;
}

UiWebEditorStyleThemeTokenRecord CreateToken(
    std::uint32_t token_key,
    UiWebEditorStyleThemeTokenRole role,
    UiWebEditorStyleThemeValueKind value_kind,
    std::string_view name,
    std::uint32_t value0) {
    UiWebEditorStyleThemeTokenRecord record{};
    record.token_key = token_key;
    record.role = role;
    record.value_kind = value_kind;
    CopyString(name, record.name, UI_WEB_EDITOR_STYLE_THEME_NAME_CAPACITY);
    record.value0 = value0;
    return record;
}

UiWebEditorStyleThemeRecord CreateTheme(
    std::uint32_t theme_key,
    std::string_view name,
    std::uint32_t first_token_index,
    bool is_default) {
    UiWebEditorStyleThemeRecord record{};
    record.theme_key = theme_key;
    CopyString(name, record.name, UI_WEB_EDITOR_STYLE_THEME_NAME_CAPACITY);
    record.first_token_index = first_token_index;
    record.token_count = 3U;
    record.is_default = is_default;
    return record;
}
}

UiWebEditorStyleThemeStatus UiWebEditorStyleThemeData::WriteDefaultCatalog(
    std::span<UiWebEditorStyleThemeTokenRecord> out_tokens,
    std::span<UiWebEditorStyleThemeRecord> out_themes,
    UiWebEditorStyleThemeCatalogResult *out_result) const {
    if (out_result == nullptr) {
        return UiWebEditorStyleThemeStatus::InvalidInput;
    }

    *out_result = UiWebEditorStyleThemeCatalogResult{};
    out_result->token_count = UI_WEB_EDITOR_STYLE_THEME_DEFAULT_TOKEN_COUNT;
    out_result->theme_count = UI_WEB_EDITOR_STYLE_THEME_DEFAULT_THEME_COUNT;
    out_result->token_output_capacity = static_cast<std::uint32_t>(out_tokens.size());
    out_result->theme_output_capacity = static_cast<std::uint32_t>(out_themes.size());
    if (!HasStorage(out_tokens.data(), out_tokens.size(), UI_WEB_EDITOR_STYLE_THEME_DEFAULT_TOKEN_COUNT) ||
        !HasStorage(out_themes.data(), out_themes.size(), UI_WEB_EDITOR_STYLE_THEME_DEFAULT_THEME_COUNT)) {
        out_result->status = UiWebEditorStyleThemeStatus::OutputCapacityExceeded;
        return UiWebEditorStyleThemeStatus::OutputCapacityExceeded;
    }

    out_tokens[0U] = CreateToken(
        TOKEN_KEY_DEFAULT_PRIMARY,
        UiWebEditorStyleThemeTokenRole::Color,
        UiWebEditorStyleThemeValueKind::ColorRgba8,
        "PrimaryColor",
        COLOR_WHITE);
    out_tokens[1U] = CreateToken(
        TOKEN_KEY_DEFAULT_ACCENT,
        UiWebEditorStyleThemeTokenRole::Color,
        UiWebEditorStyleThemeValueKind::ColorRgba8,
        "AccentColor",
        COLOR_BLUE);
    out_tokens[2U] = CreateToken(
        TOKEN_KEY_DEFAULT_TEXT_SIZE,
        UiWebEditorStyleThemeTokenRole::Number,
        UiWebEditorStyleThemeValueKind::Number1000,
        "TextSize",
        TEXT_SIZE_DEFAULT);
    out_tokens[3U] = CreateToken(
        TOKEN_KEY_HIGH_CONTRAST_PRIMARY,
        UiWebEditorStyleThemeTokenRole::Color,
        UiWebEditorStyleThemeValueKind::ColorRgba8,
        "PrimaryColor",
        COLOR_BLACK);
    out_tokens[4U] = CreateToken(
        TOKEN_KEY_HIGH_CONTRAST_ACCENT,
        UiWebEditorStyleThemeTokenRole::Color,
        UiWebEditorStyleThemeValueKind::ColorRgba8,
        "AccentColor",
        COLOR_YELLOW);
    out_tokens[5U] = CreateToken(
        TOKEN_KEY_HIGH_CONTRAST_TEXT_SIZE,
        UiWebEditorStyleThemeTokenRole::Number,
        UiWebEditorStyleThemeValueKind::Number1000,
        "TextSize",
        TEXT_SIZE_LARGE);

    out_themes[0U] = CreateTheme(THEME_KEY_DEFAULT, "Default", 0U, true);
    out_themes[1U] = CreateTheme(THEME_KEY_HIGH_CONTRAST, "HighContrast", 3U, false);
    out_result->status = UiWebEditorStyleThemeStatus::Success;
    return UiWebEditorStyleThemeStatus::Success;
}

UiWebEditorStyleThemeStatus UiWebEditorStyleThemeData::BuildSchema(
    const UiWebEditorStyleThemeSchemaRequest &request,
    UiWebEditorStyleThemeSchemaOutput output) const {
    if (output.result == nullptr) {
        return UiWebEditorStyleThemeStatus::InvalidInput;
    }

    *output.result = UiWebEditorStyleThemeSchemaResult{};
    output.result->style_edit_count = static_cast<std::uint32_t>(request.style_edits.size());
    output.result->style_ref_count = static_cast<std::uint32_t>(
        request.base_schema.style_refs.size() + request.style_edits.size());

    if (!IsBaseSchemaValid(request.base_schema) ||
        !IsStorageValid(request.style_edits.data(), request.style_edits.size())) {
        output.result->status = UiWebEditorStyleThemeStatus::InvalidInput;
        return UiWebEditorStyleThemeStatus::InvalidInput;
    }

    if (output.schema == nullptr) {
        output.result->status = UiWebEditorStyleThemeStatus::InvalidOutput;
        return UiWebEditorStyleThemeStatus::InvalidOutput;
    }

    if (!HasSchemaOutputCapacity(request, output)) {
        output.result->status = UiWebEditorStyleThemeStatus::OutputCapacityExceeded;
        return UiWebEditorStyleThemeStatus::OutputCapacityExceeded;
    }

    for (const UiWebEditorStyleEditRecord &record : request.style_edits) {
        if (IsStyleEditValid(request.base_schema, record)) {
            continue;
        }

        output.result->status = UiWebEditorStyleThemeStatus::InvalidStyleEdit;
        return UiWebEditorStyleThemeStatus::InvalidStyleEdit;
    }

    WriteSchema(request, output);
    output.result->status = UiWebEditorStyleThemeStatus::Success;
    output.result->schema_ready = true;
    return UiWebEditorStyleThemeStatus::Success;
}

bool UiWebEditorStyleThemeData::IsKnownTokenRole(UiWebEditorStyleThemeTokenRole role) const {
    if (role == UiWebEditorStyleThemeTokenRole::Color) {
        return true;
    }

    if (role == UiWebEditorStyleThemeTokenRole::Number) {
        return true;
    }

    return role == UiWebEditorStyleThemeTokenRole::Resource;
}

bool UiWebEditorStyleThemeData::IsKnownValueKind(
    UiWebEditorStyleThemeValueKind value_kind) const {
    if (value_kind == UiWebEditorStyleThemeValueKind::ColorRgba8) {
        return true;
    }

    if (value_kind == UiWebEditorStyleThemeValueKind::Number1000) {
        return true;
    }

    return value_kind == UiWebEditorStyleThemeValueKind::ResourceKey;
}

bool UiWebEditorStyleThemeData::IsBaseSchemaValid(
    const yuengine::uicore::UiFileSchemaDesc &schema) const {
    if (schema.header.layout_id == 0U) {
        return false;
    }

    if (!schema.header.root_node_id.IsValid()) {
        return false;
    }

    if (!IsStorageValid(schema.nodes.data(), schema.nodes.size()) ||
        !IsStorageValid(schema.layouts.data(), schema.layouts.size()) ||
        !IsStorageValid(schema.style_refs.data(), schema.style_refs.size()) ||
        !IsStorageValid(schema.resource_refs.data(), schema.resource_refs.size()) ||
        !IsStorageValid(schema.event_bindings.data(), schema.event_bindings.size())) {
        return false;
    }

    return ContainsNodeId(schema.nodes, schema.header.root_node_id);
}

bool UiWebEditorStyleThemeData::IsStyleEditValid(
    const yuengine::uicore::UiFileSchemaDesc &schema,
    const UiWebEditorStyleEditRecord &record) const {
    if (!record.node_id.IsValid()) {
        return false;
    }

    if (!ContainsNodeId(schema.nodes, record.node_id)) {
        return false;
    }

    if (record.style_key == 0U || record.theme_key == 0U || record.token_key == 0U) {
        return false;
    }

    return IsKnownValueKind(record.value_kind);
}

bool UiWebEditorStyleThemeData::ContainsNodeId(
    std::span<const yuengine::uicore::UiFileNodeRecord> nodes,
    yuengine::uicore::UiNodeId node_id) const {
    if (!node_id.IsValid()) {
        return false;
    }

    for (const yuengine::uicore::UiFileNodeRecord &record : nodes) {
        if (record.node_id.value == node_id.value) {
            return true;
        }
    }

    return false;
}

bool UiWebEditorStyleThemeData::HasSchemaOutputCapacity(
    const UiWebEditorStyleThemeSchemaRequest &request,
    const UiWebEditorStyleThemeSchemaOutput &output) const {
    const std::uint32_t required_count = static_cast<std::uint32_t>(
        request.base_schema.style_refs.size() + request.style_edits.size());
    return HasStorage(output.style_refs.data(), output.style_refs.size(), required_count);
}

void UiWebEditorStyleThemeData::WriteSchema(
    const UiWebEditorStyleThemeSchemaRequest &request,
    UiWebEditorStyleThemeSchemaOutput output) const {
    std::size_t write_index = 0U;
    for (const yuengine::uicore::UiFileStyleRef &record : request.base_schema.style_refs) {
        output.style_refs[write_index] = record;
        ++write_index;
    }

    for (const UiWebEditorStyleEditRecord &record : request.style_edits) {
        yuengine::uicore::UiFileStyleRef style_ref{};
        style_ref.node_id = record.node_id;
        style_ref.style_key = record.style_key;
        output.style_refs[write_index] = style_ref;
        ++write_index;
    }

    output.schema->header = request.base_schema.header;
    output.schema->nodes = request.base_schema.nodes;
    output.schema->layouts = request.base_schema.layouts;
    output.schema->style_refs = std::span<const yuengine::uicore::UiFileStyleRef>(
        output.style_refs.data(),
        write_index);
    output.schema->resource_refs = request.base_schema.resource_refs;
    output.schema->event_bindings = request.base_schema.event_bindings;
    output.result->style_ref_count = static_cast<std::uint32_t>(write_index);
}
}
