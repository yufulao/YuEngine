// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderViewPacketStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Explicit RenderViewPacket operation status.
 */
enum class RenderViewPacketStatus {
    Success,
    InvalidArgument,
    InvalidViewId,
    InvalidFrameId,
    InvalidTarget,
    InvalidCaptureStorage,
    MismatchedPassId,
    MismatchedMaterialId,
    MaterialFailed,
    DrawFailed,
    DuplicateViewId,
    ViewCapacityExceeded
};
}
