// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneAssemblyManifestStreamStatus.h

#pragma once

namespace yuengine::world {
enum class WorldSceneAssemblyManifestStreamStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWriter,
    InvalidReader,
    InvalidAttachmentInput,
    InvalidBindingInput,
    InvalidAttachmentOutput,
    InvalidBindingOutput,
    InvalidAttachmentOutputCount,
    InvalidBindingOutputCount,
    InputCountExceeded,
    OutputCapacityExceeded,
    UnsupportedVersion,
    MalformedRecordCount,
    InvalidWorldObjectId,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    InvalidResourceHandle,
    InvalidResourceTypeId,
    MissingAttachment,
    DuplicateAttachment,
    DuplicateBinding,
    SerializeFailure
};
}
