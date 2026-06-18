// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawableFramePipelineSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainSnapshot.h"

namespace yuengine::rendercore {
/**
 * @comment Contains bounded drawable frame pipeline counters and last status values.
 */
struct RenderDrawableFramePipelineSnapshot final {
    std::size_t frame_record_capacity = 0U;
    std::size_t frame_record_count = 0U;
    std::uint64_t accepted_frame_count = 0U;
    std::uint64_t completed_frame_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t rhi_failure_count = 0U;
    std::uint64_t material_failure_count = 0U;
    std::uint64_t frame_packet_failure_count = 0U;
    std::uint64_t frame_record_capacity_rejected_count = 0U;
    std::uint64_t capture_count = 0U;
    std::uint32_t last_frame_id = 0U;
    std::uint32_t last_pass_id = 0U;
    std::uint32_t last_material_id = 0U;
    std::size_t last_recorded_command_count = 0U;
    std::size_t last_capture_bytes_written = 0U;
    RenderDrawableFramePipelineStatus last_status = RenderDrawableFramePipelineStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    yuengine::rhi::RhiSwapchainSnapshot last_swapchain_snapshot{};
};
}
