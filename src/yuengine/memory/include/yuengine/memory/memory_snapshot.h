#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::memory {
struct memory_snapshot_t {
    std::uint64_t AllocationCount;
    std::uint64_t FreeCount;
    std::size_t RetainedBytes;
    std::size_t PeakRetainedBytes;
    std::size_t LeakCount;

    bool HasLeaks() const {
        if (RetainedBytes != 0U) {
            return true;
        }

        return LeakCount != 0U;
    }
};
}
