// Module: YuEngine Memory
// File: Src/YuEngine/Memory/Include/YuEngine/Memory/ActiveAllocationRecord.h

#pragma once

#include <array>
#include <cstddef>

#include "YuEngine/Memory/MemoryAllocationId.h"

namespace yuengine::memory {
constexpr std::size_t MAX_MEMORY_OWNER_ID_BYTES = 64U;
constexpr std::size_t MAX_MEMORY_TAG_BYTES = 64U;

struct ActiveAllocationRecord final {
    bool is_active = false;
    MemoryAllocationId allocation_id{};
    std::size_t bytes = 0U;
    std::array<char, MAX_MEMORY_OWNER_ID_BYTES> owner{};
    std::size_t owner_length = 0U;
    std::array<char, MAX_MEMORY_TAG_BYTES> tag{};
    std::size_t tag_length = 0U;
};
}
