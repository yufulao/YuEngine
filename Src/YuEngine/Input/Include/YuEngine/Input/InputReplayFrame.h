// Module: YuEngine Input
// File: Src/YuEngine/Input/Include/YuEngine/Input/InputReplayFrame.h

#pragma once

#include <array>
#include <cstddef>

#include "YuEngine/Input/InputConstants.h"
#include "YuEngine/Input/InputEvent.h"

namespace yuengine::input {
struct InputReplayFrame final {
    std::array<InputEvent, MAX_EVENTS_PER_FRAME> events{};
    std::size_t event_count = 0U;
};
}
