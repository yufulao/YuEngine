// Module: YuEngine RenderCore
// File: Src/YuEngine/RenderCore/Include/YuEngine/RenderCore/RenderShaderProgramRequest.h

#pragma once

#include <cstdint>

#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"

namespace yuengine::rendercore {
/**
 * @comment Describes one value-only shader program request.
 */
struct RenderShaderProgramRequest final {
    std::uint32_t program_id = 0U;
    yuengine::rhi::RhiShaderModuleHandle vertex_shader{};
    yuengine::rhi::RhiShaderModuleHandle pixel_shader{};
    yuengine::rhi::RhiInputLayoutDesc input_layout{};
};
}
