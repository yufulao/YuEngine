// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/MaterialBindingFixtureStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 定义 explicit material 绑定 fixture 状态 值.
 */
enum class MaterialBindingFixtureStatus {
    Success,
    InvalidArgument,
    InvalidMaterialId,
    InvalidPipeline,
    InvalidTextureBinding,
    InvalidSamplerBinding,
    OversizedConstants,
    DuplicateMaterialId,
    BindingCapacityExceeded,
    RenderFixturePassFailed
};
}
