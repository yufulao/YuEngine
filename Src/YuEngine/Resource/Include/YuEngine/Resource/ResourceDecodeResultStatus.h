/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodeResultStatus.h
 * @brief Resource 模块解码结果状态值契约。
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief 描述 validation 和 生命周期 状态 用于 decode-结果 metadata.
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
