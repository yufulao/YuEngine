// 模块: YuEngine UiCore
// 文件: Src/YuEngine/UiCore/Include/YuEngine/UiCore/UiButtonInteractionDesc.h

#pragma once

#include "YuEngine/UiCore/UiNodeId.h"

namespace yuengine::uicore {
struct UiButtonInteractionDesc final {
    UiNodeId hit_node_id;
    bool pointer_is_down = false;
    bool pointer_released = false;
    bool pointer_pressed_on_button = false;
    bool keyboard_activate_requested = false;
    bool gamepad_activate_requested = false;
};
}
