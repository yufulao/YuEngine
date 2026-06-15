// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Include/YuEngine/Platform/FixedFrameClock.h

#pragma once

#include <cstdint>

#include "YuEngine/Platform/IFrameClock.h"

namespace yuengine::platform {
class FixedFrameClock final : public IFrameClock {
public:
    /**
     * @comment Constructs a FixedFrameClock instance.
     * @param first_tick_nanoseconds Input first tick nanoseconds.
     * @param step_nanoseconds Input step nanoseconds.
     */
    FixedFrameClock(std::uint64_t first_tick_nanoseconds, std::uint64_t step_nanoseconds);

    /**
     * @comment Returns the next tick time in nanoseconds.
     * @return Next tick nanoseconds value.
     */
    std::uint64_t NextTickNanoseconds() override;

private:
    std::uint64_t next_tick_nanoseconds_;
    std::uint64_t step_nanoseconds_;
};
}
