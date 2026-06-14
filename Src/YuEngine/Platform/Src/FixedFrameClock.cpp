#include "YuEngine/Platform/FixedFrameClock.h"

namespace yuengine::platform {
FixedFrameClock::FixedFrameClock(std::uint64_t firstTickNanoseconds, std::uint64_t stepNanoseconds)
    : next_tick_nanoseconds_(firstTickNanoseconds),
      step_nanoseconds_(stepNanoseconds) {
}

std::uint64_t FixedFrameClock::NextTickNanoseconds() {
    const std::uint64_t currentTickNanoseconds = next_tick_nanoseconds_;
    next_tick_nanoseconds_ += step_nanoseconds_;
    return currentTickNanoseconds;
}
}
