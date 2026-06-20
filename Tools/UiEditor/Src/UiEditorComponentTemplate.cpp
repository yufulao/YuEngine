// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Src/UiEditorComponentTemplate.cpp

#include "YuEngine/UiEditor/UiEditorComponentTemplate.h"

#include <cstring>
#include <string_view>

namespace yuengine::uieditor {
namespace {
constexpr std::uint32_t DEFAULT_FONT_KEY = 1001U;
constexpr std::uint32_t DEFAULT_LOCALIZATION_KEY = 2001U;
constexpr std::uint32_t DEFAULT_SPRITE_KEY = 3001U;
constexpr std::uint32_t DEFAULT_STYLE_KEY = 4001U;
constexpr std::uint32_t DEFAULT_EVENT_ID = 5001U;
constexpr const char *TEXT_TEMPLATE_NAME = "TextTemplate";
constexpr const char *IMAGE_TEMPLATE_NAME = "ImageTemplate";
constexpr const char *BUTTON_TEMPLATE_NAME = "ButtonTemplate";
constexpr const char *SLIDER_TEMPLATE_NAME = "SliderTemplate";
constexpr const char *GRID_VIEW_TEMPLATE_NAME = "GridViewTemplate";
constexpr const char *TEXT_TEMPLATE_TYPE = "Text";
constexpr const char *IMAGE_TEMPLATE_TYPE = "Image";
constexpr const char *BUTTON_TEMPLATE_TYPE = "Button";
constexpr const char *SLIDER_TEMPLATE_TYPE = "Slider";
constexpr const char *GRID_VIEW_TEMPLATE_TYPE = "GridView";
constexpr const char *BUTTON_EVENT_NAME = "OnClick";
constexpr const char *SLIDER_EVENT_NAME = "OnValueChanged";
constexpr const char *GRID_VIEW_EVENT_NAME = "OnItemSelected";

bool CopyString(std::string_view input, char *output, std::uint32_t output_capacity) {
    if (output == nullptr) {
        return false;
    }

    if (output_capacity == 0U) {
        return false;
    }

    if ((input.size() + 1U) > output_capacity) {
        return false;
    }

    std::memset(output, 0, output_capacity);
    if (input.empty()) {
        return true;
    }

    std::memcpy(output, input.data(), input.size());
    return true;
}

bool IsKnownTemplateKind(UiEditorComponentTemplateKind kind) {
    if (kind == UiEditorComponentTemplateKind::Text) {
        return true;
    }

    if (kind == UiEditorComponentTemplateKind::Image) {
        return true;
    }

    if (kind == UiEditorComponentTemplateKind::Button) {
        return true;
    }

    if (kind == UiEditorComponentTemplateKind::Slider) {
        return true;
    }

    return kind == UiEditorComponentTemplateKind::GridView;
}

const char *TemplateName(UiEditorComponentTemplateKind kind) {
    switch (kind) {
        case UiEditorComponentTemplateKind::Text:
            return TEXT_TEMPLATE_NAME;
        case UiEditorComponentTemplateKind::Image:
            return IMAGE_TEMPLATE_NAME;
        case UiEditorComponentTemplateKind::Button:
            return BUTTON_TEMPLATE_NAME;
        case UiEditorComponentTemplateKind::Slider:
            return SLIDER_TEMPLATE_NAME;
        case UiEditorComponentTemplateKind::GridView:
            return GRID_VIEW_TEMPLATE_NAME;
        case UiEditorComponentTemplateKind::Invalid:
        default:
            break;
    }

    return "";
}

const char *TemplateType(UiEditorComponentTemplateKind kind) {
    switch (kind) {
        case UiEditorComponentTemplateKind::Text:
            return TEXT_TEMPLATE_TYPE;
        case UiEditorComponentTemplateKind::Image:
            return IMAGE_TEMPLATE_TYPE;
        case UiEditorComponentTemplateKind::Button:
            return BUTTON_TEMPLATE_TYPE;
        case UiEditorComponentTemplateKind::Slider:
            return SLIDER_TEMPLATE_TYPE;
        case UiEditorComponentTemplateKind::GridView:
            return GRID_VIEW_TEMPLATE_TYPE;
        case UiEditorComponentTemplateKind::Invalid:
        default:
            break;
    }

    return "";
}

UiEditorComponentTemplateDefaultState DefaultState(UiEditorComponentTemplateKind kind) {
    switch (kind) {
        case UiEditorComponentTemplateKind::Text:
            return UiEditorComponentTemplateDefaultState::TextReady;
        case UiEditorComponentTemplateKind::Image:
            return UiEditorComponentTemplateDefaultState::ImageReady;
        case UiEditorComponentTemplateKind::Button:
            return UiEditorComponentTemplateDefaultState::ButtonNormal;
        case UiEditorComponentTemplateKind::Slider:
            return UiEditorComponentTemplateDefaultState::SliderIdle;
        case UiEditorComponentTemplateKind::GridView:
            return UiEditorComponentTemplateDefaultState::GridViewReady;
        case UiEditorComponentTemplateKind::Invalid:
        default:
            break;
    }

    return UiEditorComponentTemplateDefaultState::Invalid;
}

std::uint32_t ResourceCount(UiEditorComponentTemplateKind kind) {
    if (kind == UiEditorComponentTemplateKind::Image) {
        return 1U;
    }

    if (IsKnownTemplateKind(kind)) {
        return 2U;
    }

    return 0U;
}

std::uint32_t EventCount(UiEditorComponentTemplateKind kind) {
    if (kind == UiEditorComponentTemplateKind::Button) {
        return 1U;
    }

    if (kind == UiEditorComponentTemplateKind::Slider) {
        return 1U;
    }

    if (kind == UiEditorComponentTemplateKind::GridView) {
        return 1U;
    }

    return 0U;
}

std::uint32_t SelectKey(std::uint32_t input_key, std::uint32_t default_key) {
    if (input_key != 0U) {
        return input_key;
    }

    return default_key;
}

UiEditorResourceReference MakeResourceReference(
    std::uint32_t node_id,
    UiEditorResourceReferenceKind kind,
    std::uint32_t key) {
    UiEditorResourceReference reference{};
    reference.node_id = node_id;
    reference.kind = kind;
    reference.key = key;
    return reference;
}

UiEditorEventBinding MakeEventBinding(
    std::uint32_t node_id,
    std::uint32_t event_id,
    std::string_view event_name) {
    UiEditorEventBinding event_binding{};
    event_binding.node_id = node_id;
    event_binding.event_id = SelectKey(event_id, DEFAULT_EVENT_ID);
    event_binding.event_name = event_name;
    return event_binding;
}

bool IsOutputStorageValid(
    std::span<UiEditorResourceReference> out_resources,
    std::span<UiEditorEventBinding> out_events) {
    if (!out_resources.empty() && out_resources.data() == nullptr) {
        return false;
    }

    if (!out_events.empty() && out_events.data() == nullptr) {
        return false;
    }

    return true;
}
}

UiEditorComponentTemplateStatus UiEditorComponentTemplateFactory::Create(
    const UiEditorComponentTemplateDesc &desc,
    UiEditorComponentTemplateRecord *out_record,
    std::span<UiEditorResourceReference> out_resources,
    std::span<UiEditorEventBinding> out_events,
    UiEditorComponentTemplateResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorComponentTemplateStatus::InvalidInput;
    }

