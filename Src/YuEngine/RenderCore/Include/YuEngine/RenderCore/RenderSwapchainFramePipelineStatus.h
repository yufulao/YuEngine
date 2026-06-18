// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderSwapchainFramePipelineStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit RenderCore swapchain frame pipeline statuses.
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
