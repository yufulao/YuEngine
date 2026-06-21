// 模块: YuEngine Rhi
// 文件: Src/YuEngine/Rhi/Include/YuEngine/Rhi/RhiCommandRecord.h

#pragma once

#include "YuEngine/Rhi/RhiBlendStateDesc.h"
#include "YuEngine/Rhi/RhiColor.h"
#include "YuEngine/Rhi/RhiCommandType.h"
#include "YuEngine/Rhi/RhiDrawDesc.h"
#include "YuEngine/Rhi/RhiDrawIndexedDesc.h"
#include "YuEngine/Rhi/RhiIndexBufferView.h"
#include "YuEngine/Rhi/RhiPipelineHandle.h"
#include "YuEngine/Rhi/RhiSampledTextureBinding.h"
#include "YuEngine/Rhi/RhiSamplerBinding.h"
#include "YuEngine/Rhi/RhiTextureHandle.h"
#include "YuEngine/Rhi/RhiVertexBufferView.h"

namespace yuengine::rhi {
struct RhiCommandRecord final {
    RhiCommandType type = RhiCommandType::BeginFrame;
    RhiTextureHandle target{};
    RhiColor color{};
    RhiPipelineHandle pipeline{};
    RhiVertexBufferView vertex_buffer{};
    RhiIndexBufferView index_buffer{};
    RhiSampledTextureBinding sampled_texture{};
    RhiSamplerBinding sampler{};
    RhiBlendStateDesc blend_state{};
    RhiDrawDesc draw{};
    RhiDrawIndexedDesc draw_indexed{};
};
}
