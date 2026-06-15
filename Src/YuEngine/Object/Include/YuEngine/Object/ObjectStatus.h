// Module: YuEngine Object
// File: Src/YuEngine/Object/Include/YuEngine/Object/ObjectStatus.h

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
