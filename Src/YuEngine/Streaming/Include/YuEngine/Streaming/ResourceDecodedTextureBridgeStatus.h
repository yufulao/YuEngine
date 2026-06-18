// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceDecodedTextureBridgeStatus.h

#pragma once

namespace yuengine::streaming {
enum class ResourceDecodedTextureBridgeStatus {
    Success,
    InvalidArgument,
    ResourceQueryFailed,
    ResourceReadFailed,
    ScratchBufferTooSmall,
    TextureByteCountMismatch,
    UploadSubmitFailed,
    UploadProcessFailed,
    UploadCompletionMissing
};
}
