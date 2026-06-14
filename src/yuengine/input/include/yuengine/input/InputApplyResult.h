#pragma once

#include <cstddef>

#include "yuengine/input/InputStatus.h"

namespace yuengine::input {
struct InputApplyResult final {
    InputStatus Status = InputStatus::Success;
    std::size_t FrameIndex = 0U;
};
}
