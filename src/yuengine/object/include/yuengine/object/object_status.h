#pragma once

namespace yuengine::object {
enum class OBJECT_STATUS {
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
