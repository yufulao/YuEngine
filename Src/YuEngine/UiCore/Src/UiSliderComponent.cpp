// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Src/UiSliderComponent.cpp

#include "YuEngine/UiCore/UiSliderComponent.h"

#include "YuEngine/UiCore/UiNodeTreeResult.h"

namespace yuengine::uicore {
namespace {
constexpr float FLOAT_EPSILON = 0.0001F;

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

bool IsVisualValid(const UiSliderVisualDesc &visual) {
    if (visual.sprite_key == 0U) {
        return false;
    }

    return IsTintValid(visual.tint);
}

bool ValuesDiffer(float left, float right) {
    float diff = left - right;
    if (diff < 0.0F) {
        diff = -diff;
    }

    return diff > FLOAT_EPSILON;
}

float Clamp(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }

    if (value > max_value) {
        return max_value;
    }

    return value;
}

float RoundToNearest(float value) {
    if (value >= 0.0F) {
        return static_cast<float>(static_cast<int>(value + 0.5F));
    }

    return static_cast<float>(static_cast<int>(value - 0.5F));
}

float ResolveSteppedValue(float value, float min_value, float max_value, float step_size) {
    const float clamped_value = Clamp(value, min_value, max_value);
    if (step_size <= 0.0F) {
        return clamped_value;
    }

    const float step_count = RoundToNearest((clamped_value - min_value) / step_size);
    const float stepped_value = min_value + (step_count * step_size);
    return Clamp(stepped_value, min_value, max_value);
}

float NormalizeValue(float value, float min_value, float max_value) {
    const float range = max_value - min_value;
    if (range <= 0.0F) {
        return 0.0F;
    }

    return Clamp((value - min_value) / range, 0.0F, 1.0F);
}

float ResolveHorizontalPointerValue(const UiSliderComponentDesc &desc, const UiRect &track_rect) {
    const float normalized_value = Clamp((desc.interaction.pointer_position.x - track_rect.x) / track_rect.width, 0.0F, 1.0F);
    return desc.min_value + ((desc.max_value - desc.min_value) * normalized_value);
}

float ResolveVerticalPointerValue(const UiSliderComponentDesc &desc, const UiRect &track_rect) {
    const float normalized_value = Clamp((desc.interaction.pointer_position.y - track_rect.y) / track_rect.height, 0.0F, 1.0F);
    return desc.min_value + ((desc.max_value - desc.min_value) * normalized_value);
}

UiRect BuildHorizontalFillRect(const UiRect &track_rect, float normalized_value) {
    UiRect rect{};
    rect.x = track_rect.x;
    rect.y = track_rect.y;
    rect.width = track_rect.width * normalized_value;
    rect.height = track_rect.height;
    return rect;
}

UiRect BuildVerticalFillRect(const UiRect &track_rect, float normalized_value) {
    UiRect rect{};
    rect.x = track_rect.x;
    rect.y = track_rect.y;
    rect.width = track_rect.width;
    rect.height = track_rect.height * normalized_value;
    return rect;
}

UiVector2 BuildHorizontalHandleCenter(const UiRect &track_rect, float normalized_value) {
    UiVector2 center{};
    center.x = track_rect.x + (track_rect.width * normalized_value);
    center.y = track_rect.y + (track_rect.height * 0.5F);
    return center;
}

UiVector2 BuildVerticalHandleCenter(const UiRect &track_rect, float normalized_value) {
    UiVector2 center{};
    center.x = track_rect.x + (track_rect.width * 0.5F);
    center.y = track_rect.y + (track_rect.height * normalized_value);
    return center;
}

UiRect BuildHandleRect(const UiRect &handle_rect, const UiVector2 &center) {
    UiRect rect{};
    rect.width = handle_rect.width;
    rect.height = handle_rect.height;
    rect.x = center.x - (handle_rect.width * 0.5F);
    rect.y = center.y - (handle_rect.height * 0.5F);
    return rect;
}

void SetFailure(UiSliderComponentResult *out_result, UiSliderComponentStatus status) {
    out_result->status = status;
}
}

