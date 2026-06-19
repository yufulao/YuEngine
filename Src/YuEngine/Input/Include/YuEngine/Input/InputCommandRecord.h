// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputCommandRecord.h

#pragma once

#include <cstdint>

#include "YuEngine/Input/InputActionId.h"
#include "YuEngine/Input/InputCommandValueKind.h"

namespace yuengine::input {
struct InputCommandRecord final {
    InputActionId action;
    InputCommandValueKind value_kind = InputCommandValueKind::Button;
    bool pressed_this_frame = false;
    bool released_this_frame = false;
    bool held = false;
    std::int32_t axis_value = 0;
};
}
