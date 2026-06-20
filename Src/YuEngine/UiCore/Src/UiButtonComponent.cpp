// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiButtonComponent.cpp

#include "YuEngine/UiCore/UiButtonComponent.h"

#include "YuEngine/UiCore/UiNodeTreeResult.h"

namespace yuengine::uicore {
namespace {
bool IsTintChannelValid(float value) {
    if (value < 0.0F) {
        return false;
    }

    return value <= 1.0F;
}

bool IsTintValid(const UiImageTint &tint) {
    if (!IsTintChannelValid(tint.red)) {
        return false;
    }

    if (!IsTintChannelValid(tint.green)) {
        return false;
    }

    if (!IsTintChannelValid(tint.blue)) {
        return false;
    }

    return IsTintChannelValid(tint.alpha);
}

bool IsVisualValid(const UiButtonVisualStateDesc &visual, bool image_required) {
    if (image_required && visual.image_sprite_key == 0U) {
        return false;
    }

    if (!IsTintValid(visual.image_tint)) {
        return false;
    }

    return IsTintValid(visual.text_tint);
}

bool HitMatchesButton(const UiButtonComponentDesc &desc) {
    return desc.interaction.hit_node_id.value == desc.node_id.value;
}

bool IsButtonEnabled(const UiButtonComponentDesc &desc, const UiNodeRecord &record) {
    if (!desc.is_enabled) {
        return false;
    }

    if (!record.is_enabled) {
        return false;
    }

    return record.is_visible;
}

void SetFailure(UiButtonComponentResult *out_result, UiButtonComponentStatus status) {
    out_result->status = status;
}
}

UiButtonComponentStatus UiButtonComponent::Build(
    const UiNodeTree &tree,
    const UiButtonComponentDesc &desc,
    UiButtonComponentResult *out_result) const {
    if (out_result == nullptr) {
        return UiButtonComponentStatus::InvalidOutputBuffer;
    }

    *out_result = UiButtonComponentResult{};
    out_result->node_id = desc.node_id;
    UiButtonComponentStatus status = ValidateDesc(desc);
    if (status != UiButtonComponentStatus::Success) {
        SetFailure(out_result, status);
        return status;
    }

    const UiNodeTreeResult node_result = tree.QueryNode(desc.node_id);
    if (!node_result.Succeeded()) {
        SetFailure(out_result, UiButtonComponentStatus::NodeNotFound);
        return UiButtonComponentStatus::NodeNotFound;
    }

    const UiButtonState state = ResolveState(desc, node_result.record);
    const UiButtonActivationSource activation_source = ResolveActivationSource(desc, state);
    const UiButtonVisualStateDesc &visual = SelectVisual(desc, state);

    out_result->status = UiButtonComponentStatus::Success;
    out_result->state = state;
    out_result->activation_source = activation_source;
    out_result->activated = activation_source != UiButtonActivationSource::None;
    out_result->visual_update = BuildVisualUpdate(desc, visual);
    if (out_result->activated) {
        out_result->activation_event_key = desc.hooks.activation_event_key;
        out_result->activation_sound_key = desc.hooks.activation_sound_key;
    }

    return UiButtonComponentStatus::Success;
}

UiButtonComponentStatus UiButtonComponent::ValidateDesc(const UiButtonComponentDesc &desc) const {
    if (!desc.node_id.IsValid()) {
        return UiButtonComponentStatus::InvalidDesc;
    }

    const bool has_image = desc.image_node_id.IsValid();
    const bool has_text = desc.text_node_id.IsValid();
    if (!has_image && !has_text) {
        return UiButtonComponentStatus::InvalidDesc;
    }

    if (!IsVisualValid(desc.normal_visual, has_image)) {
        return UiButtonComponentStatus::InvalidDesc;
    }

    if (!IsVisualValid(desc.hover_visual, has_image)) {
        return UiButtonComponentStatus::InvalidDesc;
    }

    if (!IsVisualValid(desc.pressed_visual, has_image)) {
        return UiButtonComponentStatus::InvalidDesc;
    }

    if (!IsVisualValid(desc.disabled_visual, has_image)) {
        return UiButtonComponentStatus::InvalidDesc;
    }

    if (!IsVisualValid(desc.selected_visual, has_image)) {
        return UiButtonComponentStatus::InvalidDesc;
    }

    return UiButtonComponentStatus::Success;
}

UiButtonState UiButtonComponent::ResolveState(
    const UiButtonComponentDesc &desc,
    const UiNodeRecord &record) const {
    if (!IsButtonEnabled(desc, record)) {
        return UiButtonState::Disabled;
    }

    const bool hit_matches = HitMatchesButton(desc);
    if (desc.interaction.pointer_is_down && hit_matches) {
        return UiButtonState::Pressed;
    }

    if (desc.is_selected) {
        return UiButtonState::Selected;
    }

    if (hit_matches) {
        return UiButtonState::Hover;
    }

    return UiButtonState::Normal;
}

UiButtonActivationSource UiButtonComponent::ResolveActivationSource(
    const UiButtonComponentDesc &desc,
    UiButtonState state) const {
    if (state == UiButtonState::Disabled) {
        return UiButtonActivationSource::None;
    }

    const bool hit_matches = HitMatchesButton(desc);
    const bool pointer_activated = desc.interaction.pointer_released &&
        desc.interaction.pointer_pressed_on_button &&
        hit_matches;
    if (pointer_activated) {
        return UiButtonActivationSource::Pointer;
    }

    if (desc.interaction.keyboard_activate_requested) {
        return UiButtonActivationSource::Keyboard;
    }

    if (desc.interaction.gamepad_activate_requested) {
        return UiButtonActivationSource::Gamepad;
    }

    return UiButtonActivationSource::None;
}

const UiButtonVisualStateDesc &UiButtonComponent::SelectVisual(
    const UiButtonComponentDesc &desc,
    UiButtonState state) const {
    switch (state) {
        case UiButtonState::Hover:
            return desc.hover_visual;
        case UiButtonState::Pressed:
            return desc.pressed_visual;
        case UiButtonState::Disabled:
            return desc.disabled_visual;
        case UiButtonState::Selected:
            return desc.selected_visual;
        case UiButtonState::Normal:
            return desc.normal_visual;
        default:
            break;
    }

    return desc.normal_visual;
}

UiButtonVisualUpdate UiButtonComponent::BuildVisualUpdate(
    const UiButtonComponentDesc &desc,
    const UiButtonVisualStateDesc &visual) const {
    UiButtonVisualUpdate update{};
    if (desc.image_node_id.IsValid()) {
        update.has_image_update = true;
        update.image_desc.node_id = desc.image_node_id;
        update.image_desc.sprite_key = visual.image_sprite_key;
        update.image_desc.style_key = visual.image_style_key;
        update.image_desc.material_key = visual.image_material_key;
        update.image_desc.tint = visual.image_tint;
        update.image_desc.scissor_enabled = desc.scissor_enabled;
    }

    if (desc.text_node_id.IsValid()) {
        update.has_text_update = true;
        update.text_desc.node_id = desc.text_node_id;
        update.text_desc.style_key = visual.text_style_key;
        update.text_desc.material_key = visual.text_material_key;
        update.text_desc.tint = visual.text_tint;
        update.text_desc.enabled = true;
    }

    return update;
}
}
