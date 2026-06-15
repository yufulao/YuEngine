// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceStatus.h

#pragma once

namespace yuengine::resource {
enum class ResourceStatus {
    Success,
    NotFound,
    DuplicateResource,
    CapacityExceeded,
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
