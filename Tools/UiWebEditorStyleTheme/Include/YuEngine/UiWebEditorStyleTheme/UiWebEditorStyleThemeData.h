// 模块: Tools UiWebEditorStyleTheme
// 文件: Tools/UiWebEditorStyleTheme/Include/YuEngine/UiWebEditorStyleTheme/UiWebEditorStyleThemeData.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"

namespace yuengine::ui_web_editor_style_theme {
constexpr std::uint32_t UI_WEB_EDITOR_STYLE_THEME_DATA_VERSION = 1U;
constexpr std::uint32_t UI_WEB_EDITOR_STYLE_THEME_DEFAULT_TOKEN_COUNT = 6U;
constexpr std::uint32_t UI_WEB_EDITOR_STYLE_THEME_DEFAULT_THEME_COUNT = 2U;
constexpr std::uint32_t UI_WEB_EDITOR_STYLE_THEME_NAME_CAPACITY = 32U;

enum class UiWebEditorStyleThemeTokenRole {
    Invalid = 0,
    Color,
    Number,
    Resource
};

enum class UiWebEditorStyleThemeValueKind {
    Invalid = 0,
    ColorRgba8,
    Number1000,
    ResourceKey
};

enum class UiWebEditorStyleThemeStatus {
    Success = 0,
    InvalidInput,
    InvalidCatalog,
    InvalidStyleEdit,
    InvalidOutput,
    OutputCapacityExceeded
};

struct UiWebEditorStyleThemeTokenRecord final {
    std::uint32_t token_key = 0U;
    UiWebEditorStyleThemeTokenRole role = UiWebEditorStyleThemeTokenRole::Invalid;
    UiWebEditorStyleThemeValueKind value_kind = UiWebEditorStyleThemeValueKind::Invalid;
    char name[UI_WEB_EDITOR_STYLE_THEME_NAME_CAPACITY]{};
    std::uint32_t value0 = 0U;
    std::uint32_t value1 = 0U;
    std::uint32_t value2 = 0U;
    std::uint32_t value3 = 0U;
};

struct UiWebEditorStyleThemeRecord final {
    std::uint32_t theme_key = 0U;
    char name[UI_WEB_EDITOR_STYLE_THEME_NAME_CAPACITY]{};
    std::uint32_t first_token_index = 0U;
    std::uint32_t token_count = 0U;
    bool is_default = false;
};

struct UiWebEditorStyleEditRecord final {
    yuengine::uicore::UiNodeId node_id;
    std::uint32_t style_key = 0U;
    std::uint32_t theme_key = 0U;
    std::uint32_t token_key = 0U;
    UiWebEditorStyleThemeValueKind value_kind = UiWebEditorStyleThemeValueKind::Invalid;
    std::uint32_t value0 = 0U;
    std::uint32_t value1 = 0U;
    std::uint32_t value2 = 0U;
    std::uint32_t value3 = 0U;
    bool overrides_theme = false;
};

struct UiWebEditorStyleThemeCatalogResult final {
    UiWebEditorStyleThemeStatus status = UiWebEditorStyleThemeStatus::Success;
    std::uint32_t data_version = UI_WEB_EDITOR_STYLE_THEME_DATA_VERSION;
    std::uint32_t token_count = 0U;
    std::uint32_t theme_count = 0U;
    std::uint32_t token_output_capacity = 0U;
    std::uint32_t theme_output_capacity = 0U;

    /**
     * @comment 检查 style/theme catalog 是否写入成功。
     * @return catalog 写入成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorStyleThemeStatus::Success;
    }
};

struct UiWebEditorStyleThemeSchemaRequest final {
    yuengine::uicore::UiFileSchemaDesc base_schema;
    std::span<const UiWebEditorStyleEditRecord> style_edits{};
};

struct UiWebEditorStyleThemeSchemaResult final {
    UiWebEditorStyleThemeStatus status = UiWebEditorStyleThemeStatus::Success;
    std::uint32_t data_version = UI_WEB_EDITOR_STYLE_THEME_DATA_VERSION;
    std::uint32_t style_edit_count = 0U;
    std::uint32_t style_ref_count = 0U;
    bool schema_ready = false;

    /**
     * @comment 检查 style/theme schema 输出是否构建成功。
     * @return schema 构建成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorStyleThemeStatus::Success;
    }
};

struct UiWebEditorStyleThemeSchemaOutput final {
    std::span<yuengine::uicore::UiFileStyleRef> style_refs{};
    yuengine::uicore::UiFileSchemaDesc *schema = nullptr;
    UiWebEditorStyleThemeSchemaResult *result = nullptr;
};

class UiWebEditorStyleThemeData final {
public:
    /**
     * @comment 写入 Web editor 通用 style/theme catalog 数据。
     * @param out_tokens 调用方持有的 token record 输出。
     * @param out_themes 调用方持有的 theme record 输出。
     * @param out_result 输出 catalog result。
     * @return 显式 catalog 写入状态。
     */
    UiWebEditorStyleThemeStatus WriteDefaultCatalog(
        std::span<UiWebEditorStyleThemeTokenRecord> out_tokens,
        std::span<UiWebEditorStyleThemeRecord> out_themes,
        UiWebEditorStyleThemeCatalogResult *out_result) const;

    /**
     * @comment 基于 style edit 数据输出 schema-compatible style refs。
     * @param request style/theme schema 构建输入。
     * @param output 调用方持有的 schema record 输出。
     * @return 显式 schema 构建状态。
     */
    UiWebEditorStyleThemeStatus BuildSchema(
        const UiWebEditorStyleThemeSchemaRequest &request,
        UiWebEditorStyleThemeSchemaOutput output) const;

private:
    bool IsKnownTokenRole(UiWebEditorStyleThemeTokenRole role) const;
    bool IsKnownValueKind(UiWebEditorStyleThemeValueKind value_kind) const;
    bool IsBaseSchemaValid(const yuengine::uicore::UiFileSchemaDesc &schema) const;
    bool IsStyleEditValid(
        const yuengine::uicore::UiFileSchemaDesc &schema,
        const UiWebEditorStyleEditRecord &record) const;
    bool ContainsNodeId(
        std::span<const yuengine::uicore::UiFileNodeRecord> nodes,
        yuengine::uicore::UiNodeId node_id) const;
    bool HasSchemaOutputCapacity(
        const UiWebEditorStyleThemeSchemaRequest &request,
        const UiWebEditorStyleThemeSchemaOutput &output) const;
    void WriteSchema(
        const UiWebEditorStyleThemeSchemaRequest &request,
        UiWebEditorStyleThemeSchemaOutput output) const;
};
}
