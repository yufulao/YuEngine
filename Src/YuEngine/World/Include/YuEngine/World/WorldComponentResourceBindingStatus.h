// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentResourceBindingStatus.h

#pragma once

namespace yuengine::world {
enum class WorldComponentResourceBindingStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidAttachmentSource,
    InvalidResourceRegistry,
    InvalidWorldObjectId,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    AttachmentNotFound,
    InvalidResourceHandle,
    StaleResourceHandle,
    ResourceTypeMismatch,
    DuplicateComponentBinding,
    CapacityExceeded,
    BindingNotFound,
    ResourceAcquireFailed,
    ResourceReleaseFailed
};
}
