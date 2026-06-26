// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderMaterialStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 报告 render material validation 和 build 状态。
 */
enum class RenderMaterialStatus {
    Success,
    InvalidArgument,
    InvalidMaterialId,
    InvalidProgramId,
    InvalidPipeline,
    InvalidBlendState,
    InvalidTextureBinding,
    InvalidSamplerBinding,
    OversizedConstants,
    DuplicateMaterialId,
    MaterialCapacityExceeded
};
}
