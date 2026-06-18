// 模块: YuEngine Streaming
// 文件: Src/YuEngine/Streaming/Include/YuEngine/Streaming/ResourceUploadKind.h

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