    *out_result = MakeResult(desc);

    const UiEditorComponentTemplateStatus desc_status = ValidateDesc(desc, out_result);
    if (desc_status != UiEditorComponentTemplateStatus::Success) {
        out_result->status = desc_status;
        return desc_status;
    }

    if (out_record == nullptr) {
        out_result->status = UiEditorComponentTemplateStatus::InvalidOutput;
        return UiEditorComponentTemplateStatus::InvalidOutput;
    }

    const UiEditorComponentTemplateStatus output_status = ValidateOutput(out_resources, out_events, *out_result);
    if (output_status != UiEditorComponentTemplateStatus::Success) {
        out_result->status = output_status;
        return output_status;
    }

    UiEditorComponentTemplateRecord record{};
    if (!WriteRecord(desc, &record)) {
        out_result->status = UiEditorComponentTemplateStatus::InvalidOutput;
        return UiEditorComponentTemplateStatus::InvalidOutput;
    }

    WriteResources(desc, out_resources, out_result);
    WriteEvents(desc, out_events, out_result);
    *out_record = record;
    out_result->status = UiEditorComponentTemplateStatus::Success;
    return UiEditorComponentTemplateStatus::Success;
}

UiEditorComponentTemplateStatus UiEditorComponentTemplateFactory::ValidateDesc(
    const UiEditorComponentTemplateDesc &desc,
    UiEditorComponentTemplateResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorComponentTemplateStatus::InvalidInput;
    }

    if (!IsKnownTemplateKind(desc.kind)) {
        return UiEditorComponentTemplateStatus::InvalidTemplateKind;
    }

    out_result->failed_node_id = desc.node_id;
    if (desc.node_id == 0U) {
        return UiEditorComponentTemplateStatus::InvalidLayoutNode;
    }

    if (desc.parent_node_id == 0U) {
        return UiEditorComponentTemplateStatus::InvalidLayoutNode;
    }

    return UiEditorComponentTemplateStatus::Success;
}

