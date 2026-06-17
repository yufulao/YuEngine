/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultStatus.h
 * @brief Resource module decode result status value contract.
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief Describes validation and lifecycle status for decode-result metadata.
 */
enum class ResourceDecodeResultStatus {
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
    InvalidPayloadId,
    InvalidDecodePlanId,
    InvalidDecodeResultId,
    DuplicateDecodeResultId,
    AssetClassMismatch,
    ResultClassMismatch,
    DecodedByteCountMismatch,
    CapacityExceeded,
    BudgetExceeded
};

} // namespace yuengine::resource
