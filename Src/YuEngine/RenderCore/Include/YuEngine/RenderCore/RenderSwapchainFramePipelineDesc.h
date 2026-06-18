// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipelineDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderSwapchainFramePipelineConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Describes bounded storage for swapchain frame pipeline records and commands.
 */
struct RenderSwapchainFramePipelineDesc final {
    std::size_t frame_record_capacity = MAX_RENDER_SWAPCHAIN_FRAME_RECORDS;
    std::size_t command_capacity = DEFAULT_RENDER_SWAPCHAIN_FRAME_COMMAND_CAPACITY;
};
}
