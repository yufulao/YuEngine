// 模块: YuEngine Resource
// 文件: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceStatus.h

#pragma once

namespace yuengine::resource {
enum class ResourceStatus {
    Success,
    NotFound,
    DuplicateResource,
    CapacityExceeded,
    InvalidDescriptor,
    InvalidHandle,
    GenerationMismatch,
    TypeMismatch,
    NotAcquired,
    ReferenceCountOverflow,
    StillReferenced,
    StillDependedOn,
    DependencyMissing,
    DependencyCycle,
    UnsupportedInThisGate
};
}
