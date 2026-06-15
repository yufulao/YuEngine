// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Src/FixedFrameClock.cpp

#include "YuEngine/Platform/FixedFrameClock.h"

namespace yuengine::platform {
FixedFrameClock::FixedFrameClock(std::uint64_t first_tick_nanoseconds, std::uint64_t step_nanoseconds)
    : next_tick_nanoseconds_(first_tick_nanoseconds),
      step_nanoseconds_(step_nanoseconds) {
}

std::uint64_t FixedFrameClock::NextTickNanoseconds() {
    const std::uint64_t current_tick_nanoseconds = next_tick_nanoseconds_;
    next_tick_nanoseconds_ += step_nanoseconds_;
    return current_tick_nanoseconds;
}
}
