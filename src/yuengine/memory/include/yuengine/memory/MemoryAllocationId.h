#pragma once

#include <cstdint>

namespace yuengine::memory
{
struct MemoryAllocationId
{
    std::uint64_t Value;

    bool IsValid() const
    {
        return Value != 0U;
    }
};
}
