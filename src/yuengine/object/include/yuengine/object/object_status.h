#pragma once

namespace yuengine::object {
enum class ObjectStatus {
    Success,
    InvalidType,
    CapacityExceeded,
    InvalidHandle,
    GenerationMismatch,
    NotAcquired,
    ReferenceCountOverflow,
    StillReferenced
};
}
