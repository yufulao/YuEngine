/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodedPayloadStatus.h
 * @brief Resource module decoded payload status value contract.
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief Describes validation and lifecycle status for decoded payload storage.
 */
enum class ResourceDecodedPayloadStatus {
    Success,
    InvalidArgument,
    InvalidHandle,
    GenerationMismatch,
    TypeMismatch,
    NotUploaded,
    FailedLoad,
    NotResident,
    MissingCachePayload,
    MissingDecodePlan,
    MissingDecodeResult,
    MissingDecodedPayload,
    InvalidPayloadId,
    InvalidDecodePlanId,
    InvalidDecodeResultId,
    InvalidDecodedPayloadId,
    DuplicateDecodedPayloadId,
    AssetClassMismatch,
    ResultClassMismatch,
    DecodedByteCountMismatch,
    EmptyPayload,
    OutputBufferTooSmall,
    CapacityExceeded,
    BudgetExceeded
};

} // namespace yuengine::resource
