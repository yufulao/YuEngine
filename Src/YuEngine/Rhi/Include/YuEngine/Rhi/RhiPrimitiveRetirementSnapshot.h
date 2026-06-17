// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPrimitiveRetirementSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

namespace yuengine::rhi {
struct RhiPrimitiveRetirementSnapshot final {
    std::size_t capacity = 0U;
    std::size_t pending_count = 0U;
    std::uint64_t requested_count = 0U;
    std::uint64_t drained_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint64_t duplicate_request_count = 0U;
    std::uint64_t invalid_handle_count = 0U;
    std::uint64_t wrong_kind_count = 0U;
    std::uint64_t capacity_rejected_count = 0U;
    std::uint64_t fence_not_ready_count = 0U;
    std::uint64_t next_retirement_id = 0U;
};
}
