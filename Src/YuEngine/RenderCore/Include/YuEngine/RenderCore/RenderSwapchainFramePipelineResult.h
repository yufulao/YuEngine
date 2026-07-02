// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipelineResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h"
#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainResizeResult.h"
#include "YuEngine/Rhi/RhiSwapchainSnapshot.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 一个 RenderCore swapchain frame pipeline 结果。
 */
struct RenderSwapchainFramePipelineResult final {
    RenderSwapchainFramePipelineStatus status = RenderSwapchainFramePipelineStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    yuengine::rhi::RhiSwapchainResizeResult resize_result{};
    yuengine::rhi::RhiSwapchainSnapshot swapchain_snapshot{};
    yuengine::rhi::RhiTextureHandle target{};
    std::uint32_t frame_id = 0U;
    std::size_t recorded_command_count = 0U;
    std::size_t required_command_count = 0U;
    std::size_t required_frame_record_count = 0U;
    std::size_t failed_command_index = 0U;
    std::size_t failed_frame_record_index = 0U;
    std::uint32_t failed_frame_id = 0U;
    std::size_t capture_bytes_written = 0U;
    std::size_t capture_byte_capacity = 0U;
    std::size_t capture_current_byte_count = 0U;
    std::size_t capture_required_byte_count = 0U;
    yuengine::rhi::RhiExtent2D capture_extent{};
    yuengine::rhi::RhiTextureHandle capture_target{};
    bool resize_requested = false;
    bool resized = false;
};
}
