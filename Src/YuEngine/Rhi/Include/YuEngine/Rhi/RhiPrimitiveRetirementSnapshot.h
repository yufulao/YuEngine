// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPrimitiveRetirementSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Rhi/RhiFenceHandle.h"
#include "YuEngine/Rhi/RhiPrimitiveKind.h"

namespace yuengine::rhi {
struct RhiPrimitiveRetirementSnapshot final {
    std::size_t capacity = 0U;
    std::size_t pending_count = 0U;
    std::size_t required_retirement_record_count = 0U;
    std::uint64_t requested_count = 0U;
    std::uint64_t drained_count = 0U;
    std::uint64_t rejected_count = 0U;
    std::uint64_t duplicate_request_count = 0U;
    std::uint64_t invalid_handle_count = 0U;
    std::uint64_t wrong_kind_count = 0U;
    std::uint64_t capacity_rejected_count = 0U;
    std::uint64_t fence_not_ready_count = 0U;
    std::uint64_t next_retirement_id = 0U;
    std::size_t last_failed_retirement_capacity = 0U;
    std::size_t last_failed_retirement_current_count = 0U;
    std::size_t last_failed_retirement_required_count = 0U;
    std::uint64_t last_failed_retirement_request_id = 0U;
    RhiPrimitiveKind last_failed_retirement_kind = RhiPrimitiveKind::Unsupported;
    std::uint32_t last_failed_retirement_slot = 0U;
    std::uint32_t last_failed_retirement_generation = 0U;
    RhiFenceHandle last_failed_retirement_wait_fence{};
};
}
