// 模块: YuEngine RenderCore
// 文件: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgramStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment 报告 shader program validation 和 build 状态。
 */
enum class RenderShaderProgramStatus {
    Success,
    InvalidArgument,
    InvalidProgramId,
    InvalidVertexShader,
    InvalidPixelShader,
    InvalidInputLayout,
    DuplicateProgramId,
    ProgramCapacityExceeded
};
}
