#pragma once

#include <array>
#include <cstddef>

#include "yuengine/memory/memory_allocation_id.h"

namespace yuengine::memory {
constexpr std::size_t MAX_MEMORY_OWNER_ID_BYTES = 64U;
constexpr std::size_t MAX_MEMORY_TAG_BYTES = 64U;

struct active_allocation_record_t final {
    bool IsActive = false;
    memory_allocation_id_t AllocationId{};
    std::size_t Bytes = 0U;
    std::array<char, MAX_MEMORY_OWNER_ID_BYTES> Owner{};
    std::size_t OwnerLength = 0U;
    std::array<char, MAX_MEMORY_TAG_BYTES> Tag{};
    std::size_t TagLength = 0U;
};
}
