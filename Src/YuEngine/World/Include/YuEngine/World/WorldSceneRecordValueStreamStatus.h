// 模块: YuEngine World
// 文件: Src/YuEngine/World/Include/YuEngine/World/WorldSceneRecordValueStreamStatus.h

#pragma once

namespace yuengine::world {
enum class WorldSceneRecordValueStreamStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWriter,
    InvalidReader,
    InvalidIdentityInput,
    InvalidTransformInput,
    InvalidAttachmentInput,
    InvalidBindingInput,
    InvalidIdentityOutput,
    InvalidTransformOutput,
    InvalidAttachmentOutput,
    InvalidBindingOutput,
    InvalidIdentityOutputCount,
    InvalidTransformOutputCount,
    InvalidAttachmentOutputCount,
    InvalidBindingOutputCount,
    InputCountExceeded,
    OutputCapacityExceeded,
    UnsupportedVersion,
    InvalidWorldObjectId,
    InvalidObjectHandle,
    InvalidComponentTypeId,
    InvalidComponentSlotId,
    InvalidResourceHandle,
    InvalidResourceTypeId,
    DuplicateIdentityWorldObjectId,
    DuplicateIdentityObjectHandle,
    DuplicateTransformWorldObjectId,
    DuplicateAttachment,
    DuplicateBinding,
    MissingIdentityForTransform,
    MissingIdentityForAttachment,
    MissingAttachment,
    ChildStreamFailure,
    SerializeFailure
};
}
