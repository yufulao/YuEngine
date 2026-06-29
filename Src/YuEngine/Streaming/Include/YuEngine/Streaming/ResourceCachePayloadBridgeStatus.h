// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceCachePayloadBridgeStatus.h
#pragma once

namespace yuengine::streaming {

enum class ResourceCachePayloadBridgeStatus {
    Success,
    InvalidArgument,
    StagingFailed,
    StagingByteRangeOutOfBounds,
    StagingByteCountMismatch,
    PayloadWindowOutOfBounds,
    ResourceStoreFailed
};

} // namespace yuengine::streaming
