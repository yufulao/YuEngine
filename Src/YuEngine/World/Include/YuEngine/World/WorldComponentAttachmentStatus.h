// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentStatus.h

#pragma once

namespace yuengine::world {
enum class WorldComponentAttachmentStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWorldObjectId,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    DuplicateAttachment,
    CapacityExceeded,
    AttachmentNotFound
};
}
