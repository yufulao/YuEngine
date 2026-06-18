// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit RenderCore drawable frame pipeline statuses.
 */
enum class RenderDrawableFramePipelineStatus {
    Success,
    InvalidArgument,
    InvalidFrameId,
    InvalidPassId,
    InvalidMaterialId,
    InvalidSwapchain,
    FrameRecordCapacityExceeded,
    RhiFailure,
    MaterialBindingFailed,
    FramePacketFailed
};
}
