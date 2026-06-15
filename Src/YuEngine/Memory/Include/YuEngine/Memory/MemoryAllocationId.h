// Module: YuEngine Memory
// File: Src/YuEngine/Memory/Include/YuEngine/Memory/MemoryAllocationId.h

#pragma once

#include <cstdint>

namespace yuengine::memory {
struct MemoryAllocationId {
    std::uint64_t value;

    /**
     * @comment Checks whether the value is valid.
     * @return True when the condition is satisfied; false otherwise.
     */
    bool IsValid() const {
        return value != 0U;
    }
};
}
