// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiPipelineDesc.h

#pragma once

#include "YuEngine/Rhi/RhiInputLayoutDesc.h"
#include "YuEngine/Rhi/RhiShaderModuleHandle.h"

namespace yuengine::rhi {
struct RhiPipelineDesc final {
    RhiShaderModuleHandle vertex_shader{};
    RhiShaderModuleHandle pixel_shader{};
    RhiInputLayoutDesc input_layout{};
};
}
