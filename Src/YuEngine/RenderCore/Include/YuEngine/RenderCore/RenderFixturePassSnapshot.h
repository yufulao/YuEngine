// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFixturePassSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderFixturePassStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Contains bounded fixture pass counters and the last observed result.
 */
struct RenderFixturePassSnapshot final {
    std::size_t pass_record_capacity = 0U;
    std::size_t pass_record_count = 0U;
    std::size_t command_capacity = 0U;
    std::size_t required_command_count = 0U;
    std::uint64_t executed_pass_count = 0U;
    std::uint64_t completed_pass_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t rhi_failure_count = 0U;
    std::uint64_t command_capacity_rejected_count = 0U;
    std::uint64_t pass_record_capacity_rejected_count = 0U;
    std::size_t last_recorded_command_count = 0U;
    std::size_t last_capture_bytes_written = 0U;
    std::uint32_t last_pass_id = 0U;
    RenderFixturePassStatus last_status = RenderFixturePassStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
};
}
