// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipelineResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainResizeResult.h"
#include "YuEngine/Rhi/RhiSwapchainSnapshot.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rendercore {
/**
 * @comment Contains one RenderCore swapchain frame pipeline result.
 */
struct RenderSwapchainFramePipelineResult final {
    RenderSwapchainFramePipelineStatus status = RenderSwapchainFramePipelineStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    yuengine::rhi::RhiSwapchainResizeResult resize_result{};
    yuengine::rhi::RhiSwapchainSnapshot swapchain_snapshot{};
    yuengine::rhi::RhiTextureHandle target{};
    std::uint32_t frame_id = 0U;
    std::size_t recorded_command_count = 0U;
    std::size_t capture_bytes_written = 0U;
    bool resize_requested = false;
    bool resized = false;
};
}
