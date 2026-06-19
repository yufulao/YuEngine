// 模块: YuEngine Asset
// 文件: Src/YuEngine/Asset/Include/YuEngine/Asset/AssetStatus.h

#pragma once

namespace yuengine::asset {
enum class AssetStatus {
    Success,
    InvalidArgument,
    InvalidAssetType,
    InvalidAssetId,
    DuplicateAsset,
    CapacityExceeded,
    InvalidHandle,
    GenerationMismatch,
    ResourceAcquireFailed,
    ResourceReleaseFailed,
    ResourceQueryFailed,
    ReferenceCountOverflow,
    NotAcquired,
    StillReferenced,
    DependencyCycle,
    DuplicateDependency,
    OutputBufferTooSmall,
    InvalidState,
    ReadyRecordMismatch,
    TextureReadyFailed,
    AudioReadyFailed
};
}