UiSliderComponentStatus UiSliderComponent::Build(
    const UiNodeTree &tree,
    const UiSliderComponentDesc &desc,
    UiSliderComponentResult *out_result) const {
    if (out_result == nullptr) {
        return UiSliderComponentStatus::InvalidOutputBuffer;
    }

    *out_result = UiSliderComponentResult{};
    out_result->node_id = desc.node_id;
    out_result->previous_value = desc.value;
    out_result->keyboard_adjustment_path = desc.interaction.keyboard_adjustment_delta != 0.0F;
    out_result->gamepad_adjustment_path = desc.interaction.gamepad_adjustment_delta != 0.0F;

    UiSliderComponentStatus status = ValidateDesc(desc);
    if (status != UiSliderComponentStatus::Success) {
        SetFailure(out_result, status);
        return status;
    }

    const UiNodeTreeResult track_result = tree.QueryNode(desc.node_id);
    if (!track_result.Succeeded()) {
        SetFailure(out_result, UiSliderComponentStatus::NodeNotFound);
        return UiSliderComponentStatus::NodeNotFound;
    }

    const UiNodeTreeResult fill_result = tree.QueryNode(desc.fill_node_id);
    if (!fill_result.Succeeded()) {
        SetFailure(out_result, UiSliderComponentStatus::NodeNotFound);
        return UiSliderComponentStatus::NodeNotFound;
    }

    const UiNodeTreeResult handle_result = tree.QueryNode(desc.handle_node_id);
    if (!handle_result.Succeeded()) {
        SetFailure(out_result, UiSliderComponentStatus::NodeNotFound);
        return UiSliderComponentStatus::NodeNotFound;
    }

    status = ValidateGeometry(desc, track_result.record, fill_result.record, handle_result.record);
    if (status != UiSliderComponentStatus::Success) {
        SetFailure(out_result, status);
        return status;
    }

    UiSliderAdjustmentSource adjustment_source = UiSliderAdjustmentSource::None;
    float resolved_value = ResolveValueFromInput(desc, track_result.record, &adjustment_source);
    if (!IsSliderEnabled(desc, track_result.record)) {
        resolved_value = ResolveSteppedValue(desc.value, desc.min_value, desc.max_value, desc.step_size);
        adjustment_source = UiSliderAdjustmentSource::None;
    }

    const float normalized_value = NormalizeValue(resolved_value, desc.min_value, desc.max_value);
    out_result->status = UiSliderComponentStatus::Success;
    out_result->adjustment_source = adjustment_source;
    out_result->resolved_value = resolved_value;
    out_result->normalized_value = normalized_value;
    out_result->pointer_capture_active = ShouldCapturePointer(desc) && IsSliderEnabled(desc, track_result.record);
    out_result->visual_update = BuildVisualUpdate(desc, track_result.record, handle_result.record, normalized_value);
    if (out_result->pointer_capture_active) {
        out_result->captured_node_id = desc.node_id;
    }

    out_result->value_changed = ValuesDiffer(desc.value, resolved_value);
    if (out_result->value_changed) {
        out_result->value_changed_event_key = desc.hooks.value_changed_event_key;
    }

    return UiSliderComponentStatus::Success;
}

UiSliderComponentStatus UiSliderComponent::ValidateDesc(const UiSliderComponentDesc &desc) const {
    if (!desc.node_id.IsValid()) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if (!desc.fill_node_id.IsValid()) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if (!desc.handle_node_id.IsValid()) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if (desc.max_value <= desc.min_value) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if ((desc.value < desc.min_value) || (desc.value > desc.max_value)) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if (desc.step_size < 0.0F) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if (desc.axis != UiSliderAxis::Horizontal && desc.axis != UiSliderAxis::Vertical) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if (!IsVisualValid(desc.fill_visual)) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if (!IsVisualValid(desc.handle_visual)) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    return UiSliderComponentStatus::Success;
}

UiSliderComponentStatus UiSliderComponent::ValidateGeometry(
    const UiSliderComponentDesc &desc,
    const UiNodeRecord &track_record,
    const UiNodeRecord &fill_record,
    const UiNodeRecord &handle_record) const {
    if ((fill_record.world_rect.width < 0.0F) || (fill_record.world_rect.height < 0.0F)) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if ((handle_record.world_rect.width <= 0.0F) || (handle_record.world_rect.height <= 0.0F)) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    if (desc.axis == UiSliderAxis::Vertical) {
        if (track_record.world_rect.height <= 0.0F) {
            return UiSliderComponentStatus::InvalidDesc;
        }

        return UiSliderComponentStatus::Success;
    }

    if (track_record.world_rect.width <= 0.0F) {
        return UiSliderComponentStatus::InvalidDesc;
    }

    return UiSliderComponentStatus::Success;
}

