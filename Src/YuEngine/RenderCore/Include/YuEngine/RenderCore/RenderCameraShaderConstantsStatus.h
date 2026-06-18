// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderCameraShaderConstantsStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 报告 camera shader constant write 状态。
 */
enum class RenderCameraShaderConstantsStatus {
    Success,
    InvalidArgument,
    InvalidFrame
};
}
