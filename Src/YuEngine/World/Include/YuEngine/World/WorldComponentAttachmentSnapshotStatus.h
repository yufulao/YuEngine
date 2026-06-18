// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldComponentAttachmentSnapshotStatus.h

#pragma once

namespace yuengine::world {
enum class WorldComponentAttachmentSnapshotStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidSourceBridge,
    InvalidDestinationBridge,
    InvalidWriter,
    InvalidReader,
    UnsupportedVersion,
    MalformedRecordCount,
    RecordCountExceeded,
    InvalidWorldObjectId,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    DuplicateAttachment,
    DestinationCapacityExceeded,
    DestinationClearFailed,
    DestinationRestoreFailed,
    SerializeFailure
};
}
