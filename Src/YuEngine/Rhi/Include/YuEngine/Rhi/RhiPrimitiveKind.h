// Module: YuEngine Rhi
// File: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPrimitiveKind.h

#pragma once

namespace yuengine::rhi {
enum class RhiPrimitiveKind {
    Unsupported,
    Buffer,
    Texture,
    Sampler,
    ShaderModule,
    Pipeline,
    Fence
};
}
