#pragma once

#include <cstdint>

#include "yuengine/platform/i_frame_clock.h"

namespace yuengine::platform {
class FixedFrameClock final : public IFrameClock {
public:
    FixedFrameClock(std::uint64_t firstTickNanoseconds, std::uint64_t stepNanoseconds);

    std::uint64_t NextTickNanoseconds() override;

private:
    std::uint64_t _nextTickNanoseconds;
    std::uint64_t _stepNanoseconds;
};
}
