// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawableFramePipelineSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainSnapshot.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 固定容量 drawable frame pipeline 计数器 和 last 状态 值。
 */
struct RenderDrawableFramePipelineSnapshot final {
    std::size_t frame_record_capacity = 0U;
    std::size_t frame_record_count = 0U;
    std::uint64_t accepted_frame_count = 0U;
    std::uint64_t completed_frame_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t rhi_failure_count = 0U;
    std::uint64_t material_failure_count = 0U;
    std::uint64_t view_packet_failure_count = 0U;
    std::uint64_t frame_packet_failure_count = 0U;
    std::uint64_t frame_record_capacity_rejected_count = 0U;
    std::uint64_t capture_count = 0U;
    std::uint32_t last_frame_id = 0U;
    std::uint32_t last_pass_id = 0U;
    std::uint32_t last_material_id = 0U;
    std::size_t last_required_frame_record_count = 0U;
    std::uint32_t last_failed_frame_record_frame_id = 0U;
    std::uint32_t last_failed_frame_record_pass_id = 0U;
    std::uint32_t last_failed_frame_record_material_id = 0U;
    std::size_t last_failed_frame_record_current_count = 0U;
    std::size_t last_failed_frame_record_index = 0U;
    std::size_t last_failed_frame_record_capacity = 0U;
    std::size_t last_failed_frame_record_required_count = 0U;
    std::size_t last_failed_pass_count = 0U;
    std::size_t last_failed_draw_count = 0U;
    std::size_t last_recorded_command_count = 0U;
    std::size_t last_capture_bytes_written = 0U;
    yuengine::rhi::RhiExtent2D last_capture_extent{};
    RenderDrawableFramePipelineStatus last_status = RenderDrawableFramePipelineStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    yuengine::rhi::RhiSwapchainSnapshot last_swapchain_snapshot{};
};
}
