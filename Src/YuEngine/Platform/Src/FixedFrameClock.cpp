#include "YuEngine/Platform/FixedFrameClock.h"

namespace yuengine::platform {
FixedFrameClock::FixedFrameClock(std::uint64_t firstTickNanoseconds, std::uint64_t stepNanoseconds)
    : _nextTickNanoseconds(firstTickNanoseconds),
      _stepNanoseconds(stepNanoseconds) {
}

std::uint64_t FixedFrameClock::NextTickNanoseconds() {
    const std::uint64_t currentTickNanoseconds = _nextTickNanoseconds;
    _nextTickNanoseconds += _stepNanoseconds;
    return currentTickNanoseconds;
}
}
