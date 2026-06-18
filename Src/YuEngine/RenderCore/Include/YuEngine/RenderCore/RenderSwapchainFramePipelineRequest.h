// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipelineRequest.h

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "YuEngine/Rhi/IRhiDevice.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiSwapchainResizeRequest.h"

namespace yuengine::rendercore {
/**
 * @comment 描述提交到 RHI swapchain backbuffer 的一个 RenderCore 帧。
 */
struct RenderSwapchainFramePipelineRequest final {
    yuengine::rhi::IRhiDevice *rhi_device = nullptr;
    yuengine::rhi::RhiColor clear_color{};
    yuengine::rhi::RhiSwapchainResizeRequest resize_request{};
    std::span<std::uint8_t> capture_output{};
    std::size_t capture_byte_budget = 0U;
    std::uint32_t frame_id = 0U;
    bool resize_before_submit = false;
};
}
