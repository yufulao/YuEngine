#pragma once

#include <cstdint>

namespace yuengine::platform {
class IFrameClock {
public:
    virtual ~IFrameClock() = default;

    virtual std::uint64_t NextTickNanoseconds() = 0;
};
}
