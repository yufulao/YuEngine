#pragma once

#include <cstddef>

#include "yuengine/input/input_status.h"

namespace yuengine::input {
struct InputApplyResult final {
    INPUT_STATUS Status = INPUT_STATUS::Success;
    std::size_t FrameIndex = 0U;
};
}
