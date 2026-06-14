#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::memory {
struct MemorySnapshot {
    std::uint64_t allocation_count;
    std::uint64_t free_count;
    std::size_t retained_bytes;
    std::size_t peak_retained_bytes;
    std::size_t leak_count;

    bool HasLeaks() const {
        if (retained_bytes != 0U) {
            return true;
        }

        return leak_count != 0U;
    }
};
}
