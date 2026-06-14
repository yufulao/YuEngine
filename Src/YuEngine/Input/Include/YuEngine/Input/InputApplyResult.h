#pragma once

#include <cstddef>

#include "YuEngine/Input/InputStatus.h"

namespace yuengine::input {
struct InputApplyResult final {
    InputStatus status = InputStatus::Success;
    std::size_t frame_index = 0U;
};
}
