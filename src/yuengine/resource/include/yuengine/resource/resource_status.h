#pragma once

namespace yuengine::resource {
enum class RESOURCE_STATUS {
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
