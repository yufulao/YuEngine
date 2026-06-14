#pragma once

#include <cstdint>

namespace yuengine::input {
struct input_action_state_t final {
    bool IsPressed = false;
    bool ChangedThisFrame = false;
    std::int32_t AxisValue = 0;
};
}
