// Module: YuEngine Platform
// File: Src/YuEngine/Platform/Include/YuEngine/Platform/IFrameClock.h

#pragma once

#include <cstdint>

namespace yuengine::platform {
class IFrameClock {
public:
    virtual ~IFrameClock() = default;

    /**
     * @comment Returns the next tick time in nanoseconds.
     * @return Next tick nanoseconds value.
     */
    virtual std::uint64_t NextTickNanoseconds() = 0;
};
}
