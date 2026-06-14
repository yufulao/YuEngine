#pragma once

#include <cstddef>

#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct input_apply_result_t final {
    InputStatus Status = InputStatus::Success;
    std::size_t FrameIndex = 0U;
};
}
