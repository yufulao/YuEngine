// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiSliderInteractionDesc.h

#pragma once

#include "YuEngine/UiCore/UiNodeId.h"
#include "YuEngine/UiCore/UiVector2.h"

namespace yuengine::uicore {
struct UiSliderInteractionDesc final {
    UiNodeId hit_node_id;
    UiVector2 pointer_position;
    float keyboard_adjustment_delta = 0.0F;
    float gamepad_adjustment_delta = 0.0F;
    bool pointer_is_down = false;
    bool pointer_pressed_on_slider = false;
    bool pointer_capture_active = false;
};
}
