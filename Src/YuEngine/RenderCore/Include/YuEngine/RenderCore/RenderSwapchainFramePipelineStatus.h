// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义 explicit RenderCore swapchain frame 流水线状态es.
 */
enum class RenderSwapchainFramePipelineStatus {
    Success,
    InvalidArgument,
    InvalidFrameId,
    DuplicateFrameId,
    InvalidSwapchain,
    InsufficientCaptureStorage,
    CommandCapacityExceeded,
    FrameRecordCapacityExceeded,
    RhiFailure
};
}
