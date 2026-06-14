#pragma once

#include <array>
#include <cstddef>

#include "yuengine/input/input_constants.h"
#include "yuengine/input/input_event.h"

namespace yuengine::input {
struct input_replay_frame_t final {
    std::array<input_event_t, MAX_EVENTS_PER_FRAME> Events{};
    std::size_t EventCount = 0U;
};
}
