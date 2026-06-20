// 模块: Tools UiEditor
// 文件: Tools/UiEditor/Src/UiEditorStatePreview.cpp

#include "YuEngine/UiEditor/UiEditorStatePreview.h"

#include <cstring>
#include <string_view>

namespace yuengine::uieditor {
namespace {
constexpr const char *STATE_NAME_NORMAL = "Normal";
constexpr const char *STATE_NAME_HOVER = "Hover";
constexpr const char *STATE_NAME_PRESSED = "Pressed";
constexpr const char *STATE_NAME_DISABLED = "Disabled";
constexpr const char *STATE_NAME_SELECTED = "Selected";

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

bool IsKnownButtonState(yuengine::uicore::UiButtonState state) {
    if (state == yuengine::uicore::UiButtonState::Normal) {
        return true;
    }

    if (state == yuengine::uicore::UiButtonState::Hover) {
        return true;
    }

    if (state == yuengine::uicore::UiButtonState::Pressed) {
        return true;
    }

    if (state == yuengine::uicore::UiButtonState::Disabled) {
        return true;
    }

    return state == yuengine::uicore::UiButtonState::Selected;
}

const char *ButtonStateName(yuengine::uicore::UiButtonState state) {
    switch (state) {
        case yuengine::uicore::UiButtonState::Normal:
            return STATE_NAME_NORMAL;
        case yuengine::uicore::UiButtonState::Hover:
            return STATE_NAME_HOVER;
        case yuengine::uicore::UiButtonState::Pressed:
            return STATE_NAME_PRESSED;
        case yuengine::uicore::UiButtonState::Disabled:
            return STATE_NAME_DISABLED;
        case yuengine::uicore::UiButtonState::Selected:
            return STATE_NAME_SELECTED;
        default:
            break;
    }

    return "";
}

bool IsDisabledState(yuengine::uicore::UiButtonState state) {
    return state == yuengine::uicore::UiButtonState::Disabled;
}

bool IsSelectedState(yuengine::uicore::UiButtonState state) {
    return state == yuengine::uicore::UiButtonState::Selected;
}
}

UiEditorStatePreviewStatus UiEditorStatePreviewFactory::CreateButtonStatePreview(
    const UiEditorStatePreviewDesc &desc,
    UiEditorStatePreviewRecord *out_record,
    UiEditorStatePreviewResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorStatePreviewStatus::InvalidInput;
    }

    *out_result = UiEditorStatePreviewResult{};
    out_result->button_state = desc.button_state;
    out_result->node_id = desc.component_template.layout_node.node_id;

    const UiEditorStatePreviewStatus desc_status = ValidateDesc(desc, out_result);
    if (desc_status != UiEditorStatePreviewStatus::Success) {
        out_result->status = desc_status;
        return desc_status;
    }

    if (out_record == nullptr) {
        out_result->status = UiEditorStatePreviewStatus::InvalidOutput;
        return UiEditorStatePreviewStatus::InvalidOutput;
    }

    UiEditorStatePreviewRecord record{};
    if (!WriteRecord(desc, &record)) {
        out_result->status = UiEditorStatePreviewStatus::InvalidOutput;
        return UiEditorStatePreviewStatus::InvalidOutput;
    }

    *out_record = record;
    out_result->status = UiEditorStatePreviewStatus::Success;
    return UiEditorStatePreviewStatus::Success;
}

UiEditorStatePreviewStatus UiEditorStatePreviewFactory::ValidateDesc(
    const UiEditorStatePreviewDesc &desc,
    UiEditorStatePreviewResult *out_result) const {
    if (out_result == nullptr) {
        return UiEditorStatePreviewStatus::InvalidInput;
    }

    if (desc.component_template.kind != UiEditorComponentTemplateKind::Button) {
        return UiEditorStatePreviewStatus::InvalidTemplate;
    }

    if (desc.component_template.default_state != UiEditorComponentTemplateDefaultState::ButtonNormal) {
        return UiEditorStatePreviewStatus::InvalidTemplate;
    }

    if (desc.component_template.layout_node.node_id == 0U) {
        return UiEditorStatePreviewStatus::InvalidTemplate;
    }

    if (!IsKnownButtonState(desc.button_state)) {
        return UiEditorStatePreviewStatus::InvalidState;
    }

    return UiEditorStatePreviewStatus::Success;
}

bool UiEditorStatePreviewFactory::WriteRecord(
    const UiEditorStatePreviewDesc &desc,
    UiEditorStatePreviewRecord *out_record) const {
    if (out_record == nullptr) {
        return false;
    }

    UiEditorStatePreviewRecord record{};
    record.component_kind = desc.component_template.kind;
    record.button_state = desc.button_state;
    record.node_id = desc.component_template.layout_node.node_id;
    record.state_badge_visible = true;
    record.disabled_overlay_visible = IsDisabledState(desc.button_state);
    record.selected_outline_visible = IsSelectedState(desc.button_state);
    record.interaction_enabled = !IsDisabledState(desc.button_state);
    if (!CopyString(ButtonStateName(desc.button_state), record.state_name, UI_EDITOR_STATE_PREVIEW_NAME_CAPACITY)) {
        return false;
    }

    *out_record = record;
    return true;
}
}
