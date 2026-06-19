// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputCommandBinding.h

#pragma once

#include "YuEngine/Input/InputActionId.h"
#include "YuEngine/Input/InputCommandValueKind.h"
#include "YuEngine/Input/InputContextId.h"
#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceId.h"

namespace yuengine::input {
struct InputCommandBinding final {
    InputContextId context;
    InputDeviceId device;
    InputControlId control;
    InputActionId action;
    InputCommandValueKind value_kind = InputCommandValueKind::Button;
};
}
