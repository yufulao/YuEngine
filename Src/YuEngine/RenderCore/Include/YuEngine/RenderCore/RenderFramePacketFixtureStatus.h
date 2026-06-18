// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixtureStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义 explicit RenderCore frame packet fixture 状态 值.
 */
enum class RenderFramePacketFixtureStatus {
    Success,
    InvalidArgument,
    InvalidFrameId,
    InvalidBatchRequest,
    DuplicateFrameId,
    PacketCapacityExceeded,
    SubmissionBatchFailed
};
}
