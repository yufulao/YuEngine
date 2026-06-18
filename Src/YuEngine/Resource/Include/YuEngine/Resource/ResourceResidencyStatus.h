// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceResidencyStatus.h

#pragma once

namespace yuengine::resource {
enum class ResourceResidencyStatus {
    Success,
    InvalidArgument,
    InvalidHandle,
    GenerationMismatch,
    TypeMismatch,
    NotUploaded,
    FailedLoad,
    AlreadyResident,
    NotResident,
    AlreadyPinned,
    NotPinned,
    StillReferenced,
    Pinned,
    BudgetExceeded,
    CapacityExceeded,
    NoCandidate
};
}
