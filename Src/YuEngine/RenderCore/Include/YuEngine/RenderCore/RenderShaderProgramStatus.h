// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgramStatus.h

#pragma once

namespace yuengine::rendercore {
/**
 * @comment Reports shader program validation and build status.
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
