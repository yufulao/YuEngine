// 模块: YuEngine RuntimeAssetWorldAdapter
// 文件: Src/YuEngine/RuntimeAssetWorldAdapter/Include/YuEngine/RuntimeAssetWorldAdapter/RuntimeAssetWorldObjectAdapterStatus.h

#pragma once

namespace yuengine::runtimeassetworldadapter {
enum class RuntimeAssetWorldObjectAdapterStatus {
    Success,
    InvalidRuntimeInstanceInput,
    InvalidSceneEntityInput,
    InvalidSceneTransformInput,
    InvalidIdentityInput,
    InvalidIdentityOutput,
    InvalidTransformOutput,
    InvalidRuntimeInstanceMapping,
    UnsupportedTargetKind,
    InvalidSceneEntityIndex,
    InvalidSceneTransformIndex,
    MissingSceneEntity,
    MissingSceneTransform,
    InvalidWorldObjectId,
    MissingIdentityRecord,
    InvalidObjectHandle,
    WorldObjectMismatch,
    DuplicateTargetId,
    DuplicateWorldObjectId,
    DuplicateObjectHandle,
    IdentityOutputCapacityExceeded,
    TransformOutputCapacityExceeded
};
}
