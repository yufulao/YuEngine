// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawableFramePipelineStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义 explicit RenderCore drawable frame 流水线状态es.
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
    ViewPacketFailed,
    FramePacketFailed
};
}
