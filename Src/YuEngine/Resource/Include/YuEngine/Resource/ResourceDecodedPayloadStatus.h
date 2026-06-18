/**
 * @file Src/YuEngine/Resource/Include/YuEngine/Resource/ResourceDecodedPayloadStatus.h
 * @brief Resource 模块解码载荷状态值契约。
 */
#pragma once

namespace yuengine::resource {

/**
 * @brief 描述 validation 和 生命周期 状态 用于 decoded payload 存储.
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
