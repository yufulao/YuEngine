#pragma once

#include <array>
#include <cstddef>

#include "YuEngine/Input/InputConstants.h"
#include "YuEngine/Input/InputEvent.h"

namespace yuengine::input {
struct InputReplayFrame final {
    std::array<InputEvent, MAX_EVENTS_PER_FRAME> Events{};
    std::size_t EventCount = 0U;
};
}
