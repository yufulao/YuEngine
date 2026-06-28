// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceCachePayloadStatus.h

#pragma once

namespace yuengine::resource {
enum class ResourceCachePayloadStatus {
    Success,
    InvalidArgument,
    InvalidHandle,
    GenerationMismatch,
    TypeMismatch,
    NotUploaded,
    FailedLoad,
    NotResident,
    Pinned,
    DuplicatePayloadId,
    MissingPayload,
    EmptyPayload,
    OutputBufferTooSmall,
    PayloadWindowOutOfBounds,
    CapacityExceeded,
    BudgetExceeded,
    ReferenceBudgetExceeded
};
}