bool UiSliderComponent::IsSliderEnabled(const UiSliderComponentDesc &desc, const UiNodeRecord &record) const {
    if (!desc.is_enabled) {
        return false;
    }

    if (!record.is_enabled) {
        return false;
    }

    return record.is_visible;
}

bool UiSliderComponent::ShouldCapturePointer(const UiSliderComponentDesc &desc) const {
    if (!desc.interaction.pointer_is_down) {
        return false;
    }

    if (desc.interaction.pointer_capture_active) {
        return true;
    }

    if (desc.interaction.pointer_pressed_on_slider) {
        return true;
    }

    return desc.interaction.hit_node_id.value == desc.node_id.value;
}

float UiSliderComponent::ResolveValueFromInput(
    const UiSliderComponentDesc &desc,
    const UiNodeRecord &track_record,
    UiSliderAdjustmentSource *out_source) const {
    float value = desc.value;
    *out_source = UiSliderAdjustmentSource::None;

    if (ShouldCapturePointer(desc)) {
        if (desc.axis == UiSliderAxis::Vertical) {
            value = ResolveVerticalPointerValue(desc, track_record.world_rect);
        }

        if (desc.axis == UiSliderAxis::Horizontal) {
            value = ResolveHorizontalPointerValue(desc, track_record.world_rect);
        }

        *out_source = UiSliderAdjustmentSource::Pointer;
        return ResolveSteppedValue(value, desc.min_value, desc.max_value, desc.step_size);
    }

    if (desc.interaction.keyboard_adjustment_delta != 0.0F) {
        value += desc.interaction.keyboard_adjustment_delta;
        *out_source = UiSliderAdjustmentSource::Keyboard;
        return ResolveSteppedValue(value, desc.min_value, desc.max_value, desc.step_size);
    }

    if (desc.interaction.gamepad_adjustment_delta != 0.0F) {
        value += desc.interaction.gamepad_adjustment_delta;
        *out_source = UiSliderAdjustmentSource::Gamepad;
        return ResolveSteppedValue(value, desc.min_value, desc.max_value, desc.step_size);
    }

    return ResolveSteppedValue(value, desc.min_value, desc.max_value, desc.step_size);
}

UiSliderVisualUpdate UiSliderComponent::BuildVisualUpdate(
    const UiSliderComponentDesc &desc,
    const UiNodeRecord &track_record,
    const UiNodeRecord &handle_record,
    float normalized_value) const {
    UiSliderVisualUpdate update{};
    update.has_fill_update = true;
    update.has_handle_update = true;
    update.fill_image_desc.node_id = desc.fill_node_id;
    update.fill_image_desc.sprite_key = desc.fill_visual.sprite_key;
    update.fill_image_desc.style_key = desc.fill_visual.style_key;
    update.fill_image_desc.material_key = desc.fill_visual.material_key;
    update.fill_image_desc.tint = desc.fill_visual.tint;
    update.fill_image_desc.scissor_enabled = desc.scissor_enabled;
    update.handle_image_desc.node_id = desc.handle_node_id;
    update.handle_image_desc.sprite_key = desc.handle_visual.sprite_key;
    update.handle_image_desc.style_key = desc.handle_visual.style_key;
    update.handle_image_desc.material_key = desc.handle_visual.material_key;
    update.handle_image_desc.tint = desc.handle_visual.tint;
    update.handle_image_desc.scissor_enabled = desc.scissor_enabled;

    if (desc.axis == UiSliderAxis::Vertical) {
        update.fill_rect = BuildVerticalFillRect(track_record.world_rect, normalized_value);
        update.handle_center = BuildVerticalHandleCenter(track_record.world_rect, normalized_value);
        update.handle_rect = BuildHandleRect(handle_record.world_rect, update.handle_center);
        return update;
    }

    update.fill_rect = BuildHorizontalFillRect(track_record.world_rect, normalized_value);
    update.handle_center = BuildHorizontalHandleCenter(track_record.world_rect, normalized_value);
    update.handle_rect = BuildHandleRect(handle_record.world_rect, update.handle_center);
    return update;
}
}
