// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderDrawPacketStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Explicit RenderDrawPacket operation status.
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
