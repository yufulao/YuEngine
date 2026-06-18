// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgramDesc.h

#pragma once

#include <cstddef>

#include "YuEngine/RenderCore/RenderShaderProgramConstants.h"

namespace yuengine::rendercore {
/**
 * @comment Describes bounded RenderShaderProgram storage.
 */
struct RenderShaderProgramDesc final {
    std::size_t program_record_capacity = MAX_RENDER_SHADER_PROGRAM_RECORDS;
};
}
