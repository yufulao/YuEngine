// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentStatus.h

#pragma once

namespace yuengine::world {
enum class WorldComponentAttachmentStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWorldObjectId,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    InvalidOutputBuffer,
    DuplicateAttachment,
    CapacityExceeded,
    AttachmentNotFound
};
}
