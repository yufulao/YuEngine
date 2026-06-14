#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/Rhi/RhiAccountingStatus.h"

namespace yuengine::rhi {
struct RhiDeviceSnapshot final {
    std::size_t color_target_capacity = 0U;
    std::size_t color_target_count = 0U;
    std::size_t command_storage_capacity_before_frame = 0U;
    std::size_t command_storage_capacity_after_last_frame = 0U;
    std::uint64_t created_target_count = 0U;
    std::uint64_t destroyed_target_count = 0U;
    std::uint64_t recorded_command_count = 0U;
    std::uint64_t submit_count = 0U;
    std::uint64_t present_count = 0U;
    std::uint64_t capture_count = 0U;
    std::uint64_t failed_operation_count = 0U;
    std::size_t last_capture_bytes_written = 0U;
    RhiAccountingStatus allocation_accounting_status = RhiAccountingStatus::DeferredUntilYuMemoryIntegration;
};
}