UiEditorComponentTemplateStatus UiEditorComponentTemplateFactory::ValidateOutput(
    std::span<UiEditorResourceReference> out_resources,
    std::span<UiEditorEventBinding> out_events,
    const UiEditorComponentTemplateResult &result) const {
    if (!IsOutputStorageValid(out_resources, out_events)) {
        return UiEditorComponentTemplateStatus::InvalidOutput;
    }

    if (out_resources.size() < result.required_resource_count) {
        return UiEditorComponentTemplateStatus::OutputCapacityExceeded;
    }

    if (out_events.size() < result.required_event_count) {
        return UiEditorComponentTemplateStatus::OutputCapacityExceeded;
    }

    return UiEditorComponentTemplateStatus::Success;
}

bool UiEditorComponentTemplateFactory::WriteRecord(
    const UiEditorComponentTemplateDesc &desc,
    UiEditorComponentTemplateRecord *out_record) const {
    if (out_record == nullptr) {
        return false;
    }

    UiEditorComponentTemplateRecord record{};
    record.kind = desc.kind;
    record.default_state = DefaultState(desc.kind);
    record.layout_node.node_id = desc.node_id;
    record.layout_node.parent_node_id = desc.parent_node_id;
    record.layout_node.order = desc.order;
    if (!CopyString(TemplateName(desc.kind), record.layout_node.name, UI_EDITOR_LAYOUT_NAME_CAPACITY)) {
        return false;
    }

    if (!CopyString(TemplateType(desc.kind), record.layout_node.type, UI_EDITOR_LAYOUT_TYPE_CAPACITY)) {
        return false;
    }

    *out_record = record;
    return true;
}

