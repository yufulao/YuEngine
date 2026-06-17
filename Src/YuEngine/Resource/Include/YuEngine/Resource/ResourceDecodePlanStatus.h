// Module: YuEngine Resource
// File: Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodePlanStatus.h

#pragma once

namespace yuengine::resource {
enum class ResourceDecodePlanStatus {
    Success,
    InvalidArgument,
    InvalidHandle,
    GenerationMismatch,
    TypeMismatch,
    NotUploaded,
    FailedLoad,
    NotResident,
    MissingCachePayload,
    MissingPlan,
    InvalidPayloadId,
    InvalidHeader,
    UnsupportedHeaderVersion,
    DuplicatePlanId,
    CapacityExceeded,
    BudgetExceeded
};
}
