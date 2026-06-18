// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h

#pragma once

namespace yuengine::streaming {
enum class ResourceDecodedTextureBridgeStatus {
    Success,
    InvalidArgument,
    ResourceQueryFailed,
    ResourceReadFailed,
    ScratchBufferTooSmall,
    TextureByteCountMismatch,
    SampledTextureSlotOutOfRange,
    UploadSubmitFailed,
    UploadProcessFailed,
    UploadCompletionMissing
};
}