void UiEditorComponentTemplateFactory::WriteResources(
    const UiEditorComponentTemplateDesc &desc,
    std::span<UiEditorResourceReference> out_resources,
    UiEditorComponentTemplateResult *out_result) const {
    if (out_result == nullptr) {
        return;
    }

    if (desc.kind == UiEditorComponentTemplateKind::Text) {
        out_resources[0U] = MakeResourceReference(
            desc.node_id,
            UiEditorResourceReferenceKind::Font,
            SelectKey(desc.primary_resource_key, DEFAULT_FONT_KEY));
        out_resources[1U] = MakeResourceReference(
            desc.node_id,
            UiEditorResourceReferenceKind::Localization,
            SelectKey(desc.secondary_resource_key, DEFAULT_LOCALIZATION_KEY));
        out_result->written_resource_count = 2U;
        return;
    }

    if (desc.kind == UiEditorComponentTemplateKind::Image) {
        out_resources[0U] = MakeResourceReference(
            desc.node_id,
            UiEditorResourceReferenceKind::Sprite,
            SelectKey(desc.primary_resource_key, DEFAULT_SPRITE_KEY));
        out_result->written_resource_count = 1U;
        return;
    }

    if (desc.kind == UiEditorComponentTemplateKind::Button) {
        out_resources[0U] = MakeResourceReference(
            desc.node_id,
            UiEditorResourceReferenceKind::Sprite,
            SelectKey(desc.primary_resource_key, DEFAULT_SPRITE_KEY));
        out_resources[1U] = MakeResourceReference(
            desc.node_id,
            UiEditorResourceReferenceKind::Style,
            SelectKey(desc.secondary_resource_key, DEFAULT_STYLE_KEY));
        out_result->written_resource_count = 2U;
        return;
    }

    if (desc.kind == UiEditorComponentTemplateKind::Slider) {
        out_resources[0U] = MakeResourceReference(
            desc.node_id,
            UiEditorResourceReferenceKind::Sprite,
            SelectKey(desc.primary_resource_key, DEFAULT_SPRITE_KEY));
        out_resources[1U] = MakeResourceReference(
            desc.node_id,
            UiEditorResourceReferenceKind::Style,
            SelectKey(desc.secondary_resource_key, DEFAULT_STYLE_KEY));
        out_result->written_resource_count = 2U;
        return;
    }

    if (desc.kind == UiEditorComponentTemplateKind::GridView) {
        out_resources[0U] = MakeResourceReference(
            desc.node_id,
            UiEditorResourceReferenceKind::Style,
            SelectKey(desc.primary_resource_key, DEFAULT_STYLE_KEY));
        out_resources[1U] = MakeResourceReference(
            desc.node_id,
            UiEditorResourceReferenceKind::Localization,
            SelectKey(desc.secondary_resource_key, DEFAULT_LOCALIZATION_KEY));
        out_result->written_resource_count = 2U;
        return;
    }
}

void UiEditorComponentTemplateFactory::WriteEvents(
    const UiEditorComponentTemplateDesc &desc,
    std::span<UiEditorEventBinding> out_events,
    UiEditorComponentTemplateResult *out_result) const {
    if (out_result == nullptr) {
        return;
    }

    if (desc.kind == UiEditorComponentTemplateKind::Button) {
        out_events[0U] = MakeEventBinding(desc.node_id, desc.event_id, BUTTON_EVENT_NAME);
        out_result->written_event_count = 1U;
        return;
    }

    if (desc.kind == UiEditorComponentTemplateKind::Slider) {
        out_events[0U] = MakeEventBinding(desc.node_id, desc.event_id, SLIDER_EVENT_NAME);
        out_result->written_event_count = 1U;
        return;
    }

    if (desc.kind == UiEditorComponentTemplateKind::GridView) {
        out_events[0U] = MakeEventBinding(desc.node_id, desc.event_id, GRID_VIEW_EVENT_NAME);
        out_result->written_event_count = 1U;
        return;
    }
}

UiEditorComponentTemplateResult UiEditorComponentTemplateFactory::MakeResult(
    const UiEditorComponentTemplateDesc &desc) const {
    UiEditorComponentTemplateResult result{};
    result.kind = desc.kind;
    result.default_state = DefaultState(desc.kind);
    result.required_resource_count = ResourceCount(desc.kind);
    result.required_event_count = EventCount(desc.kind);
    result.failed_node_id = desc.node_id;
    return result;
}
}
