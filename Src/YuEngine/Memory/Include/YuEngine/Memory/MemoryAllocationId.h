#pragma once

#include <cstdint>

namespace yuengine::memory {
struct MemoryAllocationId {
    std::uint64_t value;

    bool IsValid() const {
        return value != 0U;
    }
};
}
