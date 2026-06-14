#pragma once

#include <cstdint>

namespace yuengine::input {
struct InputActionState final {
    bool is_pressed = false;
    bool changed_this_frame = false;
    std::int32_t axis_value = 0;
};
}
