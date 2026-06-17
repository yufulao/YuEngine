// Module: YuEngine Streaming
// File: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadKind.h

#pragma once

namespace yuengine::streaming {
enum class ResourceUploadKind {
    Unsupported,
    CreateBuffer,
    UpdateBuffer,
    CreateTexture,
    UpdateTexture
};
}
