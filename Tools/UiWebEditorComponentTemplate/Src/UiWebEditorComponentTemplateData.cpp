// 模块: Tools UiWebEditorComponentTemplate
// 文件: Tools/UiWebEditorComponentTemplate/Src/UiWebEditorComponentTemplateData.cpp

#include "YuEngine/UiWebEditorComponentTemplate/UiWebEditorComponentTemplateData.h"

#include <cstddef>
#include <string_view>

#include "YuEngine/UiCore/UiLayoutContainerType.h"
#include "YuEngine/UiCore/UiStackDirection.h"

namespace yuengine::ui_web_editor_component_template {
namespace {
constexpr std::uint32_t TEMPLATE_ID_CONTAINER = 1001U;
constexpr std::uint32_t TEMPLATE_ID_TEXT = 1002U;
constexpr std::uint32_t TEMPLATE_ID_BUTTON = 1003U;
constexpr std::uint32_t TEMPLATE_NODE_ID = 1U;
constexpr std::uint32_t TEMPLATE_TEXT_STYLE_KEY = 2001U;
constexpr std::uint32_t TEMPLATE_BUTTON_STYLE_KEY = 2002U;
constexpr std::uint32_t TEMPLATE_TEXT_RESOURCE_KEY = 3001U;
constexpr std::uint32_t TEMPLATE_BUTTON_BINDING_KEY = 4001U;
constexpr std::uint32_t TEMPLATE_BUTTON_EVENT_KEY = 5001U;

yuengine::uicore::UiNodeId NodeId(std::uint32_t value) {
    return yuengine::uicore::UiNodeId{value};
}

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

bool HasStorage(const void *data, std::size_t capacity, std::uint32_t required_count) {
    if (required_count == 0U) {
        return true;
    }

    if (capacity < static_cast<std::size_t>(required_count)) {
        return false;
    }

    return data != nullptr;
}

yuengine::uicore::UiFileNodeRecord BaseNode() {
    yuengine::uicore::UiFileNodeRecord record{};
    record.node_id = NodeId(TEMPLATE_NODE_ID);
    record.parent_id = yuengine::uicore::UiNodeId{};
    record.rect_transform.anchor_min = {0.0F, 0.0F};
    record.rect_transform.anchor_max = {1.0F, 1.0F};
    record.rect_transform.pivot = {0.5F, 0.5F};
    record.rect_transform.dpi_scale = 1.0F;
    record.sibling_order = 0U;
    record.layer = 0;
    record.is_visible = true;
    record.is_enabled = true;
    record.is_hit_testable = true;
    return record;
}

yuengine::uicore::UiFileLayoutRecord StackLayout() {
    yuengine::uicore::UiFileLayoutRecord record{};
    record.container.container_id = NodeId(TEMPLATE_NODE_ID);
    record.container.type = yuengine::uicore::UiLayoutContainerType::Stack;
    record.container.stack_direction = yuengine::uicore::UiStackDirection::Vertical;
    record.container.grid_column_count = 1U;
    record.container.spacing_y = 4.0F;
    return record;
}

yuengine::uicore::UiFileStyleRef StyleRef(std::uint32_t style_key) {
    yuengine::uicore::UiFileStyleRef record{};
    record.node_id = NodeId(TEMPLATE_NODE_ID);
    record.style_key = style_key;
    return record;
}

yuengine::uicore::UiFileResourceRef FontResourceRef() {
    yuengine::uicore::UiFileResourceRef record{};
    record.node_id = NodeId(TEMPLATE_NODE_ID);
    record.kind = yuengine::uicore::UiFileResourceKind::Font;
    record.resource_key = TEMPLATE_TEXT_RESOURCE_KEY;
    return record;
}

yuengine::uicore::UiFileEventBinding ButtonEventBinding() {
    yuengine::uicore::UiFileEventBinding record{};
    record.node_id = NodeId(TEMPLATE_NODE_ID);
    record.binding_key = TEMPLATE_BUTTON_BINDING_KEY;
    record.event_key = TEMPLATE_BUTTON_EVENT_KEY;
    return record;
}

UiWebEditorComponentTemplateRecord CreateTemplateBase(
    std::uint32_t template_id,
    UiWebEditorComponentTemplateKind kind,
    UiWebEditorComponentTemplateCategory category,
    std::string_view name,
    std::string_view category_name) {
    UiWebEditorComponentTemplateRecord record{};
    record.template_id = template_id;
    record.kind = kind;
    record.category = category;
    CopyString(name, record.name, UI_WEB_EDITOR_COMPONENT_TEMPLATE_NAME_CAPACITY);
    CopyString(category_name, record.category_name, UI_WEB_EDITOR_COMPONENT_TEMPLATE_CATEGORY_CAPACITY);
    record.node = BaseNode();
    return record;
}

UiWebEditorComponentTemplateRecord CreateContainerTemplate() {
    UiWebEditorComponentTemplateRecord record = CreateTemplateBase(
        TEMPLATE_ID_CONTAINER,
        UiWebEditorComponentTemplateKind::Container,
        UiWebEditorComponentTemplateCategory::Layout,
        "Container",
        "Layout");
    record.layout = StackLayout();
    record.has_layout = true;
    record.property_flags = UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_LAYOUT;
    return record;
}

UiWebEditorComponentTemplateRecord CreateTextTemplate() {
    UiWebEditorComponentTemplateRecord record = CreateTemplateBase(
        TEMPLATE_ID_TEXT,
        UiWebEditorComponentTemplateKind::Text,
        UiWebEditorComponentTemplateCategory::Display,
        "Text",
        "Display");
    record.style_ref = StyleRef(TEMPLATE_TEXT_STYLE_KEY);
    record.resource_ref = FontResourceRef();
    record.has_style_ref = true;
    record.has_resource_ref = true;
    record.property_flags =
        UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_STYLE |
        UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_RESOURCE;
    return record;
}

UiWebEditorComponentTemplateRecord CreateButtonTemplate() {
    UiWebEditorComponentTemplateRecord record = CreateTemplateBase(
        TEMPLATE_ID_BUTTON,
        UiWebEditorComponentTemplateKind::Button,
        UiWebEditorComponentTemplateCategory::Input,
        "Button",
        "Input");
    record.style_ref = StyleRef(TEMPLATE_BUTTON_STYLE_KEY);
    record.event_binding = ButtonEventBinding();
    record.has_style_ref = true;
    record.has_event_binding = true;
    record.property_flags =
        UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_STYLE |
        UI_WEB_EDITOR_COMPONENT_TEMPLATE_PROPERTY_EVENT;
    return record;
}
}

UiWebEditorComponentTemplateStatus UiWebEditorComponentTemplateData::WriteDefaultCatalog(
    std::span<UiWebEditorComponentTemplateRecord> out_templates,
    UiWebEditorComponentTemplateCatalogResult *out_result) const {
    if (out_result == nullptr) {
        return UiWebEditorComponentTemplateStatus::InvalidInput;
    }

    *out_result = UiWebEditorComponentTemplateCatalogResult{};
    out_result->template_count = UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT;
    out_result->output_capacity = static_cast<std::uint32_t>(out_templates.size());
    if (!HasStorage(out_templates.data(), out_templates.size(), UI_WEB_EDITOR_COMPONENT_TEMPLATE_DEFAULT_COUNT)) {
        out_result->status = UiWebEditorComponentTemplateStatus::OutputCapacityExceeded;
        return UiWebEditorComponentTemplateStatus::OutputCapacityExceeded;
    }

    out_templates[0U] = CreateContainerTemplate();
    out_templates[1U] = CreateTextTemplate();
    out_templates[2U] = CreateButtonTemplate();
    out_result->status = UiWebEditorComponentTemplateStatus::Success;
    return UiWebEditorComponentTemplateStatus::Success;
}

UiWebEditorComponentTemplateStatus UiWebEditorComponentTemplateData::BuildSchema(
    const UiWebEditorComponentTemplateSchemaRequest &request,
    UiWebEditorComponentTemplateSchemaOutput output) const {
    if (output.result == nullptr) {
        return UiWebEditorComponentTemplateStatus::InvalidInput;
    }

    *output.result = UiWebEditorComponentTemplateSchemaResult{};
    output.result->template_id = request.component_template.template_id;
    output.result->layout_id = request.layout_id;
    output.result->property_flags = request.component_template.property_flags;
    if (request.layout_id == 0U) {
        output.result->status = UiWebEditorComponentTemplateStatus::InvalidInput;
        return UiWebEditorComponentTemplateStatus::InvalidInput;
    }

    if (!IsTemplateRecordValid(request.component_template)) {
        output.result->status = UiWebEditorComponentTemplateStatus::InvalidTemplate;
        return UiWebEditorComponentTemplateStatus::InvalidTemplate;
    }

    if (output.schema == nullptr) {
        output.result->status = UiWebEditorComponentTemplateStatus::InvalidOutput;
        return UiWebEditorComponentTemplateStatus::InvalidOutput;
    }

    if (!HasSchemaOutputCapacity(request.component_template, output)) {
        output.result->status = UiWebEditorComponentTemplateStatus::OutputCapacityExceeded;
        return UiWebEditorComponentTemplateStatus::OutputCapacityExceeded;
    }

    WriteTemplateSchema(request, output);
    output.result->status = UiWebEditorComponentTemplateStatus::Success;
    output.result->schema_ready = true;
    return UiWebEditorComponentTemplateStatus::Success;
}

bool UiWebEditorComponentTemplateData::IsKnownTemplateKind(
    UiWebEditorComponentTemplateKind kind) const {
    if (kind == UiWebEditorComponentTemplateKind::Container) {
        return true;
    }

    if (kind == UiWebEditorComponentTemplateKind::Text) {
        return true;
    }

    return kind == UiWebEditorComponentTemplateKind::Button;
}

bool UiWebEditorComponentTemplateData::IsKnownTemplateCategory(
    UiWebEditorComponentTemplateCategory category) const {
    if (category == UiWebEditorComponentTemplateCategory::Layout) {
        return true;
    }

    if (category == UiWebEditorComponentTemplateCategory::Display) {
        return true;
    }

    return category == UiWebEditorComponentTemplateCategory::Input;
}

bool UiWebEditorComponentTemplateData::IsTemplateRecordValid(
    const UiWebEditorComponentTemplateRecord &record) const {
    if (record.template_id == 0U) {
        return false;
    }

    if (!IsKnownTemplateKind(record.kind)) {
        return false;
    }

    if (!IsKnownTemplateCategory(record.category)) {
        return false;
    }

    if (record.name[0U] == '\0' || record.category_name[0U] == '\0') {
        return false;
    }

    if (!record.node.node_id.IsValid() || record.node.rect_transform.dpi_scale <= 0.0F) {
        return false;
    }

    if (record.has_layout && record.layout.container.container_id.value != record.node.node_id.value) {
        return false;
    }

    if (record.has_style_ref && record.style_ref.node_id.value != record.node.node_id.value) {
        return false;
    }

    if (record.has_resource_ref && record.resource_ref.node_id.value != record.node.node_id.value) {
        return false;
    }

    if (record.has_event_binding && record.event_binding.node_id.value != record.node.node_id.value) {
        return false;
    }

    return true;
}

bool UiWebEditorComponentTemplateData::HasSchemaOutputCapacity(
    const UiWebEditorComponentTemplateRecord &record,
    const UiWebEditorComponentTemplateSchemaOutput &output) const {
    if (!HasStorage(output.nodes.data(), output.nodes.size(), 1U)) {
        return false;
    }

    if (!HasStorage(output.layouts.data(), output.layouts.size(), record.has_layout ? 1U : 0U)) {
        return false;
    }

    if (!HasStorage(output.style_refs.data(), output.style_refs.size(), record.has_style_ref ? 1U : 0U)) {
        return false;
    }

    if (!HasStorage(output.resource_refs.data(), output.resource_refs.size(), record.has_resource_ref ? 1U : 0U)) {
        return false;
    }

    return HasStorage(output.event_bindings.data(), output.event_bindings.size(), record.has_event_binding ? 1U : 0U);
}

void UiWebEditorComponentTemplateData::WriteTemplateSchema(
    const UiWebEditorComponentTemplateSchemaRequest &request,
    UiWebEditorComponentTemplateSchemaOutput output) const {
    const UiWebEditorComponentTemplateRecord &record = request.component_template;
    output.nodes[0U] = record.node;

    std::uint32_t layout_count = 0U;
    if (record.has_layout) {
        output.layouts[0U] = record.layout;
        layout_count = 1U;
    }

    std::uint32_t style_ref_count = 0U;
    if (record.has_style_ref) {
        output.style_refs[0U] = record.style_ref;
        style_ref_count = 1U;
    }

    std::uint32_t resource_ref_count = 0U;
    if (record.has_resource_ref) {
        output.resource_refs[0U] = record.resource_ref;
        resource_ref_count = 1U;
    }

    std::uint32_t event_binding_count = 0U;
    if (record.has_event_binding) {
        output.event_bindings[0U] = record.event_binding;
        event_binding_count = 1U;
    }

    yuengine::uicore::UiFileSchemaHeader header{};
    header.layout_id = request.layout_id;
    header.root_node_id = record.node.node_id;
    output.schema->header = header;
    output.schema->nodes = std::span<const yuengine::uicore::UiFileNodeRecord>(output.nodes.data(), 1U);
    output.schema->layouts = std::span<const yuengine::uicore::UiFileLayoutRecord>(
        output.layouts.data(),
        static_cast<std::size_t>(layout_count));
    output.schema->style_refs = std::span<const yuengine::uicore::UiFileStyleRef>(
        output.style_refs.data(),
        static_cast<std::size_t>(style_ref_count));
    output.schema->resource_refs = std::span<const yuengine::uicore::UiFileResourceRef>(
        output.resource_refs.data(),
        static_cast<std::size_t>(resource_ref_count));
    output.schema->event_bindings = std::span<const yuengine::uicore::UiFileEventBinding>(
        output.event_bindings.data(),
        static_cast<std::size_t>(event_binding_count));
    output.result->node_count = 1U;
    output.result->layout_count = layout_count;
    output.result->style_ref_count = style_ref_count;
    output.result->resource_ref_count = resource_ref_count;
    output.result->event_binding_count = event_binding_count;
}
}
