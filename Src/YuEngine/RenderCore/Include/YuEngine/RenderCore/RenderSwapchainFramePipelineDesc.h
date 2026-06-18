// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipelineDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderSwapchainFramePipelineConstants.h"

namespace yuengine::rendercore {
/**
 * @comment 描述 固定容量 存储 用于 swapchain frame pipeline 记录 和 commands.
 */
struct RenderSwapchainFramePipelineDesc final {
    std::size_t frame_record_capacity = MAX_RENDER_SWAPCHAIN_FRAME_RECORDS;
    std::size_t command_capacity = DEFAULT_RENDER_SWAPCHAIN_FRAME_COMMAND_CAPACITY;
};
}
