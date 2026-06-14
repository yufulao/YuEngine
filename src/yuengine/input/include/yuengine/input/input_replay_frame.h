#pragma once

#include <array>
#include <cstddef>

#include "yuengine/input/input_constants.h"
#include "yuengine/input/input_event.h"

namespace yuengine::input {
struct InputReplayFrame final {
    std::array<InputEvent, MAX_EVENTS_PER_FRAME> Events{};
    std::size_t EventCount = 0U;
};
}
