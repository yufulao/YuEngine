// 模块: Tools UiWebEditorComponentTemplate
// 文件: Tools/UiWebEditorComponentTemplate/Include/YuEngine/UiWebEditorComponentTemplate/UiWebEditorComponentTemplateData.h

#pragma once

#include <cstdint>
#include <span>

#include "YuEngine/UiCore/UiFileSchemaValidator.h"

namespace yuengine::ui_web_editor_component_template {
constexpr std::uint32_t UI_WEB_EDITOR_COMPONENT_TEMPLATE_DATA_VERSION = 1U;
constexpr std::uint32_t UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT = 3U;
constexpr std::uint32_t UI_WEB_EDITOR_COMPONENT_TEMPLATE_NAME_CAPACITY = 32U;
constexpr std::uint32_t UI_WEB_EDITOR_COMPONENT_TEMPLATE_CATEGORY_CAPACITY = 32U;
constexpr std::uint32_t UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_STYLE = 1U << 0U;
constexpr std::uint32_t UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_RESOURCE = 1U << 1U;
constexpr std::uint32_t UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_EVENT = 1U << 2U;
constexpr std::uint32_t UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_LAYOUT = 1U << 3U;

enum class UiWebEditorComponentTemplateKind {
    Invalid = 0,
    Container,
    Text,
    Button
};

enum class UiWebEditorComponentTemplateCategory {
    Invalid = 0,
    Layout,
    Display,
    Input
};

enum class UiWebEditorComponentTemplateStatus {
    Success = 0,
    InvalidInput,
    InvalidTemplate,
    InvalidOutput,
    OutputCapacityExceeded
};

struct UiWebEditorComponentTemplateRecord final {
    std::uint32_t template_id = 0U;
    UiWebEditorComponentTemplateKind kind = UiWebEditorComponentTemplateKind::Invalid;
    UiWebEditorComponentTemplateCategory category = UiWebEditorComponentTemplateCategory::Invalid;
    char name[UI_WEB_EDITOR_COMPONENT_TEMPLATE_NAME_CAPACITY]{};
    char category_name[UI_WEB_EDITOR_COMPONENT_TEMPLATE_CATEGORY_CAPACITY]{};
    std::uint32_t property_flags = 0U;
    yuengine::uicore::UiFileNodeRecord node;
    yuengine::uicore::UiFileLayoutRecord layout;
    yuengine::uicore::UiFileStyleRef style_ref;
    yuengine::uicore::UiFileResourceRef resource_ref;
    yuengine::uicore::UiFileEventBinding event_binding;
    bool has_layout = false;
    bool has_style_ref = false;
    bool has_resource_ref = false;
    bool has_event_binding = false;
};

struct UiWebEditorComponentTemplateCatalogResult final {
    UiWebEditorComponentTemplateStatus status = UiWebEditorComponentTemplateStatus::Success;
    std::uint32_t data_version = UI_WEB_EDITOR_COMPONENT_TEMPLATE_DATA_VERSION;
    std::uint32_t template_count = 0U;
    std::uint32_t output_capacity = 0U;

    /**
     * @comment 检查 component template catalog 是否写入成功。
     * @return catalog 写入成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorComponentTemplateStatus::Success;
    }
};

struct UiWebEditorComponentTemplateSchemaRequest final {
    UiWebEditorComponentTemplateRecord component_template;
    std::uint32_t layout_id = 0U;
};

struct UiWebEditorComponentTemplateSchemaResult final {
    UiWebEditorComponentTemplateStatus status = UiWebEditorComponentTemplateStatus::Success;
    std::uint32_t data_version = UI_WEB_EDITOR_COMPONENT_TEMPLATE_DATA_VERSION;
    std::uint32_t template_id = 0U;
    std::uint32_t layout_id = 0U;
    std::uint32_t node_count = 0U;
    std::uint32_t layout_count = 0U;
    std::uint32_t style_ref_count = 0U;
    std::uint32_t resource_ref_count = 0U;
    std::uint32_t event_binding_count = 0U;
    std::uint32_t property_flags = 0U;
    bool schema_ready = false;

    /**
     * @comment 检查 component template schema 是否构建成功。
     * @return schema 构建成功时返回 true，否则返回 false。
     */
    bool Succeeded() const {
        return status == UiWebEditorComponentTemplateStatus::Success;
    }
};

struct UiWebEditorComponentTemplateSchemaOutput final {
    std::span<yuengine::uicore::UiFileNodeRecord> nodes{};
    std::span<yuengine::uicore::UiFileLayoutRecord> layouts{};
    std::span<yuengine::uicore::UiFileStyleRef> style_refs{};
    std::span<yuengine::uicore::UiFileResourceRef> resource_refs{};
    std::span<yuengine::uicore::UiFileEventBinding> event_bindings{};
    yuengine::uicore::UiFileSchemaDesc *schema = nullptr;
    UiWebEditorComponentTemplateSchemaResult *result = nullptr;
};

class UiWebEditorComponentTemplateData final {
public:
    /**
     * @comment 写入 Web editor 通用 component template catalog 数据。
     * @param out_templates 调用方持有的 template record 输出。
     * @param out_result 输出 catalog result。
     * @return 显式 catalog 写入状态。
     */
    UiWebEditorComponentTemplateStatus WriteDefaultCatalog(
        std::span<UiWebEditorComponentTemplateRecord> out_templates,
        UiWebEditorComponentTemplateCatalogResult *out_result) const;

    /**
     * @comment 基于 component template 数据构建 schema-compatible UI file records。
     * @param request schema 构建输入。
     * @param output 调用方持有的 schema record 输出。
     * @return 显式 schema 构建状态。
     */
    UiWebEditorComponentTemplateStatus BuildSchema(
        const UiWebEditorComponentTemplateSchemaRequest &request,
        UiWebEditorComponentTemplateSchemaOutput output) const;

private:
    bool IsKnownTemplateKind(UiWebEditorComponentTemplateKind kind) const;
    bool IsKnownTemplateCategory(UiWebEditorComponentTemplateCategory category) const;
    bool IsTemplateRecordValid(const UiWebEditorComponentTemplateRecord &record) const;
    bool HasSchemaOutputCapacity(
        const UiWebEditorComponentTemplateRecord &record,
        const UiWebEditorComponentTemplateSchemaOutput &output) const;
    void WriteTemplateSchema(
        const UiWebEditorComponentTemplateSchemaRequest &request,
        UiWebEditorComponentTemplateSchemaOutput output) const;
};
}
