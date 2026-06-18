// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacketStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 显式 RenderDrawPacket 操作状态。
 */
enum class RenderDrawPacketStatus {
    Success,
    InvalidArgument,
    InvalidDrawId,
    InvalidPassId,
    InvalidMaterialId,
    MissingVertexBuffer,
    MissingIndexBuffer,
    InvalidDraw,
    DuplicateDrawId,
    DrawCapacityExceeded
};
}
