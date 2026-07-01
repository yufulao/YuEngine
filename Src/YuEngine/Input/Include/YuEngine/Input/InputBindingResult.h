// 模块: YuEngine Input
// 文件: Src/YuEngine/Input/Include/YuEngine/Input/InputBindingResult.h

#pragma once

#include <cstddef>

#include "YuEngine/Input/InputActionId.h"
#include "YuEngine/Input/InputControlId.h"
#include "YuEngine/Input/InputDeviceId.h"
#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputBindingResult final {
    InputStatus status = InputStatus::Success;
    InputActionId action{};
    InputDeviceId device{};
    InputControlId control{};
    std::size_t binding_capacity = 0U;
    std::size_t binding_count = 0U;
    std::size_t required_binding_count = 0U;
};
}
