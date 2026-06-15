// Module: YuEngine World
// File: Src/YuEngine/World/Include/YuEngine/World/WorldSceneObjectTransformManifestStreamStatus.h

#pragma once

namespace yuengine::world {
enum class WorldSceneObjectTransformManifestStreamStatus {
    Success,
    InvalidBridgeCapacity,
    InvalidWriter,
    InvalidReader,
    InvalidIdentityInput,
    InvalidTransformInput,
    InvalidIdentityOutput,
    InvalidTransformOutput,
    InvalidIdentityOutputCount,
    InvalidTransformOutputCount,
    InputCountExceeded,
    OutputCapacityExceeded,
    UnsupportedVersion,
    MalformedRecordCount,
    InvalidWorldObjectId,
    InvalidObjectHandle,
    DuplicateIdentityWorldObjectId,
    DuplicateIdentityObjectHandle,
    DuplicateTransformWorldObjectId,
    MissingIdentityForTransform,
    SerializeFailure
};
}
