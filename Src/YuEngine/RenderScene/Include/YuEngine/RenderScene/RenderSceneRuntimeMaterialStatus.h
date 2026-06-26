// 模块: YuEngine RenderScene
// 文件: Src/YuEngine/RenderScene/Include/YuEngine/RenderScene/RenderSceneRuntimeMaterialStatus.h

#pragma once

namespace yuengine::renderscene {
enum class RenderSceneRuntimeMaterialStatus {
    Success,
    NullPointer,
    InvalidMaterialAsset,
    InvalidMaterialId,
    InvalidPipeline,
    MissingTextureSlot,
    InvalidTextureAsset,
    InvalidTextureBinding,
    InvalidSamplerBinding,
    DuplicateTextureSlot,
    TextureSlotCapacityExceeded,
    MaterialConstantCapacityExceeded,
    MissingMaterialRecord
};
}
