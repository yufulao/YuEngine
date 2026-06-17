// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderFramePacketFixtureStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Defines explicit RenderCore frame packet fixture status values.
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
