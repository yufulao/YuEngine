// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgramResult.h

#pragma once

#include <cstdint>

#include "YuEngine/RenderCore/RenderShaderProgramStatus.h"

namespace yuengine::rendercore {
/**
 * @comment Contains the result of one shader program operation.
 */
struct RenderShaderProgramResult final {
    RenderShaderProgramStatus status = RenderShaderProgramStatus::InvalidArgument;
    std::uint32_t program_id = 0U;
};
}
