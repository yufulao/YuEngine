// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderViewPacketStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 显式 RenderViewPacket 操作状态。
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
