// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipelineSnapshot.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h"
#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainSnapshot.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 固定容量 RenderCore swapchain frame pipeline 计数器 和 last 状态 值。
 */
struct RenderSwapchainFramePipelineSnapshot final {
    std::size_t frame_record_capacity = 0U;
    std::size_t frame_record_count = 0U;
    std::size_t command_capacity = 0U;
    std::size_t required_command_count = 0U;
    std::size_t required_frame_record_count = 0U;
    std::size_t last_failed_command_index = 0U;
    std::size_t last_failed_frame_record_index = 0U;
    std::uint32_t last_failed_frame_id = 0U;
    std::uint64_t accepted_frame_count = 0U;
    std::uint64_t completed_frame_count = 0U;
    std::uint64_t failed_validation_count = 0U;
    std::uint64_t rhi_failure_count = 0U;
    std::uint64_t duplicate_frame_id_count = 0U;
    std::uint64_t command_capacity_rejected_count = 0U;
    std::uint64_t frame_record_capacity_rejected_count = 0U;
    std::uint64_t resize_request_count = 0U;
    std::uint64_t resized_frame_count = 0U;
    std::uint64_t capture_count = 0U;
    std::uint32_t last_frame_id = 0U;
    std::size_t last_recorded_command_count = 0U;
    std::size_t last_capture_bytes_written = 0U;
    yuengine::rhi::RhiExtent2D last_capture_extent{};
    std::size_t last_failed_capture_byte_capacity = 0U;
    std::size_t last_failed_capture_current_byte_count = 0U;
    std::size_t last_failed_capture_required_byte_count = 0U;
    yuengine::rhi::RhiExtent2D last_failed_capture_extent{};
    yuengine::rhi::RhiTextureHandle last_failed_capture_target{};
    RenderSwapchainFramePipelineStatus last_status = RenderSwapchainFramePipelineStatus::InvalidArgument;
    yuengine::rhi::RhiStatus last_rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    yuengine::rhi::RhiSwapchainSnapshot last_swapchain_snapshot{};
};
}
