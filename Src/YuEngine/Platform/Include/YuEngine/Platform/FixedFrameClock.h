#pragma once

#include <cstdint>

#include "YuEngine/Platform/IFrameClock.h"

namespace yuengine::platform {
class FixedFrameClock final : public IFrameClock {
public:
    FixedFrameClock(std::uint64_t first_tick_nanoseconds, std::uint64_t step_nanoseconds);

    std::uint64_t NextTickNanoseconds() override;

private:
    std::uint64_t next_tick_nanoseconds_;
    std::uint64_t step_nanoseconds_;
};
}
