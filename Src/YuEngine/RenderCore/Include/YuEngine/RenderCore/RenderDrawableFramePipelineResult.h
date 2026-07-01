// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawableFramePipelineResult.h

#pragma once

#include <cstddef>
#include <cstdint>

#include "YuEngine/RenderCore/MaterialBindingFixtureResult.h"
#include "YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h"
#include "YuEngine/RenderCore/RenderFixturePassResult.h"
#include "YuEngine/RenderCore/RenderFramePacketFixtureResult.h"
#include "YuEngine/RenderCore/RenderViewPacketResult.h"
#include "YuEngine/Rhi/RhiExtent2D.h"
#include "YuEngine/Rhi/RhiStatus.h"
#include "YuEngine/Rhi/RhiSwapchainSnapshot.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"

namespace yuengine::rendercore {
/**
 * @comment 包含 一个 drawable frame pipeline 结果。
 */
struct RenderDrawableFramePipelineResult final {
    RenderDrawableFramePipelineStatus status = RenderDrawableFramePipelineStatus::InvalidArgument;
    yuengine::rhi::RhiStatus rhi_status = yuengine::rhi::RhiStatus::InvalidLifecycle;
    RenderViewPacketResult view_result{};
    MaterialBindingFixtureResult material_result{};
    RenderFramePacketFixtureResult frame_result{};
    RenderFixturePassResult pass_result{};
    yuengine::rhi::RhiSwapchainSnapshot swapchain_snapshot{};
    yuengine::rhi::RhiTextureHandle target{};
    std::uint32_t frame_id = 0U;
    std::uint32_t pass_id = 0U;
    std::uint32_t material_id = 0U;
    std::size_t required_frame_record_count = 0U;
    std::size_t recorded_command_count = 0U;
    std::size_t capture_bytes_written = 0U;
    yuengine::rhi::RhiExtent2D capture_extent{};
};
}
