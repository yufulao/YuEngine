#pragma once

#include <cstdint>

#include "YuEngine/Platform/IFrameClock.h"

namespace yuengine::platform {
class FixedFrameClock final : public IFrameClock {
public:
    FixedFrameClock(std::uint64_t firstTickNanoseconds, std::uint64_t stepNanoseconds);

    std::uint64_t NextTickNanoseconds() override;

private:
    std::uint64_t next_tick_nanoseconds_;
    std::uint64_t step_nanoseconds_;
};
}
