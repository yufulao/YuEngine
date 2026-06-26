// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandType.h

#pragma once

namespace yuengine::rhi {
enum class RhiCommandType {
    BeginFrame,
    ClearColor,
    BindPipeline,
    BindVertexBuffer,
    BindIndexBuffer,
    BindSampledTexture,
    BindSampler,
    BindConstantBuffer,
    BindBlendState,
    Draw,
    DrawIndexed,
    EndFrame
};
}
