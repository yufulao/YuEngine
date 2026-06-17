// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPrimitiveRetirementStatus.h

#pragma once

namespace yuengine::rhi {
enum class RhiPrimitiveRetirementStatus {
    Invalid,
    Pending,
    Drained,
    RejectedInvalidRequest,
    RejectedInvalidHandle,
    RejectedWrongKind,
    RejectedDuplicate,
    RejectedCapacity,
    RejectedFenceNotReady
};
}
